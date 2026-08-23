#include "ftrace.h"
#include <common.h>            // NEMU 公共头文件，提供 Log, Assert, MUXDEF 等
#include <isa.h>               // 提供 RESET_VECTOR
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <memory/paddr.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>


/* ---------- 内部静态数据 ---------- */
static ftrace_sym_t *ftrace_syms = NULL;
static int ftrace_sym_cnt = 0;
static int call_depth = 0;           // 调用深度，用于缩进
static uint64_t ftrace_base;         // 加载基址（客户程序在 NEMU 内存中的起始地址）
static bool ftrace_enabled = false;
/* 日志输出目标，复用 NEMU 的日志文件指针 */
extern FILE *log_fp;

/* ---------- 比较函数，按地址升序排序 ---------- */
static int sym_cmp(const void *a, const void *b) {
    const ftrace_sym_t *sa = a, *sb = b;
    if (sa->addr < sb->addr) return -1;
    if (sa->addr > sb->addr) return 1;
    return 0;
}

/* ---------- 初始化：解析 ELF 文件，建立函数符号表 ---------- */
void init_ftrace(const char *elf_path) {
    Assert(elf_path != NULL, "elf_path is NULL");

    int fd = open(elf_path, O_RDONLY);
    Assert(fd != -1, "Cannot open '%s'", elf_path);
    struct stat st;
    fstat(fd, &st);
    void *map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    Assert(map != MAP_FAILED, "mmap failed for '%s'", elf_path);

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map;
    Assert(memcmp(ehdr->e_ident, ELFMAG, 4) == 0, "Not an ELF file: %s", elf_path);
    Assert(ehdr->e_machine == EM_RISCV, "Not a RISC-V ELF: %s", elf_path);

    Elf32_Shdr *shdrs = (Elf32_Shdr *)(map + ehdr->e_shoff);
    const char *shstrtab = (char *)(map + shdrs[ehdr->e_shstrndx].sh_offset);

    // 找 .symtab 和 .strtab（或 .dynsym / .dynstr）
    Elf32_Shdr *sym_hdr = NULL, *str_hdr = NULL;
    for (int i = 0; i < ehdr->e_shnum; i++) {
        const char *name = shstrtab + shdrs[i].sh_name;
        if (strcmp(name, ".symtab") == 0) sym_hdr = &shdrs[i];
        if (strcmp(name, ".strtab") == 0) str_hdr = &shdrs[i];
    }
    if (!sym_hdr || !str_hdr) {
        for (int i = 0; i < ehdr->e_shnum; i++) {
            const char *name = shstrtab + shdrs[i].sh_name;
            if (strcmp(name, ".dynsym") == 0) sym_hdr = &shdrs[i];
            if (strcmp(name, ".dynstr") == 0) str_hdr = &shdrs[i];
        }
    }
    Assert(sym_hdr && str_hdr, "No symbol table found in '%s'", elf_path);

    // 符号表
    Elf32_Sym *symtab = (Elf32_Sym *)(map + sym_hdr->sh_offset);
    const char *strtab = (char *)(map + str_hdr->sh_offset);
    int total = sym_hdr->sh_size / sizeof(Elf32_Sym);  // 条目数

    // 第一遍：统计 FUNC 数量
    int cnt = 0;
    for (int i = 0; i < total; i++) {
        if (ELF32_ST_TYPE(symtab[i].st_info) == STT_FUNC && symtab[i].st_value != 0) {
            cnt++;
        }
    }

    // 分配数组并填充
    ftrace_syms = malloc(cnt * sizeof(ftrace_sym_t));
    Assert(ftrace_syms, "malloc failed");
    ftrace_sym_cnt = 0;
    for (int i = 0; i < total; i++) {
        if (ELF32_ST_TYPE(symtab[i].st_info) == STT_FUNC && symtab[i].st_value != 0) {
            ftrace_syms[ftrace_sym_cnt].addr = symtab[i].st_value;
            ftrace_syms[ftrace_sym_cnt].size = symtab[i].st_size;
            ftrace_syms[ftrace_sym_cnt].name = strdup(strtab + symtab[i].st_name);
            ftrace_sym_cnt++;
        }
    }

    qsort(ftrace_syms, ftrace_sym_cnt, sizeof(ftrace_sym_t), sym_cmp);

    ftrace_base = 0; 
    ftrace_enabled = true;

    Log("ftrace: loaded %d function symbols from '%s', base=0x%lx",
        ftrace_sym_cnt, elf_path, ftrace_base);

    munmap(map, st.st_size);  
}

/* ---------- 地址查找：返回函数名或 NULL ---------- */
const char *ftrace_lookup(uint64_t vaddr) {
    if (!ftrace_enabled) return NULL;
    if (ftrace_sym_cnt == 0) return NULL;

    uint64_t off = vaddr - ftrace_base;

    int l = 0, r = ftrace_sym_cnt - 1;
    int idx = -1;
    while (l <= r) {
        int mid = (l + r) / 2;
        if (ftrace_syms[mid].addr <= off) {
            idx = mid;
            l = mid + 1;
        } else {
            r = mid - 1;
        }
    }
    if (idx == -1) return NULL;

    /* 检查 off 是否在该函数范围内 */
    if (ftrace_syms[idx].size > 0) {
        if (off >= ftrace_syms[idx].addr + ftrace_syms[idx].size)
            return NULL;
    } else {
        /* 无 size 信息时，只要 off 小于下一个函数起始地址即可 */
        if (idx + 1 < ftrace_sym_cnt && off >= ftrace_syms[idx + 1].addr)
            return NULL;
    }
    return ftrace_syms[idx].name;
}

/* ---------- 辅助输出函数：写入日志文件（或 stdout） ---------- */
static void ftrace_output(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    if (log_fp) {
        vfprintf(log_fp, fmt, ap);
        fflush(log_fp);
    } else {
        vprintf(fmt, ap);
    }
    va_end(ap);
}

void ftrace_call(uint32_t caller_addr, uint32_t target_addr) {
    if (!ftrace_enabled) return;
    const char *target = ftrace_lookup(target_addr);
    if (!target) target = "???";

    /* 输出缩进 */
    for (int i = 0; i < call_depth; i++) ftrace_output("  ");
    ftrace_output("0x%08x: call [%s@0x%08x]\n",
                  (uint32_t)caller_addr, target, (uint32_t)target_addr);
    call_depth++;
}

void ftrace_ret(uint32_t pc, uint32_t ret_target) {
    if (!ftrace_enabled) return;
    if (call_depth > 0) call_depth--;

    const char *func = ftrace_lookup(ret_target);
    if (!func) func = "???";

    /* 输出缩进 */
    for (int i = 0; i < call_depth; i++) ftrace_output("  ");
    ftrace_output("0x%08x: ret  [%s]\n", (uint32_t)pc, func);
}

/* ---------- 预留接口：输出统计信息（目前为空） ---------- */
void ftrace_dump() {
    // 将来可输出调用次数统计等
}