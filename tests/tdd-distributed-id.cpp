// 分布式 ID 的单元测试
//
// 语义对齐 Go id_test.go / Rust distributed/id/tests.rs / Python tests.py,
// 并补充跨语言二进制兼容性验证:
//   - CRC32-IEEE 已知向量 (与 Go hash/crc32 一致);
//   - base64url 编码的标准向量 (与 Go base64.RawURLEncoding / Python 一致);
//   - 旧版 18 字节追加式状态记录的迁移 (与 Go/Rust/Python 的文件格式一致).
#include "quant1x/test/test.h"

#include <quant1x/distributed/id/crc32.h>
#include <quant1x/distributed/id/error.h>
#include <quant1x/distributed/id/generator.h>
#include <quant1x/distributed/id/hlc.h>
#include <quant1x/distributed/id/id.h>
#include <quant1x/distributed/id/queue.h>
#include <quant1x/distributed/id/state_store.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

using namespace quant1x::distributed::id;

namespace {

/// 生成临时状态文件路径 (测试结束后由调用方清理)
std::string temp_state_path(const std::string &name) {
    const auto dir = std::filesystem::temp_directory_path() / "quant1x-id-test";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    return (dir / name).string();
}

/// 固定时钟: 始终返回给定的毫秒值
std::function<int64_t()> fixed_clock(int64_t millis) {
    return [millis]() -> int64_t { return millis; };
}

void write_be64(uint8_t *out, uint64_t value) {
    for (int i = 0; i < 8; ++i) {
        out[i] = static_cast<uint8_t>((value >> (56 - 8 * i)) & 0xFFu);
    }
}

void write_be32(uint8_t *out, uint32_t value) {
    for (int i = 0; i < 4; ++i) {
        out[i] = static_cast<uint8_t>((value >> (24 - 8 * i)) & 0xFFu);
    }
}

}  // namespace

TEST_CASE("distributed-id/crc32-known-vectors", "[distributed-id]") {
    // 与 Go crc32.ChecksumIEEE / Python zlib.crc32 的已知输出对照
    REQUIRE(crc32_ieee(nullptr, 0) == 0x00000000u);
    const std::string s1 = "123456789";
    REQUIRE(crc32_ieee(reinterpret_cast<const uint8_t *>(s1.data()), s1.size()) == 0xCBF43926u);
    const std::string s2 = "The quick brown fox jumps over the lazy dog";
    REQUIRE(crc32_ieee(reinterpret_cast<const uint8_t *>(s2.data()), s2.size()) == 0x414FA339u);
}

TEST_CASE("distributed-id/id-bytes-round-trip", "[distributed-id]") {
    const Id id(0x123456789ABCDEF0ull);
    uint8_t buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    id.bytes(buf);
    REQUIRE(Id::from_bytes(buf) == id);
}

TEST_CASE("distributed-id/id-base64url-standard-vectors", "[distributed-id]") {
    // 与 Go base64.RawURLEncoding / Python base64.urlsafe_b64encode 对齐的标准向量
    REQUIRE(Id(0).to_string() == "AAAAAAAAAAA");
    REQUIRE(Id(UINT64_MAX).to_string() == "__________8");

    // 往返: 覆盖 0 / 1 / 小值 / 典型值 / 最大值
    const uint64_t values[] = {0ull, 1ull, 42ull, 0x123456789ABCDEF0ull, UINT64_MAX};
    for (const uint64_t value : values) {
        const Id id(value);
        const std::string encoded = id.to_string();
        REQUIRE(encoded.size() == 11);
        const auto parsed = Id::parse(encoded);
        REQUIRE(parsed.ok());
        REQUIRE(*parsed == id);
    }
}

TEST_CASE("distributed-id/id-parse-rejects-invalid", "[distributed-id]") {
    REQUIRE_FALSE(Id::parse("").ok());
    REQUIRE_FALSE(Id::parse("AAAAAAAAAA").ok());        // 长度不足
    REQUIRE_FALSE(Id::parse("AAAAAAAAAAAA").ok());      // 长度超出
    REQUIRE_FALSE(Id::parse("AAAAAAAAAA*").ok());       // 非法字符
    REQUIRE(Id::parse("AAAAAAAAAAA").ok());
}

