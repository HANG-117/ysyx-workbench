module irom(
    input logic clk,
    input logic [31:0] addr,
    output logic [31:0] data
);
    import "DPI-C" function int pmem_read(int raddr);
    always @(posedge clk) begin
        data <= pmem_read(int'(addr));
    end
endmodule
