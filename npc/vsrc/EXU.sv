module EXU(
    input  logic [31:0] inst_i,          // 指令（用于立即数扩展）
    input  logic [31:0] pc_i,            // 当前 PC
    input  logic [31:0] rs1_rdata_i,     // 寄存器 rs1
    input  logic [31:0] rs2_rdata_i,     // 寄存器 rs2
    input  logic        alu_rs1_sel_i,   // rs1 选择：0=rs1 数据, 1=PC
    input  logic        alu_rs2_sel_i,   // rs2 选择：0=rs2 数据, 1=立即数
    input  logic [3:0]  alu_op_i,        // ALU 运算码
    input  logic [2:0]  branch_cond_i,   // 分支条件类型
    input  logic        branch_en_i,     // 分支使能
    input  logic        jump_en_i,       // 跳转使能
    input  logic        jump_base_rs1_i, // JALR：目标 = rs1 + imm
    output logic [31:0] alu_result_o,
    output logic        branch_taken_o,  // 分支是否跳转（来自 ALU）
    output logic [31:0] next_pc_o        // 计算出的下一条 PC
);

    // 立即数扩展：指令所需立即数
    logic [31:0] imm;
    always_comb begin
        case (inst_i[6:5])
            2'b00: imm = {{20{inst_i[31]}}, inst_i[31:20]};              // I-type
            2'b01: imm = {{20{inst_i[31]}}, inst_i[31:25], inst_i[11:7]}; // S-type
            2'b10: imm = {{20{inst_i[31]}}, inst_i[7], inst_i[30:25],
                          inst_i[11:8], 1'b0};                            // B-type
            2'b11: begin
                if (inst_i[3])
                    imm = {{12{inst_i[31]}}, inst_i[19:12], inst_i[20],
                           inst_i[30:21], 1'b0};                          // J-type (JAL)
                else
                    imm = {{20{inst_i[31]}}, inst_i[31:20]};              // I-type (JALR)
            end
        endcase
    end

    logic [31:0] rs1;
    logic [31:0] rs2;
    always_comb begin
        rs1 = alu_rs1_sel_i ? pc_i : rs1_rdata_i;
        rs2 = alu_rs2_sel_i ? imm : rs2_rdata_i;
    end

    // ALU：计算 + 内部完成分支判定
    alu alu_inst(
        .rs1_i         (rs1),
        .rs2_i         (rs2),
        .alu_op_i      (alu_op_i),
        .branch_cond_i (branch_cond_i),
        .branch_en_i   (branch_en_i),
        .alu_result_o  (alu_result_o),
        .branch_taken_o(branch_taken_o)
    );

    // 目标地址计算
    logic [31:0] target_addr;
    always_comb begin
        if (jump_base_rs1_i)       // JALR: 目标 = (rs1 + imm) 清最低位
            target_addr = (rs1_rdata_i + imm) & ~32'h1;
        else                       // JAL / Branch: 目标 = PC + imm
            target_addr = pc_i + imm;
    end

    // next_pc：跳转/分支命中则用目标地址，否则顺序取 PC+4
    assign next_pc_o = (branch_taken_o || jump_en_i) ? target_addr : (pc_i + 32'h4);

endmodule