#include "sim/sim.hpp"

#include <iostream>

#include <VNPC___024root.h>

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
      exit_code_(0),
      difftest_(nullptr) {
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
    // 记录本条指令执行前的状态（difftest 需要）
    uint32_t pc_before = pc();
    uint32_t inst_before = inst();
    CPU_state pre = snapshot();

    top_->clk = 0;
    top_->eval();
    top_->clk = 1;
    top_->eval();
    if (should_exit_) {
        return;  // 本条指令是 ebreak，不做 difftest 对比
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

    if (difftest_) {
        difftest_->step(pre, snapshot(), inst_before);
    }
}

uint32_t Simulator::pc() const {
    return top_->rootp->NPC__DOT__ifu_inst__DOT__pc_reg_inst__DOT__pc;
}

uint32_t Simulator::inst() const {
    return top_->rootp->NPC__DOT__inst;
}

uint32_t Simulator::reg(int idx) const {
    return top_->rootp->NPC__DOT__regfile_inst__DOT__rf[idx];
}

CPU_state Simulator::snapshot() const {
    CPU_state s{};
    for (int i = 0; i < 32; i++) {
        s.gpr[i] = reg(i);
    }
    s.gpr[0] = 0;  // x0 硬连线为 0
    s.pc = pc();
    return s;
}
