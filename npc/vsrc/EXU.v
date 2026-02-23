module EXU(
    input [31:0] data1,
    input [31:0] data2,
    input [2:0] alu_option,
    output [31:0] result
);
    assign result = (alu_option == 3'b000) ? data1 + data2 :
                    (alu_option == 3'b001) ? data1 - data2 :
                    (alu_option == 3'b010) ? data1 & data2 :
                    (alu_option == 3'b011) ? data1 | data2 :
                    (alu_option == 3'b100) ? data1 ^ data2 : 32'b0;
endmodule