#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case 11: 
        switch (c->GPR1) {
          case -1: ev.event = EVENT_YIELD; break;
          default: ev.event = EVENT_ERROR; break;
        }
        break;
      default: ev.event = EVENT_ERROR; break;
    }
    c = user_handler(ev, c);
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}
Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context *ret = (Context *)(kstack.end - sizeof(Context));
  ret->mepc = (uintptr_t)entry;
  ret->GPRx = (uintptr_t)arg;
  ret->mcause = 11;
  ret->mstatus = 0x1800;

  return ret;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  uintptr_t mstatus;
  asm volatile("csrr %0, mstatus" : "=r"(mstatus));
  return (mstatus & 0x8) != 0;
}

void iset(bool enable) {
  if(enable) {
    asm volatile("csrsi mstatus, 0x8");
  } else {
    asm volatile("csrci mstatus, 0x8");
  }
}
