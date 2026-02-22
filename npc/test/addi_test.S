# addi_test.S
.section .text
.global _start

_start:
    # 测试 1: addi a0, x0, 42  → a0 = 0 + 42 = 42
    addi x10, x0, 42     # x10 = a0

    # 测试 2: addi a1, a0, -5  → a1 = 42 - 5 = 37
    addi x11, x10, -5    # x11 = a1

    addi x2, x0, 0x7ff   # x2 = sp

    # 结束（死循环，方便观察最终寄存器值）
_loop:
    j _loop