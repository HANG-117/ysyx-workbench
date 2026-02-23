module IDU(
    input [31:0] inst,
    output [6:0] opcode,
    output [4:0] rd,
    output [2:0] funct3,
    output [4:0] rs1,
    output [4:0] rs2,
    output [6:0] funct7,
    output [31:0] imm,

    input [31:0] reg_rdata1,
    input [31:0] reg_rdata2,
    output [4:0] reg_raddr1,
    output [4:0] reg_raddr2,
    output [4:0] reg_waddr,
    output reg_wen,

    input  [31:0] alu_result,
    output [31:0] alu_data1,
    output [31:0] alu_data2,
    output [2:0] alu_option,
    output [31:0] reg_wdata,
    
    
    output [31:0] PC_branch,
    output pc_jump
);


    assign opcode = inst[6:0];
    assign rd = inst[11:7];
    assign funct3 = inst[14:12];
    assign rs1 = inst[19:15];
    assign rs2 = inst[24:20];
    assign funct7 = inst[31:25];
    assign imm = (opcode == 7'b0010011 || opcode == 7'b1100111) ? {{20{inst[31]}}, inst[31:20]} : 
                 (opcode == 7'b0110111) ? {{inst[31:12]}, 12'b0} : 32'b0;

    assign reg_raddr1 = rs1;
    assign reg_raddr2 = rs2;
    assign reg_waddr = rd;
    

    assign alu_data1 = (opcode == 7'b0010011 || opcode == 7'b1100111 || opcode == 7'b0110011) ? reg_rdata1 : 32'b0;
    assign alu_data2 = (opcode == 7'b0010011 || opcode == 7'b1100111) ? imm : 
                       (opcode == 7'b0110011) ? reg_rdata2 : 32'b0;
    assign alu_option = (opcode == 7'b0010011) ? ((funct3 == 3'b000) ? 3'b000 : 3'b111) :
                        (opcode == 7'b1100111) ? ((funct3 == 3'b000) ? 3'b000 : 3'b111) :
                        (opcode == 7'b0110011) ? ((funct3 == 3'b000) ? 3'b000 : 3'b111) : 3'b111;

    assign pc_jump = (opcode == 7'b1100111) ? 1'b1 : 1'b0;
    assign PC_branch = (opcode == 7'b1100111) ? (alu_result & ~1) : 32'b0;
endmodule