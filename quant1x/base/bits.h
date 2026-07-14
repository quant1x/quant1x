#pragma once
#ifndef QUANT1X_BASE_BITS_H
#define QUANT1X_BASE_BITS_H 1

// bits.h
// 实现：round_up_to_power_of_two（属于 base 的 bits 模块）
// 说明：将输入无符号整数向上舍入到最小的 2 的幂。
// - 当输入为 0 时返回 1（用于 ring buffer 容量对齐契约）。
// - 如果计算结果对目标类型发生溢出，则退化为该类型能表示的最大 2 的幂。
// 实现：constexpr 模板，使用位传播（bit-propagation）技术，支持任意无符号整数类型。
#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <climits>

namespace quant1x {
namespace bits {

// round_up_to_power_of_two: 向上舍入到最小的 2 的 N 次幂
// 专用于 RingBuffer 容量对齐，以便使用 (index & (capacity - 1)) 替代取模。
// 跨语言契约 (与 Go/Rust 严格一致):
//  - 当输入为 0 时返回 1。
//  - 如果结果超出类型可表示范围（发生加法溢出），安全退化为该类型能表示的最大 2 的幂。
template <typename T>
constexpr T round_up_to_power_of_two(T v) noexcept {
    static_assert(std::is_unsigned<T>::value, "requires unsigned integer type");
    
    // 1. 减法下溢防御
    if (v == 0) return 1; 

    // 2. 核心位传播逻辑
    v -= 1;
    constexpr std::size_t bits = sizeof(T) * CHAR_BIT;

    // 循环条件 shift < bits 天然避免了 C++ 中的移位 UB (Undefined Behavior)
    for (std::size_t shift = 1; shift < bits; shift <<= 1) {
        v |= (v >> shift);
    }
    
    // 3. 加法溢出防御 (关键修复)
    v += 1;
    // 如果 v+1 发生无符号溢出，结果会回绕变成 0。
    // 此时说明原值超出了该类型能表示的最大 2 的幂，安全退化为 1 << (bits - 1)
    if (v == 0) {
        v = T(1) << (bits - 1);
    }
    
    return v;
}

} // namespace bits
} // namespace quant1x

#endif // QUANT1X_BASE_BITS_H
