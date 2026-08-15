#include "monitor/monitor.hpp"

#include <iostream>

Monitor::Monitor(Simulator& sim, Memory& mem) : sim_(sim), mem_(mem) {}

int Monitor::run() {
    while (true) {
        char choice;
        std::cin >> choice;
        switch (choice) {
            case 's': cmd_step(); break;
            case 'm': cmd_read_mem(); break;
            case 'c': cmd_continue(); break;
            case 'p': cmd_dump_regs(); break;
            case 'q': return 0;
            default:
                std::cout << "Unknown command. Use 's' to step, 'q' to quit." << std::endl;
        }
        if (sim_.cycle() >= MAX_CYCLES) break;
        if (sim_.exited()) break;
    }
    return 0;
}

void Monitor::cmd_step() {
    std::cout << "请输入需要前进的周期数" << std::endl;
    int num;
    std::cin >> num;
    for (int i = 0; i < num; i++) {
        sim_.step();
        if (sim_.exited()) break;
    }
}

void Monitor::cmd_read_mem() {
    std::cout << "Enter address to read: ";
    uint32_t addr;
    std::cin >> std::hex >> addr;
    for (int i = -4; i < 5; i++) {
        std::cout << "0x" << std::hex << (addr + i * 4) << ": 0x"
                  << std::hex << mem_.read(addr + i * 4) << std::endl;
    }
}

void Monitor::cmd_continue() {
    while (true) {
        sim_.step();
        if (sim_.exited()) break;
    }
}

void Monitor::cmd_dump_regs() {
    std::cout << "=======当前寄存器信息=======" << std::endl;
    for (int i = 0; i < 32; i++) {
        std::cout << "x" << std::dec << i << ": 0x"
                  << std::hex << sim_.reg(i);
        if (i % 4 == 3) std::cout << std::endl;
        else std::cout << "\t";
    }
}
