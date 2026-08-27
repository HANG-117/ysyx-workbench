`include "core_defs.sv"

module WBU(
    input  core_types_pkg::wbu_req_t   req_i,
    output core_types_pkg::reg_write_t reg_write_o
);

always_comb begin
    case(req_i.wb_sel)
        `WB_SEL_ALU: reg_write_o.data = req_i.alu_result;
        `WB_SEL_PC4: reg_write_o.data = req_i.meta.pc + 32'd4;
        `WB_SEL_MEM: reg_write_o.data = req_i.load_data;
        `WB_SEL_CSR: reg_write_o.data = req_i.csr_data;
        default:     reg_write_o.data = 32'b0;
    endcase
    reg_write_o.addr = req_i.meta.rd_addr;
    reg_write_o.wen  = req_i.meta.rd_valid && (req_i.meta.rd_addr != 5'd0);
end

endmodule
