#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cstdint>

#include "common.hpp"

// 物理内存 + MMIO 模型（串口 / RTC）。
// RTL 侧通过 DPI-C（pmem_read / pmem_write）访问本模块。
class Memory {
public:
    Memory();  // 记录 RTC 基准时间

    // 将二进制镜像加载到内存基址处
    void load_image(const char* filename);

    // 读一个 32 位字：先查 MMIO，再查 RAM；未映射地址返回 0
    uint32_t read(uint32_t addr);

    // 按字节使能掩码写：MMIO 串口输出 / RAM 写入
    void write(uint32_t addr, uint32_t data, uint32_t wmask);

private:
    static uint64_t now_us();  // 自系统启动以来的微秒数

    uint64_t boot_time_us_;                 // RTC 基准时间
    static uint32_t mem[MEM_SIZE_WORDS];    // 物理内存（1GiB BSS）
};

#endif
