module LSU(
    input clk,
    input dmem_write,
    input [31:0] dmem_addr,
    input [31:0] dmem_wdata,
    output reg [31:0] dmem_rdata,
    input [31:0] dmem_bytes
    
);
    localparam MEM_BASE = 32'h80000000;
    localparam MEM_SIZE = 32'h1000000;
    import "DPI-C" function int pmem_read(input int raddr);
    import "DPI-C" function void pmem_write(input int waddr, input int wdata, input int wmask);

    always @(*) begin
        if(dmem_addr > MEM_BASE + MEM_SIZE) begin
            $display("Error: Address out of range");
            dmem_rdata = 0;
        end
        else begin
            dmem_rdata = pmem_read(dmem_addr);
        end
    end
    always @(posedge clk) begin
        if (dmem_write) begin
            pmem_write(dmem_addr, dmem_wdata, dmem_bytes);
        end
    end

endmodule