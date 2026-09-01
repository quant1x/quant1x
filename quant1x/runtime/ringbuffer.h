// Vyukov 有界 MPMC 队列的 C++17 头文件单文件移植(模板)
// 实现说明(中文):
// - 基于每个槽位的序号(per-slot sequence)以及原子性的入队/出队索引实现无锁并发
// - 槽按 64 字节对齐以最小化伪共享(false sharing)带来的性能下降
// - 提供两套入队/出队语义:
//   * 非阻塞 try_push / try_pop: 满/空/竞争失败时立即返回 false,
//     热路径绝不执行 yield/sleep 等 OS 调度操作 (最坏等待为单次 CPU pause)
//   * 阻塞式 push / pop: 与 Rust `Queue::push` / `Queue::pop` 语义一致,
//     CAS 竞争失败与槽位瞬态冲突在内部按退避策略重试直到成功, 仅队满(push)
//     或"已关闭且为空"(pop) 时返回 false. 适用于 MPMC 高竞争场景, 由队列
//     内部吸收纳秒级竞争, 避免调用方在 try_ 外层做粗粒度退避
// - 提供 close() 方法, 供消费者观察队列关闭并在耗尽数据后退出
// - 元素类型须满足 nothrow 构造与 nothrow 析构 (编译期 static_assert 强制):
//   无锁算法无法回滚已推进的全局入队索引, 抛异常的构造会破坏队列一致性
// - 待修复/注意: 析构的线程安全性与潜在内存泄漏需要在使用时注意
#pragma once
#ifndef QUANT1X_RUNTIME_RINGBUFFER_H
#define QUANT1X_RUNTIME_RINGBUFFER_H 1

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <type_traits>
#include <new>
#include <utility>
#include <stdexcept>
// <chrono> 曾为 backoff_spin 第三级的 sleep_for(50us) 而包含; 该级现改走
// safe::sleep_for_microseconds(50) 以绕开 Windows 15.6ms 定时器粒度, 故不再需要.
#include <thread>

#include <quant1x/base/safe.h>

#if defined(_MSC_VER)
#  include <intrin.h>
#  define CPU_PAUSE() _mm_pause()
#elif defined(__aarch64__) || defined(__arm64__)
// 与 Rust `core::hint::spin_loop` 在 aarch64 上的实现(isb)对齐.
// 注意: 不要用 aarch64 `yield` 指令作为自旋退避提示 —— 该指令的 ARM 语义是
// "降低自旋等待线程的调度优先级/节流执行", 在 Apple Silicon 上会被激进化实现,
// 导致 8 路 CAS 争抢(dequeue_pos/enqueue_pos)时所有线程被同步节流, 形成 convoy
// 正反馈: 争抢越久线程越慢, 线程越慢争抢越久. 实测同工作负载下 drain 阶段从
// ~600us 恶化到 ~3400us 且方差极大(138-7700us); 改用 isb(指令同步屏障, 无节流
// 副作用)后与 Rust 性能完全对齐.
#  define CPU_PAUSE() __asm__ volatile("isb" ::: "memory")
#else
#  include <immintrin.h>
#  define CPU_PAUSE() _mm_pause()
#endif

// 跨编译器的属性宏: 在 GCC/Clang 上使用 always_inline/hot, 在 MSVC 上使用 __forceinline.
// 必须定义于热路径包装层之前: 包装层依赖它保证被内联进 try_push / try_pop / push / pop.
#if defined(_MSC_VER)
#  define ATTR_ALWAYS_INLINE_HOT __forceinline
#else
#  define ATTR_ALWAYS_INLINE_HOT inline __attribute__((always_inline, hot))
#endif

