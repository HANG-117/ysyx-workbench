module dram(
    input logic clk,
    input logic valid,
    input logic wen,
    input logic [31:0] addr,
    input logic [31:0] wdata,
    input logic [3:0] mask,
    output logic [31:0] rdata
);

    import "DPI-C" function void pmem_write(input int addr, input int data, input int mask);
    import "DPI-C" function int pmem_read(input int addr);

    always_ff @(posedge clk) begin
        if (valid) begin
            if (wen) begin
                pmem_write(int'(addr), int'(wdata), int'(mask));
            end else begin
                rdata <= pmem_read(int'(addr));
            end
        end
    end
endmodule
