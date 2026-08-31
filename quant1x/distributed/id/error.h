// 分布式 ID 的统一错误类型 (对应 Rust distributed/id/error.rs, Go 包内 error 使用)
//
// 说明(中文):
// - 四种语言 (Go/Rust/C++/Python) 共享同一组错误分类, 便于跨语言语义对齐与测试向量复用;
// - C++ 侧以 `Error` 值对象承载错误, 不使用异常做控制流 (对齐 Rust 的 Result 语义);
// - `Result<T>` 为轻量结果包装: 成功时持有值, 失败时持有 `Error`.
#pragma once
#ifndef QUANT1X_DISTRIBUTED_ID_ERROR_H
#define QUANT1X_DISTRIBUTED_ID_ERROR_H 1

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

namespace quant1x::distributed::id {

/// distributed/id 的运行时错误分类
enum class ErrorCode {
    /// 无错误 (成功)
    None = 0,
    /// 队列容量无效 (0)
    InvalidSize,
    /// 队列已满 (try_push)
    QueueFull,
    /// 队列为空 (try_pop)
    QueueEmpty,
    /// 队列已关闭
    Closed,
    /// 操作被取消 (serve 收到停止信号)
    Canceled,
    /// 时间戳超出 41 位可表示范围
    EpochElapsedOutOfRange,
    /// 节点编号超出可用 worker 位范围
    NodeIdOutOfRange,
    /// seq 位数不在 [4, 21] 范围内
    InvalidSeqBits,
    /// 节点数过大, 推导出的 seq 位数小于 4
    NodeCountTooLarge,
    /// 状态文件操作失败 (IO / mmap / 损坏)
    StateFile,
    /// 内部互斥锁中毒 (C++ 侧: 互斥量不可用)
    LockPoisoned,
    /// ID 字符串解析失败
    ParseId,
};

/// 错误值对象: code == ErrorCode::None 表示无错误
struct Error {
    ErrorCode code = ErrorCode::None;
    /// 可读描述 (与 Rust Display 输出保持一致)
    std::string message;
    /// 可选数值载荷 (对齐 Rust 枚举的关联值, 无载荷时为 0)
    int64_t value = 0;

    Error() = default;
    Error(ErrorCode code_, std::string message_, int64_t value_ = 0)
        : code(code_), message(std::move(message_)), value(value_) {}

    /// 是否无错误
    bool ok() const noexcept { return code == ErrorCode::None; }
    /// 是否出错 (可作布尔判断)
    explicit operator bool() const noexcept { return code != ErrorCode::None; }
    /// 异常兼容接口
    const char *what() const noexcept { return message.empty() ? "ok" : message.c_str(); }

    // ---- 工厂方法 (与 Rust Error 枚举变体一一对应) ----
    static Error invalid_size();
    static Error queue_full();
    static Error queue_empty();
    static Error closed();
    static Error canceled();
    static Error epoch_elapsed_out_of_range(int64_t elapsed);
    static Error node_id_out_of_range(uint32_t node_id);
    static Error invalid_seq_bits(uint8_t seq_bits);
    static Error node_count_too_large(uint32_t count);
    static Error state_file(const std::string &detail);
    static Error lock_poisoned();
    static Error parse_id(const std::string &text);
};

/// 结果包装: 成功持有值, 失败持有错误
///
/// 值用 optional 承载, 因此不要求 T 可默认构造 (Generator 等类型无默认构造).
template<typename T>
struct Result {
    std::optional<T> value;
    Error error;

    Result() = default;
    /// 成功构造
    Result(T v) : value(std::move(v)) {}  // NOLINT: 允许隐式转换以对齐 Rust 的 Ok(v)
    /// 失败构造
    Result(Error e) : error(std::move(e)) {}  // NOLINT: 允许隐式转换以对齐 Rust 的 Err(e)

    bool ok() const noexcept { return error.ok(); }
    explicit operator bool() const noexcept { return error.ok(); }

    const T &operator*() const { return *value; }
    T &operator*() { return *value; }
};

/// 无值结果 (仅状态)
using Status = Error;

}  // namespace quant1x::distributed::id

#endif  // QUANT1X_DISTRIBUTED_ID_ERROR_H
