`include "core_defs.sv"
module alu(
    input  logic [31:0] rs1_i,
    input  logic [31:0] rs2_i,
    input  logic [3:0]  alu_op_i,
    input  logic [2:0]  branch_cond_i,   // 新增：分支条件类型
    input  logic        branch_en_i,     // 新增：本指令是否为分支
    output logic [31:0] alu_result_o,
    output logic        branch_taken_o   // 新增：分支跳转信号
);

    logic [31:0] diff;          // rs1 - rs2

    always_comb begin
        case (alu_op_i)
            `ALU_ADD:  alu_result_o = rs1_i + rs2_i;
            `ALU_SLL:  alu_result_o = rs1_i << rs2_i[4:0];
            `ALU_SLT:  alu_result_o = ($signed(rs1_i) < $signed(rs2_i)) ? 32'b1 : 32'b0;
            `ALU_SLTU: alu_result_o = (rs1_i < rs2_i) ? 32'b1 : 32'b0;
            `ALU_XOR:  alu_result_o = rs1_i ^ rs2_i;
            `ALU_SRL:  alu_result_o = rs1_i >> rs2_i[4:0];
            `ALU_SRA:  alu_result_o = $signed(rs1_i) >> rs2_i[4:0];
            `ALU_OR:   alu_result_o = rs1_i | rs2_i;
            `ALU_AND:  alu_result_o = rs1_i & rs2_i;
            `ALU_SUB:  alu_result_o = rs1_i - rs2_i;
            `ALU_LUI:  alu_result_o = rs2_i;
            default:  alu_result_o = 32'b0;
        endcase
    end

    // ---------- 分支判定：完全基于减法结果 diff = rs1 - rs2 ----------
    assign diff = rs1_i - rs2_i;

    // 有符号小于：符号位异或「符号不同」
    // 当 rs1、rs2 符号相同时，结果符号位即比较结果；符号不同时用 rs1 符号判断
    logic slt_signed;
    assign slt_signed = diff[31] ^ (rs1_i[31] != rs2_i[31]);

    logic eq, neq, blt, bge, bltu, bgeu;
    assign eq   = (diff == 32'b0);
    assign neq  = (diff != 32'b0);
    assign blt  = slt_signed;
    assign bge  = ~blt;
    assign bltu = diff[31];        // 无符号 a<b ⟺ 无符号减法最高位为 1
    assign bgeu = ~diff[31];

    always_comb begin
        if (!branch_en_i) begin
            branch_taken_o = 1'b0;
        end else begin
            // branch_cond_i 直接使用 RISC-V funct3 编码
            // 000=BEQ, 001=BNE, 100=BLT, 101=BGE, 110=BLTU, 111=BGEU
            case (branch_cond_i)
                3'b000: branch_taken_o = eq;   // BEQ
                3'b001: branch_taken_o = neq;  // BNE
                3'b100: branch_taken_o = blt;  // BLT
                3'b101: branch_taken_o = bge;  // BGE
                3'b110: branch_taken_o = bltu; // BLTU
                3'b111: branch_taken_o = bgeu; // BGEU
                default: branch_taken_o = 1'b0;
            endcase
        end
    end
endmodule