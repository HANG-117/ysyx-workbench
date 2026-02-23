module RegisterFile #(ADDR_WIDTH = 5, DATA_WIDTH = 32) (
  input clk,
  input rst,
  input [31:0] pc,
  output [DATA_WIDTH-1:0] rdata1,
  input [ADDR_WIDTH-1:0] raddr1,
  output [DATA_WIDTH-1:0] rdata2,
  input [ADDR_WIDTH-1:0] raddr2,
  input [DATA_WIDTH-1:0] wdata,
  input [ADDR_WIDTH-1:0] waddr,
  input wen
);
  reg [DATA_WIDTH-1:0] rf [2**ADDR_WIDTH-1:0];
  assign rdata1 = raddr1== 0 ? {DATA_WIDTH{1'b0}} : rf[raddr1];
  assign rdata2 = raddr2== 0 ? {DATA_WIDTH{1'b0}} : rf[raddr2];

  always @(posedge clk) begin
    if (wen) begin
      rf[waddr] <= wdata;
      $strobe("x10 = %08h x11 = %08h x2 = %08h", rf[10], rf[11], rf[2]);

    end
  end
endmodule