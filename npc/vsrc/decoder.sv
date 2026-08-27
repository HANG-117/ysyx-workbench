//-----------------------------------------------------------------
// 精简 RV32I 指令译码器（小核用）
// 支持：ALU (R/I), LUI, AUIPC, 加载, 存储, 分支 (B), JAL, JALR
//-----------------------------------------------------------------

`include "core_defs.sv"

module decoder(
    input  logic [31:0] opcode_i,
    input  logic [31:0] pc_i,
    input  logic [31:0] imm_i,
    output core_types_pkg::decoded_uop_t uop_o
);

    logic [6:0] opcode;
    logic [2:0] funct3;
    logic       funct7_bit30;

    assign opcode       = opcode_i[6:0];
    assign funct3       = opcode_i[14:12];
    assign funct7_bit30 = opcode_i[30];

    // R 型 / I 型 ALU 判定
    wire is_r_type_alu = (opcode == 7'b0110011);
    wire is_i_type_alu = (opcode == 7'b0010011);

    // ---------- 指令类型判定（基于 opcode） ----------
    wire is_lui   = (opcode == 7'b0110111);
    wire is_auipc = (opcode == 7'b0010111);
    wire is_load  = (opcode == 7'b0000011);
    wire is_store = (opcode == 7'b0100011);
    wire is_branch= (opcode == 7'b1100011);
    wire is_jal   = (opcode == 7'b1101111);
    wire is_jalr  = (opcode == 7'b1100111);

    // ECALL / EBREAK（SYSTEM opcode 且 imm=0/1）
    wire is_ecall  = (opcode == 7'b1110011) && (opcode_i[31:7] == 25'b0);
    wire is_ebreak = (opcode == 7'b1110011) && (opcode_i[31:7] == 25'h0200000);

    // ---------- 加载 / 存储 ----------
    assign uop_o.load  = is_load;   // LB/LH/LW/LBU/LHU
    assign uop_o.store = is_store;  // SB/SH/SW

    // ---------- ALU 运算码 ----------
    logic use_alt;
    always_comb begin
        use_alt = 1'b0;
        if (is_r_type_alu && (funct3 == 3'b000)) begin
            use_alt = funct7_bit30; // ADD(funct7=0) / SUB(funct7=1)
        end else if ((is_i_type_alu || is_r_type_alu) && (funct3 == 3'b101)) begin
            use_alt = funct7_bit30; // SRL(funct7=0) / SRA(funct7=1)
        end
    end

    always_comb begin
        if (is_lui) begin
            uop_o.alu_op = `ALU_LUI;
        end else if (is_r_type_alu || is_i_type_alu) begin
            uop_o.alu_op = {use_alt, funct3};
        end else if (is_branch) begin
            uop_o.alu_op = `ALU_SUB;   // 分支判定基于减法
        end else begin
            uop_o.alu_op = `ALU_ADD;
        end
    end

    // ---------- ALU 输入选择 ----------
    always_comb begin
        uop_o.alu_rs1_sel = 1'b0;
        uop_o.alu_rs2_sel = 1'b0;
        case (opcode)
            7'b0010011, 7'b0000011, 7'b0100011, 7'b1100111: begin // I型 / Load / Store / JALR
                uop_o.alu_rs1_sel = 1'b0;
                uop_o.alu_rs2_sel = 1'b1;
            end
            7'b0110111: begin // LUI
                uop_o.alu_rs1_sel = 1'b0;
                uop_o.alu_rs2_sel = 1'b1;
            end
            7'b1100011: begin // Branch: 两个操作数都来自寄存器
                uop_o.alu_rs1_sel = 1'b0;
                uop_o.alu_rs2_sel = 1'b0;
            end
            7'b1101111, 7'b0010111: begin // JAL / AUIPC
                uop_o.alu_rs1_sel = 1'b1;
                uop_o.alu_rs2_sel = 1'b1;
            end
            default: begin
                uop_o.alu_rs1_sel = 1'b0;
                uop_o.alu_rs2_sel = 1'b0;
            end
        endcase
    end

    // ---------- 存储器访问属性 ----------
    assign uop_o.mem_size     = (is_load || is_store) ? funct3[1:0] : `MEM_SIZE_WORD;
    assign uop_o.mem_unsigned = is_load && funct3[2];

    // ---------- 分支 / 跳转 ----------
    assign uop_o.branch = is_branch;
    assign uop_o.jump   = is_jal || is_jalr;
    assign uop_o.jump_base_rs1 = is_jalr;

    assign uop_o.branch_cond = funct3;

    // ---------- 写回 ----------LU
    assign uop_o.wb_sel = (is_jal || is_jalr) ? `WB_SEL_PC4 :
                          is_load             ? `WB_SEL_MEM :
                          is_csr_wen          ? `WB_SEL_CSR :
                                               `WB_SEL_ALU;

    // 是否有写回目标寄存器
    assign uop_o.meta.rd_valid = is_lui  || is_auipc || is_load || is_jal || is_jalr ||
                                 is_i_type_alu || is_r_type_alu || is_csr_wen;

    // 是否执行 ALU 运算
    assign uop_o.exec = is_lui || is_auipc || is_i_type_alu || is_r_type_alu;

    // ---------- 寄存器地址 ----------
    assign uop_o.meta.pc        = pc_i;
    assign uop_o.meta.inst      = opcode_i;
    assign uop_o.meta.rs1_addr  = opcode_i[19:15];
    assign uop_o.meta.rs2_addr  = opcode_i[24:20];
    assign uop_o.meta.rd_addr   = opcode_i[11:7];
    assign uop_o.imm            = imm_i;

    //------------csr指令判定------------
    assign uop_o.csr = (opcode == 7'b1110011); // CSR 指令（ECALL/EBREAK 除外
    assign uop_o.csr_addr = opcode_i[31:20];
    always_comb begin
        if(opcode == 7'b1110011) begin
            case (funct3)
                3'b000: begin
                    case (opcode_i[31:20])
                        12'h000: uop_o.csr_type = `CSR_ECALL;
                        12'h001: uop_o.csr_type = `CSR_EBREAK;
                        12'h302: uop_o.csr_type = `CSR_MRET;
                        default: uop_o.csr_type = `CSR_NONE;
                    endcase
                end
                3'b001: uop_o.csr_type = `CSR_CSRRW;
                3'b010: uop_o.csr_type = `CSR_CSRRS;
                3'b011: uop_o.csr_type = `CSR_CSRRC;
                3'b101: uop_o.csr_type = `CSR_CSRRWI;
                3'b110: uop_o.csr_type = `CSR_CSRRSI;
                3'b111: uop_o.csr_type = `CSR_CSRRCI;
                default: uop_o.csr_type = `CSR_NONE;
            endcase
        end else begin
            uop_o.csr_type = `CSR_NONE;
        end
    end


    // 必须是连续赋值（net 声明赋值），否则只在时间 0 求值一次、永远不变。
    // 必须用 opcode==SYSTEM 限定：funct3 与其它指令冲突（如 auipc 也是 funct3=001），
    // 不限定会把 auipc/slti/slt 等误判为 CSR 指令。
    wire is_csr_wen = (opcode == 7'b1110011) &&
                      ((funct3 == 3'b001) || (funct3 == 3'b010) || (funct3 == 3'b011) ||
                       (funct3 == 3'b101) || (funct3 == 3'b110) || (funct3 == 3'b111));


endmodule
