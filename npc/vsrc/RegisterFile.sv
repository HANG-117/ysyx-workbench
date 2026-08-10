module RegisterFile #(ADDR_WIDTH = 1, DATA_WIDTH = 1) (
	input clk,
	input rst,
	input [ADDR_WIDTH-1:0] raddr1,
	input [ADDR_WIDTH-1:0] raddr2,
	output [DATA_WIDTH-1:0] rdata1,
	output [DATA_WIDTH-1:0] rdata2,

	input [DATA_WIDTH-1:0] wdata,
	input [ADDR_WIDTH-1:0] waddr,
	input wen,
	output [DATA_WIDTH-1:0] a0_o
);


	assign rdata1 = raddr1 == 0 ? 0 : rf[raddr1];
	assign rdata2 = raddr2 == 0 ? 0 : rf[raddr2];
	assign a0_o = rf[10]; // a0 is x10
	reg [DATA_WIDTH-1:0] rf [2**ADDR_WIDTH-1:0];
	always @(posedge clk) begin
    	if(rst) begin
			for(int i = 0; i < 2**ADDR_WIDTH; i = i + 1) begin
				rf[i] <= 0;
			end
		end
		else if (wen && waddr != 0) begin
			rf[waddr] <= wdata;
		end
	end
endmodule