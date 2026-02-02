module top(
  input [7:0]in,
  input en,
  output reg is_in,
  output reg [2:0]out,
  output [7:0]led0,
  output [7:0]led1,
  output [7:0]led2,
  output [7:0]led3
  
 );
  reg [31:0] count;
  parameter CLK_NUM = 5000000;
  always @(in or en) begin
    if(en)begin
      is_in = (in != 8'b0);    
      casez(in)
          8'b1??????? : out = 3'b111;
          8'b01?????? : out = 3'b110;
          8'b001????? : out = 3'b101;
          8'b0001???? : out = 3'b100;
          8'b00001??? : out = 3'b011;
          8'b000001?? : out = 3'b010;
          8'b0000001? : out = 3'b001;
          8'b00000001 : out = 3'b000;
          default     : out = 3'b000;
      endcase
    end
    else begin
      is_in = 1'b0;
      out = 3'b0;
    end
  end
  assign led0 = ~(out[0] ?  8'b01100000: 8'b11111100);
  assign led1 = ~(out[1] ?  8'b01100000 : 8'b11111100);
  assign led2 = ~(out[2] ?  8'b01100000 : 8'b11111100);
  assign led3 = ~(is_in ?  8'b01100000: 8'b11111100);
endmodule
