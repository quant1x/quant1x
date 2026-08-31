// Vyukov 有界 MPMC 队列演示: 多生产者/多消费者并发生产消费并统计吞吐
//
// 本文件已注册为 CTest 用例 (见 examples/CMakeLists.txt), 因此除打印统计信息外
// 还校验并发语义: 生产总数与消费总数均须等于预期值 (无丢失、无重复消费),
// 校验失败以非 0 退出码结束, 避免"永远通过"的无效测试.
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <thread>
#include <vector>

#include <quant1x/runtime/ringbuffer.h>

using namespace runtime::ringbuffer;

int main() {
    constexpr size_t kProducers = 4;
    constexpr size_t kConsumers = 4;
    constexpr size_t kItemsPerProducer = 100000;
    constexpr size_t kTotal = kProducers * kItemsPerProducer;
    // 超时保护: 消费者以"消费总数达到预期"为退出条件, 若生产者异常终止 (线程
    // 崩溃等) 消费者会无限自旋. 作为 CTest 用例必须避免挂死整个测试套件.
    constexpr std::chrono::seconds kTimeout{60};

    queue<uint64_t> q(1024);
    std::atomic<size_t> produced{0};
    std::atomic<size_t> consumed{0};
    const auto deadline = std::chrono::steady_clock::now() + kTimeout;

    const auto start = std::chrono::steady_clock::now();

    std::vector<std::thread> ths;
    ths.reserve(kProducers + kConsumers);

    for (size_t p = 0; p < kProducers; ++p) {
        ths.emplace_back([&q, &produced]() {
            for (size_t i = 0; i < kItemsPerProducer; ++i) {
                // 队列满时让出时间片, 等待消费者腾位
                while (!q.try_push(static_cast<uint64_t>(i))) {
                    std::this_thread::yield();
                }
                ++produced;
            }
        });
    }

    for (size_t c = 0; c < kConsumers; ++c) {
        ths.emplace_back([&q, &consumed, deadline]() {
            uint64_t value = 0;
            while (consumed.load(std::memory_order_relaxed) < kTotal) {
                if (q.try_pop(value)) {
                    consumed.fetch_add(1, std::memory_order_relaxed);
                    continue;
                }
                if (std::chrono::steady_clock::now() >= deadline) {
                    // 超时退出, 由主线程的计数校验报错
                    return;
                }
                std::this_thread::yield();
            }
        });
    }

    for (auto &t : ths) {
        t.join();
    }
    const auto end = std::chrono::steady_clock::now();
    q.close();

    const double secs = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    const size_t produced_count = produced.load();
    const size_t consumed_count = consumed.load();
    std::cout << "Produced=" << produced_count << " Consumed=" << consumed_count << " time=" << secs << "s\n";
    if (secs > 0) {
        std::cout << "Throughput=" << (static_cast<double>(produced_count) / secs) << " ops/sec\n";
    }

    // 并发语义校验: 生产总数与消费总数都必须等于预期, 否则说明存在丢失或重复
    if (produced_count != kTotal || consumed_count != kTotal) {
        std::cerr << "vyukov_demo: expected produced=consumed=" << kTotal << ", got produced=" << produced_count
                  << " consumed=" << consumed_count << "\n";
        return 1;
    }
    return 0;
}
