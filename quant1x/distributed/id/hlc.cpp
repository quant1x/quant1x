// 混合逻辑时钟 (HLC) 的实现
#include <quant1x/distributed/id/hlc.h>

#include <quant1x/distributed/id/id.h>

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <random>

namespace quant1x::distributed::id {

namespace {

/// seq 掩码
uint32_t seq_mask(uint8_t seq_bits) noexcept {
    return (1u << seq_bits) - 1u;
}

/// 进程级随机种子
///
/// 优先使用 std::random_device (Windows 为 CryptGenRandom, Unix 通常读取 /dev/urandom);
/// 熵源不可用时退化为 UnixNano 与 PID 混洗 (对齐 Rust/Go 的退化策略).
uint16_t random_u16() {
    try {
        std::random_device device;
        const uint32_t value = static_cast<uint32_t>(device());
        if (device.entropy() > 0.0 || value != 0) {
            return static_cast<uint16_t>(value & 0xFFFFu);
        }
    } catch (...) {
        // 退化为时间戳混洗
    }
    // 熵源不可用: 退化为纳秒时间戳的位混洗 (对应 Rust UnixNano ^ PID 的退化策略)
    const auto nanos = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
                                                 std::chrono::system_clock::now().time_since_epoch())
                                                 .count());
    return static_cast<uint16_t>((nanos ^ (nanos >> 16) ^ (nanos >> 32)) & 0xFFFFu);
}

/// 计算 count 所需的二进制位数 (对应 Go bits.Len(uint(count)))
uint8_t bit_length(uint32_t count) noexcept {
    uint8_t bits = 0;
    while (count > 0) {
        ++bits;
        count >>= 1;
    }
    return bits;
}

}  // namespace

int64_t unix_millis() noexcept {
    const auto now = std::chrono::system_clock::now();
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return static_cast<int64_t>(millis.count());
}

// ---------------- Hlc ----------------

Result<std::shared_ptr<Hlc>> Hlc::create(uint32_t node_count) {
    auto builder = HlcBuilder().with_node_count(node_count);
    if (!builder.ok()) {
        return builder.error;
    }
    return (*builder).build();
}

Result<std::pair<int64_t, uint32_t>> Hlc::now() {
    const int64_t now_ms = clock_();
    std::lock_guard<std::mutex> guard(mutex_);
    if (store_) {
        const PersistentState local{physical_, seq_};
        auto advanced = store_->next(local, now_ms, seq_bits_);
        if (!advanced.ok()) {
            return advanced.error;
        }
        physical_ = (*advanced).physical;
        seq_ = (*advanced).seq;
        return std::make_pair(physical_, seq_);
    }
    const uint32_t mask = seq_mask(seq_bits_);
    if (now_ms > physical_) {
        physical_ = now_ms;
        seq_ = 0;
    } else if (seq_ >= mask) {
        physical_ += 1;
        seq_ = 0;
    } else {
        seq_ += 1;
    }
    return std::make_pair(physical_, seq_);
}

int64_t Hlc::timestamp() {
    std::lock_guard<std::mutex> guard(mutex_);
    return physical_;
}

Status Hlc::close() {
    std::lock_guard<std::mutex> guard(mutex_);
    if (store_) {
        // 即使 flush 失败也继续释放缓存资源 (mmap/文件句柄)
        return store_->close();
    }
    return Error{};
}

// ---------------- HlcBuilder ----------------

HlcBuilder::HlcBuilder()
    : clock_(unix_millis),
      seed_(random_u16()),
      seq_bits_(DEFAULT_SEQ_BITS),
      sync_every_(default_sync_every_value()),
      strict_(false) {}

HlcBuilder &HlcBuilder::with_clock(std::function<int64_t()> clock) {
    clock_ = std::move(clock);
    return *this;
}

HlcBuilder &HlcBuilder::with_seq_seed(uint16_t seed) {
    seed_ = seed;
    return *this;
}

HlcBuilder &HlcBuilder::with_state_file(std::string path) {
    store_path_ = std::move(path);
    return *this;
}

HlcBuilder &HlcBuilder::with_state_sync_every(uint32_t every) {
    if (every > 0) {
        sync_every_ = every;
    }
    return *this;
}

HlcBuilder &HlcBuilder::with_state_strict() {
    strict_ = true;
    return *this;
}

Result<HlcBuilder> HlcBuilder::with_node_count(uint32_t count) const {
    const uint32_t normalized = count < 1 ? 1 : count;
    const uint8_t bits = bit_length(normalized);
    const int seq_bits = static_cast<int>(PAYLOAD_BITS) - static_cast<int>(bits);
    if (seq_bits < 4) {
        return Error::node_count_too_large(count);
    }
    HlcBuilder copy = *this;
    copy.seq_bits_ = static_cast<uint8_t>(seq_bits);
    return copy;
}

Result<HlcBuilder> HlcBuilder::with_seq_bits(uint8_t seq_bits) const {
    if (seq_bits < 4 || seq_bits > PAYLOAD_BITS - 1) {
        return Error::invalid_seq_bits(seq_bits);
    }
    HlcBuilder copy = *this;
    copy.seq_bits_ = seq_bits;
    return copy;
}

Result<std::shared_ptr<Hlc>> HlcBuilder::build() const {
    std::shared_ptr<FileStateStore> store;
    if (!store_path_.empty()) {
        store = std::make_shared<FileStateStore>(store_path_, sync_every_, strict_);
    }
    auto hlc = std::shared_ptr<Hlc>(new Hlc(clock_, seq_bits_, std::move(store)));
    bool restored = false;
    PersistentState restored_state{};
    if (hlc->store_) {
        auto loaded = hlc->store_->load();
        if (!loaded.ok()) {
            return loaded.error;
        }
        if ((*loaded).ok) {
            restored = true;
            restored_state = (*loaded).state;
        }
    }
    {
        std::lock_guard<std::mutex> guard(hlc->mutex_);
        if (restored) {
            hlc->physical_ = restored_state.physical;
            hlc->seq_ = restored_state.seq;
        } else {
            hlc->physical_ = clock_();
            hlc->seq_ = static_cast<uint32_t>(seed_) & seq_mask(seq_bits_);
        }
    }
    return hlc;
}

}  // namespace quant1x::distributed::id
