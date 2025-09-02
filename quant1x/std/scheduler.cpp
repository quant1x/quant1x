#include <quant1x/std/scheduler.h>

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

    try {
        const auto id        = next_id_++;
        const auto cron      = cron::make_cron(cron_expr);
        const auto first_run = cron::cron_next(cron, Clock::now());
        cron_tasks_.emplace(id, CronTask{false, cron, std::move(task)});
        enqueue_task(ScheduledTask{first_run, [this, id, name] { execute_cron_task(id, name); }, id, name});

        return id;
    } catch (const cron::bad_cronexpr &e) {
        throw std::invalid_argument("Invalid cron expression: " + std::string(e.what()));
    }
}

void AsyncScheduler::cancel(runtime::task_id id) {
    std::lock_guard lock(mutex_);
    // 防御性检查：确保任务存在
    if (cron_tasks_.find(id) != cron_tasks_.end()) {
        canceled_tasks_.insert(id);
        cron_tasks_.erase(id);
    }
    condition_.notify_all();
}

void AsyncScheduler::scheduler_loop() {
    spdlog::info("scheduler_loop...start");
    while (running_) {
        std::unique_lock lock(mutex_);
        if (task_queue_.empty()) {
            spdlog::info("任务队列为空，等待新任务...");
            // 添加运行状态检查的条件等待
            condition_.wait(lock, [this] { return !task_queue_.empty() || !running_; });
            if (!running_) {
                break;
            }
            spdlog::info("任务队列不为空，继续调度...");
            continue;
        }

        // 检查队列顶部任务是否可以执行
        const auto &top_task = task_queue_.top();
        if (Clock::now() >= top_task.next_run) {
            spdlog::info("执行任务: id={}, name={}", top_task.id, top_task.name);
            // 先拷贝任务数据，再 pop
            ScheduledTask task_to_run = top_task;  // 拷贝而非引用
            task_queue_.pop();
            lock.unlock();
            if (!canceled_tasks_.count(task_to_run.id)) {
                // 在线程池任务中添加运行状态检查
                pool_.detach_task([this, task_to_run] {
                    if (running_) {
                        task_to_run.task();
                    }
                });
            } else {
                spdlog::warn("执行任务: id={}, name={}, 未执行", task_to_run.id, task_to_run.name);
            }
        } else {
            //spdlog::debug("等待下一个任务执行时间: {}, {}", task_to_run.name, std::chrono::duration_cast<std::chrono::milliseconds>(task_to_run.next_run - Clock::now()).count());
            // 带条件的超时等待
            condition_.wait_for(lock, std::chrono::milliseconds(100), [this] { return !running_; });
            if (!running_) {
                break;
            }
        }
    }
    spdlog::info("scheduler_loop...stop");
}

void AsyncScheduler::reschedule_cron(runtime::task_id id, const std::string &name, const cron::cronexpr &cron) {
    std::lock_guard lock(mutex_);
    if (canceled_tasks_.count(id))
        return;

    try {
        const auto next_time = cron::cron_next(cron, Clock::now());
        enqueue_task(ScheduledTask{next_time, [this, id, name] { execute_cron_task(id, name); }, id, name});
    } catch (const cron::bad_cronexpr &e) {
        // 处理无效的cron表达式
        spdlog::error("Failed to reschedule cron task {}: {}", id, e.what());
    }
}

void AsyncScheduler::stop() {
    if (!running_.exchange(false))
        return;  // 确保只执行一次关闭

    // 1. 取消所有待处理任务
    {
        std::lock_guard lock(mutex_);
        canceled_tasks_.clear();
        cron_tasks_.clear();
        while (!task_queue_.empty()) {
            task_queue_.pop();
        }
    }

    // 2. 唤醒所有等待线程
    condition_.notify_all();

    // 3. 等待调度器线程结束
    if (scheduler_thread_.joinable()) {
        scheduler_thread_.join();
    }

    // 4. 等待线程池任务完成
    pool_.wait();
}

void AsyncScheduler::execute_cron_task(runtime::task_id id, const std::string &name) {
    std::function<void()> task;
    cron::cronexpr        cron;

    {
        std::lock_guard lock(mutex_);
        if (canceled_tasks_.count(id)) {
            spdlog::warn("Task {} canceled during execution", name);
            return;
        }
        auto &cron_task = cron_tasks_.at(id);
        // 如果已经在运行，则跳过本次调度
        if (cron_task.cron_running) {
            spdlog::warn("Task {} skipped: previous execution still running", id);
            return;
        }

        cron_task.cron_running = true;  // 标记为正在运行
        task                   = cron_task.task;
        cron                   = cron_task.expr;
    }

    try {
        task();
    } catch (const std::exception &e) {
        // 处理任务异常
        spdlog::error("execute_cron_task - 标准异常: {} (type: {})", e.what(), typeid(e).name());
    }
    {
        std::lock_guard lock(mutex_);
        if (canceled_tasks_.count(id)) {
            spdlog::warn("Task {} canceled during execution", name);
            return;
        }
        auto &cron_task        = cron_tasks_.at(id);
        cron_task.cron_running = false;  // 标记为不运行
    }
    reschedule_cron(id, name, cron);
}

void AsyncScheduler::enqueue_task(AsyncScheduler::ScheduledTask &&task) {
    task_queue_.push(std::move(task));
    condition_.notify_all();
}
