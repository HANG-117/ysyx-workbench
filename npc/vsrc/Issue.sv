// Single-issue routing stage.
//
// This module is deliberately the only place that decides which backend unit
// owns an instruction.  Keeping the decision here means a future dual-issue
// implementation can replace the scalar input/output ports with issue lanes
// (or queues) without changing IDU, EXU, or LSU request formats.
module Issue(
    input  core_types_pkg::decoded_uop_t uop_i,
    input  logic [31:0]                  rs1_data_i,
    input  logic [31:0]                  rs2_data_i,

    output core_types_pkg::exu_req_t     exu_req_o,
    output logic                         exu_valid_o,
    output core_types_pkg::lsu_req_t     lsu_req_o,
    output logic                         lsu_valid_o
);

    logic is_lsu_uop;

    // Loads and stores are issued only to LSU.  All remaining currently
    // supported instructions (integer ALU, control flow, and CSR) use EXU.
    assign is_lsu_uop = uop_i.load || uop_i.store;
    assign lsu_valid_o = is_lsu_uop;
    assign exu_valid_o = !is_lsu_uop;

    always_comb begin
        exu_req_o = '0;
        if (exu_valid_o) begin
            exu_req_o.meta          = uop_i.meta;
            exu_req_o.rs1_data      = rs1_data_i;
            exu_req_o.rs2_data      = rs2_data_i;
            exu_req_o.imm           = uop_i.imm;
            exu_req_o.alu_rs1_sel   = uop_i.alu_rs1_sel;
            exu_req_o.alu_rs2_sel   = uop_i.alu_rs2_sel;
            exu_req_o.alu_op        = uop_i.alu_op;
            exu_req_o.branch_cond   = uop_i.branch_cond;
            exu_req_o.branch_en     = uop_i.branch;
            exu_req_o.jump_en       = uop_i.jump;
            exu_req_o.jump_base_rs1 = uop_i.jump_base_rs1;
        end
    end

    always_comb begin
        lsu_req_o = '0;
        if (lsu_valid_o) begin
            lsu_req_o.meta         = uop_i.meta;
            lsu_req_o.load         = uop_i.load;
            lsu_req_o.store        = uop_i.store;
            // Memory address generation belongs to the LSU issue path, so a
            // memory operation no longer needs to be sent through EXU first.
            lsu_req_o.addr         = rs1_data_i + uop_i.imm;
            lsu_req_o.store_data   = rs2_data_i;
            lsu_req_o.mem_size     = uop_i.mem_size;
            lsu_req_o.mem_unsigned = uop_i.mem_unsigned;
        end
    end

endmodule
