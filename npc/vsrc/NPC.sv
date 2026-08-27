`include "core_defs.sv"

module NPC(
    input clk,
    input rst
);

logic [31:0] inst;
logic [31:0] pc;

logic [31:0] reg_rdata1, reg_rdata2;

logic [31:0] a0;
logic [31:0] csr_rdata;
logic        pc_redirect_valid;
logic [31:0] pc_redirect_target;

core_types_pkg::decoded_uop_t decoded_uop;
core_types_pkg::exu_req_t     exu_req;
core_types_pkg::exu_result_t  exu_result;
core_types_pkg::lsu_req_t     lsu_req;
core_types_pkg::lsu_result_t  lsu_result;
core_types_pkg::wbu_req_t     wbu_req;
core_types_pkg::reg_write_t   reg_write;
logic                         exu_issue_valid;
logic                         lsu_issue_valid;
logic [31:0]                  issue_next_pc;


IFU ifu_inst(
    .clk(clk),
    .rst(rst),
    .brach_target_i(issue_next_pc),
    .pc_redirect_valid(pc_redirect_valid),
    .pc_redirect_target(pc_redirect_target),
    .inst_o(inst),
    .pc_o(pc)
);

IDU idu_inst(
    .clk(clk),
    .rst(rst),
    .inst_i(inst),
    .pc_i(pc),
    .uop_o(decoded_uop)
);

RegisterFile #(.ADDR_WIDTH(5), .DATA_WIDTH(32)) regfile_inst(
    .clk(clk),
    .rst(rst),
    .raddr1(decoded_uop.meta.rs1_addr),
    .raddr2(decoded_uop.meta.rs2_addr),
    .rdata1(reg_rdata1),
    .rdata2(reg_rdata2),
    .waddr(reg_write.addr),
    .wdata(reg_write.data),
    .wen(reg_write.wen),
    .a0_o(a0)
);

csr_reg csr_reg_inst(
    .clk(clk),
    .rst(rst),
    .addr(decoded_uop.csr_addr),
    .csr_type(decoded_uop.csr_type),
    .rs1_addr(decoded_uop.meta.rs1_addr),
    .rs1_data(reg_rdata1),
    .pc(decoded_uop.meta.pc),
    .a0(a0),
    .csr_rdata(csr_rdata),
    .pc_redirect_valid(pc_redirect_valid),
    .pc_redirect_target(pc_redirect_target)
);

Issue issue_inst(
    .uop_i(decoded_uop),
    .rs1_data_i(reg_rdata1),
    .rs2_data_i(reg_rdata2),
    .exu_req_o(exu_req),
    .exu_valid_o(exu_issue_valid),
    .lsu_req_o(lsu_req),
    .lsu_valid_o(lsu_issue_valid)
);

EXU exu_inst(
    .valid_i(exu_issue_valid),
    .req_i(exu_req),
    .result_o(exu_result)
);

// A memory uop does not enter EXU.  It cannot redirect control flow, so its
// fetch successor is the architectural sequential PC.
always_comb begin
    issue_next_pc = exu_issue_valid ? exu_result.next_pc
                                    : (decoded_uop.meta.pc + 32'd4);
end

LSU lsu_inst(
    .clk(clk),
    .rst(rst),
    .valid_i(lsu_issue_valid),
    .req_i(lsu_req),
    .result_o(lsu_result)
);

always_comb begin
    wbu_req = '0;
    wbu_req.meta       = decoded_uop.meta;
    wbu_req.wb_sel     = decoded_uop.wb_sel;
    wbu_req.alu_result = exu_result.alu_result;
    wbu_req.load_data  = lsu_result.load_data;
    wbu_req.csr_data   = csr_rdata;
end

WBU wbu_inst(
    .req_i(wbu_req),
    .reg_write_o(reg_write)
);

endmodule
