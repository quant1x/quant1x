// Vyukov 有界 MPMC 队列的 C++17 头文件单文件移植(模板)
// 实现说明(中文): 
// - 基于每个槽位的序号(per-slot sequence)以及原子性的入队/出队索引实现无锁并发
// - 槽按 64 字节对齐以最小化伪共享(false sharing)带来的性能下降
// - 提供非阻塞的 try_push / try_pop 语义: 满/空时立即返回 false
// - 提供 close() 方法, 供消费者观察队列关闭并在耗尽数据后退出
// - 待修复/注意: 析构的线程安全性, 异常安全性与潜在内存泄漏需要在使用时注意
// - 可优化之处: 利用更现代的 C++17 特性, 改进退避策略以提升不同平台上的性能
#pragma once
#ifndef QUANT1X_RUNTIME_RINGBUFFER_H
#define QUANT1X_RUNTIME_RINGBUFFER_H 1

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <new>
#include <utility>
#include <stdexcept>
#include <exception>

#if defined(_MSC_VER)
#  include <intrin.h>
#  define CPU_PAUSE() _mm_pause()
#else
#  include <immintrin.h>
#  define CPU_PAUSE() _mm_pause()
#endif

#include <thread>
#include <chrono>

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
        buffer_ = std::make_unique<Slot[]>(cap);
        // 初始化序号
        for (size_t i = 0; i < cap; ++i) {
            buffer_[i].seq.store(i, std::memory_order_relaxed);
        }
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

    // 非阻塞出队. 成功时返回 true 并将元素写入 `out`. 
    // 如果队列为空(或为空且已关闭)则返回 false. 
    ATTR_ALWAYS_INLINE_HOT bool try_pop(T& out) noexcept(
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
                    backoff_spin(backoff);
                    continue;
                }
            } else if (seq < pos + 1) {
                // 槽为空
                if (closed_.load(std::memory_order_acquire)) {
                    return false; // 队列已关闭且为空
                }
                return false; // 队列为空但未关闭
            } else {
                // 槽正被其他生产者写入
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

private:
    struct Slot {
        alignas(64) std::atomic<size_t> seq{0};
        // 使用 std::byte 作为原始存储以在 C++17+ 中提供更好的类型安全
        alignas(alignof(T)) std::byte storage[sizeof(T)]{};
    };

    // 槽位数组, 长度为 capacity(向上取整到 2 的幂), 每个槽按 64 字节对齐
    std::unique_ptr<Slot[]> buffer_;
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

    static inline void atomic_store_release_gcc(std::atomic<size_t>& a, size_t v) noexcept {
        __atomic_store_n(reinterpret_cast<size_t*>(&a), v, __ATOMIC_RELEASE);
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
        v |= v >> 32;
        v++;
        return v;
    }

    static ATTR_ALWAYS_INLINE_HOT void backoff_spin(uint32_t& iter) noexcept {
        if (iter < 4) {
            // 短时紧自旋并执行 CPU pause
            for (uint32_t i = 0; i < (1u << iter); ++i) {
                CPU_PAUSE();
            }
        } else if (iter < 8) {
            // 让出以允许其他线程运行
            std::this_thread::yield();
        } else {
            // 更长时间睡眠以降低 CPU 使用率
            std::this_thread::sleep_for(std::chrono::microseconds(1u << (iter - 8)));
        }
        if (iter < 16) ++iter; // 限制退避上限以防止溢出
    }

    template<typename U>
    ATTR_ALWAYS_INLINE_HOT bool emplace(U&& value) noexcept(
        std::is_nothrow_constructible_v<T, U> &&
        std::is_nothrow_destructible_v<T>
    ) {
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
                    // 成功预留该槽位
                    try {
                        T* ptr = std::launder(reinterpret_cast<T*>(&slot.storage));
                        new (ptr) T(std::forward<U>(value));
                        slot.seq.store(pos + 1, std::memory_order_release);
                        return true;
                    } catch (...) {
                        // 构造失败 - 回滚预留
                        // 这种情况很少见, 但为了异常安全必须处理
                        // 若可用则使用 GCC 原子存储辅助函数
#if defined(__GNUG__)
                        atomic_store_release_gcc(enqueue_pos_, pos);
#else
                        enqueue_pos_.store(pos, std::memory_order_release);
#endif
                        slot.seq.store(pos, std::memory_order_release);
                        // 如果该实例化是 noexcept 的, 重新抛出会直接调用
                        // std::terminate(编译器会发出 -Wterminate 警告). 
                        // 因此使用 constexpr 分支: 在 noexcept 情况下调用
                        // std::terminate, 否则重新抛出异常以保留原始行为. 
                        if constexpr (
                            std::is_nothrow_constructible_v<T, U> &&
                            std::is_nothrow_destructible_v<T>
                        ) {
                            std::terminate();
                        } else {
                            throw;
                        }
                    }
                } else {
                    backoff_spin(backoff);
                    continue;
                }
            } else if (seq < pos) {
                // 队列已满
                return false;
            } else {
                // 槽正被其他消费者读取
                backoff_spin(backoff);
                continue;
            }
        }
    }
};

}  // namespace runtime::ringbuffer

#endif  // QUANT1X_RUNTIME_RINGBUFFER_H