TEST_CASE("distributed-id/id-field-extraction", "[distributed-id]") {
    // elapsed=123, node=7, seq=0 (worker_bits=11, seq_bits=11)
    const uint64_t raw = (123ull << 22) | (7ull << 11);
    const Id id(raw);
    REQUIRE(id.physical() == 123);
    REQUIRE(id.node_id(11) == 7);
    REQUIRE(id.seq(11) == 0);
}

TEST_CASE("distributed-id/id-check-epoch", "[distributed-id]") {
    REQUIRE(*Id::check_epoch(0) == 0);
    REQUIRE(*Id::check_epoch((static_cast<int64_t>(1) << 41) - 1) ==
            (static_cast<int64_t>(1) << 41) - 1);
    REQUIRE_FALSE(Id::check_epoch(-1).ok());
    REQUIRE(Id::check_epoch(-1).error.code == ErrorCode::EpochElapsedOutOfRange);
    REQUIRE_FALSE(Id::check_epoch(static_cast<int64_t>(1) << 41).ok());
}

TEST_CASE("distributed-id/generator-default-increasing", "[distributed-id]") {
    const auto hlc = HlcBuilder().with_clock(fixed_clock(EPOCH_MS)).build();
    REQUIRE(hlc.ok());
    const auto generator = Generator::create(1, *hlc);
    REQUIRE(generator.ok());
    const auto first = (*generator).next();
    REQUIRE(first.ok());
    const auto second = (*generator).next();
    REQUIRE(second.ok());
    REQUIRE(*second > *first);
    REQUIRE((*first).node_id((*generator).worker_bits()) == 1);
}

TEST_CASE("distributed-id/generator-node-id-out-of-range", "[distributed-id]") {
    // 默认 seq_bits=11 → worker_bits=11, 节点编号上限为 2048
    const auto hlc = HlcBuilder().with_clock(fixed_clock(EPOCH_MS)).build();
    REQUIRE(hlc.ok());
    REQUIRE(Generator::create(0, *hlc).ok());
    REQUIRE(Generator::create(2047, *hlc).ok());
    const auto invalid = Generator::create(2048, *hlc);
    REQUIRE_FALSE(invalid.ok());
    REQUIRE(invalid.error.code == ErrorCode::NodeIdOutOfRange);
}

TEST_CASE("distributed-id/hlc-rollback-monotonic", "[distributed-id]") {
    std::atomic<int64_t> now{EPOCH_MS};
    const auto hlc = HlcBuilder()
                         .with_clock([&now]() -> int64_t { return now.load(std::memory_order_seq_cst); })
                         .with_seq_seed(9)
                         .build();
    REQUIRE(hlc.ok());
    const auto first = (*hlc)->now();
    REQUIRE(first.ok());
    now.store(EPOCH_MS - 1, std::memory_order_seq_cst);
    const auto second = (*hlc)->now();
    REQUIRE(second.ok());
    const bool monotonic = (*second).first > (*first).first ||
                           ((*second).first == (*first).first && (*second).second > (*first).second);
    REQUIRE(monotonic);
}

TEST_CASE("distributed-id/hlc-node-count-derivation", "[distributed-id]") {
    struct Case {
        uint32_t count;
        uint8_t worker_bits;
        uint8_t seq_bits;
    };
    const Case cases[] = {
        {1024u, 11u, 11u},
        {5000u, 13u, 9u},
        {3u, 2u, 20u},
        {131072u, 18u, 4u},
    };
    for (const auto &c : cases) {
        const auto builder = HlcBuilder().with_node_count(c.count);
        REQUIRE(builder.ok());
        const auto hlc = (*builder).build();
        REQUIRE(hlc.ok());
        const auto generator = Generator::create(0, *hlc);
        REQUIRE(generator.ok());
        REQUIRE((*generator).worker_bits() == c.worker_bits);
        REQUIRE((*hlc)->seq_bits() == c.seq_bits);
    }
}

TEST_CASE("distributed-id/hlc-node-count-too-large", "[distributed-id]") {
    // count 需要 19 位时, seq_bits = 22 - 19 = 3 < 4, 应被拒绝
    const auto builder = HlcBuilder().with_node_count(262144u);
    REQUIRE_FALSE(builder.ok());
    REQUIRE(builder.error.code == ErrorCode::NodeCountTooLarge);
}

