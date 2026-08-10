#include <iostream>
#include <string>

#include "common.hpp"
#include "difftest/difftest.hpp"
#include "memory/memory.hpp"
#include "monitor/monitor.hpp"
#include "sim/sim.hpp"
#include "trace/trace.hpp"

int main(int argc, char** argv) {
    // ---- 解析命令行：镜像文件（可选），ftrace 需要同名 .elf ----
    const char* img_file = "test/mem.bin";
    std::string elf_file;  // ftrace 符号文件
    if (argc >= 2) {
        img_file = argv[1];
        if (FTRACE) {
            std::string img_str = argv[1];
            size_t dot_pos = img_str.find_last_of('.');
            if (dot_pos != std::string::npos) {
                elf_file = img_str.substr(0, dot_pos) + ".elf";
            } else {
                elf_file = img_str + ".elf";
            }
        }
    }

    // ---- 初始化：内存 -> 仿真器 -> difftest -> 监视器 ----
    Memory memory;  // 物理内存 + MMIO（构造时记录 RTC 基准时间）
    memory.load_image(img_file);

    if (FTRACE && !elf_file.empty()) {
        ftrace_init(elf_file);
    }

    Simulator sim;
    Difftest difftest(sim, memory);
    sim.set_difftest(&difftest);
    sim.reset(10);

    // 复位完成后启用 difftest（复位期间不参与对比）
    if (!difftest.init()) {
        std::cerr << "difftest: init failed, running WITHOUT reference model" << std::endl;
    }

    // ---- 进入交互式监视器 ----
    Monitor monitor(sim, memory);
    return monitor.run();
}
