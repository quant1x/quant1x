// ID 生成器的实现
#include <quant1x/distributed/id/generator.h>

#include <thread>

namespace quant1x::distributed::id {

Result<Generator> Generator::create(uint32_t node_id, std::shared_ptr<Hlc> hlc) {
    if (hlc == nullptr) {
        return Error::state_file("hlc must not be null");
    }
    const uint8_t seq_bits = hlc->seq_bits();
    const uint8_t worker_bits = static_cast<uint8_t>(PAYLOAD_BITS - seq_bits);
    if (static_cast<uint64_t>(node_id) >= (static_cast<uint64_t>(1) << worker_bits)) {
        return Error::node_id_out_of_range(node_id);
    }
    return Generator(std::move(hlc), node_id, worker_bits, seq_bits);
}

Result<Id> Generator::next() const {
    auto stamp = hlc_->now();
    if (!stamp.ok()) {
        return stamp.error;
    }
    const int64_t physical = (*stamp).first;
    const uint32_t sequence = (*stamp).second;
    auto elapsed = Id::check_epoch(physical - EPOCH_MS);
    if (!elapsed.ok()) {
        return elapsed.error;
    }
    const uint64_t value = (static_cast<uint64_t>(*elapsed) << PAYLOAD_BITS) |
                           (static_cast<uint64_t>(node_id_) << seq_bits_) |
                           (static_cast<uint64_t>(sequence) & ((static_cast<uint64_t>(1) << seq_bits_) - 1));
    return Id(value);
}

Status Generator::serve(const std::shared_ptr<Queue> &queue, const std::atomic<bool> &cancel) {
    if (queue == nullptr) {
        return Error::state_file("queue must not be null");
    }
    while (true) {
        if (cancel.load(std::memory_order_acquire)) {
            return Error::canceled();
        }
        if (queue->is_closed()) {
            return Error{};
        }
        auto produced = next();
        if (!produced.ok()) {
            return produced.error;
        }
        const Id id = *produced;
        // 队列满时重试同一个 ID (非阻塞 try_push), 每轮回到循环顶部检查取消/关闭,
        // 避免阻塞在满队列自旋上无法响应 cancel (对齐 Go 测试期望的 graceful drain)
        while (true) {
            if (cancel.load(std::memory_order_acquire)) {
                return Error::canceled();
            }
            if (queue->is_closed()) {
                return Error{};
            }
            Status pushed = queue->try_push(id);
            if (pushed.ok()) {
                break;
            }
            if (pushed.code == ErrorCode::Closed) {
                return Error{};
            }
            if (pushed.code == ErrorCode::QueueFull) {
                std::this_thread::yield();
                continue;
            }
            return pushed;
        }
    }
}

}  // namespace quant1x::distributed::id