TEST_CASE("distributed-id/hlc-seq-bits-range", "[distributed-id]") {
    REQUIRE(HlcBuilder().with_seq_bits(4).ok());
    REQUIRE(HlcBuilder().with_seq_bits(21).ok());
    const auto too_small = HlcBuilder().with_seq_bits(3);
    REQUIRE_FALSE(too_small.ok());
    REQUIRE(too_small.error.code == ErrorCode::InvalidSeqBits);
    const auto too_large = HlcBuilder().with_seq_bits(22);
    REQUIRE_FALSE(too_large.ok());
    REQUIRE(too_large.error.code == ErrorCode::InvalidSeqBits);
}

TEST_CASE("distributed-id/generator-id-fields-and-encoding", "[distributed-id]") {
    const auto hlc = HlcBuilder().with_clock(fixed_clock(EPOCH_MS + 123)).build();
    REQUIRE(hlc.ok());
    const auto generator = Generator::create(7, *hlc);
    REQUIRE(generator.ok());
    const auto id = (*generator).next();
    REQUIRE(id.ok());
    REQUIRE((*id).physical() == 123);
    REQUIRE((*id).node_id((*generator).worker_bits()) == 7);
    uint8_t buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    (*id).bytes(buf);
    REQUIRE(Id::from_bytes(buf) == *id);
    REQUIRE((*id).to_string().size() == 11);
    const auto parsed = Id::parse((*id).to_string());
    REQUIRE(parsed.ok());
    REQUIRE(*parsed == *id);
}

TEST_CASE("distributed-id/generator-concurrent-unique", "[distributed-id]") {
    const auto hlc = HlcBuilder().with_clock(fixed_clock(EPOCH_MS)).build();
    REQUIRE(hlc.ok());
    auto generator = std::make_shared<Generator>(*Generator::create(1, *hlc));
    constexpr size_t kCount = 10000;
    constexpr size_t kThreads = 8;
    std::vector<std::vector<uint64_t>> buckets(kThreads);
    std::vector<std::thread> threads;
    for (size_t t = 0; t < kThreads; ++t) {
        threads.emplace_back([&generator, &buckets, t]() {
            buckets[t].reserve(kCount / kThreads);
            for (size_t i = 0; i < kCount / kThreads; ++i) {
                const auto id = generator->next();
                if (!id.ok()) {
                    return;
                }
                buckets[t].push_back((*id).value());
            }
        });
    }
    for (auto &thread : threads) {
        thread.join();
    }
    std::set<uint64_t> seen;
    size_t total = 0;
    for (const auto &bucket : buckets) {
        total += bucket.size();
        for (const uint64_t value : bucket) {
            const bool inserted = seen.insert(value).second;
            REQUIRE(inserted);
        }
    }
    REQUIRE(total == kCount);
    REQUIRE(seen.size() == kCount);
}

TEST_CASE("distributed-id/state-file-across-restart", "[distributed-id]") {
    const std::string path = temp_state_path("restart.bin");
    std::error_code ec;
    std::filesystem::remove(path, ec);
    const int64_t now = EPOCH_MS;

    std::pair<int64_t, uint32_t> first_stamp{0, 0};
    {
        const auto hlc = HlcBuilder()
                             .with_clock(fixed_clock(now))
                             .with_state_file(path)
                             .with_seq_seed(9)
                             .build();
        REQUIRE(hlc.ok());
        const auto first = (*hlc)->now();
        REQUIRE(first.ok());
        first_stamp = *first;
        REQUIRE((*hlc)->close().ok());
    }
    {
        const auto hlc = HlcBuilder()
                             .with_clock(fixed_clock(now))
                             .with_state_file(path)
                             .with_seq_seed(9)
                             .build();
        REQUIRE(hlc.ok());
        const auto second = (*hlc)->now();
        REQUIRE(second.ok());
        REQUIRE((*hlc)->close().ok());
        const bool monotonic = (*second).first > first_stamp.first ||
                               ((*second).first == first_stamp.first &&
                                (*second).second > first_stamp.second);
        REQUIRE(monotonic);
    }
    std::filesystem::remove(path, ec);
}

TEST_CASE("distributed-id/state-file-size-is-fixed", "[distributed-id]") {
    const std::string path = temp_state_path("fixed-size.bin");
    std::error_code ec;
    std::filesystem::remove(path, ec);
    {
        const auto hlc = HlcBuilder()
                             .with_clock(fixed_clock(EPOCH_MS))
                             .with_state_file(path)
                             .with_state_sync_every(1)
                             .build();
        REQUIRE(hlc.ok());
        REQUIRE((*hlc)->now().ok());
        REQUIRE((*hlc)->close().ok());
    }
    std::error_code size_ec;
    const uint64_t size = std::filesystem::file_size(path, size_ec);
    REQUIRE_FALSE(static_cast<bool>(size_ec));
    REQUIRE(size == STATE_FILE_SIZE);
    std::filesystem::remove(path, ec);
}

