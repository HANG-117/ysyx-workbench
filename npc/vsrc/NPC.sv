`include "core_defs.sv"

module NPC(
    input clk,
    input rst,
    output logic commit_valid_o
`ifdef SYNTHESIS
    ,
    output logic [31:0] imem_addr_o,
    input  logic [31:0] imem_rdata_i,

    output logic        dmem_valid_o,
    output logic        dmem_we_o,
    output logic [31:0] dmem_addr_o,
    output logic [31:0] dmem_wdata_o,
    output logic [3:0]  dmem_wstrb_o,
    input  logic [31:0] dmem_rdata_i
`endif
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
logic                         ifu_valid;
logic                         lsu_result_valid;
logic [31:0]                  issue_next_pc;
logic [3:0]                   active_csr_type;


IFU ifu_inst(
    .clk(clk),
    .rst(rst),
    .brach_target_i(issue_next_pc),
    .pc_redirect_valid(pc_redirect_valid),
    .pc_redirect_target(pc_redirect_target),
    .ready_i(commit_valid_o),
    .valid_o(ifu_valid),
    .inst_o(inst),
    .pc_o(pc)
`ifdef SYNTHESIS
    ,
    .imem_addr_o(imem_addr_o),
    .imem_rdata_i(imem_rdata_i)
`endif
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
    .wen(reg_write.wen && commit_valid_o),
    .a0_o(a0)
);

csr_reg csr_reg_inst(
    .clk(clk),
    .rst(rst),
    .addr(decoded_uop.csr_addr),
    .csr_type(active_csr_type),
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
    .valid_i(ifu_valid && exu_issue_valid),
    .req_i(exu_req),
    .result_o(exu_result)
);

// A memory uop does not enter EXU.  It cannot redirect control flow, so its
// fetch successor is the architectural sequential PC.
always_comb begin
    issue_next_pc = exu_issue_valid ? exu_result.next_pc
                                    : (decoded_uop.meta.pc + 32'd4);
end

// Do not allow stale irom data to create architectural side effects while
// IFU is fetching. A memory instruction retires only with LSU's response.
always_comb begin
    active_csr_type = ifu_valid ? decoded_uop.csr_type : `CSR_NONE;
    commit_valid_o  = ifu_valid &&
                      (exu_issue_valid ||
                       (lsu_issue_valid && lsu_result_valid));
end

LSU lsu_inst(
    .clk(clk),
    .rst(rst),
    .valid_i(ifu_valid && lsu_issue_valid),
    .req_i(lsu_req),
    .result_valid_o(lsu_result_valid),
    .result_o(lsu_result)
`ifdef SYNTHESIS
    ,
    .dmem_valid_o(dmem_valid_o),
    .dmem_we_o(dmem_we_o),
    .dmem_addr_o(dmem_addr_o),
    .dmem_wdata_o(dmem_wdata_o),
    .dmem_wstrb_o(dmem_wstrb_o),
    .dmem_rdata_i(dmem_rdata_i)
`endif
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
