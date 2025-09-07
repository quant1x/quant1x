#include <benchmark/benchmark.h>

// ============================================================
// 当日零点的算法
// ============================================================
static void BM_Method1(benchmark::State& state) {
    for (auto _ : state) {
        time_t t = 1717741234 + state.iterations();
        benchmark::DoNotOptimize(t - (t % 86400));
    }
}

static void BM_Method2(benchmark::State& state) {
    for (auto _ : state) {
        time_t t = 1717741234 + state.iterations();
        benchmark::DoNotOptimize((t / 86400) * 86400);
    }
}

BENCHMARK(BM_Method1);
BENCHMARK(BM_Method2);

BENCHMARK_MAIN();