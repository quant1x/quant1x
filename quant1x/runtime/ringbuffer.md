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

> 注：上表 uncontended 数值含共享计数器的测量伪影（见"剩余差距"一节），修复 harness 后 C++/Rust 已在噪声带内对齐（107~131 vs 121~125 M/s）。

> 技术注记（aarch64 `isb` vs `yield`）：在 aarch64 架构下，`yield` 提示微架构降低自旋线程的优先级，极易在多路 CAS 争抢时引发调度器层面的 convoy 效应。替换为 `isb`（Instruction Synchronization Barrier）虽指令周期略长，但能有效清空流水线并打破节流死锁，实测在 Apple Silicon 高竞争场景下收益远大于成本。

### 已修复：ctor 槽区分配策略（消除 16MB 清零，对齐 Rust 不初始化语义）

Rust `Vec::with_capacity` 只分配不初始化；C++ 旧实现 `std::make_unique<Slot[]>` 会对槽区做 value-initialize（逐字节清零 16MB）。ctor 拆解实验（`ctor_variants.cpp`）确认各分配策略耗时（Apple Silicon）：

| 分配策略 | ctor 耗时 |
|---|---|
| malloc 16MB（惰性触页） | ~8µs |
| make_unique 纯 memset 16MB | ~67µs |
| make_unique + 写 seq（旧实现） | ~246µs |
| **aligned_alloc(64) + placement new 只写 seq（当前实现）** | **~185µs**（对齐 Rust ~210µs） |

**修复**：`std::aligned_alloc(64, cap * sizeof(Slot))` 惰性分配 + placement new 构造 `Slot(i)` 只写 seq（模仿 Rust `AtomicUsize::new(i)`），storage 保持未初始化（同 `MaybeUninit::uninit`），删除器为 `std::free`。注意不能用 `new Slot[cap]`（over-aligned `operator new[]` 逐元素构造路径低效，实测 424µs，是低效路径而非正确对比）。

### 已修复：Windows 退避第三级的休眠粒度（决定性根因，x86_64 慢数倍的真凶）

三级退避的第三级名义休眠 50µs，但 MSVC 的 `std::this_thread::sleep_for(50µs)` 会向上取整为 `Sleep(1)`，受 Windows **默认 15.6ms 定时器粒度**支配：

| 平台 / 手段 | 名义 | 实测单次耗时 |
|---|---|---|
| C++ `std::this_thread::sleep_for(50µs)`（MSVC） | 50µs | **15,571 µs**（311×） |
| Rust `thread::sleep(50µs)` | 50µs | **554 µs** |
| C++ `CreateWaitableTimerExW(HIGH_RESOLUTION)` | 50µs | **535 µs**（与 Rust 一致） |

**相差 28 倍**。Vyukov 队列在连续 8 次 CAS 竞争失败后进入该级退避，C++ 侧每次停摆 15.6ms，而 Rust 只停 0.55ms。这是 Windows x86_64 上 C++ 比 Rust 慢数倍的主因，且**单线程基准完全测不出**（无争抢时不触发退避）。

**修复**：新增 `safe::sleep_for_microseconds()`（`base/safe.h` / `base/safe.cpp`），Windows 上改用 `CREATE_WAITABLE_TIMER_HIGH_RESOLUTION` 可等待定时器，其余平台退化为 `sleep_for`。`windows.h` 仅在 `safe.cpp` 内包含，避免其宏泄漏到所有包含 `safe.h` 的翻译单元。

修复前后（i7-12700T / Windows 11 / cl /O2，环形容量 65536，单轮 320 万条）：

| 场景 | 修复前 | 修复后 | Rust |
|---|---|---|---|
| 1P1C | 50.1 M/s | **78.8 M/s** | 67.0 M/s |
| 4P4C | 25.1 M/s | **82.3 M/s** | 76.7 M/s |
| 8P8C | 15.6 M/s | **75.0 M/s** | 77.8 M/s |
| 16P16C | 8.5 M/s | **68.4 M/s** | 50.7 M/s |

修复后 C++ 曲线与 Rust 同样平坦（不再随线程数单调崩塌），8P8C 提升 **4.8 倍**。连续三轮复测（与 Rust 交替跑以抵消系统负载漂移），C++ 与 Rust 在各档位均落在同一噪声带内：

