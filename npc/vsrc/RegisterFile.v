module RegisterFile #(ADDR_WIDTH = 1, DATA_WIDTH = 1) (
  input clk,

	input [ADDR_WIDTH-1:0] raddr1,
	input [ADDR_WIDTH-1:0] raddr2,
	output [DATA_WIDTH-1:0] rdata1,
	output [DATA_WIDTH-1:0] rdata2,

  input [DATA_WIDTH-1:0] wdata,
  input [ADDR_WIDTH-1:0] waddr,
  input wen
);

	assign rdata1 = raddr1 == 0 ? 0 : rf[raddr1];
	assign rdata2 = raddr2 == 0 ? 0 : rf[raddr2];

  reg [DATA_WIDTH-1:0] rf [2**ADDR_WIDTH-1:0];
  always @(posedge clk) begin
    if (wen && waddr != 0) begin
			rf[waddr] <= wdata;
			$display("Write reg: x%d, data: 0x%08x", waddr, wdata);
		end
		$strobe("x1: %x, x2: %x x3: %x x4: %x x5: %x  x28: %x x29: %x ", rf[1], rf[2], rf[3], rf[4], rf[5], rf[28], rf[29]);
  end
endmodule