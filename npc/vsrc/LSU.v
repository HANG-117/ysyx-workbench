module LSU(
    input clk,
    input dmem_write,
    input [31:0] dmem_addr,
    input [31:0] dmem_wdata,
    output [31:0] dmem_rdata,
    input [31:0] dmem_bytes
    
);
    import "DPI-C" function int pmem_read(input int raddr);
    import "DPI-C" function void pmem_write(input int waddr, input int wdata, input int wmask);

    assign dmem_rdata = pmem_read(dmem_addr);
    
    always @(posedge clk) begin
        if (dmem_write) begin
            pmem_write(dmem_addr, dmem_wdata, dmem_bytes);
        end
    end

endmodule