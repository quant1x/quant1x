// vyukov.hpp
// C++17 header-only port of Vyukov bounded MPMC queue (template)
// Implementation notes:
// - Uses per-slot sequence numbers and atomic enqueue/dequeue indices
// - Aligns slots to 64 bytes to reduce false sharing
// - Provides try_push / try_pop semantics (non-blocking: returns false on full/empty)
// - Includes a close() method so consumers can observe queue closure

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#if defined(_MSC_VER)
#  include <intrin.h>
#  define CPU_PAUSE() _mm_pause()
#else
#  include <immintrin.h>
#  define CPU_PAUSE() _mm_pause()
#endif
#include <thread>
#include <chrono>
#include <type_traits>
#include <new>

// Cross-compiler attribute: prefer force-inline/hot on GCC/Clang, and __forceinline on MSVC.
#if defined(_MSC_VER)
#  define ATTR_ALWAYS_INLINE_HOT __forceinline
#else
#  define ATTR_ALWAYS_INLINE_HOT inline __attribute__((always_inline, hot))
#endif

namespace quant1x::ringbuffer {

template<typename T>
class VyukovMPMC {
public:
    explicit VyukovMPMC(size_t capacity) {
        size_t cap = 1;
        while (cap < capacity) cap <<= 1;
        mask_ = cap - 1;
        buffer_.reset(new Slot[cap]);
        for (size_t i = 0; i < cap; ++i) {
            buffer_[i].seq.store(i, std::memory_order_relaxed);
        }
        enqueue_pos_.store(0, std::memory_order_relaxed);
        dequeue_pos_.store(0, std::memory_order_relaxed);
        closed_.store(0, std::memory_order_relaxed);
    }

    ~VyukovMPMC() {
        // Attempt to drain and destroy any remaining objects to avoid leaks.
        T tmp;
        while (try_pop(tmp)) {
            // destroy by popping
        }
    }

    // Non-blocking push. Returns true on success, false when queue is full.
    bool try_push(const T& value) {
        return emplace(value);
    }

    bool try_push(T&& value) {
        return emplace(std::move(value));
    }

    // Non-blocking pop. Returns true and writes into `out` on success.
    // Returns false if queue is empty and closed (or empty at the moment).
    ATTR_ALWAYS_INLINE_HOT bool try_pop(T& out) {
        uint32_t backoff = 0;
        while (true) {
            size_t pos = dequeue_pos_.load(std::memory_order_relaxed);
            Slot& slot = buffer_[pos & mask_];
            size_t seq = slot.seq.load(std::memory_order_acquire);
            if (seq == pos + 1) {
                if (dequeue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_acquire, std::memory_order_relaxed)) {
                    T* ptr = reinterpret_cast<T*>(&slot.storage);
                    out = std::move(*ptr);
                    ptr->~T();
                    slot.seq.store(pos + mask_ + 1, std::memory_order_release);
                    return true;
                } else {
                    backoff_spin(backoff);
                    continue;
                }
            } else if (seq < pos + 1) {
                // empty at the moment
                if (closed_.load(std::memory_order_acquire) != 0) return false;
                return false;
            } else {
                backoff_spin(backoff);
                continue;
            }
        }
    }

    void close() noexcept {
        closed_.store(1, std::memory_order_release);
    }

private:
    struct Slot {
        alignas(64) std::atomic<size_t> seq{0};
        // raw storage for T
        typename std::aligned_storage<sizeof(T), alignof(T)>::type storage;
    };

    std::unique_ptr<Slot[]> buffer_;
    size_t mask_ = 0;
    alignas(64) std::atomic<size_t> enqueue_pos_{0};
    alignas(64) std::atomic<size_t> dequeue_pos_{0};
    std::atomic<int> closed_{0};

    static ATTR_ALWAYS_INLINE_HOT void backoff_spin(uint32_t &iter) {
        if (iter < 8) {
            // tight pause
            CPU_PAUSE();
        } else if (iter < 16) {
            std::this_thread::yield();
        } else {
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
        if (iter != UINT32_MAX) ++iter;
    }

    template<typename U>
    ATTR_ALWAYS_INLINE_HOT bool emplace(U&& value) {
        // Match Rust: use CAS-based reservation for producer (compare_exchange loop)
        uint32_t backoff = 0;
        while (true) {
            size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
            Slot& slot = buffer_[pos & mask_];
            size_t seq = slot.seq.load(std::memory_order_acquire);
            if (seq == pos) {
                if (enqueue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_acq_rel, std::memory_order_relaxed)) {
                    T* ptr = reinterpret_cast<T*>(&slot.storage);
                    new (ptr) T(std::forward<U>(value));
                    slot.seq.store(pos + 1, std::memory_order_release);
                    return true;
                } else {
                    backoff_spin(backoff);
                    continue;
                }
            } else if (seq < pos) {
                // full
                return false;
            } else {
                backoff_spin(backoff);
                continue;
            }
        }
    }
};

} // namespace quant1x::ringbuffer
