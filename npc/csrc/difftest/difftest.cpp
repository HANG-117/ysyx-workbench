#include "difftest/difftest.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <iostream>

#include "memory/memory.hpp"
#include "sim/sim.hpp"
#include "trace/trace.hpp"

// 参考模型动态库路径，由 Makefile 生成 (build/ref_so_path.h)
#include "ref_so_path.h"
#ifndef REF_SO_PATH
#define REF_SO_PATH "riscv32-nemu-interpreter-so"
#endif

Difftest::Difftest(Simulator& sim, Memory& mem)
    : sim_(sim),
      mem_(mem),
      enabled_(false),
      handle_(nullptr),
      ref_difftest_init(nullptr),
      ref_difftest_memcpy(nullptr),
      ref_difftest_regcpy(nullptr),
      ref_difftest_exec(nullptr),
      ref_difftest_raise_intr(nullptr) {}

bool Difftest::init() {
    // 1. 定位参考模型动态库：环境变量 > 编译期默认路径 > 默认文件名
    const char* so_path = nullptr;
    if (const char* env = getenv("NPC_REF_SO")) {
        so_path = env;
    }
    if (so_path == nullptr) {
        so_path = REF_SO_PATH;
    }

    // 2. 加载动态库
    handle_ = dlopen(so_path, RTLD_LAZY);
    if (handle_ == nullptr) {
        std::cerr << "difftest: dlopen(\"" << so_path << "\") failed: " << dlerror()
                  << std::endl;
        return false;
    }

    // 3. 解析符号
    auto load_sym = [&](const char* name, void** fn) -> bool {
        *fn = dlsym(handle_, name);
        if (*fn == nullptr) {
            std::cerr << "difftest: dlsym(\"" << name << "\") failed: " << dlerror()
                      << std::endl;
            return false;
        }
        return true;
    };
    if (!load_sym("difftest_init", (void**)&ref_difftest_init) ||
        !load_sym("difftest_memcpy", (void**)&ref_difftest_memcpy) ||
        !load_sym("difftest_regcpy", (void**)&ref_difftest_regcpy) ||
        !load_sym("difftest_exec", (void**)&ref_difftest_exec) ||
        !load_sym("difftest_raise_intr", (void**)&ref_difftest_raise_intr)) {
        return false;
    }

    // 4. 初始化参考模型 (内存 + ISA)
    ref_difftest_init(0);

    // 5. 将 DUT 的客户程序镜像同步给参考模型
    size_t img_size = mem_.image_size();
    if (img_size > 0) {
        ref_difftest_memcpy(MEM_BASE, mem_.host_addr(MEM_BASE), img_size, DIFFTEST_TO_DUT);
    }

    // 6. 将 DUT 当前状态 (复位后) 同步给参考模型
    CPU_state reset_state = sim_.snapshot();
    ref_difftest_regcpy(&reset_state, DIFFTEST_TO_DUT);

    std::cout << "difftest: reference model loaded from \"" << so_path << "\"" << std::endl;
    enabled_ = true;
    return true;
}

// 计算 load/store 指令的有效地址（rs1 + imm）。
// 返回 true 且通过 *out_addr 输出地址；若不是 load/store 则返回 false。
static bool calc_mem_addr(const CPU_state& pre, uint32_t inst, uint32_t* out_addr) {
    uint32_t opcode = inst & 0x7f;
    if (opcode != 0b0000011 && opcode != 0b0100011) return false;  // 非 LOAD/STORE

    uint32_t rs1 = pre.gpr[(inst >> 15) & 0x1f];
    int32_t  imm;
    if (opcode == 0b0000011) {  // I-type (LOAD): imm[11:0] = inst[31:20]
        imm = (int32_t)inst >> 20;
    } else {                      // S-type (STORE): imm = {inst[31:25], inst[11:7]}
        imm = (int32_t)((inst & 0xfe000000) | ((inst & 0x00000f80) << 13)) >> 20;
    }
    *out_addr = (uint32_t)((int32_t)rs1 + imm);
    return true;
}

