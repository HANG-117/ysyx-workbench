#ifndef FTRACE_H
#define FTRACE_H
#include <stdint.h>

typedef struct {
    uint32_t addr;
    uint32_t size;
    const char *name;
} ftrace_sym_t;

void ftrace_init(const char *elf_path);
const char *ftrace_lookup(uint64_t vaddr);
void ftrace_call(uint32_t call_addr,uint32_t target_addr);
void ftrace_ret(uint32_t pc, uint32_t ret_target);
void ftrace_dump();


#endif