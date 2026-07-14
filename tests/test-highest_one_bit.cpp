// test-round_up_to_power_of_two.cpp
// 简单测试 round_up_to_power_of_two 实现（属于 tests，遵循 tests/CMakeLists.txt 约定）
#include <iostream>
#include <cstdint>
#include <cassert>
#include "../quant1x/base/bits.h"

int main() {
    using quant1x::bits::round_up_to_power_of_two;

    // 32-bit 测试
    uint32_t a0 = 0u;
    uint32_t a1 = 1u;
    uint32_t a2 = 0xFFu; // 255 -> round up to 0x100
    uint32_t a3 = 0x80000000u;

    assert(round_up_to_power_of_two<uint32_t>(a0) == 1u);
    assert(round_up_to_power_of_two<uint32_t>(a1) == 1u);
    assert(round_up_to_power_of_two<uint32_t>(a2) == 0x100u);
    assert(round_up_to_power_of_two<uint32_t>(a3) == 0x80000000u);

    // 64-bit 测试
    uint64_t b0 = 0ull;
    uint64_t b1 = 0x7FFFFFFFFFFFFFFFull; // round up to 0x8000...0
    uint64_t b2 = 0x8000000000000000ull;

    assert(round_up_to_power_of_two<uint64_t>(b0) == 1ull);
    assert(round_up_to_power_of_two<uint64_t>(b1) == 0x8000000000000000ull);
    assert(round_up_to_power_of_two<uint64_t>(b2) == 0x8000000000000000ull);

    std::cout << "test-round_up_to_power_of_two passed." << std::endl;
    return 0;
}
