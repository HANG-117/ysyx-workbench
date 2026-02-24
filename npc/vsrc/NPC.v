module NPC(
    input clk,
    input rst,
    output [31:0] inst,
    
    output imem_valid,
    output [31:0] imem_addr,
    input [31:0] imem_rdata
    
    

);
    logic [31:0] PC;
    logic [31:0] PC_branch;
    logic pc_jump;

    logic [6:0] opcode;
    logic [4:0] rd;
    logic [2:0] funct3;
    logic [4:0] rs1;
    logic [4:0] rs2;
    logic [6:0] funct7;
    logic [31:0] imm;


    logic [31:0] alu_data1;
    logic [31:0] alu_data2;
    logic [2:0] alu_option;
    logic [31:0] alu_result;
    
    
    logic [4:0] reg_raddr1;
    logic [4:0] reg_raddr2;
    logic [31:0] reg_rdata1;
    logic [31:0] reg_rdata2;
    logic [31:0] reg_wdata;
    logic [4:0] reg_waddr;
    logic reg_wen;

    logic [31:0] dmem_wdata;
    logic [31:0] dmem_addr;
    logic [31:0] dmem_rdata;
    logic dmem_write;
    logic [31:0]dmem_bytes;

    logic ebreak;

    always @(posedge clk) begin
        $display("PC: 0x%08x, inst: 0x%08x", PC, inst);
    end

    RegisterFile #(.ADDR_WIDTH(5), .DATA_WIDTH(32)) gpr(
        .clk(clk),
        .raddr1(reg_raddr1),
        .raddr2(reg_raddr2),
        .rdata1(reg_rdata1),
        .rdata2(reg_rdata2),
        .wdata(reg_wdata),
        .waddr(reg_waddr),
        .wen(reg_wen)
    );


    IFU ifu(
        .PC(PC),
        .inst(inst),
        .imem_rdata(imem_rdata),
        .imem_valid(imem_valid),
        .imem_addr(imem_addr)
    );

    IDU idu(
        .inst(inst),
        .opcode(opcode),
        .rd(rd),
        .funct3(funct3),
        .rs1(rs1),
        .rs2(rs2),
        .funct7(funct7),
        .imm(imm),
        .reg_rdata1(reg_rdata1),
        .reg_rdata2(reg_rdata2),
        .reg_raddr1(reg_raddr1),
        .reg_raddr2(reg_raddr2),
        .reg_waddr(reg_waddr),
        .reg_wen(reg_wen),
        .reg_wdata(reg_wdata),
        .dmem_rdata(dmem_rdata),
        .dmem_wdata(dmem_wdata),
        .dmem_addr(dmem_addr),
        .dmem_write(dmem_write),
        .dmem_bytes(dmem_bytes),

        .alu_result(alu_result),
        .alu_data1(alu_data1),
        .alu_data2(alu_data2),
        .alu_option(alu_option),
        .ebreak(ebreak),
        .PC_branch(PC_branch),
        .pc_jump(pc_jump)
    );

    EXU exu(
        .data1(alu_data1),
        .data2(alu_data2),
        .alu_option(alu_option),
        .result(alu_result)
    );
    
    LSU lsu(
        .clk(clk),
        .dmem_write(dmem_write),
        .dmem_addr(dmem_addr),
        .dmem_wdata(dmem_wdata),
        .dmem_rdata(dmem_rdata),
        .dmem_bytes(dmem_bytes)
    );

    WBU wbu(
        .clk(clk),
        .rst(rst),
        .opcode(opcode),
        .funct3(funct3),
        .alu_result(alu_result),
        .reg_wdata(reg_wdata),
        .reg_wen(reg_wen),
        .reg_waddr(reg_waddr),
        .dmem_rdata(dmem_rdata),
        .imm(imm),
        .pc_branch(PC_branch),
        .pc_jump(pc_jump),
        .PC(PC),
        .ebreak(ebreak)
    );

endmodule