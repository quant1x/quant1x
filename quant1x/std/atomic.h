#pragma once
#ifndef QUANT1X_STD_ATOMIC_H
#define QUANT1X_STD_ATOMIC_H 1

#include <atomic>
#include <memory>

namespace base {

    /**
     * 原子共享指针封装类
     *
     * 用于兼容 C++20 的 std::atomic<std::shared_ptr<T>> 和旧标准的 std::atomic_load/store。
     * 在 C++20 中，std::atomic<std::shared_ptr<T>> 提供了更高效和安全的实现。
     * 在旧标准中，使用全局函数 std::atomic_load/store 操作 std::shared_ptr。
     */
    template <typename T>
    class atomic_share_ptr {
    public:
        atomic_share_ptr() = default;

        /**
         * 原子存储
         * @param desired 要存储的新共享指针
         */
        void store(std::shared_ptr<T> desired) {
#if defined(__cpp_lib_atomic_shared_ptr)
            _ptr.store(desired);
#else
            std::atomic_store(&_ptr, desired);
#endif
        }

        /**
         * 原子加载
         * @return 当前存储的共享指针
         */
        std::shared_ptr<T> load() const {
#if defined(__cpp_lib_atomic_shared_ptr)
            return _ptr.load();
#else
            return std::atomic_load(&_ptr);
#endif
        }

    private:
#if defined(__cpp_lib_atomic_shared_ptr)
        std::atomic<std::shared_ptr<T>> _ptr;
#else
        std::shared_ptr<T>              _ptr;
#endif
    };
}

#endif  // QUANT1X_STD_ATOMIC_H