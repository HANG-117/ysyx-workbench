module WBU(
    input  logic        rd_valid_i,
    input  logic [2:0]  wb_sel_i,

    input  logic [4:0]  rd_addr_i,

    input  logic [31:0] alu_result_i,
    input  logic [31:0] load_data_i,
    input  logic [31:0] pc_i,

    output logic        reg_wen_o,
    output logic [31:0] reg_wdata_o
);

always_comb begin
    case(wb_sel_i)
        `WB_SEL_ALU: reg_wdata_o = alu_result_i;
        `WB_SEL_PC4: reg_wdata_o = pc_i + 32'd4;
        `WB_SEL_MEM: reg_wdata_o = load_data_i;
        default:     reg_wdata_o = 32'b0;
    endcase
end

always_comb begin
    reg_wen_o = rd_valid_i && (rd_addr_i != 5'd0);
end

endmodule