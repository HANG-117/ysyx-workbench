#ifndef MONITOR_HPP
#define MONITOR_HPP

#include <cstdint>

#include "memory/memory.hpp"
#include "sim/sim.hpp"

// 交互式监视器（SDB 风格）：
//   s <n>  单步前进 n 个周期
//   m <addr>  以 0x80000000 为基准查看周围内存
//   c      连续运行直到仿真结束
//   p      打印 32 个通用寄存器
//   q      退出
class Monitor {
public:
    Monitor(Simulator& sim, Memory& mem);

    // 主交互循环，返回进程退出码
    int run();

private:
    void cmd_step();
    void cmd_read_mem();
    void cmd_continue();
    void cmd_dump_regs();

    Simulator& sim_;
    Memory& mem_;
};

#endif
