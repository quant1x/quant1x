#pragma once
#ifndef QUANT1X_CORE_LAZY_H
#define QUANT1X_CORE_LAZY_H

#include <mutex>
#include <functional>

namespace quant1x {
namespace core {

// 懒加载模板类，提供线程安全的延迟初始化
template <typename T>
class Lazy {
public:
    // 构造函数，接受初始化函数
    explicit Lazy(std::function<T()> initializer)
        : initializer_(std::move(initializer)) {}

    // 获取值的引用（适用于不可拷贝的类型）
    const T& get() {
        std::call_once(flag_, [this]() {
            value_ = initializer_();
        });
        return value_;
    }

    // 获取值的拷贝（适用于可拷贝的类型）
    T get_copy() {
        std::call_once(flag_, [this]() {
            value_ = initializer_();
        });
        return value_;
    }

private:
    std::function<T()> initializer_;
    mutable std::once_flag flag_;
    mutable T value_;
};

// 简化版本：直接返回值的懒加载（适用于可默认构造的类型）
template <typename T>
class LazyValue {
public:
    explicit LazyValue(std::function<void(T&)> initializer)
        : initializer_(std::move(initializer)) {}

    const T& get() {
        std::call_once(flag_, [this]() {
            initializer_(value_);
        });
        return value_;
    }

    T get_copy() {
        std::call_once(flag_, [this]() {
            initializer_(value_);
        });
        return value_;
    }

private:
    std::function<void(T&)> initializer_;
    mutable std::once_flag flag_;
    mutable T value_;
};

} // namespace core
} // namespace quant1x

#endif // QUANT1X_CORE_LAZY_H