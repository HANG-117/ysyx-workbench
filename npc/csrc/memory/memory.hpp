#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cstddef>
#include <cstdint>

#include "common.hpp"
// 物理内存 + MMIO 模型（串口 / RTC）。
// RTL 侧通过 DPI-C（pmem_read / pmem_write）访问本模块。
class Memory {
public:
    Memory(); 

    void load_image(const char* filename);

    uint32_t read(uint32_t addr);

    void write(uint32_t addr, uint32_t data, uint32_t wmask);

    uint8_t* host_addr(uint32_t guest_addr);

    // 已加载镜像的大小（字节）
    size_t image_size() const { return image_size_; }

private:
    static uint64_t now_us();  

    uint64_t boot_time_us_;                
    size_t image_size_ = 0;                
    static uint32_t mem[MEM_SIZE_WORDS];  
};

#endif
