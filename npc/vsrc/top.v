module top(
    input clk,
    input rst,
    output [7:0] out,
    output reg [7:0]led[1:0]
);
    reg [3:0]PC;
    reg [7:0]ROM[15:0];
    reg [7:0]GPR[3:0];
    reg [7:0]tmp_led[1:0];
    initial begin
        ROM[0]  = 8'b1000_1010;
        ROM[1]  = 8'b1001_0000;
        ROM[2]  = 8'b1010_0000;
        ROM[3]  = 8'b1011_0001;
        ROM[4]  = 8'b0001_0111;
        ROM[5]  = 8'b0010_1001;
        ROM[6]  = 8'b1101_0001;
        ROM[7]  = 8'b0100_0000;
        ROM[8]  = 8'b1110_0011;
        ROM[9]  = 8'b0000_0000;
        ROM[10] = 8'b0000_0000;
        ROM[11] = 8'b0000_0000;
        ROM[12] = 8'b0000_0000;
        ROM[13] = 8'b0000_0000;
        ROM[14] = 8'b0000_0000;
        ROM[15] = 8'b0000_0000;
    end
    always @(posedge clk)begin
        $display("PC=%d,GPR0=%d,GPR1=%d,GPR2=%d,GPR3=%d",PC,GPR[0],GPR[1],GPR[2],GPR[3]);

        if(rst)begin
            PC <= 0;
        end
        else begin
            if(ROM[PC][7:6] == 2'b10)begin
                GPR[ROM[PC][5:4]] <= {4'b0000,ROM[PC][3:0]};
                PC <= PC+1;
            end
            else if(ROM[PC][7:6] == 2'b00)begin
                GPR[ROM[PC][5:4]] <= GPR[ROM[PC][3:2]] + GPR[ROM[PC][1:0]];
                PC <= PC +1;
            end
            else if(ROM[PC][7:6] == 2'b11)begin
                if(GPR[ROM[PC][1:0]] != GPR[0])begin
                    PC <= ROM[PC][5:2];
                end
                else begin
                    PC <= PC + 1;
                end
            end
            else if(ROM[PC][7:6] == 2'b01)begin
                led[0] <= tmp_led[0];
                led[1] <= tmp_led[1];
                PC <= PC + 1;
            end
        end
    end
    assign out = GPR[2];
    seg se1(
        .in(out[3:0]),
        .seg(tmp_led[0])
    );
    seg se2(
        .in(out[7:4]),
        .seg(tmp_led[1])
    );
endmodule
