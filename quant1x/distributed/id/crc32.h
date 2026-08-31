// CRC32-IEEE 校验 (对应 Rust distributed/id/crc32.rs, 与 Go hash/crc32 ChecksumIEEE 一致)
//
// 多项式 0xEDB88320 (反射表示), 初值 0xFFFFFFFF, 结果取反.
// 状态文件 (checkpoint / legacy 记录) 依赖该校验值, 必须保证四语言输出一致.
#pragma once
#ifndef QUANT1X_DISTRIBUTED_ID_CRC32_H
#define QUANT1X_DISTRIBUTED_ID_CRC32_H 1

#include <array>
#include <cstddef>
#include <cstdint>

namespace quant1x::distributed::id {

/// 建表: 256 项反射多项式查表 (与 Rust build_crc32_table 逐位一致)
constexpr std::array<uint32_t, 256> build_crc32_table() {
    std::array<uint32_t, 256> table{};
    for (uint32_t n = 0; n < 256; ++n) {
        uint32_t c = n;
        for (int k = 0; k < 8; ++k) {
            if ((c & 1u) != 0u) {
                c = 0xEDB88320u ^ (c >> 1);
            } else {
                c >>= 1;
            }
        }
        table[n] = c;
    }
    return table;
}

/// CRC32-IEEE 查表 (初值 0xFFFFFFFF, 结果取反)
constexpr std::array<uint32_t, 256> CRC32_IEEE_TABLE = build_crc32_table();

/// 计算 CRC32-IEEE, 与 Go `crc32.ChecksumIEEE` 输出一致
uint32_t crc32_ieee(const uint8_t *data, size_t size) noexcept;

}  // namespace quant1x::distributed::id

#endif  // QUANT1X_DISTRIBUTED_ID_CRC32_H
