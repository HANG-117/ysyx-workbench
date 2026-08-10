/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

/* 参考模型 (ref) 接口实现。
 * 这些函数被编译进 riscv32-nemu-interpreter-so，由 DUT (NPC) 通过 dlopen/dlsym 调用。
 *
 * 调用约定 (见 include/difftest-def.h):
 *   direction == DIFFTEST_TO_DUT: 数据从 DUT 流向 NEMU (DUT -> ref)
 *   direction == DIFFTEST_TO_REF: 数据从 NEMU 流向 DUT (ref -> DUT)
 */

#include <string.h>

#include <isa.h>
#include <cpu/cpu.h>
#include <difftest-def.h>
#include <memory/paddr.h>

/* 将 DUT 的物理内存与参考模型同步。
 * addr/buf/n 均以字节为单位；addr 为客户端 (guest) 物理地址。 */
__EXPORT void difftest_memcpy(paddr_t addr, void *buf, size_t n, bool direction) {
  if (direction == DIFFTEST_TO_DUT) {
    memcpy(guest_to_host(addr), buf, n);
  } else {
    memcpy(buf, guest_to_host(addr), n);
  }
}

/* 寄存器同步。CPU_state 的内存布局为 { gpr[32], pc }，
 * 与 DUT 侧定义的 CPU_state 完全一致，因此可以整体 memcpy。 */
__EXPORT void difftest_regcpy(void *dut, bool direction) {
  if (direction == DIFFTEST_TO_DUT) {
    memcpy(&cpu, dut, DIFFTEST_REG_SIZE);
  } else {
    memcpy(dut, &cpu, DIFFTEST_REG_SIZE);
  }
}

/* 让参考模型连续执行 n 条指令 (从当前 cpu.pc 开始)。 */
__EXPORT void difftest_exec(uint64_t n) {
  cpu_exec(n);
}

/* 通知参考模型发生中断 (NPC 目前无中断，保留接口备用)。 */
__EXPORT void difftest_raise_intr(word_t NO) {
  isa_raise_intr(NO, cpu.pc);
}

__EXPORT void difftest_init(int port) {
  void init_mem();
  init_mem();
  /* Perform ISA dependent initialization. */
  init_isa();
}
