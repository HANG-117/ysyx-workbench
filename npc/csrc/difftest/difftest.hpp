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

    bool init();

    void enable() { enabled_ = true; }
    bool enabled() const { return enabled_; }
    void disable() { enabled_ = false; }

    void step(const CPU_state& pre, const CPU_state& post, uint32_t inst);

private:

    bool checkregs(const CPU_state& dut, const CPU_state& ref, uint32_t pc, uint32_t inst);

    Simulator& sim_;
    Memory& mem_;

    bool enabled_;
    void* handle_;

    void (*ref_difftest_init)(int port);
    void (*ref_difftest_memcpy)(uint32_t addr, void* buf, size_t n, int direction);
    void (*ref_difftest_regcpy)(void* dut, int direction);
    void (*ref_difftest_exec)(uint64_t n);
    void (*ref_difftest_raise_intr)(uint64_t NO);
};

#endif
