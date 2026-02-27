module NPC(
    input clk,
    input rst,
    output [31:0] inst,
    
    output imem_valid,
    output [31:0] imem_addr,
    input [31:0] imem_rdata,
    output logic [31:0] PC
    
    

);
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
    initial begin
        PC = 32'h80000000;
    end

    RegisterFile #(.ADDR_WIDTH(5), .DATA_WIDTH(32)) gpr(
        .clk(clk),
        .rst(rst),
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
        .dmem_addr(dmem_addr),
        .dmem_rdata(dmem_rdata),
        .imm(imm),
        .pc_branch(PC_branch),
        .pc_jump(pc_jump),
        .PC(PC),
        .ebreak(ebreak)
    );
    always @(posedge clk) begin
        if (PC == 32'h198) begin
            $display("=== Register Dump at PC = 0x%08x ===", PC);
            $display("x00=0x%08x  x01=0x%08x  x02=0x%08x  x03=0x%08x", 
                    32'h0, gpr.rf[1], gpr.rf[2], gpr.rf[3]);
            $display("x04=0x%08x  x05=0x%08x  x06=0x%08x  x07=0x%08x", 
                    gpr.rf[4], gpr.rf[5], gpr.rf[6], gpr.rf[7]);
            $display("x08=0x%08x  x09=0x%08x  x10=0x%08x  x11=0x%08x", 
                    gpr.rf[8], gpr.rf[9], gpr.rf[10], gpr.rf[11]);
            $display("x12=0x%08x  x13=0x%08x  x14=0x%08x  x15=0x%08x", 
                    gpr.rf[12], gpr.rf[13], gpr.rf[14], gpr.rf[15]);
            $display("=====================================");
            end
        if(PC == 32'h198)begin
            $display("=== Test Passed ===");
            $display("reg_wen : %d, reg_waddr : %08x, reg_wdata : %08x", reg_wen, reg_waddr, reg_wdata);
            $display("rs1: %08x, rs2: %08x, rd: %08x, funct3: %08x, funct7: %08x, opcode: %08x", rs1, rs2, rd, funct3, funct7, opcode);
            $display("reg_rdata1 : %08x, reg_rdata2 : %08x", reg_rdata1, reg_rdata2);
            $display("alu_data1 : %08x, alu_data2 : %08x, alu_result : %08x", alu_data1, alu_data2, alu_result);
            $display("dmem_write : %08x, dmem_addr : %08x, dmem_wdata : %d, dmem_bytes %d, dmem_rdata : %08x", dmem_write, dmem_addr, dmem_wdata,dmem_bytes,dmem_rdata);
            $display("imm : %0x", imm);
            $display("a0 : %08x", gpr.rf[10]);
        end
    end
endmodule