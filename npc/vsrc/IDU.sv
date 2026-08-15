module IDU(
    input clk,
    input rst,
    input logic [31:0] inst_i,
    output logic exec_o,
    output logic load_o,
    output logic store_o,
    output logic branch_o,
    output logic rd_valid_o,
    output logic jump_o,
    output logic jump_base_rs1_o,
    output logic [1:0] mem_size_o,
    output logic mem_unsigned_o,
    output logic [3:0] alu_op_o,
    output logic [2:0] wb_sel_o,
    output logic alu_rs1_sel_o,
    output logic alu_rs2_sel_o,
    output logic [2:0] branch_cond_o,
    output logic [4:0] rs1_addr_o,
    output logic [4:0] rs2_addr_o,
    output logic [4:0] rd_addr_o
    
);

    decoder decoder_inst(
        .opcode_i(inst_i),
        .exec_o(exec_o),
        .load_o(load_o),
        .store_o(store_o),
        .branch_o(branch_o),
        .rd_valid_o(rd_valid_o),
        .jump_o(jump_o),
        .jump_base_rs1_o(jump_base_rs1_o),
        .mem_size_o(mem_size_o),
        .mem_unsigned_o(mem_unsigned_o),
        .alu_op_o(alu_op_o),
        .wb_sel_o(wb_sel_o),
        .alu_rs1_sel_o(alu_rs1_sel_o),
        .alu_rs2_sel_o(alu_rs2_sel_o),
        .branch_cond_o(branch_cond_o),
        .rs1_addr_o(rs1_addr_o),
        .rs2_addr_o(rs2_addr_o),
        .rd_addr_o(rd_addr_o)
    );
    
endmodule   
