`include "core_defs.sv"

module NPC(
    input clk,
    input rst
);

logic [31:0] next_pc;
logic [31:0] inst;
logic [31:0] pc;

logic exec, load, store, branch, rd_valid, jump, jump_base_rs1;
logic [1:0] mem_size;
logic mem_unsigned;
logic [3:0] alu_op;
logic [2:0] wb_sel;
logic alu_rs1_sel, alu_rs2_sel;
logic [2:0] branch_cond;
logic [4:0] rs1_addr, rs2_addr, rd_addr;

logic [31:0] reg_rdata1, reg_rdata2;
logic [31:0] reg_wdata;
logic reg_wen;

logic [31:0] alu_result;
logic branch_taken;
logic [31:0] exu_next_pc;

logic [31:0] load_data;

logic csr;
logic [31:0] a0;
logic [3:0] csr_type;
logic [11:0] csr_addr;
logic [31:0] csr_rdata;
logic        pc_redirect_valid;
logic [31:0] pc_redirect_target;


IFU ifu_inst(
    .clk(clk),
    .rst(rst),
    .brach_target_i(exu_next_pc),
    .pc_redirect_valid(pc_redirect_valid),
    .pc_redirect_target(pc_redirect_target),
    .inst_o(inst),
    .pc_o(pc)
);

IDU idu_inst(
    .clk(clk),
    .rst(rst),
    .inst_i(inst),
    .csr_o(csr),
    .exec_o(exec),
    .load_o(load),
    .store_o(store),
    .branch_o(branch),
    .rd_valid_o(rd_valid),
    .jump_o(jump),
    .jump_base_rs1_o(jump_base_rs1),
    .mem_size_o(mem_size),
    .mem_unsigned_o(mem_unsigned),
    .csr_type_o(csr_type),
    .csr_addr_o(csr_addr),
    .alu_op_o(alu_op),
    .wb_sel_o(wb_sel),
    .alu_rs1_sel_o(alu_rs1_sel),
    .alu_rs2_sel_o(alu_rs2_sel),
    .branch_cond_o(branch_cond),
    .rs1_addr_o(rs1_addr),
    .rs2_addr_o(rs2_addr),
    .rd_addr_o(rd_addr)
);

RegisterFile #(.ADDR_WIDTH(5), .DATA_WIDTH(32)) regfile_inst(
    .clk(clk),
    .rst(rst),
    .raddr1(rs1_addr),
    .raddr2(rs2_addr),
    .rdata1(reg_rdata1),
    .rdata2(reg_rdata2),
    .waddr(rd_addr),
    .wdata(reg_wdata),
    .wen(reg_wen),
    .a0_o(a0)
);

csr_reg csr_reg_inst(
    .clk(clk),
    .rst(rst),
    .addr(csr_addr),
    .csr_type(csr_type),
    .rs1_addr(rs1_addr),
    .rs1_data(reg_rdata1),
    .pc(pc),
    .a0(a0),
    .csr_rdata(csr_rdata),
    .pc_redirect_valid(pc_redirect_valid),
    .pc_redirect_target(pc_redirect_target)
);

EXU exu_inst(
    .inst_i(inst),
    .pc_i(pc),
    .rs1_rdata_i(reg_rdata1),
    .rs2_rdata_i(reg_rdata2),
    .alu_rs1_sel_i(alu_rs1_sel),
    .alu_rs2_sel_i(alu_rs2_sel),
    .alu_op_i(alu_op),
    .branch_cond_i(branch_cond),
    .branch_en_i(branch),
    .jump_en_i(jump),
    .jump_base_rs1_i(jump_base_rs1),
    .alu_result_o(alu_result),
    .branch_taken_o(branch_taken),
    .next_pc_o(exu_next_pc)
);

LSU lsu_inst(
    .clk(clk),
    .rst(rst),
    .load_i(load),
    .store_i(store),
    .addr_i(alu_result),
    .store_data_i(reg_rdata2),
    .mem_size_i(mem_size),
    .mem_unsigned_i(mem_unsigned),
    .load_data_o(load_data)
);

WBU wbu_inst(
    .rd_valid_i(rd_valid),
    .wb_sel_i(wb_sel),
    .alu_result_i(alu_result),
    .load_data_i(load_data),
    .csr_rdata_i(csr_rdata),
    .pc_i(pc),
    .rd_addr_i(rd_addr),
    .reg_wen_o(reg_wen),
    .reg_wdata_o(reg_wdata)
);

endmodule