// ---------------------------------------------------------------------------
// 热路径原子包装层
//
// 设计目标只有一个: 让各编译器在 x86_64 / arm64 上生成与 Rust 参考实现(LLVM 后端)
// 等价的指令序列. "使用平台内建"本身不是目的, 指令条数才是.
//
// 历史教训 (2026-09, Windows x86_64, cl /O2 汇编实测):
// 曾以 `_InterlockedCompareExchange64(p, 0, 0)` 实现 MSVC 下的 relaxed 读游标.
// 该内建是**带 lock 前缀的读-改-写**, 而 Rust 的 `load(Relaxed)` 只是一条 `mov`:
//   * 每次"读游标"都要独占 enqueue_pos_ / dequeue_pos_ 所在缓存行(MESI → E/M 态),
//     8 路生产者/消费者在同一行上互相失效, 把本已串化的 CAS 争抢流量再翻一倍.
//   * 单线程消融基准暴露不了: 无跨核争用时 lock 指令只是一次本地 RMW, 与 `mov`
//     的差距被完全掩盖(见 scripts/msvc_mini_bench.bat 的定位局限).
// 实测 (cl /O2, push 热循环, 游标在 [obj+64]):
//   * 修复前: 2 × `lock cmpxchg` + 0 × `mov`  (读游标与 CAS 都是 lock RMW)
//   * 修复后: 1 × `mov` + 1 × `lock cmpxchg` (与 rustc / clang-cl 一致)
// 结论: relaxed 读一律回归 std::atomic 接口; 只有 CAS 保留平台内建 —— 它在 x64 上
// 与 `lock cmpxchg` 逐字节等价, 且可杜绝编译器额外插入 `mfence` / `xchg` 序列.
//
// ⚠️ 但本项**不是** Windows 上比 Rust 慢数倍的主因: 单独修它 8P8C 实测 1.3 vs
// 1.4 M/s, 落在噪声带内. 真正的根因是 backoff_spin 第三级的休眠粒度(见下方
// backoff_spin 注释与 base/safe.h 中 sleep_for_microseconds 的说明). 保留本修复
// 因为它确实消除了多余的 lock 流量, 但不要误当作性能差距的解释.
// ---------------------------------------------------------------------------
namespace runtime::ringbuffer {
namespace detail {

// Relaxed 读取入队/出队游标. x86_64 / arm64 上均退化为普通 load, 无 lock, 无 fence.
ATTR_ALWAYS_INLINE_HOT size_t atomic_load_relaxed(const std::atomic<size_t>& a) noexcept {
    return a.load(std::memory_order_relaxed);
}

// 竞争入队/出队游标. 成功序 acq_rel / 失败序 relaxed: 失败路径只重读游标, 不需要同步;
// x86_64 上 acq_rel 的 `lock cmpxchg` 本身即全序, 不会再额外插入 fence.
// 不使用 SeqCst: 本算法除游标外还有 per-slot seq 做 release/acquire 配对, 游标只需
// "不重复发号"的原子性, 无需全局序(实测 arm64/x64 与 Rust SeqCst 生成同码).
ATTR_ALWAYS_INLINE_HOT bool atomic_cas_weak(std::atomic<size_t>& a, size_t& expected, size_t desired) noexcept {
#if defined(_MSC_VER) && !defined(__clang__) && (defined(_M_X64) || defined(_M_AMD64))
    const long long exp = static_cast<long long>(expected);
    const long long res = _InterlockedCompareExchange64(
        reinterpret_cast<volatile long long*>(std::addressof(a)),
        static_cast<long long>(desired), exp);
    if (res == exp) {
        return true;
    }
    expected = static_cast<size_t>(res);
    return false;
#else
    return a.compare_exchange_weak(expected, desired, std::memory_order_acq_rel, std::memory_order_relaxed);
#endif
}

}  // namespace detail
}  // namespace runtime::ringbuffer




