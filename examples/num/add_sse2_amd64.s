// add_sse2_amd64.s
#include "textflag.h"

TEXT ·addSSE2(SB), NOSPLIT, $0
    MOVQ a_base+0(FP), SI     // a 数组指针
    MOVQ b_base+24(FP), DI    // b 数组指针
    MOVQ result_base+48(FP), DX  // 结果数组指针
    MOVQ len+72(FP), CX       // 数组长度

LOOP:
    MOVUPD (SI), X0           // 加载 2 个 double 到 XMM0
    ADDPD  (DI), X0           // 向量加法
    MOVUPD X0, (DX)           // 存储结果
    ADDQ $16, SI              // 指针前进 16 字节（2 元素）
    ADDQ $16, DI
    ADDQ $16, DX
    SUBQ $2, CX               // 剩余元素计数
    JNZ  LOOP
    RET
