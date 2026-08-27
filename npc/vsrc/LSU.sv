module LSU(
    input  logic        clk,
    input  logic        rst,
    input  logic        valid_i,
    input  core_types_pkg::lsu_req_t    req_i,
    output core_types_pkg::lsu_result_t result_o
);

import "DPI-C" function void pmem_write(input int addr, input int data, input int mask);
import "DPI-C" function int pmem_read(input int addr);

logic [31:0] rdata;
logic [31:0] store_data;   // 按地址偏移对齐后的写数据
logic [3:0] wmask;

always_comb begin
    rdata = pmem_read(int'(req_i.addr & 32'hfffffffc));
end

// 写数据对齐：RISC-V 中 sb/sh 写入的是寄存器的低 8/16 位，
// 需要先移位到目标字节/半字位置（与 wmask 选中的位置对应），
// 否则未对齐地址（addr[1:0] != 0）会写错位置。
always_comb begin
    case (req_i.mem_size)
        2'b00: store_data = {4{req_i.store_data[7:0]}}  << (8 * req_i.addr[1:0]); // byte
        2'b01: store_data = {2{req_i.store_data[15:0]}} << (8 * req_i.addr[1:0]); // half
        default: store_data = req_i.store_data;                                      // word / 其它
    endcase
end

always_comb begin
    result_o.meta      = req_i.meta;
    result_o.load_data = 32'b0;

    if(req_i.load) begin
        case(req_i.mem_size)
            2'b00: begin // byte
                case(req_i.addr[1:0])
                    2'b00: result_o.load_data = req_i.mem_unsigned ? {24'b0,rdata[7:0]}   : {{24{rdata[7]}},rdata[7:0]};
                    2'b01: result_o.load_data = req_i.mem_unsigned ? {24'b0,rdata[15:8]}  : {{24{rdata[15]}},rdata[15:8]};
                    2'b10: result_o.load_data = req_i.mem_unsigned ? {24'b0,rdata[23:16]} : {{24{rdata[23]}},rdata[23:16]};
                    2'b11: result_o.load_data = req_i.mem_unsigned ? {24'b0,rdata[31:24]} : {{24{rdata[31]}},rdata[31:24]};
                endcase
            end

            2'b01: begin // half
                if(req_i.addr[1:0]==2'b00)
                    result_o.load_data = req_i.mem_unsigned ? {16'b0,rdata[15:0]} : {{16{rdata[15]}},rdata[15:0]};
                else if(req_i.addr[1:0]==2'b10)
                    result_o.load_data = req_i.mem_unsigned ? {16'b0,rdata[31:16]} : {{16{rdata[31]}},rdata[31:16]};
            end

            2'b10: begin // word
                result_o.load_data = rdata;
            end

            default:
                result_o.load_data = 32'b0;
        endcase
    end
end

always_comb begin
    case(req_i.mem_size)
        2'b00: wmask = 4'b0001 << req_i.addr[1:0]; // byte
        2'b01: wmask = (req_i.addr[1:0]==2'b00) ? 4'b0011 :
                       (req_i.addr[1:0]==2'b10) ? 4'b1100 : 4'b0000; // half
        2'b10: wmask = 4'b1111; // word
        default: wmask = 4'b0000;
    endcase
end

always_ff @(posedge clk) begin
    if(valid_i && req_i.store)
        pmem_write(int'(req_i.addr), int'(store_data), int'(wmask));
end

endmodule
