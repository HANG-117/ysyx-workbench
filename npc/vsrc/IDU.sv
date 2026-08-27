module IDU(
    input clk,
    input rst,
    input logic [31:0] inst_i,
    input logic [31:0] pc_i,
    output core_types_pkg::decoded_uop_t uop_o
);

    logic [31:0] imm;

    ImmGen immgen_inst(
        .inst_i(inst_i),
        .imm_o(imm)
    );

    decoder decoder_inst(
        .opcode_i(inst_i),
        .pc_i(pc_i),
        .imm_i(imm),
        .uop_o(uop_o)
    );
    
endmodule   
