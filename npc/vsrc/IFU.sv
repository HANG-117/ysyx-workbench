module IFU(
    input clk,
    input rst,
    input [31:0] next_pc_i,
    output logic [31:0] inst_o,
    output logic [31:0] pc_o
);
    import "DPI-C" function int pmem_read(int raddr);

    pc_reg pc_reg_inst(
        .clk(clk),
        .rst(rst),
        .next_pc_i(next_pc_i),
        .pc_o(pc_o)
    );

    always_comb begin
    inst_o = pmem_read(int'(pc_o));
    end

endmodule