namespace runtime::ringbuffer {

template<typename T>
class queue {
public:
    explicit queue(size_t capacity) {
        if (capacity == 0) {
            throw std::invalid_argument("Queue capacity must be at least 1");
        }
        size_t cap = round_up_to_power_of_two(capacity);
        mask_ = cap - 1;
        // 槽区分配策略: 64 字节对齐整块惰性分配 + placement new 只写 seq.
        // 对齐 Rust `Vec::with_capacity`(不初始化) 语义, 避免 `std::make_unique<Slot[]>`
        // 对 16MB 槽区的 value-initialize(逐字节清零). 实测(Apple Silicon, ctor_variants):
        //   - malloc 16MB(惰性)                    ~8us
        //   - make_unique 纯 memset 16MB          ~67us
        //   - make_unique + 写 seq(旧 ctor)       ~246us
        //   - aligned_alloc + placement 只写 seq  ~185us  (对齐 Rust ~210us, 省 ~60us)
        // 注: 之前用 `new Slot[cap]` 反而慢(424us) 是 over-aligned operator new[] 逐元素
        // 构造路径低效(非正确对比); aligned_alloc 整块惰性分配 + 单步 placement 无此问题.
        // 平台差异(Windows CRT 不提供 aligned_alloc)已收敛至 base/safe.h 的
        // safe::aligned_alloc / safe::aligned_free, 参数顺序统一为 (alignment, size).
        void* raw = safe::aligned_alloc(64, cap * sizeof(Slot));
        if (!raw) {
            throw std::bad_alloc();
        }
        Slot* slots = static_cast<Slot*>(raw);
        // 只写 seq(模仿 Rust AtomicUsize::new(i)), storage 保持未初始化
        for (size_t i = 0; i < cap; ++i) {
            new (&slots[i]) Slot(i);
        }
        buffer_.reset(slots);
        enqueue_pos_.store(0, std::memory_order_relaxed);
        dequeue_pos_.store(0, std::memory_order_relaxed);
        closed_.store(false, std::memory_order_relaxed);
    }

    // 禁止拷贝/移动以防止意外复制
    queue(const queue&) = delete;
    queue& operator=(const queue&) = delete;
    queue(queue&&) = delete;
    queue& operator=(queue&&) = delete;

    ~queue() noexcept {
        close();

        // 线程安全的析构: 清空剩余元素
        // 注意: 此处假定没有其他线程再访问队列
        // 在生产代码中, 应确保所有生产者/消费者线程已停止
        T dummy;
        while (try_pop(dummy)) {
            // 元素在 try_pop 中被析构
        }
    }

    // 非阻塞入队. 成功返回 true, 队列满时返回 false. 
    bool try_push(const T& value) {
        return emplace(value);
    }

    bool try_push(T&& value) {
        return emplace(std::move(value));
    }

    // 阻塞式入队. 与 Rust `Queue::push` 语义一致: CAS 竞争失败与槽位瞬态冲突
    // 在内部按 backoff_spin 退避并无限重试直到成功, 仅队列已满(seq < pos)时
    // 返回 false. 与 try_push 的区别: 竞争失败不会快速失败, 调用方不应假设
    // 本函数在高竞争下会立即返回.
    bool push(const T& value) {
        return emplace_blocking(value);
    }

    bool push(T&& value) {
        return emplace_blocking(std::move(value));
    }

    // 非阻塞出队. 成功时返回 true 并将元素写入 `out`.
    // 队列为空(或为空且已关闭)、队满竞争或槽位瞬态冲突时立即返回 false.
    ATTR_ALWAYS_INLINE_HOT bool try_pop(T& out) noexcept(
        std::is_nothrow_move_assignable_v<T> && std::is_nothrow_destructible_v<T>
    ) {
        // try_ 契约: 竞争失败最多以一次 CPU pause 吸收纳秒级瞬态 (相邻线程正在
        // 完成槽位读写) 并重试一次, 仍不可行立即返回 false; 绝不 yield/sleep,
        // 阻塞等待策略由调用者控制
        uint32_t retry = 0;
        while (true) {
            size_t pos = detail::atomic_load_relaxed(dequeue_pos_);
            Slot& slot = buffer_[pos & mask_];
            size_t seq = slot.seq.load(std::memory_order_acquire);

            if (seq == pos + 1) {
                size_t expected_pos = pos;
                if (detail::atomic_cas_weak(dequeue_pos_, expected_pos, pos + 1)) {
                    // 成功获取该槽位的所有权
                    T* ptr = std::launder(reinterpret_cast<T*>(&slot.storage));
                    out = std::move(*ptr);
                    ptr->~T();
                    slot.seq.store(pos + mask_ + 1, std::memory_order_release);
                    return true;
                } else {
                    // CAS 竞争失败: 快速失败 (见函数头注释)
                    if (retry++ != 0) {
                        return false;
                    }
                    CPU_PAUSE();
                    continue;
                }
            } else if (seq < pos + 1) {
                // 槽为空
                if (closed_.load(std::memory_order_acquire)) {
                    return false; // 队列已关闭且为空
                }
                return false; // 队列为空但未关闭
            } else {
                // 槽正被其他生产者写入: 快速失败 (见函数头注释)
                if (retry++ != 0) {
                    return false;
                }
                CPU_PAUSE();
                continue;
            }
        }
    }

