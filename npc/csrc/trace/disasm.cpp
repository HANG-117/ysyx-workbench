#include <sstream>
#include <string>

#include "trace/trace.hpp"

std::string disassemble_rv32e(uint32_t instr, uint32_t pc) {
    // 提取字段
    uint32_t opcode = instr & 0x7F;
    uint32_t rd     = (instr >> 7) & 0x1F;
    uint32_t funct3 = (instr >> 12) & 0x7;
    uint32_t rs1    = (instr >> 15) & 0x1F;
    uint32_t rs2    = (instr >> 20) & 0x1F;
    uint32_t funct7 = (instr >> 25) & 0x7F;

    // 辅助：寄存器名称（RV32E 只用 x0~x15，但这里保持通用）
    auto reg_name = [](uint32_t r) -> std::string {
        return "x" + std::to_string(r);
    };

    // 立即数提取辅助（符号扩展）
    auto sext = [](uint32_t val, int bits) -> int32_t {
        bool sign = (val >> (bits - 1)) & 1;
        if (sign) {
            return (int32_t)(val | (~((1U << bits) - 1)));
        } else {
            return (int32_t)(val & ((1U << bits) - 1));
        }
    };

    std::ostringstream oss;

    // 根据 opcode 分发
    switch (opcode) {
        // ---------- R-type ----------
        case 0b0110011: { // OP
            if (funct7 == 0x00) {
                switch (funct3) {
                    case 0b000: oss << "add  "; break;   // add
                    case 0b001: oss << "sll  "; break;   // sll
                    case 0b010: oss << "slt  "; break;   // slt
                    case 0b011: oss << "sltu "; break;   // sltu
                    case 0b100: oss << "xor  "; break;   // xor
                    case 0b101: oss << "srl  "; break;   // srl
                    case 0b110: oss << "or   "; break;   // or
                    case 0b111: oss << "and  "; break;   // and
                }
            } else if (funct7 == 0x20) {
                switch (funct3) {
                    case 0b000: oss << "sub  "; break;   // sub
                    case 0b101: oss << "sra  "; break;   // sra
                    default: goto unknown;
                }
            } else {
                goto unknown;
            }
            oss << reg_name(rd) << ", " << reg_name(rs1) << ", " << reg_name(rs2);
            break;
        }

        // ---------- I-type ----------
        case 0b0010011: { // OP-IMM
            int32_t imm = sext((instr >> 20), 12);
            switch (funct3) {
                case 0b000: oss << "addi "; oss << reg_name(rd) << ", " << reg_name(rs1) << ", " << imm; break;
                case 0b001: { // slli (shamt 仅低 5 位)
                    uint32_t shamt = (instr >> 20) & 0x1F;
                    oss << "slli "; oss << reg_name(rd) << ", " << reg_name(rs1) << ", " << shamt;
                    break;
                }
                case 0b010: oss << "slti "; oss << reg_name(rd) << ", " << reg_name(rs1) << ", " << imm; break;
                case 0b011: oss << "sltiu"; oss << reg_name(rd) << ", " << reg_name(rs1) << ", " << imm; break;
                case 0b100: oss << "xori "; oss << reg_name(rd) << ", " << reg_name(rs1) << ", " << imm; break;
                case 0b101: { // srli / srai
                    uint32_t shamt = (instr >> 20) & 0x1F;
                    if ((instr >> 25) & 0x40) { // funct7[6] = 1 => srai
                        oss << "srai ";
                    } else {
                        oss << "srli ";
                    }
                    oss << reg_name(rd) << ", " << reg_name(rs1) << ", " << shamt;
                    break;
                }
                case 0b110: oss << "ori  "; oss << reg_name(rd) << ", " << reg_name(rs1) << ", " << imm; break;
                case 0b111: oss << "andi "; oss << reg_name(rd) << ", " << reg_name(rs1) << ", " << imm; break;
                default: goto unknown;
            }
            break;
        }

        // ---------- LUI / AUIPC (U-type) ----------
        case 0b0110111: { // LUI
            int32_t imm = (instr & 0xFFFFF000); // 高 20 位，已左移 12 位，直接作为有符号数
            oss << "lui  " << reg_name(rd) << ", 0x" << std::hex << (imm >> 12) << std::dec;
            break;
        }
        case 0b0010111: { // AUIPC
            int32_t imm = (instr & 0xFFFFF000);
            oss << "auipc " << reg_name(rd) << ", 0x" << std::hex << (imm >> 12) << std::dec;
            break;
        }

        // ---------- JAL (J-type) ----------
        case 0b1101111: { // JAL
            int32_t imm = 0;
            imm |= (instr & 0x000FF000) >> 20;      // bits 31~12: imm[19:12]
            imm |= (instr & 0x00100000) >> 9;       // bit 20 -> imm[11]
            imm |= (instr & 0x7FE00000) >> 20;      // bits 30~21 -> imm[10:1]
            imm |= (instr & 0x80000000) >> 11;      // bit 31 -> imm[20]
            imm = sext(imm, 21);                    // 符号扩展
            int32_t target = pc + imm;
            oss << "jal  " << reg_name(rd) << ", 0x" << std::hex << target << std::dec;
            break;
        }

        // ---------- JALR (I-type) ----------
        case 0b1100111: { // JALR
            int32_t imm = sext((instr >> 20), 12);
            oss << "jalr " << reg_name(rd) << ", " << imm << "(" << reg_name(rs1) << ")";
            break;
        }

        // ---------- Branches (B-type) ----------
        case 0b1100011: { // BRANCH
            int32_t imm = 0;
            imm |= (instr & 0x00000F00) >> 7;       // bits 11~8 -> imm[4:1]
            imm |= (instr & 0x7E000000) >> 20;      // bits 30~25 -> imm[10:5]
            imm |= (instr & 0x00000080) << 4;       // bit 7 -> imm[11]
            imm |= (instr & 0x80000000) >> 19;      // bit 31 -> imm[12]
            imm = sext(imm, 13);
            int32_t target = pc + imm;
            const char* mnemonic;
            switch (funct3) {
                case 0b000: mnemonic = "beq  "; break;
                case 0b001: mnemonic = "bne  "; break;
                case 0b100: mnemonic = "blt  "; break;
                case 0b101: mnemonic = "bge  "; break;
                case 0b110: mnemonic = "bltu "; break;
                case 0b111: mnemonic = "bgeu "; break;
                default: goto unknown;
            }
            oss << mnemonic << reg_name(rs1) << ", " << reg_name(rs2) << ", 0x" << std::hex << target << std::dec;
            break;
        }

        // ---------- Loads (I-type) ----------
        case 0b0000011: { // LOAD
            int32_t imm = sext((instr >> 20), 12);
            const char* mnemonic;
            switch (funct3) {
                case 0b000: mnemonic = "lb   "; break;
                case 0b001: mnemonic = "lh   "; break;
                case 0b010: mnemonic = "lw   "; break;
                case 0b100: mnemonic = "lbu  "; break;
                case 0b101: mnemonic = "lhu  "; break;
                default: goto unknown;
            }
            oss << mnemonic << reg_name(rd) << ", " << imm << "(" << reg_name(rs1) << ")";
            break;
        }

        // ---------- Stores (S-type) ----------
        case 0b0100011: { // STORE
            int32_t imm = 0;
            imm |= (instr & 0x00000F00) >> 7;       // bits 11~8 -> imm[4:0]
            imm |= (instr & 0x7E000000) >> 20;      // bits 30~25 -> imm[10:5]
            imm = sext(imm, 12);
            const char* mnemonic;
            switch (funct3) {
                case 0b000: mnemonic = "sb   "; break;
                case 0b001: mnemonic = "sh   "; break;
                case 0b010: mnemonic = "sw   "; break;
                default: goto unknown;
            }
            oss << mnemonic << reg_name(rs2) << ", " << imm << "(" << reg_name(rs1) << ")";
            break;
        }

        // ---------- 环境调用 / 断点 (ECALL/EBREAK) ----------
        case 0b1110011: { // SYSTEM (仅支持 ECALL, EBREAK)
            if (instr == 0x00000073) {
                oss << "ecall";
            } else if (instr == 0x00100073) {
                oss << "ebreak";
            } else {
                goto unknown;
            }
            break;
        }

        default:
            goto unknown;
    }

    return oss.str();

unknown:
    // 无法识别的指令，输出原始十六进制
    std::ostringstream unknown_oss;
    unknown_oss << ".word 0x" << std::hex << instr;
    return unknown_oss.str();
}