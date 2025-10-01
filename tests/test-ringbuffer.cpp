// 运行多个 Vyukov MPMC 工作负载采样，并将每次运行的 ops/sec 写入 CSV。
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <string>
#include <quant1x/runtime/ringbuffer.h>

using namespace runtime::ringbuffer;

int main() {
    const size_t producers = 4;
    const size_t consumers = 4;
    const size_t items_per_producer = 1000000; // 1M
    const int samples = 10;
    const std::string out_path = "cpp_perf_samples.csv";

    std::ofstream ofs(out_path, std::ofstream::out | std::ofstream::trunc);
    if (!ofs) {
        std::cerr << "Failed to open output file: " << out_path << "\n";
        return 1;
    }
    ofs << "ops_per_sec" << std::endl;

    for (int s = 0; s < samples; ++s) {
        queue<uint64_t> q(1024*8);
        std::atomic<size_t> produced{0};
        std::atomic<size_t> consumed{0};

        auto start = std::chrono::steady_clock::now();

        std::vector<std::thread> ths;
        ths.reserve(producers + consumers);

        for (size_t p = 0; p < producers; ++p) {
            ths.emplace_back([&q, &produced]() {
                for (size_t i = 0; i < items_per_producer; ++i) {
                    while (!q.try_push(uint64_t(i))) std::this_thread::yield();
                    ++produced;
                }
            });
        }

        for (size_t c = 0; c < consumers; ++c) {
            ths.emplace_back([&q, &consumed]() {
                uint64_t tmp;
                while (consumed.load(std::memory_order_relaxed) < producers * items_per_producer) {
                    if (q.try_pop(tmp)) {
                        consumed.fetch_add(1, std::memory_order_relaxed);
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
        double ops = static_cast<double>(produced.load()) / secs;
        std::cout << "Sample " << (s+1) << ": produced=" << produced.load() << " time=" << secs << "s ops/sec=" << ops << "\n";
        ofs << ops << std::endl;
    }
    ofs.close();
    std::cout << "Wrote samples to " << out_path << "\n";
    return 0;
}
