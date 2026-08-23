#ifndef COMMON_HPP
#define COMMON_HPP

#include <cstdint>

// ==================== 物理内存 ====================
#define MEM_SIZE_WORDS (1 << 28) // 按字寻址，共 1GiB
#define MEM_BASE       0x80000000U

// ==================== MMIO 地址映射 ====================
#define SERIAL_PORT    0x10000000U  // 串口地址 (SoC 兼容)
#define RTC_ADDR_LOW   0xa0000048U  // RTC 时间低 32 位
#define RTC_ADDR_HIGH  0xa000004cU  // RTC 时间高 32 位

// ==================== 仿真上限 ====================
#define MAX_CYCLES     100000

// ==================== 追踪开关 ====================
#define ITRACE false

#define MTRACE false
#define MTRACE_START 0x80000000
#define MTRACE_END   0xFFFFFFFF

#define FTRACE false

#endif
