"""
quant1x.base.bits 模块

提供底层、高性能的位运算原语工具集。
本模块的所有核心函数均遵循严格的跨语言契约（与 C++/Go/Rust 端保持绝对一致），
专用于量化系统核心组件（如 RingBuffer、无锁队列等）的底层对齐与掩码计算。
"""

# 定义系统支持的最大位宽（通常与底层 C++/Rust 的 uint64_t 对齐）
# 在 64 位无符号整数中，最大的 2 的幂是 2^63 (因为 2^64 会溢出 uint64_t)
MAX_BITS = 64
MAX_POWER_OF_TWO = 1 << (MAX_BITS - 1)


def round_up_to_power_of_two(x: int, max_bits: int = MAX_BITS) -> int:
    """
    返回大于等于 x 的最小 2 的幂次方。
    
    跨语言契约 (与 C++/Go/Rust 严格一致):
    - 当 x == 0 时返回 1。
    - 如果 x 已经是 2 的幂则返回 x。
    - 【溢出防护】如果计算结果超出了指定的最大位宽 (如 64 位)，
      安全退化为该位宽能表示的最大 2 的幂 (如 2^63)，防止与底层 C++/Rust 数据面行为不一致。
      
    :param x: 输入的非负整数
    :param max_bits: 限制的最大位宽，默认为 64 (对齐 uint64_t)
    :return: 对齐后的 2 的幂。如果超出 max_bits，则退化为 1 << (max_bits - 1)
    """
    if x <= 0:
        return 1
    
    # 计算数学上正确的 2 的幂所需的位宽
    # 例如：x=8 -> x-1=7 -> bit_length=3 -> 1<<3 = 8
    # 例如：x=9 -> x-1=8 -> bit_length=4 -> 1<<4 = 16
    target_bits = (x - 1).bit_length()
    
    # 【核心防护】：跨语言行为一致性截断
    # 如果目标位宽超过了系统支持的最大位宽（例如算出了 65 位），
    # 则退化为该位宽能表示的最大 2 的幂 (即 max_bits - 1)
    if target_bits > max_bits:
        return 1 << (max_bits - 1)
        
    return 1 << target_bits


if __name__ == "__main__":
    # 全面自测，重点验证跨语言边界行为
    test_values = [
        0,                  # 边界：0 -> 1
        1,                  # 边界：1 -> 1
        2,                  # 2 的幂：2 -> 2
        3,                  # 普通：3 -> 4
        1023,               # 普通：1023 -> 1024
        1 << 63,            # 64位边界：2^63 -> 2^63
        (1 << 63) + 1,      # 【关键测试】超出 64 位无符号整数范围
        1 << 100,           # 【关键测试】超大整数
    ]
    
    print(f"{'Input':<30} | {'Output':<30} | {'Match C++/Rust?'}")
    print("-" * 75)
    for t in test_values:
        res = round_up_to_power_of_two(t)
        
        # 模拟 C++/Rust 的行为用于对比
        if t == 0:
            cpp_res = 1
        elif t > (1 << 63):
            cpp_res = 1 << 63  # C++ 溢出退化
        else:
            cpp_res = 1 << (t - 1).bit_length()
            
        match = "✅ YES" if res == cpp_res else "❌ NO"
        
        t_str = f"{t} (0x{t:x})" if t > 1000 else str(t)
        res_str = f"{res} (0x{res:x})" if res > 1000 else str(res)
        print(f"{t_str:<30} | {res_str:<30} | {match}")