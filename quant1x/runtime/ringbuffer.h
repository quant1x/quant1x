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
#include <chrono>
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

// 跨编译器的属性宏: 在 GCC/Clang 上使用 always_inline/hot, 以期望代码内联和热路径优化；
// 在 MSVC 上使用 __forceinline. 
#if defined(_MSC_VER)
#  define ATTR_ALWAYS_INLINE_HOT __forceinline
#else
#  define ATTR_ALWAYS_INLINE_HOT inline __attribute__((always_inline, hot))
#endif

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
            size_t pos =
#if defined(__GNUG__)
                atomic_load_relaxed_gcc(dequeue_pos_);
#else
                dequeue_pos_.load(std::memory_order_relaxed);
#endif
            Slot& slot = buffer_[pos & mask_];
            size_t seq = slot.seq.load(std::memory_order_acquire);

            if (seq == pos + 1) {
                size_t expected_pos = pos;
#if defined(__GNUG__)
                if (atomic_cas_weak_gcc(dequeue_pos_, expected_pos, pos + 1)) {
#else
                if (dequeue_pos_.compare_exchange_weak(expected_pos, pos + 1,
                    std::memory_order_acquire, std::memory_order_relaxed)) {
#endif
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
            size_t pos =
#if defined(__GNUG__)
                atomic_load_relaxed_gcc(dequeue_pos_);
#else
                dequeue_pos_.load(std::memory_order_relaxed);
#endif
            Slot& slot = buffer_[pos & mask_];
            size_t seq = slot.seq.load(std::memory_order_acquire);

            if (seq == pos + 1) {
                size_t expected_pos = pos;
#if defined(__GNUG__)
                if (atomic_cas_weak_gcc(dequeue_pos_, expected_pos, pos + 1)) {
#else
                if (dequeue_pos_.compare_exchange_weak(expected_pos, pos + 1,
                    std::memory_order_acquire, std::memory_order_relaxed)) {
#endif
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
    alignas(64) std::atomic<size_t> enqueue_pos_{0};
    // 消费者游标(下一个待读取序号), 按缓存行对齐以减少伪共享
    alignas(64) std::atomic<size_t> dequeue_pos_{0};
    // 关闭标志(true 表示队列已关闭), 消费者可据此在空队列时退出
    std::atomic<bool> closed_{false};

#if defined(__GNUG__)
    // 针对 GCC 的轻量封装, 基于 __atomic 内建函数, 用于在 Windows (MinGW) 上
    // 尝试引导更高效的热路径原子操作代码生成
    static inline size_t atomic_load_relaxed_gcc(const std::atomic<size_t>& a) noexcept {
        return __atomic_load_n(reinterpret_cast<const size_t*>(&a), __ATOMIC_RELAXED);
    }

    static inline bool atomic_cas_weak_gcc(std::atomic<size_t>& a, size_t& expected, size_t desired) noexcept {
        return __atomic_compare_exchange_n(reinterpret_cast<size_t*>(&a), &expected, desired, true, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED);
    }
#endif

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
    // - 之后: sleep 50us, 毫秒级, 长时间等待时彻底释放 CPU
    // 退避计数器在重试间保持, 竞争越久退避越激进, 避免多生产者/消费者互相饿死.
    static void backoff_spin(uint32_t& iter) noexcept {
        if (iter < 4) {
            CPU_PAUSE();
        } else if (iter < 8) {
            std::this_thread::yield();
        } else {
            std::this_thread::sleep_for(std::chrono::microseconds(50));
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
            size_t pos =
#if defined(__GNUG__)
                atomic_load_relaxed_gcc(enqueue_pos_);
#else
                enqueue_pos_.load(std::memory_order_relaxed);
#endif
            Slot& slot = buffer_[pos & mask_];
            size_t seq = slot.seq.load(std::memory_order_acquire);

            if (seq == pos) {
                // 槽位已准备好写入
                size_t expected_pos = pos;
#if defined(__GNUG__)
                if (atomic_cas_weak_gcc(enqueue_pos_, expected_pos, pos + 1)) {
#else
                if (enqueue_pos_.compare_exchange_weak(expected_pos, pos + 1,
                    std::memory_order_acq_rel, std::memory_order_relaxed)) {
#endif
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
            size_t pos =
#if defined(__GNUG__)
                atomic_load_relaxed_gcc(enqueue_pos_);
#else
                enqueue_pos_.load(std::memory_order_relaxed);
#endif
            Slot& slot = buffer_[pos & mask_];
            size_t seq = slot.seq.load(std::memory_order_acquire);

            if (seq == pos) {
                // 槽位已准备好写入
                size_t expected_pos = pos;
#if defined(__GNUG__)
                if (atomic_cas_weak_gcc(enqueue_pos_, expected_pos, pos + 1)) {
#else
                if (enqueue_pos_.compare_exchange_weak(expected_pos, pos + 1,
                    std::memory_order_acq_rel, std::memory_order_relaxed)) {
#endif
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
