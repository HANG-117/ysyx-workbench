module LSU(
    input  logic        clk,
    input  logic        rst,
    input  logic        load_i,
    input  logic        store_i,
    input  logic [31:0] addr_i,
    input  logic [31:0] store_data_i,
    input  logic [1:0]  mem_size_i,
    input  logic        mem_unsigned_i,
    output logic [31:0] load_data_o
);

import "DPI-C" function void pmem_write(input int addr, input int data, input int mask);
import "DPI-C" function int pmem_read(input int addr);

logic [31:0] rdata;
logic [3:0] wmask;

always_comb begin
    rdata = pmem_read(int'(addr_i & 32'hfffffffc));
end

always_comb begin
    load_data_o = 32'b0;

    if(load_i) begin
        case(mem_size_i)
            2'b00: begin // byte
                case(addr_i[1:0])
                    2'b00: load_data_o = mem_unsigned_i ? {24'b0,rdata[7:0]}   : {{24{rdata[7]}},rdata[7:0]};
                    2'b01: load_data_o = mem_unsigned_i ? {24'b0,rdata[15:8]}  : {{24{rdata[15]}},rdata[15:8]};
                    2'b10: load_data_o = mem_unsigned_i ? {24'b0,rdata[23:16]} : {{24{rdata[23]}},rdata[23:16]};
                    2'b11: load_data_o = mem_unsigned_i ? {24'b0,rdata[31:24]} : {{24{rdata[31]}},rdata[31:24]};
                endcase
            end

            2'b01: begin // half
                if(addr_i[1:0]==2'b00)
                    load_data_o = mem_unsigned_i ? {16'b0,rdata[15:0]} : {{16{rdata[15]}},rdata[15:0]};
                else if(addr_i[1:0]==2'b10)
                    load_data_o = mem_unsigned_i ? {16'b0,rdata[31:16]} : {{16{rdata[31]}},rdata[31:16]};
            end

            2'b10: begin // word
                load_data_o = rdata;
            end

            default:
                load_data_o = 32'b0;
        endcase
    end
end

always_comb begin
    case(mem_size_i)
        2'b00: wmask = 4'b0001 << addr_i[1:0]; // byte
        2'b01: wmask = (addr_i[1:0]==2'b00) ? 4'b0011 :
                       (addr_i[1:0]==2'b10) ? 4'b1100 : 4'b0000; // half
        2'b10: wmask = 4'b1111; // word
        default: wmask = 4'b0000;
    endcase
end

always_ff @(posedge clk) begin
    if(store_i)
        pmem_write(int'(addr_i), int'(store_data_i), int'(wmask));
end

endmodule