    // 阻塞式出队. 与 Rust `Queue::pop` 语义一致:
    // - CAS 竞争失败/槽位瞬态冲突: 内部退避并无限重试直到成功
    // - 队列为空但未关闭: 内部退避并继续等待 (调用方无需自行 yield/sleep)
    // - 队列已关闭且为空: 返回 false, 消费者可据此退出
    ATTR_ALWAYS_INLINE_HOT bool pop(T& out) noexcept(
        std::is_nothrow_move_assignable_v<T> && std::is_nothrow_destructible_v<T>
    ) {
        uint32_t backoff = 0;
        while (true) {
            size_t pos = detail::atomic_load_relaxed(dequeue_pos_);
            Slot& slot = buffer_[pos & mask_];
            size_t seq = slot.seq.load(std::memory_order_acquire);

            if (seq == pos + 1) {
                size_t expected_pos = pos;
                if (detail::atomic_cas_weak(dequeue_pos_, expected_pos, pos + 1)) {
                    // 成功获取该槽位的所有权
                    T* ptr = std::launder(reinterpret_cast<T*>(&slot.storage));
                    out = std::move(*ptr);
                    ptr->~T();
                    slot.seq.store(pos + mask_ + 1, std::memory_order_release);
                    return true;
                } else {
                    // CAS 竞争失败: 退避后重试 (不会返回, 见函数头注释)
                    backoff_spin(backoff);
                    continue;
                }
            } else if (seq < pos + 1) {
                // 槽为空
                if (closed_.load(std::memory_order_acquire)) {
                    return false; // 队列已关闭且为空: 唯一返回 false 的路径
                }
                backoff_spin(backoff); // 空但未关闭: 退避后继续等待
                continue;
            } else {
                // 槽正被其他生产者写入: 退避后重试 (不会返回, 见函数头注释)
                backoff_spin(backoff);
                continue;
            }
        }
    }

    // 关闭队列. 关闭后, 当队列为空时 try_pop 将返回 false. 
    void close() noexcept {
        closed_.store(true, std::memory_order_release);
    }

    // 检查队列是否已关闭
    bool is_closed() const noexcept {
        return closed_.load(std::memory_order_acquire);
    }

    // 获取容量(用于调试/信息)
    size_t capacity() const noexcept {
        return mask_ + 1;
    }

    // 当前队列中元素数量的近似值(Relaxed 顺序, 非精确快照)
    // 与 Rust `runtime::Queue::len` 语义一致: enqueue - dequeue, 下溢按 0 处理.
    size_t len() const noexcept {
        const size_t enqueue = enqueue_pos_.load(std::memory_order_relaxed);
        const size_t dequeue = dequeue_pos_.load(std::memory_order_relaxed);
        return enqueue > dequeue ? enqueue - dequeue : 0;
    }

    // 队列是否为空(近似判断)
    bool is_empty() const noexcept {
        return len() == 0;
    }

private:
    // 整个槽位独占一个缓存行: struct 级 64 字节对齐确保槽的起始地址与大小
    // 都是 64 的整数倍, 无论成员如何调整都不产生跨槽伪共享
    struct alignas(64) Slot {
        // 只写 seq 的构造函数(模仿 Rust `AtomicUsize::new(i)`), storage 保持未初始化
        // (与 Rust `MaybeUninit::uninit` 语义一致). 配合 ctor 的 64 字节对齐惰性
        // 分配, 避免 make_unique 对 16MB 槽区的 value-initialize 清零
        // (实测 ctor 246us → 185us, 对齐 Rust ~210us, 见 ctor 内说明).
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init) -- storage 故意不初始化
        explicit Slot(size_t s) noexcept : seq(s) {}
        std::atomic<size_t> seq;
        alignas(alignof(T)) std::byte storage[sizeof(T)];
    };

