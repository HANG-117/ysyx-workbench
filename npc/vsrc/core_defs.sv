// RV32I 掩码
`define INST_ANDI       32'h7013
`define INST_ANDI_MASK  32'h707f
`define INST_ADDI       32'h13
`define INST_ADDI_MASK  32'h707f
`define INST_SLTI       32'h2013
`define INST_SLTI_MASK  32'h707f
`define INST_SLTIU      32'h3013
`define INST_SLTIU_MASK 32'h707f
`define INST_ORI        32'h6013
`define INST_ORI_MASK   32'h707f
`define INST_XORI       32'h4013
`define INST_XORI_MASK  32'h707f
`define INST_SLLI       32'h1013
`define INST_SLLI_MASK  32'hfc00707f
`define INST_SRLI       32'h5013
`define INST_SRLI_MASK  32'hfc00707f
`define INST_SRAI       32'h40005013
`define INST_SRAI_MASK  32'hfc00707f
`define INST_LUI        32'h37
`define INST_LUI_MASK   32'h7f
`define INST_AUIPC      32'h17
`define INST_AUIPC_MASK 32'h7f
`define INST_ADD        32'h33
`define INST_ADD_MASK   32'hfe00707f
`define INST_SUB        32'h40000033
`define INST_SUB_MASK   32'hfe00707f
`define INST_SLT        32'h2033
`define INST_SLT_MASK   32'hfe00707f
`define INST_SLTU       32'h3033
`define INST_SLTU_MASK  32'hfe00707f
`define INST_XOR        32'h4033
`define INST_XOR_MASK   32'hfe00707f
`define INST_OR         32'h6033
`define INST_OR_MASK    32'hfe00707f
`define INST_AND        32'h7033
`define INST_AND_MASK   32'hfe00707f
`define INST_SLL        32'h1033
`define INST_SLL_MASK   32'hfe00707f
`define INST_SRL        32'h5033
`define INST_SRL_MASK   32'hfe00707f
`define INST_SRA        32'h40005033
`define INST_SRA_MASK   32'hfe00707f
`define INST_JAL        32'h6f
`define INST_JAL_MASK   32'h7f
`define INST_JALR       32'h67
`define INST_JALR_MASK  32'h707f
`define INST_BEQ        32'h63
`define INST_BEQ_MASK   32'h707f
`define INST_BNE        32'h1063
`define INST_BNE_MASK   32'h707f
`define INST_BLT        32'h4063
`define INST_BLT_MASK   32'h707f
`define INST_BGE        32'h5063
`define INST_BGE_MASK   32'h707f
`define INST_BLTU       32'h6063
`define INST_BLTU_MASK  32'h707f
`define INST_BGEU       32'h7063
`define INST_BGEU_MASK  32'h707f
`define INST_LB         32'h3
`define INST_LB_MASK    32'h707f


`define INST_LH         32'h1003
`define INST_LH_MASK    32'h707f
`define INST_LW         32'h2003
`define INST_LW_MASK    32'h707f
`define INST_LBU        32'h4003
`define INST_LBU_MASK   32'h707f
`define INST_LHU        32'h5003
`define INST_LHU_MASK   32'h707f
`define INST_SB         32'h23
`define INST_SB_MASK    32'h707f
`define INST_SH         32'h1023
`define INST_SH_MASK    32'h707f
`define INST_SW         32'h2023
`define INST_SW_MASK    32'h707f


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

