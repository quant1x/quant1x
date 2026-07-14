// bits.h
// 实现：highestOneBit（属于 base 的 bits 模块）
// 说明：返回输入无符号整数中最高位的掩码（仅保留最高位为1，其余位为0）。
//       如果输入为0，则返回0。
// 注释：constexpr 模板，支持任意无符号整数类型（如 uint32_t / uint64_t）。
//       使用移位传播技术把低位全部置1，随后减去右移一位得到仅最高位为1的值。
#ifndef QUANT1X_BASE_BITS_H
#define QUANT1X_BASE_BITS_H

#include <type_traits>
#include <cstdint>

namespace quant1x {
namespace bits {

// highestOneBit: 返回仅包含最高位的掩码
// 模板要求：T 必须为无符号整数类型
template <typename T>
constexpr T highestOneBit(T v) noexcept {
    static_assert(std::is_unsigned<T>::value, "highestOneBit requires unsigned integer type");
    if (v == 0) return 0;

    T x = v;
    const unsigned int bits = sizeof(T) * 8;
    for (unsigned int shift = 1; shift < bits; shift <<= 1) {
        x |= (x >> shift);
    }
    return x - (x >> 1);
}

} // namespace bits
} // namespace quant1x

#endif // QUANT1X_BASE_BITS_H