    // 槽位数组, 长度为 capacity(向上取整到 2 的幂), 每个槽按 64 字节对齐.
    // 内存由 safe::aligned_alloc 分配, 删除器为与之成对的 safe::aligned_free
    // (平台分支见 base/safe.h, 分配/释放混用属 UB).
    std::unique_ptr<Slot[], void (*)(Slot*)> buffer_{nullptr, +[](Slot* p) noexcept {
        safe::aligned_free(p);
    }};
    // 用于将序号映射为数组索引的掩码(mask = capacity - 1)
    size_t mask_;
    // 生产者游标(下一个待写入序号), 按缓存行对齐以减少伪共享
    // 注: 保持 64 字节(与 Rust `#[repr(align(64))]` 一致)以保证跨语言布局等价;
    // x86 的相邻扇区预取(128B)可能让两行仍在同一扇区内, 若要实验 alignas(128)
    // 需 C++/Rust 同时改, 否则对比失去意义.
    alignas(64) std::atomic<size_t> enqueue_pos_{0};
    // 消费者游标(下一个待读取序号), 按缓存行对齐以减少伪共享
    alignas(64) std::atomic<size_t> dequeue_pos_{0};
    // 关闭标志(true 表示队列已关闭), 消费者可据此在空队列时退出
    // 独占缓存行: 与上面对齐的游标不同, alignas 只影响偏移不影响大小, 不显式对齐
    // 时 closed_ 会落在 dequeue_pos_ 的同一行内(cl /O2 实测偏移 [obj+128] 与
    // [obj+136]), close() 的一次 release 写会把 8 路消费者正在争抢的 dequeue_pos_
    // 整行失效. Rust 侧 `AlignedAtomicUsize` 的 repr(align(64)) 使其 size = 64,
    // `closed` 天然独占一行, 此处是为对齐该布局.
    alignas(64) std::atomic<bool> closed_{false};

    // 向上取整到下一个 2 的幂
    static size_t round_up_to_power_of_two(size_t v) noexcept {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        // 32 位 size_t 下移位计数 >= 位宽是 UB, 编译期剪除该分支
        if constexpr (sizeof(size_t) > 4) {
            v |= v >> 32;
        }
        v++;
        return v;
    }

    // 自旋退避策略: 与 Rust `Queue::backoff_spin` 逐级对齐.
    // - 前 4 次: CPU_PAUSE 自旋提示, 纳秒级, 吸收瞬时竞争
    // - 中 4 次: std::this_thread::yield 让出时间片, 微秒级, 竞争持续时降低总线争用
    // - 之后: 休眠 50us, 长时间等待时彻底释放 CPU
    // 退避计数器在重试间保持, 竞争越久退避越激进, 避免多生产者/消费者互相饿死.
    //
    // 第三级**必须**走 safe::sleep_for_microseconds 而非 std::this_thread::sleep_for:
    // MSVC 的后者会把 50us 向上取整为 Sleep(1), 受 Windows 默认 15.6ms 定时器粒度
    // 支配, 实测单次实际耗时 15.57ms(名义值的 311 倍); Rust 的 thread::sleep(50us)
    // 实测 0.55ms. 相差 28 倍 —— 这是 Windows x86_64 上本实现比 Rust 慢数倍的
    // 主因: 8 生产者场景下每次退避停摆 15.6ms, 实测吞吐 1.1M/s vs 修复后 23.1M/s.
    // 详见 base/safe.h 中 sleep_for_microseconds 的说明.
    static void backoff_spin(uint32_t& iter) noexcept {
        if (iter < 4) {
            CPU_PAUSE();
        } else if (iter < 8) {
            std::this_thread::yield();
        } else {
            safe::sleep_for_microseconds(50);
        }
        ++iter;
    }

