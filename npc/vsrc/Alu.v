module Alu(
    input [31:0] in1,
    input [31:0] in2,
    input [2:0] alu_op,
    output reg [31:0] result
);
    always_comb begin
        case(alu_op)
            3'b000: result = in1 + in2;  // ADD
            3'b001: result = in1 - in2;  // SUB
            3'b010: result = in1 & in2;  // AND
            3'b011: result = in1 | in2;  // OR
            3'b100: result = in1 ^ in2;  // XOR
            default: result = 32'h0;
        endcase
    end
endmodule