module ImmGen(
    input  logic [31:0] inst_i,
    output logic [31:0] imm_o
);

    // Generate the sign-extended immediate from the complete opcode.  Using
    // only a subset of opcode bits is not sufficient to distinguish U/B/J
    // formats reliably.
    always_comb begin
        case (inst_i[6:0])
            // I-type: OP-IMM / LOAD / JALR / SYSTEM
            7'b0010011, 7'b0000011, 7'b1100111, 7'b1110011:
                imm_o = {{20{inst_i[31]}}, inst_i[31:20]};

            // S-type: STORE
            7'b0100011:
                imm_o = {{20{inst_i[31]}}, inst_i[31:25], inst_i[11:7]};

            // B-type: BRANCH
            7'b1100011:
                imm_o = {{20{inst_i[31]}}, inst_i[7], inst_i[30:25],
                         inst_i[11:8], 1'b0};

            // J-type: JAL
            7'b1101111:
                imm_o = {{12{inst_i[31]}}, inst_i[19:12], inst_i[20],
                         inst_i[30:21], 1'b0};

            // U-type: LUI / AUIPC
            7'b0110111, 7'b0010111:
                imm_o = {inst_i[31:12], 12'b0};

            // R-type and instructions that do not use an immediate.
            default:
                imm_o = 32'b0;
        endcase
    end

endmodule
