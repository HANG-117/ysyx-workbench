#ifndef COMMON_HPP
#define COMMON_HPP
#include <string>
#include <sstream>
#include <cstdint>
#include <iostream>

#define MEM_SIZE_WORDS (1 << 28) // 256MB 内存
#define MEM_BASE       0x80000000U 




// MMIO 地址映射
#define SERIAL_PORT    0x10000000U  // 串口地址 (SoC 兼容)
#define RTC_ADDR_LOW   0xa0000048U  // RTC 时间低 32 位
#define RTC_ADDR_HIGH  0xa000004cU  // RTC 时间高 32 位


#define MAX_CYCLES     100000

#define ITRACE false

#define MTRACE true
#define MTRACE_START 0x80000000
#define MTRACE_END   0xFFFFFFFF

#define FTRACE true



std::string disassemble_rv32e(uint32_t instr, uint32_t pc);
void mtrace_write(uint32_t addr, uint32_t data);
void mtrace_read(uint32_t addr, uint32_t data);

typedef struct {
    uint32_t addr;
    uint32_t size;
    const char* name;
}ftrace_sym_t;

void ftrace_init(std::string elf_file);
std::string ftrace_lookup(uint32_t addr);
void ftrace_call(uint32_t call_addr,uint32_t target_addr);
void ftrace_return(uint32_t pc,uint32_t ret_target);
void ftrace_check(uint32_t pc,uint32_t inst,uint32_t ret_addr);
#endif