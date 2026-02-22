module top(
    input  [3:0]a,
    input  [3:0]b,
    input  [2:0]sel,
    output reg [3:0]out,
    output reg jud,
    output reg overflow
);
   wire [3:0]b_neg;
   assign b_neg = ~b + 1;
    
always @(a or b or sel)begin
    out = 4'b0;
    jud = 1'b0;
    overflow = 1'b0;
    case(sel)
        3'b000:begin
            out = a + b;
            overflow = (a[3]==b[3]) && (a[3] != out[3]);
        end
        3'b001:begin
            out = a + b_neg;
            overflow = (a[3]==b_neg[3]) && (a[3] != out[3]);
        end
        3'b010:begin
            out = a ^ 4'b1111;
        end
        3'b011:begin
            out = a & b;
        end
        3'b100:begin
            out = a | b;
        end
        3'b101:begin
            out = a ^ b;
        end
        3'b110:begin
            jud = ((a-b) & 4'b1000) == 4'b1000 ? 1'b1:1'b0;
        end
        3'b111:begin
            jud = (a - b) == 0 ? 1'b1:1'b0;
        end
    endcase 
end
endmodule