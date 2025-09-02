#pragma once
#ifndef QUANT1X_RUNTIME_CACHE1D_H
#define QUANT1X_RUNTIME_CACHE1D_H 1

// ==============================
// 每日定时更新的缓存机制
// ==============================

#include "once.h"

namespace runtime {

    template <typename T>
    class cache1d {
    public:
        template <typename Func, typename = std::enable_if_t<!std::is_same_v<std::decay_t<Func>, cache1d>>>
        explicit cache1d(const std::string &name, Func &&init, const std::string &spec = "0 0 9 * * *")
            : once_(PeriodicOnce<T>(std::forward<Func>(init))), task_name_(name) {
            task_id_ = runtime::add_task(task_name_, spec, [&]() { once_.reset(); });
            spdlog::warn("cache1d add({}:{})", task_id_, task_name_);
        }

        ~cache1d() {
            spdlog::warn("cache1d cancel({}:{})", task_id_, task_name_);
            runtime::cancel_task(task_id_);
        }

        T &get() { return once_.get(); }

        // 允许隐式转换
        operator T &() { return once_.get(); }
        // 允许隐式转换
        operator const T &() const { return once_.get(); }

    private:
        PeriodicOnce<T> once_;
        int64_t         task_id_;
        std::string     task_name_;
    };
}  // namespace runtime

#endif  // QUANT1X_RUNTIME_CACHE1D_H
