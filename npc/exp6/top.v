module top(
    input clk,
    input rst,
    input [7:0] in,
    output reg [7:0] out
);
    reg [31:0] count;
    reg num;
    always @(posedge clk) begin
        if(rst) begin
            count <= 0;
            out <= 8'b0;
        end
        else begin
            if(count == 0)begin
                out = in;
            end
            else begin
                if (out == 8'b0)begin 
                    out = 8'b11111111;
                end
                else begin
                    num = out[0] ^ out[1] ^ out[2] ^ out[3];
                    out = {num, out[7:1]};
                end
            end
            count <= count + 1;
        end
    end
    
endmodule