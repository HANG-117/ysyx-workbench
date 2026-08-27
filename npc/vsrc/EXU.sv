module EXU(
    input  logic                         valid_i,
    input  core_types_pkg::exu_req_t    req_i,
    output core_types_pkg::exu_result_t result_o
);

    logic [31:0] rs1;
    logic [31:0] rs2;
    always_comb begin
        rs1 = req_i.alu_rs1_sel ? req_i.meta.pc : req_i.rs1_data;
        rs2 = req_i.alu_rs2_sel ? req_i.imm : req_i.rs2_data;
    end

    // ALU：计算 + 内部完成分支判定
    logic [31:0] alu_result;
    logic        branch_taken;
    alu alu_inst(
        .rs1_i         (rs1),
        .rs2_i         (rs2),
        .alu_op_i      (req_i.alu_op),
        .branch_cond_i (req_i.branch_cond),
        .branch_en_i   (req_i.branch_en),
        .alu_result_o  (alu_result),
        .branch_taken_o(branch_taken)
    );

    // 目标地址计算
    logic [31:0] target_addr;
    always_comb begin
        if (req_i.jump_base_rs1)       // JALR: 目标 = (rs1 + imm) 清最低位
            target_addr = (req_i.rs1_data + req_i.imm) & ~32'h1;
        else                       // JAL / Branch: 目标 = PC + imm
            target_addr = req_i.meta.pc + req_i.imm;
    end

    // next_pc：跳转/分支命中则用目标地址，否则顺序取 PC+4
    always_comb begin
        result_o = '0;
        if (valid_i) begin
            result_o.meta         = req_i.meta;
            result_o.alu_result   = alu_result;
            result_o.branch_taken = branch_taken;
            result_o.next_pc      = (branch_taken || req_i.jump_en)
                                  ? target_addr
                                  : (req_i.meta.pc + 32'h4);
        end
    end

endmodule
