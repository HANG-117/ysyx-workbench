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

#include <isa.h>

#ifdef CONFIG_ETRACE
  #define ETRACE_BUF_SIZE 16
  static char etrace_buf[ETRACE_BUF_SIZE][128];
  static int etrace_head = 0;
  static int etrace_cnt = 0;

  static const char *etrace_exc_name(word_t NO) {
    if (NO & 0x80000000) {
      switch (NO & 0x7fffffff) {
        case 7:  return "EVENT_IRQ_TIMER";
        case 11: return "EVENT_IRQ_IODEV";
        default: return "Unknown interrupt";
      }
    } else {
      switch (NO) {
        case 11: return "EVENT_SYSCALL";
        case 12: return "EVENT_PAGEFAULT";
        case 13: return "EVENT_PAGEFAULT";
        case 15: return "EVENT_PAGEFAULT";
        default: return "Unknown exception";
      }
    }
  }

  static void etrace_push(const char *s) {
    snprintf(etrace_buf[etrace_head], 128, "%s", s);
    etrace_head = (etrace_head + 1) % ETRACE_BUF_SIZE;
    if (etrace_cnt < ETRACE_BUF_SIZE) etrace_cnt++;
  }

  void etrace_display() {
    if (etrace_cnt == 0) return;
    printf("etrace:\n");
    int start = (etrace_head + ETRACE_BUF_SIZE - etrace_cnt) % ETRACE_BUF_SIZE;
    int last = (etrace_head - 1 + ETRACE_BUF_SIZE) % ETRACE_BUF_SIZE;
    for (int i = 0; i < etrace_cnt; i++) {
      int idx = (start + i) % ETRACE_BUF_SIZE;
      printf("%s%s\n", idx == last ? "--> " : "    ", etrace_buf[idx]);
    }
  }
#endif

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */

#ifdef CONFIG_ETRACE
  char log[128];
  snprintf(log, 128, "pc=" FMT_WORD ": %s (NO=%d), epc=" FMT_WORD ", mtvec=" FMT_WORD,
           cpu.pc, etrace_exc_name(NO), (int)NO, epc, cpu.mtvec);
  etrace_push(log);
  printf("etrace: %s\n", log);
#endif

  cpu.mcause = NO;
  cpu.mepc = epc +4;
  return cpu.mtvec;
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}