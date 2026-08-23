module IFU(
    input clk,
    input rst,
    input [31:0] brach_target_i,
    input        pc_redirect_valid,
    input [31:0] pc_redirect_target,
    output logic [31:0] inst_o,
    output logic [31:0] pc_o
);
    import "DPI-C" function int pmem_read(int raddr);
    logic [31:0] next_pc;
    assign next_pc = pc_redirect_valid ? pc_redirect_target : brach_target_i;
    pc_reg pc_reg_inst(
        .clk(clk),
        .rst(rst),
        .next_pc_i(next_pc),
        .pc_o(pc_o)
    );

    always_comb begin
    inst_o = pmem_read(int'(pc_o));
    end

endmodule