| 场景 | C++ run1/2/3 | Rust run1/2/3 |
|---|---|---|
| 1P1C | 81.3 / 81.8 / 63.9 | 78.9 / 69.9 / 66.5 |
| 4P4C | 80.1 / 73.8 / 76.5 | 77.8 / 73.7 / 76.0 |
| 8P8C | 76.4 / 70.4 / 69.9 | 76.9 / 69.8 / 74.0 |
| 16P16C | 72.0 / 65.0 / 68.8 | 69.6 / 66.0 / 73.6 |

> **测量环境警告**：本测量的绝对数值对系统负载极度敏感。开发机后台常驻 IDE / 浏览器 / IM 等高占用进程时，同一份二进制实测可从 62 M/s 跌到 2.1 M/s（30 倍），且连纯内存分配耗时（ctor）都会同步从 25ms 涨到 135ms —— 后者与队列逻辑无关，是识别"环境噪声"而非"实现回退"的可靠判据。任何对比都应与 Rust 基线**交替**跑多轮取最好值，不可单轮定论。

> 定位方法备忘：本问题被"单线程消融基准"长期掩盖。`scripts/msvc_mini_bench.bat` 测出单线程 167M ops/s（健康），无法暴露退避停摆；只有把入队侧与出队侧拆开、并测 1P1C→16P16C 的**扩展性曲线**，才能看出"吞吐随线程数单调下降"这一特征。新增 `benches/ringbuffer_mpmc_probe.cpp`（配 `scripts/msvc_mpmc_probe.bat`）固化该测量。

### 诊断工具（Windows）

| 工具 | 用途 | 能测出什么 |
|---|---|---|
| `scripts/msvc_sched_cost_probe.bat` | 调度原语开销（`sleep_for` / `yield` / `Sleep` / 可等待定时器） | **休眠粒度**——本次根因，也用于回归验证 `safe::sleep_for_microseconds` 是否仍生效 |
| `scripts/msvc_mpmc_probe.bat` | 8P8C 争抢探针，同时构建 cl 与 clang-cl | **多线程扩展性曲线**，判定队列吞吐随线程数的变化趋势 |
| `scripts/msvc_mini_bench.bat` | 单线程 push/pop 消融 | 仅热路径指令成本；**测不出争用与退避停摆**（见"定位方法备忘"） |

三者分工的关键：先看调度原语（排除休眠粒度），再看扩展性曲线（排除争抢），最后才用单线程消融定位热路径指令。顺序反了会误判——本次就曾在单线程数字"健康"的情况下长期找不到根因。

### 已排除的候选方案（均实测后回退，代码中留有注释）

1. **strong + SeqCst CAS**（完全照抄 Rust 参考实现）：实测更差（2421µs vs 1946µs），C++ 保留 weak + acq_rel（在 arm64 / x86 上与 Rust SeqCst 编译产物一致，语义等价）。

2. **MSVC 下用 `_InterlockedCompareExchange64(p, 0, 0)` 实现 relaxed 读游标**：该内建是**带 lock 前缀的读-改-写**，而 Rust 的 `load(Relaxed)` 只是一条 `mov`。`cl /O2` 汇编实测：修复前 push 热循环为 `2 × lock cmpxchg`（读游标 + CAS），修复后为 `1 × mov + 1 × lock cmpxchg`，与 rustc / clang-cl 一致。该改动**必要**（每次读游标都要独占争抢中的缓存行，把 CAS 争抢流量翻倍），但**不是** Windows 慢数倍的主因 —— 单独修它实测无显著变化（8P8C 1.3 vs 1.4 M/s，在噪声带内），真正的根因是上面的休眠粒度。现已回归 `std::atomic` 接口，仅 CAS 保留平台内建。

### 剩余差距（已知，非缺陷）

uncontended 场景基准曾显示 C++ 慢约 25%，经隔离探针（`isolate_probe`）消融验证，该差距**全部来自基准 harness 的共享原子计数器 `consumed.fetch_add()`，是测量伪影而非队列实现缺陷**：

