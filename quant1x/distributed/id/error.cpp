// 分布式 ID 错误类型的实现 (消息文本对齐 Rust Error 的 Display 输出)
#include <quant1x/distributed/id/error.h>

namespace quant1x::distributed::id {

Error Error::invalid_size() {
    return Error(ErrorCode::InvalidSize, "invalid queue size");
}

Error Error::queue_full() {
    return Error(ErrorCode::QueueFull, "queue full");
}

Error Error::queue_empty() {
    return Error(ErrorCode::QueueEmpty, "queue empty");
}

Error Error::closed() {
    return Error(ErrorCode::Closed, "closed");
}

Error Error::canceled() {
    return Error(ErrorCode::Canceled, "canceled");
}

Error Error::epoch_elapsed_out_of_range(int64_t elapsed) {
    return Error(ErrorCode::EpochElapsedOutOfRange,
                 "epoch elapsed out of range: " + std::to_string(elapsed), elapsed);
}

Error Error::node_id_out_of_range(uint32_t node_id) {
    return Error(ErrorCode::NodeIdOutOfRange,
                 "node id out of range: " + std::to_string(node_id),
                 static_cast<int64_t>(node_id));
}

Error Error::invalid_seq_bits(uint8_t seq_bits) {
    return Error(ErrorCode::InvalidSeqBits,
                 "invalid seq bits: " + std::to_string(static_cast<int>(seq_bits)),
                 static_cast<int64_t>(seq_bits));
}

Error Error::node_count_too_large(uint32_t count) {
    return Error(ErrorCode::NodeCountTooLarge,
                 "node count too large: " + std::to_string(count),
                 static_cast<int64_t>(count));
}

Error Error::state_file(const std::string &detail) {
    return Error(ErrorCode::StateFile, "state file: " + detail);
}

Error Error::lock_poisoned() {
    return Error(ErrorCode::LockPoisoned, "mutex poisoned");
}

Error Error::parse_id(const std::string &text) {
    return Error(ErrorCode::ParseId, "parse id: " + text);
}

}  // namespace quant1x::distributed::id
