#include "trace/trace.hpp"

#include <iostream>

void mtrace_write(uint32_t addr, uint32_t data) {
    if (MTRACE && addr >= MTRACE_START && addr <= MTRACE_END) {
        std::cout << "Memory Write Trace - Address: 0x" << std::hex << addr
                  << ", Data: 0x" << std::hex << data << std::endl;
    }
}

void mtrace_read(uint32_t addr, uint32_t data) {
    if (MTRACE && addr >= MTRACE_START && addr <= MTRACE_END) {
        std::cout << "Memory Read Trace - Address: 0x" << std::hex << addr
                  << ", Data: 0x" << std::hex << data << std::endl;
    }
}