// 地址是否在物理内存 [0x8000_0000, 0x8800_0000) 之外（即 MMIO 或未映射）
static bool is_mmio_addr(uint32_t addr) {
    return addr < 0x80000000u || addr >= 0x88000000u;
}

void Difftest::step(const CPU_state& pre, const CPU_state& post, uint32_t inst) {
    if (!enabled_) {
        return;
    }

    // 预判本条指令是不是访问 MMIO 的 load/store
    uint32_t mem_addr = 0;
    bool is_mmio_access = calc_mem_addr(pre, inst, &mem_addr) && is_mmio_addr(mem_addr);

    // 1. 将 DUT 执行本条指令前的状态推给参考模型 (寄存器 + PC)
    ref_difftest_regcpy((void*)&pre, DIFFTEST_TO_DUT);

    if (is_mmio_access) {
        // ===== MMIO 跳过分支 =====
        ref_difftest_regcpy((void*)&post, DIFFTEST_TO_DUT);
    } else {
        // 2. 参考模型执行一条指令
        ref_difftest_exec(1);

        // 3. 取回参考模型状态
        CPU_state ref{};
        ref_difftest_regcpy(&ref, DIFFTEST_TO_REF);

        // 4. 与 DUT 执行后的状态比较
        if (!checkregs(post, ref, pre.pc, inst)) {
            fflush(stdout);
            exit(1);
        }
    }
}

bool Difftest::checkregs(const CPU_state& dut, const CPU_state& ref, uint32_t pc, uint32_t inst) {
    // ---- PC 比较 ----
    if (dut.pc != ref.pc) {
        std::cerr << "\n[DIFFTEST] MISMATCH: pc is different after executing instruction "
                  << "at pc = 0x" << std::hex << pc << std::endl;
        std::cerr << "[DIFFTEST]   instruction = 0x" << std::hex << inst << "  ("
                  << disassemble_rv32e(inst, pc) << ")" << std::endl;
        std::cerr << "[DIFFTEST]   pc: DUT = 0x" << std::hex << dut.pc
                  << ", NEMU = 0x" << std::hex << ref.pc << std::endl;
        std::cerr << "[DIFFTEST]   >>> first mismatch: pc" << std::endl;
        return false;
    }

    // ---- 通用寄存器比较 (x0 恒为 0，跳过) ----
    int first_mismatch = -1;
    for (int i = 1; i < 32; i++) {
        if (dut.gpr[i] != ref.gpr[i]) {
            first_mismatch = i;
            break;
        }
    }
    if (first_mismatch == -1) {
        return true;
    }

    std::cerr << "\n[DIFFTEST] MISMATCH: register x" << std::dec << first_mismatch
              << " is different after executing instruction at pc = 0x" << std::hex << pc
              << std::endl;
    std::cerr << "[DIFFTEST]   instruction = 0x" << std::hex << inst << "  ("
              << disassemble_rv32e(inst, pc) << ")" << std::endl;

    // 打印全部寄存器 (DUT vs NEMU)，不一致的标 ***
    for (int row = 0; row < 32; row += 4) {
        for (int col = 0; col < 4; col++) {
            int i = row + col;
            bool diff = (i != 0) && (dut.gpr[i] != ref.gpr[i]);
            std::cerr << "x" << std::dec << i << " DUT=0x" << std::hex << dut.gpr[i]
                      << " NEMU=0x" << std::hex << ref.gpr[i] << (diff ? " ***" : "") << "\t";
        }
        std::cerr << std::endl;
    }

    std::cerr << "[DIFFTEST]   >>> first mismatch: x" << std::dec << first_mismatch
              << " (DUT=0x" << std::hex << dut.gpr[first_mismatch]
              << ", NEMU=0x" << std::hex << ref.gpr[first_mismatch] << ")" << std::endl;
    return false;
}
