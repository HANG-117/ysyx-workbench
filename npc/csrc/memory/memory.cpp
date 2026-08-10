#include "memory/memory.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sys/time.h>

#include "trace/trace.hpp"

// DPI-C 全局入口：RTL 通过 import "DPI-C" 调用
static Memory* g_mem = nullptr;

extern "C" int pmem_read(int raddr) {
    if (g_mem) {
        return g_mem->read((uint32_t)raddr);
    }
    return 0;
}

extern "C" void pmem_write(int waddr, int wdata, int wmask) {
    if (g_mem) {
        g_mem->write((uint32_t)waddr, (uint32_t)wdata, (uint32_t)wmask);
    }
}

// 物理内存本体（BSS，按字寻址，从 MEM_BASE 开始）
uint32_t Memory::mem[MEM_SIZE_WORDS];

Memory::Memory() : boot_time_us_(now_us()) {
    if (g_mem == nullptr) {
        g_mem = this;
    }
}

uint64_t Memory::now_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

void Memory::load_image(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot open " << filename << std::endl;
        exit(1);
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buffer = new char[size];
    file.read(buffer, size);
    file.close();

    uint32_t* words = (uint32_t*)buffer;
    size_t word_count = size / 4;
    for (size_t i = 0; i < word_count; i++) {
        mem[i] = words[i];
    }

    delete[] buffer;
    std::cout << "Loaded " << word_count << " instructions from " << filename << std::endl;
}

uint32_t Memory::read(uint32_t addr) {
    // 1. MMIO：实时时钟 RTC
    if (addr == RTC_ADDR_LOW) {
        uint64_t now = now_us() - boot_time_us_;
        return (uint32_t)(now & 0xFFFFFFFF);  // 低 32 位（单位：微秒）
    }
    if (addr == RTC_ADDR_HIGH) {
        uint64_t now = now_us() - boot_time_us_;
        return (uint32_t)(now >> 32);         // 高 32 位
    }

    // 2. 物理内存读取
    if (addr < MEM_BASE || addr >= (MEM_BASE + (MEM_SIZE_WORDS * 4))) {
        return 0;  // 越界/未映射区域返回 0，防止组合逻辑 Glitch 导致崩溃
    }

    return mem[(addr - MEM_BASE) >> 2];
}

void Memory::write(uint32_t addr, uint32_t data, uint32_t wmask) {
    // 访存追踪（在 MMIO 分发之前记录）
    mtrace_write(addr, data);

    // 1. MMIO：串口输出
    if (addr == SERIAL_PORT) {
        putchar((char)(data & 0xFF));
        return;
    }

    // 2. 物理内存写入（忽略非法地址，防止崩溃）
    if (addr < MEM_BASE || addr >= (MEM_BASE + (MEM_SIZE_WORDS * 4))) {
        return;
    }

    uint32_t mem_idx = (addr - MEM_BASE) >> 2;

    if (wmask == 0b0001) {
        mem[mem_idx] = (mem[mem_idx] & 0xFFFFFF00) | (data & 0x000000FF);
    } else if (wmask == 0b0010) {
        mem[mem_idx] = (mem[mem_idx] & 0xFFFF00FF) | (data & 0x0000FF00);
    } else if (wmask == 0b0100) {
        mem[mem_idx] = (mem[mem_idx] & 0xFF00FFFF) | (data & 0x00FF0000);
    } else if (wmask == 0b1000) {
        mem[mem_idx] = (mem[mem_idx] & 0x00FFFFFF) | (data & 0xFF000000);
    } else if (wmask == 0b0011) {
        mem[mem_idx] = (mem[mem_idx] & 0xFFFF0000) | (data & 0x0000FFFF);
    } else if (wmask == 0b1100) {
        mem[mem_idx] = (mem[mem_idx] & 0x0000FFFF) | (data & 0xFFFF0000);
    } else if (wmask == 0b1111) {
        mem[mem_idx] = data;
    } else {
        // 其它掩码（含偏移情况），按位写入
        uint32_t mask_val = 0;
        if (wmask & 0x1) mask_val |= 0x000000FF;
        if (wmask & 0x2) mask_val |= 0x0000FF00;
        if (wmask & 0x4) mask_val |= 0x00FF0000;
        if (wmask & 0x8) mask_val |= 0xFF000000;
        mem[mem_idx] = (mem[mem_idx] & ~mask_val) | (data & mask_val);
    }
}
