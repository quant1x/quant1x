#include <quant1x/runtime/scheduler.h>

AsyncScheduler::AsyncScheduler(size_t thread_count) : pool_(thread_count), running_(true), next_id_(1) {
    spdlog::info("start scheduler...");
    scheduler_thread_ = std::thread([this] { scheduler_loop(); });
    spdlog::info("start scheduler...OK");
}

AsyncScheduler::~AsyncScheduler() {
    spdlog::warn("stop scheduler begin");
    stop();
    spdlog::warn("stop scheduler end");
}

runtime::task_id
AsyncScheduler::schedule_cron(const std::string &name, const std::string &cron_expr, std::function<void()> task) {
    std::lock_guard lock(mutex_);

    if (!running_) {
        throw std::runtime_error("schedule_cron called after scheduler stopped");
    }

    try {
        const auto id        = next_id_++;
        const auto cron      = cron::make_cron(cron_expr);
        const auto first_run = cron::cron_next(cron, Clock::now());
    cron_tasks_.emplace(id, CronTask{false, false, cron, std::move(task)});
        enqueue_task(ScheduledTask{first_run, [this, id, name] { execute_cron_task(id, name); }, id, name});
        ++st_scheduled_;

        return id;
    } catch (const cron::bad_cronexpr &e) {
        throw std::invalid_argument("Invalid cron expression: " + std::string(e.what()));
    }
}

void AsyncScheduler::cancel(runtime::task_id id) {
    std::lock_guard lock(mutex_);
    auto it = cron_tasks_.find(id);
    if (it != cron_tasks_.end()) {
        it->second.canceled = true; // 软取消，避免执行路径崩溃
        ++st_canceled_;
    }
    condition_.notify_all();
}

void AsyncScheduler::scheduler_loop() {
    spdlog::info("scheduler_loop...start");
    while (running_) {
        std::unique_lock lock(mutex_);

        if (task_queue_.empty()) {
            condition_.wait(lock, [this]{ return !task_queue_.empty() || !running_; });
            if (!running_) break;
            continue;
        }

        const auto &top_task = task_queue_.top();
        auto now = Clock::now();
        if (now < top_task.next_run) {
            condition_.wait_until(lock, top_task.next_run, [this]{ return !running_; });
            if (!running_) break;
            continue;
        }

        ScheduledTask task_to_run = top_task;
        task_queue_.pop();
        lock.unlock();

        if (!running_) break;
        // 如果是 cron 任务且已经标记取消, 不执行
        if (auto it = cron_tasks_.find(task_to_run.id); it != cron_tasks_.end() && it->second.canceled) {
            spdlog::debug("跳过取消任务 id={}, name={}", task_to_run.id, task_to_run.name);
            ++st_skipped_cancel_;
            continue;
        }

        pool_.detach_task([this, task_to_run] {
            try {
                if (running_) {
                    task_to_run.task();
                }
            } catch (const std::exception &e) {
                spdlog::error("任务执行异常 id={}, name={}, error={}", task_to_run.id, task_to_run.name, e.what());
            } catch (...) {
                spdlog::error("任务执行未知异常 id={}, name={}", task_to_run.id, task_to_run.name);
            }
        });
    }
    spdlog::info("scheduler_loop...stop");
}

void AsyncScheduler::reschedule_cron(runtime::task_id id, const std::string &name, const cron::cronexpr &cron) {
    std::lock_guard lock(mutex_);
    if (auto it = cron_tasks_.find(id); it == cron_tasks_.end() || it->second.canceled)
        return; // 已删除或已取消

    try {
        const auto next_time = cron::cron_next(cron, Clock::now());
        enqueue_task(ScheduledTask{next_time, [this, id, name] { execute_cron_task(id, name); }, id, name});
        ++st_rescheduled_;
    } catch (const cron::bad_cronexpr &e) {
        // 处理无效的cron表达式
        spdlog::error("Failed to reschedule cron task {}: {}", id, e.what());
    }
}

void AsyncScheduler::stop() {
    if (!running_.exchange(false)) {
        return;  // 确保只执行一次关闭
    }
    // 1. 唤醒调度线程使其尽快退出（不急于清容器，避免执行路径访问已清空结构）
    condition_.notify_all();

    // 2. 等待调度器线程结束
    if (scheduler_thread_.joinable()) {
        scheduler_thread_.join();
    }

    // 3. 等待线程池中已经派发的任务完成
    pool_.wait();

    // 4. 现在没有并发访问了，安全清理容器
    {
        std::lock_guard lock(mutex_);
        cron_tasks_.clear();
        while (!task_queue_.empty()) task_queue_.pop();
    }
}

void AsyncScheduler::execute_cron_task(runtime::task_id id, const std::string &name) {
    std::function<void()> task;
    cron::cronexpr        expr;
    bool                  need_reschedule = false;

    {
        std::lock_guard lock(mutex_);
        auto it = cron_tasks_.find(id);
        if (it == cron_tasks_.end()) return; // 已被stop清理或不存在
        auto &ct = it->second;
        if (ct.canceled) return;             // 已取消
        if (ct.cron_running) {
            spdlog::warn("Task {} skipped: previous execution still running", id);
            ++st_skipped_running_;
            return;
        }
        ct.cron_running = true;
        task            = ct.task;
        expr            = ct.expr;
        need_reschedule = true;              // 先假定要重排，再根据后续状态确认
    }

    try {
        task();
    } catch (const std::exception &e) {
        spdlog::error("execute_cron_task - 标准异常: {} (type: {})", e.what(), typeid(e).name());
    } catch (...) {
        spdlog::error("execute_cron_task - 未知异常 id={}", id);
    }

    {
        std::lock_guard lock(mutex_);
        auto it = cron_tasks_.find(id);
        if (it != cron_tasks_.end()) {
            it->second.cron_running = false;
            if (it->second.canceled || !running_) {
                need_reschedule = false;
            }
        } else {
            need_reschedule = false;
        }
    }

    if (need_reschedule) {
        ++st_executed_;
        reschedule_cron(id, name, expr);
    }
}

void AsyncScheduler::enqueue_task(AsyncScheduler::ScheduledTask &&task) {
    task_queue_.push(std::move(task));
    condition_.notify_all();
}

AsyncScheduler::Stats AsyncScheduler::get_stats() const {
    Stats s;
    s.scheduled       = st_scheduled_.load(std::memory_order_relaxed);
    s.executed        = st_executed_.load(std::memory_order_relaxed);
    s.skipped_cancel  = st_skipped_cancel_.load(std::memory_order_relaxed);
    s.skipped_running = st_skipped_running_.load(std::memory_order_relaxed);
    s.rescheduled     = st_rescheduled_.load(std::memory_order_relaxed);
    s.canceled        = st_canceled_.load(std::memory_order_relaxed);
    return s;
}
