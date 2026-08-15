module pc_reg(
    input  clk,
    input  rst,
    input  [31:0] next_pc_i,
    output logic [31:0] pc_o
);
    logic [31:0] pc;
    always @(posedge clk or posedge  rst) begin
        if (rst) begin
            pc <= 32'h80000000;
        end else begin
            pc <= next_pc_i;
        end
    end

    assign pc_o = pc;
endmodule