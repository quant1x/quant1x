// Vyukov MPMC 队列的单线程消融基准 (不依赖 Google Benchmark, 单文件可直接 cl / clang-cl 编译)
//
// 用途与**定位局限**:
// 测量单线程下 push / pop 的原始吞吐, 用于把"热路径指令成本"与"多线程争抢成本"
// 分离. 但**它测不出缓存行争用, 也测不出退避策略的停摆**:
//   * 无跨核争用时, `lock` 前缀指令只是一次本地 RMW, 与 `mov` 的差距被掩盖 →
//     无法区分 relaxed 读游标是否退化成了带 lock 的 RMW(详见 ringbuffer.h 注释).
//   * 无争抢时不触发 backoff_spin, 因此完全测不出休眠粒度问题 —— 而那正是
//     Windows x86_64 上比 Rust 慢数倍的主因(见 ringbuffer.md).
// 结论: 本基准只能用于"排除热路径本身的异常", 判定多线程性能必须看
// benches/ringbuffer_mpmc_probe.cpp 的扩展性曲线(吞吐随线程数的变化趋势).
//
// ⚠️ 两个场景都必须保证队列**不会队满**: 一旦队满, push 立即返回 false, 剩余迭代
// 全部退化成"快速失败"的空转, 吞吐会虚高一个量级(曾实测出无意义的 387 M/s).
// 因此 Case A 采用 push/pop 交替(队列深度恒为 1), Case B 只填充容量上限的条数.
// 两处均对返回值与数据内容做校验, 失败即以非 0 退出.
//
// 用法: 见 scripts/msvc_mini_bench.bat

#include <quant1x/runtime/ringbuffer.h>

#include <chrono>
#include <cstdint>
#include <cstdio>

using namespace runtime::ringbuffer;
using clk = std::chrono::steady_clock;

namespace {

/// Case A 的迭代次数: push 与 pop 各 N 次, 合计 2N 次队列操作
constexpr int64_t N = 10'000'000;
/// Case A 的容量: push 后立即 pop, 队列深度恒为 1, 故 1024 绰绰有余.
/// 槽区仅 64KB, 全程命中缓存 —— 测的是纯指令成本, 不含 cache miss.
constexpr size_t CAP_STEADY = 1024;
/// Case B 的容量(同时也是填充条数): 8MB 槽区, 超出 L2, 含逐槽首次触碰
constexpr size_t CAP_BULK = 1 << 17;

double elapsed_ms(clk::time_point a, clk::time_point b) {
    return std::chrono::duration<double, std::milli>(b - a).count();
}

}  // namespace

int main() {
    // ---- Case A: push/pop 交替, 队列永不满, 稳态单操作成本 ----
    {
        queue<int64_t> q(CAP_STEADY);
        const auto t0 = clk::now();
        for (int64_t i = 0; i < N; ++i) {
            if (!q.push(i)) {
                std::printf("Case A FAILED: push returned false at i=%lld (unexpected: queue never full)\n",
                            static_cast<long long>(i));
                return 1;
            }
            int64_t v = 0;
            // 单线程 FIFO: 取出的值必须与放入的严格相等
            if (!q.try_pop(v) || v != i) {
                std::printf("Case A FAILED: pop mismatch at i=%lld (got=%lld)\n",
                            static_cast<long long>(i), static_cast<long long>(v));
                return 1;
            }
        }
        const auto t1 = clk::now();
        const double ms = elapsed_ms(t0, t1);
        std::printf("Case A  push+pop alternating  x%lld pairs : %8.2f ms  %7.1f M ops/s\n",
                    static_cast<long long>(N), ms,
                    static_cast<double>(N) * 2.0 / (ms / 1000.0) / 1e6);
    }

    // ---- Case B: 批量 fill 再 drain, 含逐槽首次触碰的 cache miss ----
    {
        const int64_t m = static_cast<int64_t>(CAP_BULK);
        queue<int64_t> q(CAP_BULK);
        const auto f0 = clk::now();
        for (int64_t i = 0; i < m; ++i) {
            if (!q.push(i)) {
                std::printf("Case B FAILED: push returned false at i=%lld\n",
                            static_cast<long long>(i));
                return 1;
            }
        }
        const auto f1 = clk::now();
        int64_t popped = 0;
        int64_t v = 0;
        while (q.try_pop(v)) {
            if (v != popped) {
                std::printf("Case B FAILED: pop mismatch at %lld (got=%lld)\n",
                            static_cast<long long>(popped), static_cast<long long>(v));
                return 1;
            }
            ++popped;
        }
        const auto f2 = clk::now();

        const double fill_ms = elapsed_ms(f0, f1);
        const double drain_ms = elapsed_ms(f1, f2);
        if (popped != m) {
            std::printf("Case B FAILED: popped=%lld want=%lld\n",
                        static_cast<long long>(popped), static_cast<long long>(m));
            return 1;
        }
        std::printf("Case B  fill(cap=%zu)                     : %8.2f ms  %7.1f M ops/s\n",
                    CAP_BULK, fill_ms, static_cast<double>(m) / (fill_ms / 1000.0) / 1e6);
        std::printf("Case B  drain(cap=%zu)                    : %8.2f ms  %7.1f M ops/s\n",
                    CAP_BULK, drain_ms, static_cast<double>(m) / (drain_ms / 1000.0) / 1e6);
    }
    return 0;
}
