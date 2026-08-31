// ID 生成器 (对应 Go Generator, Python generator.py, Rust generator.rs)
//
// 由 HLC 提供 (physical, seq), 拼装节点编号生成 64 位 ID.
#pragma once
#ifndef QUANT1X_DISTRIBUTED_ID_GENERATOR_H
#define QUANT1X_DISTRIBUTED_ID_GENERATOR_H 1

#include <atomic>
#include <cstdint>
#include <memory>

#include <quant1x/distributed/id/error.h>
#include <quant1x/distributed/id/hlc.h>
#include <quant1x/distributed/id/id.h>
#include <quant1x/distributed/id/queue.h>

namespace quant1x::distributed::id {

/// ID 生成器
class Generator {
public:
    /// 创建生成器; 节点编号超出 worker 位可表示范围返回 ErrorCode::NodeIdOutOfRange
    static Result<Generator> create(uint32_t node_id, std::shared_ptr<Hlc> hlc);

    /// worker 位数
    uint8_t worker_bits() const noexcept { return worker_bits_; }

    /// seq 位数
    uint8_t seq_bits() const noexcept { return seq_bits_; }

    /// 生成下一个 ID (const: 内部状态由 HLC 的互斥量保护, 可跨线程共享)
    Result<Id> next() const;

    /// 把发号器接入 ID 队列: 持续发号并写入 queue, 队列满时等待消费腾位,
    /// 直到 cancel 置位 (对应 Go 的 ctx 取消) 或队列关闭.
    ///
    /// 返回: 队列已关闭返回成功 (未消费的存量 ID 仍可由消费者排空);
    /// 取消返回 ErrorCode::Canceled.
    Status serve(const std::shared_ptr<Queue> &queue, const std::atomic<bool> &cancel);

private:
    Generator(std::shared_ptr<Hlc> hlc, uint32_t node_id, uint8_t worker_bits, uint8_t seq_bits)
        : hlc_(std::move(hlc)), node_id_(node_id), worker_bits_(worker_bits), seq_bits_(seq_bits) {}

    std::shared_ptr<Hlc> hlc_;
    uint32_t node_id_;
    uint8_t worker_bits_;
    uint8_t seq_bits_;
};

}  // namespace quant1x::distributed::id

#endif  // QUANT1X_DISTRIBUTED_ID_GENERATOR_H
