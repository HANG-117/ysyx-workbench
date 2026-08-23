module alu(
    input  logic [31:0] rs1_i,
    input  logic [31:0] rs2_i,
    input  logic [3:0]  alu_op_i,

    input  logic [2:0]  branch_cond_i,
    input  logic        branch_en_i,

    output logic [31:0] alu_result_o,
    output logic        branch_taken_o
);

    // ============================================================
    // ALU 运算
    // ============================================================

    always_comb begin
        case (alu_op_i)
            `ALU_ADD: begin
                alu_result_o = rs1_i + rs2_i;
            end
            `ALU_SUB: begin
                alu_result_o = rs1_i - rs2_i;
            end
            `ALU_SLL: begin
                alu_result_o = rs1_i << rs2_i[4:0];
            end
            `ALU_SLT: begin
                alu_result_o =
                    ($signed(rs1_i) < $signed(rs2_i))
                    ? 32'd1
                    : 32'd0;
            end

            `ALU_SLTU: begin
                alu_result_o =
                    (rs1_i < rs2_i)
                    ? 32'd1
                    : 32'd0;
            end

            `ALU_XOR: begin
                alu_result_o = rs1_i ^ rs2_i;
            end

            `ALU_SRL: begin
                alu_result_o = rs1_i >> rs2_i[4:0];
            end

            `ALU_SRA: begin
                alu_result_o =
                    $signed(rs1_i) >>> rs2_i[4:0];
            end

            `ALU_OR: begin
                alu_result_o = rs1_i | rs2_i;
            end

            `ALU_AND: begin
                alu_result_o = rs1_i & rs2_i;
            end

            `ALU_LUI: begin
                alu_result_o = rs2_i;
            end

            default: begin
                alu_result_o = 32'b0;
            end

        endcase
    end


    // ============================================================
    // Branch 判定
    // ============================================================

    always_comb begin
        branch_taken_o = 1'b0;

        if (branch_en_i) begin
            case (branch_cond_i)

                `BC_BEQ: begin
                    branch_taken_o = (rs1_i == rs2_i);
                end

                `BC_BNE: begin
                    branch_taken_o = (rs1_i != rs2_i);
                end

                `BC_BLT: begin
                    branch_taken_o =
                        ($signed(rs1_i) < $signed(rs2_i));
                end

                `BC_BGE: begin
                    branch_taken_o =
                        ($signed(rs1_i) >= $signed(rs2_i));
                end

                `BC_BLTU: begin
                    branch_taken_o = (rs1_i < rs2_i);
                end

                `BC_BGEU: begin
                    branch_taken_o = (rs1_i >= rs2_i);
                end

                default: begin
                    branch_taken_o = 1'b0;
                end

            endcase
        end
    end

endmodule