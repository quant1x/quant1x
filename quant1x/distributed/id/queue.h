// ID 队列 (对应 Go Queue, Python queue.py, Rust queue.rs)
//
// 底层复用 C++ 基建 runtime::ringbuffer::queue (Vyukov 有界 MPMC 队列).
// 语义对齐 Go/Rust:
//   - try_push / try_pop 非阻塞;
//   - push / pop 阻塞 (忙等退避);
//   - close 后进入只读排空: 存量 ID 仍可消费, 队空后再返回 ErrorCode::Closed.
#pragma once
#ifndef QUANT1X_DISTRIBUTED_ID_QUEUE_H
#define QUANT1X_DISTRIBUTED_ID_QUEUE_H 1

#include <atomic>
#include <cstddef>
#include <memory>

#include <quant1x/distributed/id/error.h>
#include <quant1x/distributed/id/id.h>
#include <quant1x/runtime/ringbuffer.h>

namespace quant1x::distributed::id {

/// ID 队列 (不可拷贝, 通过 shared_ptr 在生产者/消费者之间共享)
class Queue {
public:
    Queue(const Queue &) = delete;
    Queue &operator=(const Queue &) = delete;

    /// 创建队列, 容量向上取整到 2 的幂; 容量 0 返回 ErrorCode::InvalidSize
    static Result<std::shared_ptr<Queue>> create(size_t capacity);

    /// 非阻塞推入; 队列已满返回 QueueFull, 已关闭返回 Closed
    Status try_push(const Id &value);

    /// 非阻塞弹出; 队列为空返回 QueueEmpty, 已关闭且空返回 Closed
    Result<Id> try_pop();

    /// 阻塞推入: 队列满时忙等腾位, 直到成功或队列关闭
    Status push(const Id &value);

    /// 阻塞弹出: 队列空时忙等数据, 直到成功; 已关闭且空返回 Closed
    Result<Id> pop();

    /// 当前队列长度 (近似值)
    size_t len() const noexcept;

    /// 队列容量 (2 的幂)
    size_t cap() const noexcept;

    /// 关闭队列: 进入只读排空模式
    void close() noexcept;

    /// 队列是否已关闭
    bool is_closed() const noexcept;

    /// 阻塞等待队列排空 (存量消费完毕)
    void wait_for_close() const;

private:
    explicit Queue(size_t capacity) : inner_(capacity), closed_(false) {}

    runtime::ringbuffer::queue<Id> inner_;
    std::atomic<bool> closed_;
};

}  // namespace quant1x::distributed::id

#endif  // QUANT1X_DISTRIBUTED_ID_QUEUE_H
