module LSU(
    input clk,
    input dmem_write,
    input [31:0] dmem_addr,
    input [31:0] dmem_wdata,
    output reg [31:0] dmem_rdata,
    input  [2:0]  dmem_bytes
    
);
    localparam MEM_BASE = 32'h80000000;
    localparam MEM_SIZE = 32'h1000000;
    import "DPI-C" function int pmem_read(int raddr);
    import "DPI-C" function void pmem_write(int waddr,int wdata,int wmask);
    reg [3:0] wmask;
    always @(*) begin
        case(dmem_bytes)
            3'b100: wmask = 4'b1111;
            3'b001: wmask = 4'b0001 << dmem_addr[1:0];
            default: wmask = 4'b0000;
        endcase
    end    
    always @(clk) begin
        if (dmem_write) begin
            pmem_write(dmem_addr, dmem_wdata,{28'b0, wmask});
        end
    end
    always @(*) begin
        dmem_rdata = pmem_read(dmem_addr);
    end
    

endmodule