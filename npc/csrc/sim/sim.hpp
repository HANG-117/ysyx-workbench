#ifndef SIM_HPP
#define SIM_HPP

#include <cstdint>

#include <VNPC.h>

// 仿真核心：持有 RTL 顶层实例，负责时钟 / 复位 / 单步推进。
class Simulator {
public:
    Simulator();
    ~Simulator();

    // 复位 n 个时钟周期
    void reset(int n);

    // 单个时钟周期（含 itrace / ftrace 输出）
    void step();

    bool exited() const { return should_exit_; }
    int exit_code() const { return exit_code_; }
    uint32_t exit_pc() const { return exit_pc_; }
    int cycle() const { return cycle_; }

    // 便捷访问 RTL 内部信号
    uint32_t pc() const;              // 当前 PC
    uint32_t inst() const;            // 当前指令
    uint32_t reg(int idx) const;      // 通用寄存器 x[idx]

    // 供 DPI-C 回调（sim_exit）使用：RTL 执行 ebreak 时触发
    void notify_exit(int pc, int a0);

private:
    TOP_NAME* top_;
    int cycle_;
    bool should_exit_;
    int exit_pc_;
    int exit_code_;
};

#endif
