module imm_gen (
    input  logic [31:0] instruction,
    output logic [31:0] imm
);
    wire [6:0] opcode;
    wire [4:0] imm_4_0;
    wire [11:0] imm_11_0;
    wire [19:0] imm_19_0;
    wire [20:0] imm_20_0;

    assign opcode = instruction[6:0];
    always_comb begin
        imm = 32'b0;  // 默认值，非常重要！防止 latch

        case (opcode)
            // I-type (ADDI, ANDI, ORI, XORI, SLTI, LW, JALR 等)
            7'b0010011,  // OP-IMM
            7'b0000011,  // LOAD
            7'b1100111:  // JALR
                imm = {{20{instruction[31]}}, instruction[31:20]};

            7'b0100011:  // STORE
                imm = {{20{instruction[31]}}, instruction[31:25], instruction[11:7]};

            7'b1100011:  // BRANCHQ
                imm = {{19{instruction[31]}}, instruction[31], instruction[7], instruction[30:25], instruction[11:8], 1'b0};

            7'b0110111,  // LUI
            7'b0010111:  // AUIPC
                imm = {instruction[31:12], 12'b0};

            7'b1101111:  // JAL
                imm = {{11{instruction[31]}}, instruction[31], instruction[19:12], instruction[20], instruction[30:21], 1'b0};

            default: imm = 32'b0;  // 非法或不支持的指令 → 0
        endcase
    end

endmodule