- 队列自身速率对齐：drain 阶段 C++ 142M/s ≈ Rust 145M/s；无计数器干扰时 8P8C 消费速率 C++ 160M/s ≈ Rust 159M/s（完全对齐）。
- 消融实验：去掉消费者循环里的 `consumed.fetch_add()` 后，8P8C backlog 从 C++ 54-70k / Rust 21-28k 变为 **C++ 6031 / Rust 6246（完全对齐）**；`fetch_add` 单独测量 C++ 反而更快（184 vs 148M/s）——差距源于 pop 与 fetch_add 两个竞争点叠加的复合缓存行争用，非指令本身。
- **修复**：基准消费者改为**每线程本地计数、结束时合并**（C++ `per_consumer[]` / Rust join 返回值聚合），消除共享计数器竞争。

### 基准测试对齐方法

- 场景与参数严格一致：8P8C、每生产者 20000 条、容量 1024（backpressure）/ 262144（uncontended）。
- 差分测量：每 iteration 先跑完整生产-消费轮次，再跑等量空转线程创建，两者耗时之差为纯队列操作耗时（对齐 Rust `iter_custom` + saturating_sub）。
- 消费计数：每消费者线程本地计数，join 后合并（避免共享原子计数器引入的缓存行竞争伪影，见"剩余差距"一节）。
- 正确性校验：每轮断言 `consumed == TOTAL`（160000），C++ 用 `SkipWithError`，Rust 用 `assert_eq!`，Release 下均生效。

## 复现

```bash
# C++ (Google Benchmark) — 强制使用 Clang/LLVM 编译（g++ 对热点代码 codegen 保守，吞吐显著偏低）
cmake -S . -B build-clang -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build build-clang --target benchmark-vyukov_bench
./build-clang/benches/benchmark-vyukov_bench --benchmark_min_time=2s

# Rust (Criterion)
cargo bench --bench vyukov_bench
```

## Windows 侧复现

```bat
:: 1) 调度原语开销 —— 先确认休眠粒度正常(应约 0.55ms, 若为 15ms 则高精度定时器不可用)
scripts\msvc_sched_cost_probe.bat

:: 2) 8P8C 争抢探针, 同时构建 cl 与 clang-cl 两套做对比
scripts\msvc_mpmc_probe.bat

:: 3) 单线程消融(仅作热路径参考, 测不出争用与退避)
scripts\msvc_mini_bench.bat

:: 4) CMake 构建配置(含本机绝对路径, 使用前按需修改)
scripts\msvc_configure.bat
```

### Rust 基线（用于对照 C++ 扩展性曲线）

`benches/vyukov_bench.rs`（Criterion）只覆盖固定 8P8C。若需要 1P1C→16P16C 的
**扩展性曲线**与 C++ 侧逐档对照，可临时用 `rustc` 直接编译（无需改动 Cargo.toml，
`ringbuffer.rs` 只依赖 std）：

```rust
// rust_scale.rs —— 与 benches/ringbuffer_mpmc_probe.cpp 参数严格一致
include!("<repo>/quant1x/runtime/ringbuffer.rs");
fn main() { /* 对 np in [1,2,4,8,16] 各跑一轮, 消费者用每线程本地计数 */ }
```

```bat
rustc -O -o build-msvc\rust_scale.exe build-msvc\rust_scale.rs
```

注意：`ringbuffer.rs` 自带 `use std::sync::Arc; use std::thread;`，外侧不可重复导入
（会触发 `E0252`）。对照时应与 C++ 侧**交替**跑多轮取最好值，以抵消系统负载漂移。

## 注意事项

- 推荐 LLVM/Clang 后端（clang++ / rustc 的 LLVM）；g++ 对热点代码的 codegen 更保守，吞吐显著偏低。
- Windows 上 third tier 退避必须走 `safe::sleep_for_microseconds()`，不可用 `std::this_thread::sleep_for`（15.6ms 粒度，详见"已修复"一节）。
- 性能波动受系统调度、CPU 频率、电源策略影响，建议在受控环境（空闲系统、固定 CPU 亲和性、电源性能模式）下测量。
- arm64 数据在 Apple Silicon 上测得；x86_64 数据在 i7-12700T / Windows 11 上测得，修复后两者均与 Rust 对齐。