TEST_CASE("distributed-id/state-legacy-migration", "[distributed-id]") {
    // 构造旧版 18 字节追加式记录, 验证迁移路径与 Go/Rust/Python 的文件格式兼容
    const std::string path = temp_state_path("legacy.bin");
    std::error_code ec;
    std::filesystem::remove(path, ec);
    constexpr int64_t kPhysical = EPOCH_MS + 1000;
    constexpr uint32_t kSeq = 42;

    uint8_t record[LEGACY_RECORD_SIZE];
    std::memset(record, 0, sizeof(record));
    write_be64(record, static_cast<uint64_t>(kPhysical));
    // record[8..10) 为保留字段, 保持 0
    write_be32(record + 10, kSeq);
    const uint32_t checksum = crc32_ieee(record, 14);
    write_be32(record + 14, checksum);
    {
        std::ofstream out(path, std::ios::binary);
        REQUIRE(static_cast<bool>(out));
        out.write(reinterpret_cast<const char *>(record), static_cast<std::streamsize>(sizeof(record)));
    }

    FileStateStore store(path, 1000, false);
    const auto loaded = store.load();
    REQUIRE(loaded.ok());
    REQUIRE((*loaded).ok);
    REQUIRE((*loaded).state.physical == kPhysical);
    REQUIRE((*loaded).state.seq == kSeq);
    REQUIRE(store.close().ok());
    std::filesystem::remove(path, ec);
}

TEST_CASE("distributed-id/state-advance-persistent-state", "[distributed-id]") {
    // 时钟前进: 重置 seq
    const PersistentState base{1000, 7};
    const auto advanced = advance_persistent_state(base, 1001, 11);
    REQUIRE(advanced.physical == 1001);
    REQUIRE(advanced.seq == 0);
    // 时钟回拨: seq 递增
    const auto rolled = advance_persistent_state(base, 999, 11);
    REQUIRE(rolled.physical == 1000);
    REQUIRE(rolled.seq == 8);
    // seq 用尽: 借位到 physical + 1 (seq_bits=1 时掩码为 1)
    const PersistentState saturated{1000, 1};
    const auto borrowed = advance_persistent_state(saturated, 1000, 1);
    REQUIRE(borrowed.physical == 1001);
    REQUIRE(borrowed.seq == 0);
    // 比较
    REQUIRE(compare_persistent_state(PersistentState{1000, 1}, PersistentState{1000, 2}) == -1);
    REQUIRE(compare_persistent_state(PersistentState{1001, 0}, PersistentState{1000, 9}) == 1);
    REQUIRE(compare_persistent_state(PersistentState{1000, 3}, PersistentState{1000, 3}) == 0);
}

TEST_CASE("distributed-id/queue-uses-vyukov-ring-buffer", "[distributed-id]") {
    const auto queue = Queue::create(3);
    REQUIRE(queue.ok());
    REQUIRE((*queue)->cap() == 4);
    const Id value(42);
    REQUIRE((*queue)->try_push(value).ok());
    const auto popped = (*queue)->try_pop();
    REQUIRE(popped.ok());
    REQUIRE(*popped == value);
    REQUIRE((*queue)->try_pop().error.code == ErrorCode::QueueEmpty);
    (*queue)->close();
    REQUIRE((*queue)->try_pop().error.code == ErrorCode::Closed);
    const Status after_close = (*queue)->try_push(value);
    REQUIRE(after_close.code == ErrorCode::Closed);
}

TEST_CASE("distributed-id/queue-invalid-size", "[distributed-id]") {
    const auto queue = Queue::create(0);
    REQUIRE_FALSE(queue.ok());
    REQUIRE(queue.error.code == ErrorCode::InvalidSize);
}

