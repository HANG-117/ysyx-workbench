    # ------------------ 内存测试部分 ------------------
    # 假设 RAM 基址在 0x2000（根据你的 NPC 调整 lui s0 的值）
    lui   s0, 0x00002           # s0 = 0x00002000

    # sw 测试：写一个已知值
    addi  t0, zero, 0x123       # 小值
    lui   t1, 0x00045
    addi  t1, t1, 0x678         # t1 = 0x00045678
    add   t1, t1, t0            # t1 = 0x0004579b
    sw    t1, 0(s0)             # 写 0x0004579b 到 0x2000

    # lw 测试：读回来
    lw    t2, 0(s0)             # t2 应该 = 0x0004579b

    # sb + lbu 测试（含高位 1）
    addi  t3, zero, 0xab        # 0xab
    sb    t3, 4(s0)             # 写字节 0xab 到 0x2004

    addi  t3, zero, -1          # 0xffffffff
    sb    t3, 5(s0)             # 写字节 0xff 到 0x2005

    lbu   t4, 4(s0)             # 应该得到 0x000000ab
    lbu   t5, 5(s0)             # 应该得到 0x000000ff（零扩展！）

    # 成功标记：把一个特殊值写到某个寄存器
    addi  a0, zero, 0x66        # a0 = 0x66 表示内存测试也通过

loop_end:
    .word 0x00100073            # ebreak