module top(
    input clk,
    input ps2_clk,
    input ps2_data,
    input rst,
    input uart_rx,
    output uart_tx,
    output [7:0] data,
    output [7:0] seg1,
    output [7:0] seg2,
    output [7:0] seg3,
    output [7:0] seg4,
    output [7:0] seg5,
    output [7:0] seg6,
    output [7:0] count  
);
    reg [7:0] data2;
    wire [7:0] scan_code;
    wire data_ready;  
    wire overflow;    
    
    reg [7:0] key_count = 8'h00;  
    reg key_pressed = 1'b0;      
    reg [7:0] last_scan_code;
    reg [32:0] clk_count;
    
    reg nextdata_n;
    reg [1:0] key_state;
    localparam WAIT_KEY = 2'b00;
    localparam KEY_PRESSED = 2'b01;
    localparam WAIT_RELEASE = 2'b10;
    always @(posedge clk) begin
        if(rst) begin
            key_state <= WAIT_KEY;
            key_count <= 8'h00;
            key_pressed <= 1'b0;
            nextdata_n <= 1'b1;
        end
        else if(data_ready) begin
            case(key_state)
                WAIT_KEY: begin
                    //if(scan_code != 8'hF0) begin
                    clk_count++;
                    last_scan_code = (clk_count == 500000) ? 8'h00 : last_scan_code;
                    if(scan_code != 8'hF0 && last_scan_code != scan_code) begin
                        clk_count = 0;
                        key_pressed = 1'b1;
                        key_state <= KEY_PRESSED;
                        key_count <= key_count + 8'h01; 
                        last_scan_code <= scan_code;
                        $display("按键按下，扫描码: %h, 计数: %d", scan_code, key_count + 8'h01);
                    end
                    else begin
                        key_pressed = 1'b0;
                    end
                    nextdata_n <= 1'b0; 
                end
                KEY_PRESSED: begin
                    $display("按键准备释放，扫描码: %h", scan_code);
                    key_pressed = 1'b1;
                    if(scan_code == 8'hF0) begin
                        key_state <= WAIT_RELEASE;
                    end
                    nextdata_n <= 1'b0;
                end
                
                WAIT_RELEASE: begin
                    $display("按键已释放，准备等待下一个按键");
                    key_pressed = 1'b0;
                    key_state <= WAIT_KEY;
                    nextdata_n <= 1'b0;  
                end
                
                default: begin
                    $display("未知状态");
                    key_state <= WAIT_KEY;
                    nextdata_n <= 1'b1;
                end
            endcase
        end
        else begin
            nextdata_n <= 1'b1;
        end
    end
    uart my_uart(
    .tx(uart_tx),
    .rx(uart_rx)
    );
    ps2_keyboard u_ps2_keyboard(
        .clk(clk),
        .clrn(~rst), 
        .ps2_clk(ps2_clk),
        .ps2_data(ps2_data),
        .data(scan_code),
        .ready(data_ready),
        .nextdata_n(nextdata_n), 
        .overflow(overflow)
    );
    
    ascii u_ascii(
        .in(scan_code),
        .ascii_noshift(data2)
    );
    
    seg u_seg1(
        .in(scan_code[3:0]),
        .pressed(key_pressed),
        .seg(seg1)
    );
    seg u_seg2(
        .in(scan_code[7:4]),
        .pressed(key_pressed),
        .seg(seg2)
    );
    seg u_seg3(
        .in(data2[3:0]),
        .pressed(key_pressed),
        .seg(seg3)
    );
    seg u_seg4(
        .in(data2[7:4]),
        .pressed(key_pressed),
        .seg(seg4)
    );

    
    seg u_seg5(
        .in(key_count[3:0]), 
        .pressed(1'b1),
        .seg(seg5)
    );
    seg u_seg6(
        .in(key_count[7:4]), 
        .pressed(1'b1),
        .seg(seg6)
    );
    assign data = scan_code;
    assign count = key_count;
endmodule