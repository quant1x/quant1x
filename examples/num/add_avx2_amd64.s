// add_avx2_amd64.s
#include "textflag.h"

TEXT ·addAVX2(SB), NOSPLIT, $0
    MOVQ a_base+0(FP), SI
    MOVQ b_base+24(FP), DI
    MOVQ result_base+48(FP), DX
    MOVQ len+72(FP), CX

LOOP:
    VMOVUPD (SI), Y0          // 加载 4 个 double 到 YMM0
    VADDPD  (DI), Y0, Y0      // 向量加法
    VMOVUPD Y0, (DX)          // 存储结果
    ADDQ $32, SI              // 指针前进 32 字节（4 元素）
    ADDQ $32, DI
    ADDQ $32, DX
    SUBQ $4, CX
    JNZ  LOOP
    RET
