#pragma once
#ifndef QUANT1X_STD_SCHEDULER_H
#define QUANT1X_STD_SCHEDULER_H 1

#include <quant1x/runtime/core.h>
#include <croncpp.h>

#include <BS_thread_pool.hpp>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <unordered_map>

/**
 * @brief 调度任务自带延迟, 即上次没执行完本次延后到下一次执行
 */
class AsyncScheduler {
public:
    explicit AsyncScheduler(size_t thread_count = std::thread::hardware_concurrency());
    ~AsyncScheduler();
    runtime::task_id schedule_cron(const std::string& name, const std::string &cron_expr, std::function<void()> task);
    void             cancel(runtime::task_id id);
    void             stop();

    struct Stats {
        uint64_t scheduled      = 0; // 成功 schedule_cron 次数
        uint64_t executed       = 0; // 实际执行的任务次数
        uint64_t skipped_cancel = 0; // 因取消跳过次数
        uint64_t skipped_running= 0; // 因上次仍在运行被跳过
        uint64_t rescheduled    = 0; // 重新排程次数
        uint64_t canceled       = 0; // cancel() 调用命中次数
    };

    Stats get_stats() const;

private:
    using Clock     = std::chrono::system_clock;
    using TimePoint = Clock::time_point;

    struct ScheduledTask {
        TimePoint             next_run;  // 下一次触发的时间点
        std::function<void()> task;      // 任务回调函数
        runtime::task_id      id;        // 任务ID
        std::string           name;      // 任务名称

        bool operator<(const ScheduledTask &rhs) const {
            return next_run > rhs.next_run;  // 最小堆排序
        }
    };

    struct CronTask {
        bool                  cron_running;
        bool                  canceled{false};
        cron::cronexpr        expr;
        std::function<void()> task;
    };

    BS::thread_pool<BS::tp::none>                  pool_;
    std::thread                                    scheduler_thread_;
    std::atomic<bool>                              running_;
    std::priority_queue<ScheduledTask>             task_queue_;
    std::mutex                                     mutex_;
    std::condition_variable                        condition_;
    std::atomic<runtime::task_id>                  next_id_;
    std::unordered_map<runtime::task_id, CronTask> cron_tasks_;
    // 统计
    mutable std::atomic<uint64_t> st_scheduled_{0};
    mutable std::atomic<uint64_t> st_executed_{0};
    mutable std::atomic<uint64_t> st_skipped_cancel_{0};
    mutable std::atomic<uint64_t> st_skipped_running_{0};
    mutable std::atomic<uint64_t> st_rescheduled_{0};
    mutable std::atomic<uint64_t> st_canceled_{0};

    void enqueue_task(ScheduledTask &&task);
    void execute_cron_task(runtime::task_id id, const std::string& name);
    void reschedule_cron(runtime::task_id id, const std::string& name, const cron::cronexpr &cron);
    void scheduler_loop();
};

#endif  // QUANT1X_STD_SCHEDULER_H
