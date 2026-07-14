// test-highest_one_bit.cpp
// 简单测试 highestOneBit 实现（属于 tests，遵循 tests/CMakeLists.txt 约定）
#include <iostream>
#include <cstdint>
#include <cassert>
#include "../quant1x/base/bits.h"

int main() {
    using quant1x::bits::highestOneBit;

    // 32-bit 测试
    uint32_t a0 = 0u;
    uint32_t a1 = 1u;
    uint32_t a2 = 0xFFu; // 255 -> highest bit 0x80
    uint32_t a3 = 0x80000000u;

    assert(highestOneBit<uint32_t>(a0) == 0u);
    assert(highestOneBit<uint32_t>(a1) == 1u);
    assert(highestOneBit<uint32_t>(a2) == 0x80u);
    assert(highestOneBit<uint32_t>(a3) == 0x80000000u);

    // 64-bit 测试
    uint64_t b0 = 0ull;
    uint64_t b1 = 0x7FFFFFFFFFFFFFFFull; // highest bit should be 0x4000...0
    uint64_t b2 = 0x8000000000000000ull;

    assert(highestOneBit<uint64_t>(b0) == 0ull);
    assert(highestOneBit<uint64_t>(b1) == 0x4000000000000000ull);
    assert(highestOneBit<uint64_t>(b2) == 0x8000000000000000ull);

    std::cout << "test-highest_one_bit passed." << std::endl;
    return 0;
}
