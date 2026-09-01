// Vyukov 有界 MPMC 队列 (quant1x/runtime/ringbuffer.h) 的 C++ 基准测试
// 对应 Rust 版本: benches/vyukov_bench.rs (Criterion), 使用 Google Benchmark.
//
// 场景与参数与 Rust 版严格对齐, 确保跨语言对比可复现:
// - 8 生产者 / 8 消费者, 每生产者 20'000 条 i64, 单轮总量 160'000
// - mpmc_backpressure: 容量 1024, push 必因队满阻塞等待消费者腾位
// - mpmc_uncontended : 容量 1<<18, 队列永不满, 测量无背压吞吐
// - thread_overhead  : 16 线程创建+join 开销 (差分扣除用, 与 Rust 版一致)
//
// 差分测量: 每 iteration 先跑完整生产-消费轮次, 再跑等量空转线程创建,
// 两者耗时之差作为队列纯操作耗时 (对齐 Rust iter_custom 的 saturating_sub).

#include <benchmark/benchmark.h>
#include <chrono>
#include <cstdint>
#include <thread>
#include <vector>
#include <atomic>

#include <quant1x/runtime/ringbuffer.h>

using namespace runtime::ringbuffer;

namespace {

constexpr size_t NUM_PRODUCERS = 8;
constexpr size_t NUM_CONSUMERS = 8;
/// 每个生产者单轮生产条数: 取较大值以摊薄线程创建开销 (与 Rust 版一致)
constexpr int64_t DATA_PER_PRODUCER = 20'000;
/// 单轮总元素数: 结果断言与吞吐统计均以此为准, 杜绝硬编码脱节
constexpr int64_t TOTAL = static_cast<int64_t>(NUM_PRODUCERS) * DATA_PER_PRODUCER;

/// 背压场景: 容量远小于单轮总量, push 必然阻塞等待消费者腾位
constexpr size_t CAPACITY_BACKPRESSURE = 1024;
/// 无背压场景: 容量大于单轮总量, push 不会阻塞 (每个 slot 64B, 约 16MB)
constexpr size_t CAPACITY_UNCONTENDED = 1 << 18;

/// 运行一轮完整的生产-消费, 返回消费总数 (对齐 Rust run_round)
///
/// 生产者: push 队满时让出时间片等待消费者腾位 (对齐 Rust yield_now)
/// 消费者: pop 仅在队列关闭且为空时返回 false (对齐 Rust: Err 仅关闭且空),
///         消费成功后用 relaxed 原子累加计数
int64_t run_round(size_t capacity) {
    queue<int64_t> q(capacity);
    std::atomic<int64_t> consumed{0};

    std::vector<std::thread> producers;
    producers.reserve(NUM_PRODUCERS);
    for (size_t id = 0; id < NUM_PRODUCERS; ++id) {
        producers.emplace_back([&q, id]() {
            const int64_t base = static_cast<int64_t>(id) * DATA_PER_PRODUCER;
            for (int64_t index = 0; index < DATA_PER_PRODUCER; ++index) {
                // push 写共享内存, 不会被优化掉; 热路径不放 DoNotOptimize 以免干扰测量
                while (!q.push(base + index)) {
                    // 队满: 让出时间片等待消费者腾位 (与 Rust yield_now 一致)
                    std::this_thread::yield();
                }
            }
        });
    }

    std::vector<std::thread> consumers;
    consumers.reserve(NUM_CONSUMERS);
    for (size_t c = 0; c < NUM_CONSUMERS; ++c) {
        consumers.emplace_back([&q, &consumed]() {
            int64_t v;
            while (q.pop(v)) {
                consumed.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t : producers) {
        t.join();
    }
    q.close(); // 关闭队列, 消费者排空存量后退出
    for (auto& t : consumers) {
        t.join();
    }
    return consumed.load(std::memory_order_acquire);
}

/// 创建并回收与一轮生产消费等量的线程, 但不做任何队列操作
///
/// 用于差分扣除线程创建开销 (对齐 Rust spawn_and_join_idle_threads):
/// Windows 上 16 个线程的创建+join 约 1ms, 在单轮耗时中占比可达 10%,
/// 不扣除会把线程调度成本算进队列吞吐.
void spawn_and_join_idle_threads() {
    std::vector<std::thread> handles;
    handles.reserve(NUM_PRODUCERS + NUM_CONSUMERS);
    for (size_t i = 0; i < NUM_PRODUCERS + NUM_CONSUMERS; ++i) {
        handles.emplace_back([]() {
            benchmark::DoNotOptimize(0);
        });
    }
    for (auto& h : handles) {
        h.join();
    }
}

/// MPMC 差分基准: 完整轮次耗时减去等量空转线程开销 = 纯队列操作耗时
/// (对齐 Rust bench_mpmc 的 iter_custom + saturating_sub)
void bench_mpmc(benchmark::State& state) {
    if (state.error_occurred()) {
        return;
    }
    const size_t capacity = static_cast<size_t>(state.range(0));
    int64_t consumed_total = 0;
    for (auto _ : state) {
        // 完整轮次 (含线程创建与回收)
        const auto t0 = std::chrono::steady_clock::now();
        const int64_t consumed = run_round(capacity);
        const auto t1 = std::chrono::steady_clock::now();
        // 每轮必须消费全部元素 (对齐 Rust 的 assert_eq!, Release 下同样生效)
        if (consumed != TOTAL) {
            state.SkipWithError("每轮必须消费全部元素, 数据完整性断言失败");
            return;
        }
        consumed_total += consumed;

        // 等量轮次的空转线程开销, 从总耗时中扣除, 得到纯队列操作耗时
        const auto t2 = std::chrono::steady_clock::now();
        spawn_and_join_idle_threads();
        const auto t3 = std::chrono::steady_clock::now();

        const auto queue_us = (t1 - t0) - (t3 - t2);
        state.SetIterationTime(std::chrono::duration<double>(queue_us).count());
    }
    benchmark::DoNotOptimize(consumed_total);
    state.SetItemsProcessed(consumed_total);
}

/// 对照基准: 16 个线程的创建+join 开销
///
/// 两个 mpmc 基准已用差分扣除该开销, 本项单独保留是为了让开销可见,
/// 便于判断单轮耗时中线程调度的占比是否仍需要进一步优化
/// (对齐 Rust bench_thread_overhead).
void BM_thread_overhead(benchmark::State& state) {
    for (auto _ : state) {
        spawn_and_join_idle_threads();
    }
}

} // namespace

BENCHMARK(bench_mpmc)
    ->UseManualTime() // 与 SetIterationTime 配合, 使用扣除后的纯队列耗时
    ->Unit(benchmark::kMicrosecond)
    ->Arg(CAPACITY_BACKPRESSURE)
    ->Arg(CAPACITY_UNCONTENDED);

BENCHMARK(BM_thread_overhead);

BENCHMARK_MAIN();
