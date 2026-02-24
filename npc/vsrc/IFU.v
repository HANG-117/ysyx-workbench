module IFU(
    input [31:0] PC,
    input [31:0] imem_rdata,
    output imem_valid,
    output reg [31:0] imem_addr,
    output reg [31:0] inst
);
    assign imem_valid = 1'b1;
    assign inst = imem_rdata;
    assign imem_addr = PC;

    

endmodule