#include "textflag.h"

// func highestOneBit(x uint64) uint64
TEXT ·highestOneBit(SB), NOSPLIT, $0-16  // 定义函数符号，NOSPLIT表示不分配栈空间，参数+返回值共16字节
    MOVQ    x+0(FP), AX       // 将输入参数x（8字节）从内存加载到AX寄存器
    TESTQ   AX, AX             // 检查AX是否为0（通过AND操作自身，影响标志位）
    JZ      zero_case          // 若ZF=1（AX=0），跳转到zero_case处理

    // 快速路径：直接判断x是否为2的幂
    LEAQ    -1(AX), CX         // CX = x - 1（如x=8→7）
    TESTQ   AX, CX             // 计算x & (x-1)，结果为0则x是2的幂
    JZ      done               // ZF=1时跳转（x是2的幂，直接返回x）

    // 核心逻辑：计算最高有效位位置
    BSRQ    AX, CX             // BSR(Bit Scan Reverse)指令获取最高位索引（如x=5(101)→CX=2）
    MOVQ    $1, AX             // AX = 1（准备生成2的幂）
    SHLQ    CX, AX             // AX = 1 << CX（得到最高位对应的2的幂，如CX=2→4）

    // 调整结果：确保返回值≥原x
    CMPQ    AX, x+0(FP)        // 比较计算结果AX与原始输入x
    JAE     done               // 若AX≥x，直接返回（如x=5→4 <5，需继续调整）
    SHLQ    $1, AX             // AX <<=1（将结果翻倍，如4→8）

    // 结果返回
done:
    MOVQ    AX, ret+8(FP)     // 将结果AX写入返回值内存位置（FP+8）
    RET                        // 函数返回

    // 处理x=0的特殊情况
zero_case:
    MOVQ    $0x8000000000000000, AX // x=0时返回0x8000000000000000（1<<63）
    JMP     done               // 跳转到done返回结果
