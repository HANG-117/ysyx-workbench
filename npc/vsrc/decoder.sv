//-----------------------------------------------------------------
// 精简 RV32I 指令译码器（小核用）
// 支持：ALU (R/I), LUI, AUIPC, 加载, 存储, 分支 (B), JAL, JALR
//-----------------------------------------------------------------

`include "core_defs.sv"

module decoder(
    input  [31:0]        opcode_i,
    output               exec_o,
    output               load_o,
    output               store_o,
    output               branch_o,
    output               rd_valid_o,
    output               jump_o,
    output               jump_base_rs1_o,
    output [1:0]         mem_size_o,
    output               mem_unsigned_o,
    output logic [3:0]   alu_op_o,
    output [2:0]         wb_sel_o,
    output logic         alu_rs1_sel_o,
    output logic         alu_rs2_sel_o,
    output logic [2:0]   branch_cond_o,
    output logic [4:0]   rs1_addr_o,
    output logic [4:0]   rs2_addr_o,
    output logic [4:0]   rd_addr_o
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
    assign load_o  = is_load;   // LB/LH/LW/LBU/LHU
    assign store_o = is_store;  // SB/SH/SW

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
            alu_op_o = `ALU_LUI;
        end else if (is_r_type_alu || is_i_type_alu) begin
            alu_op_o = {use_alt, funct3};
        end else if (is_branch) begin
            alu_op_o = `ALU_SUB;   // 分支判定基于减法
        end else begin
            alu_op_o = `ALU_ADD;
        end
    end

    // ---------- ALU 输入选择 ----------
    always_comb begin
        alu_rs1_sel_o = 1'b0;
        alu_rs2_sel_o = 1'b0;
        case (opcode)
            7'b0010011, 7'b0000011, 7'b0100011, 7'b1100111: begin // I型 / Load / Store / JALR
                alu_rs1_sel_o = 1'b0;
                alu_rs2_sel_o = 1'b1;
            end
            7'b0110111: begin // LUI
                alu_rs1_sel_o = 1'b0;
                alu_rs2_sel_o = 1'b1;
            end
            7'b1100011: begin // Branch: 两个操作数都来自寄存器
                alu_rs1_sel_o = 1'b0;
                alu_rs2_sel_o = 1'b0;
            end
            7'b1101111, 7'b0010111: begin // JAL / AUIPC
                alu_rs1_sel_o = 1'b1;
                alu_rs2_sel_o = 1'b1;
            end
            default: begin
                alu_rs1_sel_o = 1'b0;
                alu_rs2_sel_o = 1'b0;
            end
        endcase
    end

    // ---------- 存储器访问属性 ----------
    assign mem_size_o     = (is_load || is_store) ? funct3[1:0] : `MEM_SIZE_WORD;
    assign mem_unsigned_o = is_load && funct3[2];

    // ---------- 分支 / 跳转 ----------
    assign branch_o = is_branch;
    assign jump_o   = is_jal || is_jalr;
    assign jump_base_rs1_o = is_jalr;

    // 分支条件：直接透传 funct3（BEQ=000,BNE=001,BLT=100,BGE=101,BLTU=110,BGEU=111）
    assign branch_cond_o = funct3;

    // ---------- 写回 ----------
    assign wb_sel_o = (is_jal || is_jalr) ? `WB_SEL_PC4 :
                      is_load             ? `WB_SEL_MEM :
                                             `WB_SEL_ALU;

    // 是否有写回目标寄存器
    assign rd_valid_o = is_lui  || is_auipc || is_load || is_jal || is_jalr ||
                        is_i_type_alu || is_r_type_alu;

    // 是否执行 ALU 运算
    assign exec_o = is_lui || is_auipc || is_i_type_alu || is_r_type_alu;

    // ---------- 寄存器地址 ----------
    assign rs1_addr_o = opcode_i[19:15];
    assign rs2_addr_o = opcode_i[24:20];
    assign rd_addr_o  = opcode_i[11:7];

endmodule

