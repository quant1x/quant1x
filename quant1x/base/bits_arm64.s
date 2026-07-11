#include "textflag.h"

// func highestOneBit(x uint64) uint64
TEXT ·highestOneBit(SB), NOSPLIT, $0-16
    // 输入参数 x 在 X0 寄存器
    MOV     X0, X1           // 备份 x 到 X1
    CMP     X0, #0           // 检查 x 是否为 0
    BEQ     zero_case        // x == 0 跳转

    // 快速路径：检查 x 是否为 2 的幂
    SUBS    X2, X0, #1       // X2 = x - 1
    ANDS    X2, X0, X2       // 计算 x & (x-1)
    BEQ     done             // 结果为 0，说明 x 是 2 的幂

    // 使用 CLZ 指令获取前导零数量
    CLZ     X2, X0           // X2 = 前导零数量
    MOV     X3, #63
    SUB     X2, X3, X2       // X2 = 63 - CLZ(x) → 最高有效位位置
    MOV     X3, #1
    LSL     X3, X3, X2       // X3 = 1 << X2（最高位的 2 的幂）

    // 检查是否需要左移一位
    CMP     X3, X1           // X3 >= x ?
    BHS     done             // 高于或等于则直接返回
    LSL     X3, X3, #1       // 否则左移一位
    MOV     X0, X3
    RET

zero_case:
    // x == 0 时返回 1<<63（根据需求可调整）
    MOV     X0, #0x8000000000000000
done:
    RET