    // 阻塞式入队核心. 与 Rust `Queue::push` 语义一致, 供 push() 调用.
    // 内存序与 try_ 族一致(acq_rel): Rust 参考实现使用 SeqCst, 但该算法在
    // arm64 / x86 目标上两者编译产物一致(casal / lock xchg), 语义等价.
    template<typename U>
    ATTR_ALWAYS_INLINE_HOT bool emplace_blocking(U&& value) noexcept(
        std::is_nothrow_constructible_v<T, U> &&
        std::is_nothrow_destructible_v<T>
    ) {
        static_assert(
            std::is_nothrow_constructible_v<T, U> &&
            std::is_nothrow_destructible_v<T>,
            "queue<T> requires nothrow-constructible and nothrow-destructible "
            "element types: a lock-free queue cannot roll back the globally "
            "monotonic enqueue position once a slot reservation is taken, so "
            "a throwing constructor would corrupt the queue state"
        );
        // 阻塞式契约: CAS 竞争失败或槽位瞬态冲突时退避重试直到成功;
        // 仅队满(seq < pos)返回 false.
        uint32_t backoff = 0;
        while (true) {
            size_t pos = detail::atomic_load_relaxed(enqueue_pos_);
            Slot& slot = buffer_[pos & mask_];
            size_t seq = slot.seq.load(std::memory_order_acquire);

            if (seq == pos) {
                // 槽位已准备好写入
                size_t expected_pos = pos;
                if (detail::atomic_cas_weak(enqueue_pos_, expected_pos, pos + 1)) {
                    // 成功预留该槽位; 构造为 nothrow (见 static_assert),
                    // 不存在需要回滚索引的异常路径
                    T* ptr = std::launder(reinterpret_cast<T*>(&slot.storage));
                    new (ptr) T(std::forward<U>(value));
                    slot.seq.store(pos + 1, std::memory_order_release);
                    return true;
                } else {
                    // CAS 竞争失败: 退避后重试 (不返回, 见函数头注释)
                    backoff_spin(backoff);
                    continue;
                }
            } else if (seq < pos) {
                // 队列已满: 唯一返回 false 的路径
                return false;
            } else {
                // 槽正被其他消费者读取: 退避后重试 (不返回, 见函数头注释)
                backoff_spin(backoff);
                continue;
            }
        }
    }

    template<typename U>
    ATTR_ALWAYS_INLINE_HOT bool emplace(U&& value) noexcept(
        std::is_nothrow_constructible_v<T, U> &&
        std::is_nothrow_destructible_v<T>
    ) {
        static_assert(
            std::is_nothrow_constructible_v<T, U> &&
            std::is_nothrow_destructible_v<T>,
            "queue<T> requires nothrow-constructible and nothrow-destructible "
            "element types: a lock-free queue cannot roll back the globally "
            "monotonic enqueue position once a slot reservation is taken, so "
            "a throwing constructor would corrupt the queue state"
        );
        // try_ 契约: 竞争失败最多以一次 CPU pause 吸收纳秒级瞬态并重试一次,
        // 仍不可行立即返回 false; 绝不 yield/sleep, 阻塞等待策略由调用者控制
        uint32_t retry = 0;
        while (true) {
            size_t pos = detail::atomic_load_relaxed(enqueue_pos_);
            Slot& slot = buffer_[pos & mask_];
            size_t seq = slot.seq.load(std::memory_order_acquire);

            if (seq == pos) {
                // 槽位已准备好写入
                size_t expected_pos = pos;
                if (detail::atomic_cas_weak(enqueue_pos_, expected_pos, pos + 1)) {
                    // 成功预留该槽位; 构造为 nothrow (见 static_assert),
                    // 不存在需要回滚索引的异常路径
                    T* ptr = std::launder(reinterpret_cast<T*>(&slot.storage));
                    new (ptr) T(std::forward<U>(value));
                    slot.seq.store(pos + 1, std::memory_order_release);
                    return true;
                } else {
                    // CAS 竞争失败: 快速失败 (见上方 try_ 契约注释)
                    if (retry++ != 0) {
                        return false;
                    }
                    CPU_PAUSE();
                    continue;
                }
            } else if (seq < pos) {
                // 队列已满
                return false;
            } else {
                // 槽正被其他消费者读取: 快速失败 (见上方 try_ 契约注释)
                if (retry++ != 0) {
                    return false;
                }
                CPU_PAUSE();
                continue;
            }
        }
    }
};

}  // namespace runtime::ringbuffer

#endif  // QUANT1X_RUNTIME_RINGBUFFER_H
