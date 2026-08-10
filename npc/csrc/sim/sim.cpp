#include "sim/sim.hpp"

#include <iostream>

#include "trace/trace.hpp"

// 供 DPI-C 回调使用的单例指针
static Simulator* g_sim = nullptr;

// DPI-C 导出：RTL 中执行 ebreak 时回调
extern "C" void sim_exit(int pc, int a0) {
    if (g_sim) {
        g_sim->notify_exit(pc, a0);
    }
}

Simulator::Simulator()
    : top_(new TOP_NAME),
      cycle_(0),
      should_exit_(false),
      exit_pc_(0),
      exit_code_(0) {
    if (g_sim == nullptr) {
        g_sim = this;
    }
}

Simulator::~Simulator() {
    delete top_;
}

void Simulator::notify_exit(int pc, int a0) {
    should_exit_ = true;
    exit_pc_ = pc;
    exit_code_ = a0;

    if (a0 != 0) {
        // 红色输出
        std::cout << "\033[31m"
                  << "Simulation exited with error code: " << a0
                  << "\033[0m" << std::endl;
    } else {
        // 绿色输出
        std::cout << "\033[32m"
                  << "HIT GOOD TRAP at pc = 0x" << std::hex << pc
                  << "\033[0m" << std::endl;
    }
}

void Simulator::reset(int n) {
    top_->rst = 1;
    while (n-- > 0) {
        step();
    }
    top_->rst = 0;
}

void Simulator::step() {
    top_->clk = 0;
    top_->eval();
    top_->clk = 1;
    top_->eval();
    if (should_exit_) {
        return;
    }

    if (ITRACE) {
        std::cout << "Cycle: " << std::dec << cycle_
                  << ", PC: 0x" << std::hex << pc()
                  << ", Instruction: 0x" << std::hex << inst()
                  << ", Disassembly: " << disassemble_rv32e(inst(), pc())
                  << std::endl;
    }
    cycle_++;

    if (FTRACE) {
        ftrace_check(pc(), inst(), reg(1));
    }
}

uint32_t Simulator::pc() const {
    return top_->NPC__DOT__ifu_inst__DOT__pc_reg_inst__DOT__pc;
}

uint32_t Simulator::inst() const {
    return top_->NPC__DOT__inst;
}

uint32_t Simulator::reg(int idx) const {
    return top_->NPC__DOT__regfile_inst__DOT__rf[idx];
}
