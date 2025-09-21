#pragma once
#ifndef QUANT1X_BASE_ONCE_H
#define QUANT1X_BASE_ONCE_H 1

#include <iostream>
#include <atomic>
#include <mutex>
#include <functional>
#include <optional>
#include <format>
#include "core.h"

/**
 * 在固定时间窗口内保证操作只执行一次，窗口通过 cron 表达式或时间间隔定义。
 * 每次窗口结束时自动重置，允许下个窗口重新执行。
 */
template<typename T>
class PeriodicOnce {
public:
    // 构造函数：接受初始化函数（支持任意可调用对象）
    template<typename Func, typename = std::enable_if_t<!std::is_same_v<std::decay_t<Func>, PeriodicOnce>>>
    explicit PeriodicOnce(Func&& init): init_(std::forward<Func>(init)) {}

    // 获取值，首次调用或重置后执行初始化函数
    T& get() {
        if (!done_.load(std::memory_order_acquire)) {
            std::lock_guard lock(mutex_);
            if (!done_) {
                value_.emplace(init_());  // 调用初始化函数
                done_.store(true, std::memory_order_release);
            }
        }
        return *value_;
    }

    // 重置状态，允许下次调用 get 时重新初始化
    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        done_.store(false, std::memory_order_release);
        value_.reset();  // 清空存储的值
    }

    // 禁止拷贝和赋值
    PeriodicOnce(const PeriodicOnce&) = delete;
    PeriodicOnce& operator=(const PeriodicOnce&) = delete;

    // 允许隐式转换
    explicit operator T&() {
        return get();
    }
    // 允许隐式转换
    explicit operator const T&() const {
        return get();
    }
private:
    std::function<T()> init_;        // 存储初始化函数
    std::atomic<bool> done_{false};  // 初始化完成标志
    std::optional<T> value_;         // 存储泛型值
    std::mutex mutex_;               // 保护并发访问的互斥锁
};

/// 滑动窗口期内的一次性初始化
/// 支持cron表达式, 定期再次初始化, 依赖全局的任务调度模块
class RollingOnce : public std::enable_shared_from_this<RollingOnce> {
public:
    // 工厂方法确保对象由 shared_ptr 管理
    static std::shared_ptr<RollingOnce> create(const std::string& name, int seconds = 5) {
#if std_cplusplus < 20
        char buf[20] = {0};
        std::snprintf(buf, sizeof(buf), "*/%d * * * * *", seconds % 60);
        std::string spec = buf;
#else
        std::string spec = std::format("*/{} * * * * *", seconds % 60);
#endif
        return create(name, spec);
    }

    static std::shared_ptr<RollingOnce> create(const std::string& name, int hour, int minute) {
#if std_cplusplus < 20
        char buf[20] = {0};
        std::snprintf(buf, sizeof(buf), "0 %d %d * * *", minute, hour);
        std::string spec = buf;
#else
        std::string spec = std::format("0 {} {} * * *", minute, hour);
#endif
        return create(name, spec);
    }

    // 工厂方法确保对象由 shared_ptr 管理
    static std::shared_ptr<RollingOnce> create(const std::string& name, const std::string &spec) {
        auto ptr = std::shared_ptr<RollingOnce>(new RollingOnce());
        ptr->schedule_reset_task(name, spec);
        return ptr;
    }

    ~RollingOnce() {
        // 取消定时任务（需 runtime 库支持）
        if (task_id_ != -1) {
            spdlog::debug("RollingOnce cancel({})", task_id_);
            runtime::cancel_task(task_id_);
        }
    }

    template<typename F>
    void Do(F&& f) {
        if (done_.load(std::memory_order_acquire) == 0) {
            // 包裹为 std::function 只在真正需要执行时发生一次分配
            std::function<void()> wrapper(std::forward<F>(f));
            doSlow(wrapper);
        }
    }

private:
    RollingOnce() : done_(0), task_id_(-1) {}

    void schedule_reset_task(const std::string& name, const std::string &spec) {
        //std::string spec = std::format("0 {} {} * * *", minute_, hour_);
        // 使用 weak_ptr 避免循环引用
        std::weak_ptr<RollingOnce> weak_this = shared_from_this();

        // 假设 runtime::add_task 返回任务 ID
        task_id_ = runtime::add_task(name, spec, [weak_this]() {
            // 尝试提升为 shared_ptr（线程安全检查）
            if (auto shared_this = weak_this.lock()) {
//                std::lock_guard<std::mutex> lock(shared_this->m_);
//                shared_this->done_.store(0, std::memory_order_release);
                shared_this->Reset();
                spdlog::debug("reset {}", shared_this->task_id_);
            }
        });
        spdlog::debug("RollingOnce add({})", task_id_);
    }

    void doSlow(const std::function<void()> &f) {
        std::unique_lock<std::mutex> lock(m_);
        if (done_.load(std::memory_order_relaxed) == 0) {
            try {
                f();
            } catch (...) {
                // 即使异常也要标记为完成
                done_.store(1, std::memory_order_release);
                throw;
            }
            done_.store(1, std::memory_order_release);
        }
    }

    void Reset() {
        std::unique_lock<std::mutex> lock(m_);
        // 确保所有正在进行的Do调用完成
        done_.store(0, std::memory_order_release);
    }

    //int hour_;
    //int minute_;
    std::atomic<uint32_t> done_;
    std::mutex m_;
    int64_t task_id_;
};

#endif //QUANT1X_BASE_ONCE_H
