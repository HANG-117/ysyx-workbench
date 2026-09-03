#ifndef SIM_HPP
#define SIM_HPP

#include <cstdint>

#include <VNPC.h>

#include "difftest/difftest.hpp"

class Difftest;

// 仿真核心：持有 RTL 顶层实例，负责时钟 / 复位 / 单步推进。
class Simulator {
public:
    Simulator();
    ~Simulator();

    // 复位 n 个时钟周期
    void reset(int n);

    // 单个时钟周期；仅在指令提交的周期执行 trace/difftest
    void step();

    // 挂载 difftest 对比器（必须在 reset 之前设置；reset 期间自动跳过对比）
    void set_difftest(Difftest* difftest) { difftest_ = difftest; }

    bool exited() const { return should_exit_; }
    int exit_code() const { return exit_code_; }
    uint32_t exit_pc() const { return exit_pc_; }
    int cycle() const { return cycle_; }

    // 便捷访问 RTL 内部信号
    uint32_t pc() const;              // 当前 PC
    uint32_t inst() const;            // 当前指令
    uint32_t reg(int idx) const;      // 通用寄存器 x[idx]

    // 取 DUT 当前完整状态快照 (gpr[32] + pc)，供 difftest 使用
    CPU_state snapshot() const;

    // 供 DPI-C 回调（sim_exit）使用：RTL 执行 ebreak 时触发
    void notify_exit(int pc, int a0);

private:
    TOP_NAME* top_;
    int cycle_;
    bool should_exit_;
    int exit_pc_;
    int exit_code_;
    Difftest* difftest_;   // 可选：逐指令对比器
};

#endif
