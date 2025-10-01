// Simple demo that creates a Vyukov MPMC queue and runs a few producers/consumers
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include "../quant1x/runtime/ringbuffer.h"

using namespace runtime::ringbuffer;

int main() {
    const size_t producers = 4;
    const size_t consumers = 4;
    const size_t items_per_producer = 100000;
    queue<uint64_t> q(1024);

    std::atomic<size_t> produced{0};
    std::atomic<size_t> consumed{0};

    auto start = std::chrono::steady_clock::now();

    std::vector<std::thread> ths;
    for (size_t p = 0; p < producers; ++p) {
        ths.emplace_back([&q, &produced]() {
            for (size_t i = 0; i < items_per_producer; ++i) {
                while (!q.try_push(uint64_t(i))) {
                    std::this_thread::yield();
                }
                ++produced;
            }
        });
    }

    for (size_t c = 0; c < consumers; ++c) {
        ths.emplace_back([&q, &consumed]() {
            uint64_t value;
            while (consumed.load() < producers * items_per_producer) {
                if (q.try_pop(value)) {
                    ++consumed;
                } else {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto &t : ths) t.join();
    auto end = std::chrono::steady_clock::now();
    q.close();

    double secs = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    std::cout << "Produced=" << produced.load() << " Consumed=" << consumed.load() << " time=" << secs << "s\n";
    std::cout << "Throughput=" << (static_cast<double>(produced.load()) / secs) << " ops/sec\n";
    return 0;
}
