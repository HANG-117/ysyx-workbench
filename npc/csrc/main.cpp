#include <cstdlib>
#include <iostream>
#include <string>

#include "common.hpp"
#include "difftest/difftest.hpp"
#include "memory/memory.hpp"
#include "monitor/monitor.hpp"
#include "sim/sim.hpp"
#include "trace/trace.hpp"

int main(int argc, char** argv) {
    // ---- 解析：difftest 开关 + 镜像路径 + ftrace elf ----
    const char* img_file = "test/mem.bin";
    std::string elf_file;
    bool sdb = false;
    // 默认：先读环境变量 NPC_DIFFTEST（0=关，其余=开）
    bool want_difftest = true;
    if (const char* env = std::getenv("NPC_DIFFTEST")) {
        if (std::string(env) == "0") want_difftest = false;
    }

    // 解析命令行：--diff / --no-diff / --sdb / 镜像路径
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--diff") {
            want_difftest = true;
        } else if (arg == "--no-diff") {
            want_difftest = false;
        } else if (arg == "--sdb") {
            sdb = true;
        } else if (!arg.empty() && arg[0] != '-') {
            img_file = argv[i];
            if (FTRACE) {
                size_t dot_pos = arg.find_last_of('.');
                if (dot_pos != std::string::npos) {
                    elf_file = arg.substr(0, dot_pos) + ".elf";
                } else {
                    elf_file = arg + ".elf";
                }
            }
        }
    }

    // ---- 初始化：内存 -> 仿真器 -> difftest -> 监视器 ----
    Memory memory;
    memory.load_image(img_file);

    if (FTRACE && !elf_file.empty()) {
        ftrace_init(elf_file);
    }

    Simulator sim;
    Difftest difftest(sim, memory);
    sim.set_difftest(&difftest);
    sim.reset(10);

    // 根据开关决定是否初始化 difftest
    if (want_difftest) {
        if (!difftest.init()) {
            std::cerr << "difftest: init failed, running WITHOUT reference model" << std::endl;
        } else {
            std::cout << "difftest: [ON]  (--no-diff 或 NPC_DIFFTEST=0 可关闭)" << std::endl;
        }
    } else {
        difftest.disable();
        std::cout << "difftest: [OFF] (--diff 或 NPC_DIFFTEST=1 可开启)" << std::endl;
    }
    if (sdb) {
        // ---- 进入交互式监视器 ----
        Monitor monitor(sim, memory);
        return monitor.run();
    }

    // ---- 批处理运行，直到程序退出或达到周期上限 ----
    while (!sim.exited() && sim.cycle() < MAX_CYCLES) {
        sim.step();
    }
    if (!sim.exited()) {
        std::cerr << "Simulation reached maximum cycle count: " << MAX_CYCLES << std::endl;
        return 1;
    }
    return sim.exit_code();
}
