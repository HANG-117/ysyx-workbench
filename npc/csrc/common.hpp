#ifndef COMMON_HPP
#define COMMON_HPP
#include <string>
#include <sstream>
#include <cstdint>
std::string disassemble_rv32e(uint32_t instr, uint32_t pc);

#endif