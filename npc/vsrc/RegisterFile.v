module RegisterFile #(ADDR_WIDTH = 1, DATA_WIDTH = 1) (
  input clk,
  input rst,
  input [31:0] pc,
	input [ADDR_WIDTH-1:0] raddr1,
	input [ADDR_WIDTH-1:0] raddr2,
	output logic [DATA_WIDTH-1:0] rdata1,
	output logic [DATA_WIDTH-1:0] rdata2,

  input [DATA_WIDTH-1:0] wdata,
  input [ADDR_WIDTH-1:0] waddr,
  input wen
);
  
  reg [DATA_WIDTH-1:0] rf [2**ADDR_WIDTH-1:0];
  integer i;
  always @(*) begin
    rdata1 = rf[raddr1];
    rdata2 = rf[raddr2];
  end
  
  always @(posedge clk) begin

    if(rst) begin
      for(i = 0; i < 2**ADDR_WIDTH; i = i + 1) begin
        rf[i] <= 0;
      end
    end
    else begin
      if (wen&&waddr != 0) begin
        rf[waddr] <= wdata;
        $display("pc : %08x Write Register: x%0d <= 0x%0h, ", pc, waddr, wdata);
      end
    end
  end
endmodule