TEST_CASE("distributed-id/queue-full", "[distributed-id]") {
    // 注意: Vyukov 队列在容量为 1 时, 槽序号在写入后仍等于下一个入队位置,
    // 连续两次 push 会互相覆盖 (Go/Rust 的 runtime::Queue 同此行为),
    // 因此用容量 2 验证"队满"语义.
    const auto queue = Queue::create(2);
    REQUIRE(queue.ok());
    REQUIRE((*queue)->cap() == 2);
    REQUIRE((*queue)->try_push(Id(1)).ok());
    REQUIRE((*queue)->try_push(Id(2)).ok());
    const auto full = (*queue)->try_push(Id(3));
    REQUIRE_FALSE(full.ok());
    REQUIRE(full.code == ErrorCode::QueueFull);
    REQUIRE((*queue)->len() == 2);
    // 弹出一个后应能再次推入
    const auto popped = (*queue)->try_pop();
    REQUIRE(popped.ok());
    REQUIRE(*popped == Id(1));
    REQUIRE((*queue)->try_push(Id(4)).ok());
}

TEST_CASE("distributed-id/queue-graceful-drain", "[distributed-id]") {
    const auto queue = Queue::create(4);
    REQUIRE(queue.ok());
    REQUIRE((*queue)->try_push(Id(1)).ok());
    REQUIRE((*queue)->try_push(Id(2)).ok());
    (*queue)->close();
    // 关闭后存量仍可消费, 排空后返回 Closed
    const auto first = (*queue)->try_pop();
    REQUIRE(first.ok());
    REQUIRE(*first == Id(1));
    const auto second = (*queue)->try_pop();
    REQUIRE(second.ok());
    REQUIRE(*second == Id(2));
    REQUIRE((*queue)->try_pop().error.code == ErrorCode::Closed);
    REQUIRE((*queue)->is_closed());
}

TEST_CASE("distributed-id/generator-serve-feeds-queue", "[distributed-id]") {
    const auto hlc = HlcBuilder().with_clock(fixed_clock(EPOCH_MS)).build();
    REQUIRE(hlc.ok());
    auto generator = std::make_shared<Generator>(*Generator::create(1, *hlc));
    const auto queue = Queue::create(1024);
    REQUIRE(queue.ok());
    auto shared_queue = *queue;
    std::atomic<bool> cancel{false};

    Status serve_status;
    std::thread producer([&generator, shared_queue, &cancel, &serve_status]() {
        serve_status = generator->serve(shared_queue, cancel);
    });

    constexpr size_t kCount = 4096;
    Id previous{0};
    size_t index = 0;
    while (index < kCount) {
        const auto popped = shared_queue->try_pop();
        if (popped.ok()) {
            if (index > 0) {
                REQUIRE(*popped > previous);
            }
            previous = *popped;
            ++index;
        } else if (popped.error.code == ErrorCode::QueueEmpty) {
            std::this_thread::yield();
        } else {
            FAIL("try_pop at " << index << ": " << popped.error.what());
        }
    }
    cancel.store(true, std::memory_order_seq_cst);
    producer.join();
    REQUIRE(serve_status.code == ErrorCode::Canceled);
}

TEST_CASE("distributed-id/generator-serve-stops-on-closed-queue", "[distributed-id]") {
    const auto hlc = HlcBuilder().with_clock(fixed_clock(EPOCH_MS)).build();
    REQUIRE(hlc.ok());
    auto generator = std::make_shared<Generator>(*Generator::create(1, *hlc));
    const auto queue = Queue::create(4);
    REQUIRE(queue.ok());
    (*queue)->close();
    std::atomic<bool> cancel{false};
    const Status status = generator->serve(*queue, cancel);
    REQUIRE(status.ok());
}

TEST_CASE("distributed-id/generator-serve-drain-after-cancel", "[distributed-id]") {
    const auto hlc = HlcBuilder().with_clock(fixed_clock(EPOCH_MS)).build();
    REQUIRE(hlc.ok());
    auto generator = std::make_shared<Generator>(*Generator::create(1, *hlc));
    const auto queue = Queue::create(8);
    REQUIRE(queue.ok());
    auto shared_queue = *queue;
    std::atomic<bool> cancel{false};

    Status serve_status;
    std::thread producer([&generator, shared_queue, &cancel, &serve_status]() {
        serve_status = generator->serve(shared_queue, cancel);
    });

    // 等待生产者至少填入一个 ID
    while (shared_queue->len() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    cancel.store(true, std::memory_order_seq_cst);
    producer.join();
    REQUIRE(serve_status.code == ErrorCode::Canceled);

    // 关闭队列进入只读排空
    shared_queue->close();
    Id last{0};
    while (true) {
        const auto popped = shared_queue->try_pop();
        if (popped.ok()) {
            REQUIRE(*popped > last);
            last = *popped;
            continue;
        }
        REQUIRE(popped.error.code == ErrorCode::Closed);
        break;
    }
}
