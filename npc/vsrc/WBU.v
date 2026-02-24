module WBU(
    input clk,
    input rst,
    input [6:0] opcode,
    input [2:0] funct3,
    input [31:0] alu_result,
    
    
    output [31:0] reg_wdata,
    output reg_wen,
    input [31:0] imm,
    input [4:0] reg_waddr,

    input [31:0] dmem_rdata,

    input [31:0] pc_branch,
    input pc_jump,
    output reg [31:0] PC,
    input ebreak
);  
    import "DPI-C" function void sim_exit();
    logic [31:0] pc_plus_4;
    assign pc_plus_4 = PC + 4;
    logic [31:0]PC_cur;
   

    assign reg_wen = (opcode == 7'b0010011 || opcode == 7'b1100111 || opcode == 7'b0110011 || opcode == 7'b0110111 || opcode == 7'b0000011) ? 1'b1 : 1'b0;
    assign reg_wdata = (opcode == 7'b0010011 || opcode == 7'b0110011) ? alu_result : 
                       (opcode == 7'b1100111) ? PC + 4: 
                       (opcode == 7'b0110111) ? imm :
                       (opcode == 7'b0000011) ? ((funct3 == 3'b010) ? dmem_rdata : 
                                                 (funct3 == 3'b100) ? {24'b0, dmem_rdata[7:0]} : 32'b0): 32'b0;

     always @(posedge clk) begin
        if(ebreak) begin
            $strobe("end simulation");
            sim_exit();
        end
        else if (rst) begin
            PC <= 32'b0;
        end else if (pc_jump) begin
            PC_cur = PC;
            $display("cur PC: 0x%08x, Jump to: 0x%08x", PC_cur, pc_branch);
            PC <= pc_branch;
        end else begin
            PC <= pc_plus_4;
        end
    end
endmodule