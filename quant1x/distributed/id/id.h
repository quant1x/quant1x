// 64 位可排序分布式 ID (对应 Go distributed/id/id.go, Python id.py, Rust id.rs)
//
// 位布局 (64 位, 大端):
//   | 41 位 elapsed(相对纪元毫秒) | worker_bits 位 node_id | seq_bits 位 seq |
//   其中 worker_bits + seq_bits == PAYLOAD_BITS (22)
//
// 编码: 8 字节大端 (bytes), 11 字符 base64url 无填充 (to_string)
//
// 跨语言一致性说明 (重要):
// - Go 使用 base64.RawURLEncoding, Python 使用 base64.urlsafe_b64encode, 二者一致:
//   末字符 (第 11 个) 承载 byte[7] 的低 4 位, 且置于 6 位值的**高 4 位**, 低 2 位补 0.
// - Rust 版 encode_base64url 将末字符写成 `alphabet[b[7] & 0x0F]`, 数据落在**低 4 位**,
//   与 Go/Python 不兼容 (自洽但不互通). Python 是 Spec 锚点, C++ 是生产真相源,
//   因此本实现严格对齐 Go/Python 的标准编码; 建议后续修正 Rust 版 (见 decode 注释).
#pragma once
#ifndef QUANT1X_DISTRIBUTED_ID_ID_H
#define QUANT1X_DISTRIBUTED_ID_ID_H 1

#include <cstdint>
#include <string>

#include <quant1x/distributed/id/error.h>

namespace quant1x::distributed::id {

/// 起始时间戳 (2026-01-01 00:00:00 UTC, 毫秒)
constexpr int64_t EPOCH_MS = 1767225600000LL;
/// 低 22 位承载 payload (节点 + 序号)
constexpr uint8_t PAYLOAD_BITS = 22;
/// 高 41 位承载物理时间 (相对起始时间的毫秒数)
constexpr uint8_t PHYSICAL_BITS = 41;
/// worker 位数上限 (seq 至少保留 4 位)
constexpr uint8_t MAX_WORKER_BITS = PAYLOAD_BITS - 4;

/// 64 位可排序分布式 ID (uint64 包装类型)
class Id {
public:
    constexpr Id() noexcept : value_(0) {}
    explicit constexpr Id(uint64_t value) noexcept : value_(value) {}

    /// 原始 uint64 值
    constexpr uint64_t value() const noexcept { return value_; }

    /// 大端序 8 字节表示 (对应 Go Bytes, Python to_bytes)
    void bytes(uint8_t out[8]) const noexcept;

    /// 从大端序 8 字节还原 (对应 Go FromBytes, Python from_bytes)
    static Id from_bytes(const uint8_t in[8]) noexcept;

    /// 相对起始时间的物理时间毫秒数 (高 41 位)
    int64_t physical() const noexcept;

    /// 节点编号 (payload 高 worker_bits 位)
    uint32_t node_id(uint8_t worker_bits) const noexcept;

    /// 序号 (payload 低 seq_bits 位, seq_bits = PAYLOAD_BITS - worker_bits)
    uint32_t seq(uint8_t worker_bits) const noexcept;

    /// 11 字符 base64url (无填充) 表示, 对齐 Go String / Python string
    std::string to_string() const;

    /// 解析 11 字符 base64url (无填充) 字符串
    static Result<Id> parse(const std::string &text);

    /// 校验相对起始时间的毫秒数是否落在 41 位可表示范围内
    static Result<int64_t> check_epoch(int64_t elapsed);

    // ---- 比较运算 (保持 ID 的可排序性) ----
    constexpr bool operator==(const Id &other) const noexcept { return value_ == other.value_; }
    constexpr bool operator!=(const Id &other) const noexcept { return value_ != other.value_; }
    constexpr bool operator<(const Id &other) const noexcept { return value_ < other.value_; }
    constexpr bool operator<=(const Id &other) const noexcept { return value_ <= other.value_; }
    constexpr bool operator>(const Id &other) const noexcept { return value_ > other.value_; }
    constexpr bool operator>=(const Id &other) const noexcept { return value_ >= other.value_; }

private:
    uint64_t value_;
};

}  // namespace quant1x::distributed::id

#endif  // QUANT1X_DISTRIBUTED_ID_ID_H
