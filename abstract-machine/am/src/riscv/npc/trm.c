#include <am.h>
#include <klib-macros.h>
#define SERIAL_PORT 0x10000000
extern char _heap_start;
int main(const char *args);

extern char _pmem_start;
#define PMEM_SIZE (128 * 1024 * 1024)
#define PMEM_END  ((uintptr_t)&_pmem_start + PMEM_SIZE)

Area heap = RANGE(&_heap_start, PMEM_END);
static const char mainargs[MAINARGS_MAX_LEN] = TOSTRING(MAINARGS_PLACEHOLDER); // defined in CFLAGS

void putch(char ch) {
  *(volatile char *)(SERIAL_PORT) = ch;
}

void halt(int code) {
   __asm__ volatile (
        ".long 0x00100073\n\t"  // ebreak
        "j .\n\t"               // 原地跳转，防止 ebreak 被跳过
    );

    // 2. 防御性死循环
    while (1);

    // 3. 【关键修复】告诉编译器这里永远不可达
    // 这消除了 "function does return" 的警告/错误
    __builtin_unreachable();
}

void _trm_init() {
  int ret = main(mainargs);
  halt(ret);
}
