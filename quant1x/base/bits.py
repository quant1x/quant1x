"""bits.py
实现：round_up_to_power_of_two
说明：返回大于等于 x 的最小 2 的幂次方。
- 当 x == 0 时返回 1<<63
- 如果 x 已经是 2 的幂则返回 x

实现使用位传播循环，支持任意整数宽度。
"""

MAX_POWER2 = 1 << 63

def round_up_to_power_of_two(x: int) -> int:
    if x == 0:
        return 1
    if x & (x - 1) == 0:
        return x
    v = x - 1
    shift = 1
    bits = 64
    while shift < bits:
        v |= v >> shift
        shift <<= 1
    return v + 1


if __name__ == "__main__":
    # 简单自测
    test_values = [0,1,2,3,5,1023,1<<63]
    for t in test_values:
        print(t, round_up_to_power_of_two(t))
