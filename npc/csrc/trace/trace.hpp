#ifndef TRACE_HPP
#define TRACE_HPP

#include <cstdint>
#include <string>

#include "common.hpp"

// ==================== itrace：指令反汇编 ====================
std::string disassemble_rv32e(uint32_t instr, uint32_t pc);

// ==================== mtrace：访存追踪 ====================
void mtrace_write(uint32_t addr, uint32_t data);
void mtrace_read(uint32_t addr, uint32_t data);

// ==================== ftrace：函数调用追踪 ====================
typedef struct {
    uint32_t addr;
    uint32_t size;
    const char* name;
} ftrace_sym_t;

void ftrace_init(std::string elf_file);
std::string ftrace_lookup(uint32_t addr);
void ftrace_call(uint32_t call_addr, uint32_t target_addr);
void ftrace_return(uint32_t pc, uint32_t ret_target);
void ftrace_check(uint32_t pc, uint32_t inst, uint32_t ret_addr);

#endif
