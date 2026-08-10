#ifndef DIFFTEST_HPP
#define DIFFTEST_HPP

#include <cstddef>
#include <cstdint>

// ==================== DUT/NEMU 共享的 CPU 状态快照 ====================
// 注意：字段顺序必须与 NEMU 的 riscv32_CPU_state 完全一致 (gpr 在前, pc 在后)，
// 因为 difftest_regcpy 按内存布局整体 memcpy (DIFFTEST_REG_SIZE = 33 字 = 132 字节)。
struct CPU_state {
    uint32_t gpr[32];
    uint32_t pc;
};

class Simulator;
class Memory;

// 与 NEMU include/difftest-def.h 保持一致
enum DifftestDirection {
    DIFFTEST_TO_DUT = 0,  // DUT -> NEMU
    DIFFTEST_TO_REF = 1,  // NEMU -> DUT
};

// ==================== Difftest：与参考模型 (NEMU) 的逐指令对比 ====================
class Difftest {
public:
    Difftest(Simulator& sim, Memory& mem);

    // dlopen 参考模型动态库并完成初始化：
    //   1. 定位 SO (NPC_REF_SO 环境变量 > REF_SO_PATH 编译宏 > 默认名)
    //   2. dlsym 5 个 difftest_* 接口
    //   3. difftest_init + 镜像同步 (memcpy) + 寄存器同步 (regcpy)
    // 成功返回 true 并启用对比；失败返回 false (打印原因，仿真继续但不对比)。
    bool init();

    void enable() { enabled_ = true; }
    bool enabled() const { return enabled_; }

    // 每执行完一条指令调用一次：
    //   pre  = DUT 执行本条指令前的状态 (pc 即本条指令地址)
    //   post = DUT 执行后的状态
    //   inst = 本条指令的机器码
    // 内部流程：DUT 状态推给 ref -> ref 执行 1 条 -> 取回 ref 状态 -> 比较。
    // 不一致时打印完整报告并 exit(1)。
    void step(const CPU_state& pre, const CPU_state& post, uint32_t inst);

private:
    // 比较 DUT/ref 状态；不一致时打印报告 (pc、指令、全部寄存器、第一个不一致点)，
    // 返回 false。pc 为当前指令地址，inst 为当前指令。
    bool checkregs(const CPU_state& dut, const CPU_state& ref, uint32_t pc, uint32_t inst);

    Simulator& sim_;
    Memory& mem_;

    bool enabled_;
    void* handle_;

    // 参考模型导出函数指针 (与 NEMU src/cpu/difftest/ref.c 对应)
    void (*ref_difftest_init)(int port);
    void (*ref_difftest_memcpy)(uint32_t addr, void* buf, size_t n, int direction);
    void (*ref_difftest_regcpy)(void* dut, int direction);
    void (*ref_difftest_exec)(uint64_t n);
    void (*ref_difftest_raise_intr)(uint64_t NO);
};

#endif
