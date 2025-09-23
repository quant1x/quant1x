// add_amd64.s
#include "textflag.h"

// func sse2Add(a, b, c *float64, n int)
TEXT ·sse2Add(SB),NOSPLIT,$0
    MOVQ a+0(FP), SI     // a指针
    MOVQ b+8(FP), DI     // b指针
    MOVQ c+16(FP), DX    // c指针
    MOVQ n+24(FP), CX    // 元素个数

    SHRQ $1, CX          // 处理128位（每次处理2个float64）
    JZ   remain

loop:
    MOVUPD (SI), X0      // 加载a的两个元素
    MOVUPD (DI), X1      // 加载b的两个元素
    ADDPD  X1, X0        // 执行加法
    MOVUPD X0, (DX)      // 存储结果

    ADDQ $16, SI         // 指针前进16字节（2*8）
    ADDQ $16, DI
    ADDQ $16, DX
    DECQ CX
    JNZ  loop

remain:
    // 处理剩余元素（n为奇数时）
    TESTQ $1, n+24(FP)
    JZ    done

    MOVQ (SI), X0
    ADDSD (DI), X0
    MOVSD X0, (DX)

done:
    RET
