# Vyukov ringbuffer — C++ / Rust 多语言实现与性能对齐状态

本目录包含 Vyukov 有界 MPMC 队列的多语言平行实现：

| 语言 | 文件 | 定位 |
|---|---|---|
| C++ | `ringbuffer.h` | 生产（模板头文件单文件实现） |
| Rust | `ringbuffer.rs` | 生态扩展（参考实现，语义锚点） |
| Go | `ringbuffer.go` + `ringbuffer_test.go` | 生态扩展 |

基准测试位于仓库根 `benches/`：

- `benches/vyukov_bench.rs` — Rust（Criterion）
- `benches/vyukov_bench.cpp` — C++（Google Benchmark，由 `benches/CMakeLists.txt` 接入 CMake，目标 `benchmark-vyukov_bench`）

## 算法与 API 契约

基于每个槽位的序号（per-slot sequence）与原子入队/出队游标（`enqueue_pos_` / `dequeue_pos_`）实现无锁并发；槽按 64 字节对齐最小化伪共享。元素类型须满足 nothrow 构造/析构（编译期 `static_assert` 强制）。

C++ 提供两套入队/出队语义，与 Rust 逐级对齐：

- **非阻塞** `try_push` / `try_pop`：满/空/竞争失败立即返回 `false`，热路径绝不执行 yield/sleep（最坏等待为单次 CPU pause + 一次重试）。
- **阻塞式** `push` / `pop`（与 Rust `Queue::push` / `Queue::pop` 语义一致）：CAS 竞争失败与槽位瞬态冲突在内部按 `backoff_spin` 退避并无限重试直到成功；`push` 仅队满时返回 `false`，`pop` 仅"已关闭且为空"时返回 `false`。
- `close()`：消费者在耗尽存量后退出。
- `backoff_spin`：前 4 次 CPU_PAUSE 自旋 → 中 4 次 `yield` → 之后 sleep 50µs，与 Rust 四级退避一致。

## 性能对比状态（Apple Silicon / arm64，2026-08 实测）

### 已修复：aarch64 退避指令选择（决定性根因）

C++ 原 `CPU_PAUSE()` 在 aarch64 上使用 `yield` 指令，其 ARM 语义是"降低自旋等待线程的调度优先级/节流执行"，在 Apple Silicon 上被激进化实现。8 路 CAS 争抢（dequeue_pos/enqueue_pos）时所有线程被同步节流，形成 convoy 正反馈：争抢越久线程越慢，线程越慢争抢越久。**修复：`yield` → `isb`**，与 Rust `core::hint::spin_loop`（`isb`）逐字节对齐。

修复前后同一工作负载（8P8C、每生产者 20000 条 i64、单轮 160000 条）：

| 指标 | C++ 修复前 | C++ 修复后 | Rust |
|---|---|---|---|
| 整轮耗时 | 4855µs | **1946µs** | 1561µs |
| drain 阶段 | 3394µs（138~7700µs 双峰抖动） | **652µs** | 198µs |
| backpressure 吞吐（容量 1024） | 15.9M/s | **21.2M/s** | 21.5M/s（**-1.1%** ✅） |
| uncontended 吞吐（容量 262144） | 33~36M/s | **87.1M/s** | 118.1M/s |

### 已排除的候选方案（均实测后回退，代码中留有注释）

1. **ctor 清零**：曾用默认初始化 `new Slot[cap]` 替代 `std::make_unique<Slot[]>` 以对齐 Rust `Vec::with_capacity` 不初始化语义——实测在 Apple Silicon 上反而变慢（ctor 291µs → 424µs）：make_unique 的连续 memset 触页为预取友好，跨 64B 步进的 seq 存储触发分散页错误。保留 make_unique。
2. **strong + SeqCst CAS**（完全照抄 Rust 参考实现）：实测更差（2421µs vs 1946µs），C++ 保留 weak + acq_rel（在 arm64 / x86 上与 Rust SeqCst 编译产物一致，语义等价）。

### 剩余差距（已知，非缺陷）

uncontended 场景 C++ 仍慢约 25%，全部集中在 drain 阶段（652 vs 198µs）：C++ 生产者速率（~180M/s）略快于消费者（~150M/s），导致 push 阶段 backlog 累积到 ~95k（Rust 仅 ~34k）。这是生产者/消费者速率平衡的微架构效应，不是算法或实现缺陷——backpressure 场景已在 1% 内对齐。

### 基准测试对齐方法

- 场景与参数严格一致：8P8C、每生产者 20000 条、容量 1024（backpressure）/ 262144（uncontended）。
- 差分测量：每 iteration 先跑完整生产-消费轮次，再跑等量空转线程创建，两者耗时之差为纯队列操作耗时（对齐 Rust `iter_custom` + saturating_sub）。
- 正确性校验：每轮断言 `consumed == TOTAL`（160000），C++ 用 `SkipWithError`，Rust 用 `assert_eq!`，Release 下均生效。

## 复现

```bash
# C++ (Google Benchmark)
ninja -C build benchmark-vyukov_bench
./build/benches/benchmark-vyukov_bench --benchmark_min_time=2s

# Rust (Criterion)
cargo bench --bench vyukov_bench
```

## 注意事项

- 推荐 LLVM/Clang 后端（clang++ / rustc 的 LLVM）；g++ 对热点代码的 codegen 更保守，吞吐显著偏低。
- 性能波动受系统调度、CPU 频率、电源策略影响，建议在受控环境（空闲系统、固定 CPU 亲和性、电源性能模式）下测量。
- 当前对比数据在 Apple Silicon（arm64）上测得；x86 平台行为可能不同，尚未实测。
