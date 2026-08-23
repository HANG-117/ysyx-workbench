// ALU 计算码
`define ALU_ADD  4'b0000
`define ALU_SLL  4'b0001
`define ALU_SLT  4'b0010
`define ALU_SLTU 4'b0011
`define ALU_XOR  4'b0100
`define ALU_SRL  4'b0101
`define ALU_OR   4'b0110
`define ALU_AND  4'b0111
`define ALU_SUB  4'b1000 
`define ALU_SRA  4'b1101
`define ALU_LUI  4'b1110 

// MEM 读写码
`define MEM_SIZE_BYTE 2'b00
`define MEM_SIZE_HALF 2'b01
`define MEM_SIZE_WORD 2'b10

// ECALL / EBREAK (完全匹配)
`define INST_ECALL_MASK   32'hFFFFFFFF
`define INST_ECALL        32'h00000073
`define INST_EBREAK_MASK  32'hFFFFFFFF
`define INST_EBREAK       32'h00100073

// FENCE (忽略 pred/succ/fm 等位，仅保留 opcode+funct3+rs1+rd)
`define INST_FENCE_MASK   32'h0000707F
`define INST_FENCE        32'h0000000F   // opcode=0001111, funct3=000, rs1=0, rd=0

// 分支条件编码 (供 ALU 内部基于减法结果作跳转判定)
`define BC_BEQ   3'b000
`define BC_BNE   3'b001
`define BC_BLT   3'b100
`define BC_BGE   3'b101
`define BC_BLTU  3'b110
`define BC_BGEU  3'b111

// 写回选择码
`define WB_SEL_ALU 3'b000
`define WB_SEL_PC4 3'b001
`define WB_SEL_MEM 3'b010
`define WB_SEL_CSR 3'b011

// CSR 指令类型
`define CSR_NONE   4'b0000
`define CSR_CSRRW  4'b0001
`define CSR_CSRRS  4'b0010
`define CSR_CSRRC  4'b0011
`define CSR_CSRRWI 4'b0101
`define CSR_CSRRSI 4'b0110
`define CSR_CSRRCI 4'b0111
`define CSR_ECALL  4'b1000
`define CSR_EBREAK 4'b1001
`define CSR_MRET   4'b1010

