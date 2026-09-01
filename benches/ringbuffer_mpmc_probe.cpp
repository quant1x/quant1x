// Vyukov MPMC 队列的多线程争抢探针 (不依赖 Google Benchmark, 单文件可直接 cl / clang-cl 编译)
//
// 存在理由: scripts/msvc_mini_bench.bat 的单线程消融测不出缓存行争用. 2026-09 定位
// Windows x86_64 性能问题时, 单线程 push+pop 在"带 lock 的 relaxed 读游标"与"普通
// mov 读游标"两种实现下几乎无差别, 而 8P8C 下相差数倍 —— 因为无跨核争用时 lock
// 指令只是一次本地 RMW, 跨核失效成本不会显现. 因此本探针固定跑多生产者/多消费者.
//
// 设计与 benches/vyukov_bench.cpp 一致以复用其已验证的防测量伪影措施:
// - 消费者用**每线程本地计数**, join 后合并, 不共享原子计数器
//   (共享计数器会与 pop 形成双竞争点, 放大 backlog, 属测量伪影)
// - 生产者 push 队满时 yield, 与 Rust 侧 yield_now 对齐
// - 结果断言 consumed == TOTAL, Release 下同样生效
//
// 用法: 见 scripts/msvc_mpmc_probe.bat (同时构建 cl 与 clang-cl 两套做对比)

#include <quant1x/runtime/ringbuffer.h>

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <thread>
#include <vector>

using namespace runtime::ringbuffer;

namespace {

constexpr size_t NUM_PRODUCERS = 8;
constexpr size_t NUM_CONSUMERS = 8;
// 每生产者 20 万条(单轮 160 万): 比 benches/vyukov_bench.cpp 的 2 万条大 10 倍.
// 原因: 16 线程创建+join 约 1ms 且在 Windows 上抖动大, 单轮仅 16 万条时该固定开销
// 占比过高(produce ~5ms 中相当部分是启动斜坡), 轮间方差掩盖真实的队列吞吐差异.
// 仍满足 DATA * NUM_PRODUCERS > CAPACITY_UNCONTENDED 之外的约束: 单轮总量必须小于
// 无背压容量, 否则"无背压"场景会退化成背压场景(全程队满, 两档测出同样结果).
// 故无背压容量同步上调到 1<<21 (2097152 > 1600000).
constexpr int64_t DATA_PER_PRODUCER = 200'000;
constexpr int64_t TOTAL = static_cast<int64_t>(NUM_PRODUCERS) * DATA_PER_PRODUCER;

// 背压场景: 容量远小于单轮总量, push 必然阻塞等待消费者腾位
constexpr size_t CAPACITY_BACKPRESSURE = 1024;
// 无背压场景: 容量必须大于单轮总量(160 万), 否则 push 全程队满, 两档测出同样结果
constexpr size_t CAPACITY_UNCONTENDED = 1 << 21;

struct round_result {
    double ctor_ms;      // 队列构造(槽区分配 + 逐槽写 seq)
    double produce_ms;   // 生产阶段(全部生产者启动 → 全部生产者结束)
    double drain_ms;     // 排空阶段(生产者结束 → 全部消费者退出)
    // 纯队列操作耗时 = produce + drain, 不含 ctor.
    // ctor 随容量线性增长(uncontended 容量 1<<21 时达 25ms, 与 backpressure 的
    // 1<<10 相差三个数量级), 计入会让两档吞吐无法直接比较 —— 两者测的都是
    // 稳态队列吞吐, 构造是一次性成本, 不属于队列操作.
    double ops_ms;
    int64_t consumed;
};

round_result run_round(size_t capacity) {
    // 阶段计时: 整轮耗时相近但吞吐相差一个量级时, 固定开销(构造/调度/退避)与
    // 真实队列吞吐必须分开看, 否则会把调度成本误判成队列缺陷.
    const auto t_ctor0 = std::chrono::steady_clock::now();
    queue<int64_t> q(capacity);
    const auto t_ctor1 = std::chrono::steady_clock::now();
    std::vector<int64_t> per_consumer(NUM_CONSUMERS, 0);

    const auto t0 = std::chrono::steady_clock::now();

    std::vector<std::thread> producers;
    producers.reserve(NUM_PRODUCERS);
    for (size_t id = 0; id < NUM_PRODUCERS; ++id) {
        producers.emplace_back([&q, id]() {
            const int64_t base = static_cast<int64_t>(id) * DATA_PER_PRODUCER;
            for (int64_t index = 0; index < DATA_PER_PRODUCER; ++index) {
                while (!q.push(base + index)) {
                    std::this_thread::yield();
                }
            }
        });
    }

    std::vector<std::thread> consumers;
    consumers.reserve(NUM_CONSUMERS);
    for (size_t c = 0; c < NUM_CONSUMERS; ++c) {
        consumers.emplace_back([&q, &per_consumer, c]() {
            int64_t v;
            int64_t local = 0;
            while (q.pop(v)) {
                ++local;
            }
            per_consumer[c] = local;
        });
    }

    for (auto& t : producers) {
        t.join();
    }
    const auto t1 = std::chrono::steady_clock::now();
    q.close();
    for (auto& t : consumers) {
        t.join();
    }

    const auto t2 = std::chrono::steady_clock::now();

    int64_t consumed = 0;
    for (size_t c = 0; c < NUM_CONSUMERS; ++c) {
        consumed += per_consumer[c];
    }
    const double produce_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    const double drain_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
    return {std::chrono::duration<double, std::milli>(t_ctor1 - t_ctor0).count(),
            produce_ms, drain_ms, produce_ms + drain_ms, consumed};
}

void bench(const char* name, size_t capacity, int rounds) {
    double best_ms = 0.0;
    for (int r = 0; r < rounds; ++r) {
        const round_result res = run_round(capacity);
        if (res.consumed != TOTAL) {
            std::printf("%-16s FAILED: consumed=%lld expected=%lld\n", name,
                        static_cast<long long>(res.consumed), static_cast<long long>(TOTAL));
            return;
        }
        best_ms = (r == 0 || res.ops_ms < best_ms) ? res.ops_ms : best_ms;
        std::printf("%-16s round %d: ctor %6.2f | produce %7.2f | drain %7.2f ms | %6.1f M/s\n",
                    name, r + 1, res.ctor_ms, res.produce_ms, res.drain_ms,
                    static_cast<double>(TOTAL) / (res.ops_ms / 1000.0) / 1e6);
    }
    std::printf("%-16s BEST   : %7.2f ms (produce+drain) | %6.1f M/s\n\n", name, best_ms,
                static_cast<double>(TOTAL) / (best_ms / 1000.0) / 1e6);
}

}  // namespace

int main() {
    std::printf("producers=%zu consumers=%zu per_producer=%lld total=%lld\n\n",
                NUM_PRODUCERS, NUM_CONSUMERS, static_cast<long long>(DATA_PER_PRODUCER),
                static_cast<long long>(TOTAL));
    bench("backpressure", CAPACITY_BACKPRESSURE, 5);
    bench("uncontended", CAPACITY_UNCONTENDED, 5);
    return 0;
}
