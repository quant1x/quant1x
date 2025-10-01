use std::collections::BinaryHeap;
use std::sync::Arc;
use std::time::Duration;
use chrono::{DateTime, Utc};
use tokio::sync::{Mutex, Notify};
use tokio::task;
use tokio::time::{sleep_until, Instant};
use cron::Schedule;
use std::str::FromStr;
use log::{debug, error, info, warn};
use std::collections::HashMap;

/// 任务ID类型
pub type TaskId = i64;

/// 调度器统计信息
#[derive(Debug, Clone, Default)]
pub struct SchedulerStats {
    pub scheduled: u64,       // 成功调度次数
    pub executed: u64,        // 实际执行的任务次数
    pub skipped_cancel: u64,  // 因取消跳过次数
    pub skipped_running: u64, // 因上次仍在运行被跳过
    pub rescheduled: u64,     // 重新排程次数
    pub canceled: u64,        // cancel() 调用命中次数
}

/// 定时任务结构体
#[derive(Clone)]
struct ScheduledTask {
    next_run: DateTime<Utc>,
    task: Arc<dyn Fn() + Send + Sync>,
    id: TaskId,
    name: String,
}

impl ScheduledTask {
    fn new(next_run: DateTime<Utc>, task: Arc<dyn Fn() + Send + Sync>, id: TaskId, name: String) -> Self {
        Self {
            next_run,
            task,
            id,
            name,
        }
    }
}

impl PartialEq for ScheduledTask {
    fn eq(&self, other: &Self) -> bool {
        self.next_run == other.next_run && self.id == other.id
    }
}

impl Eq for ScheduledTask {}

impl PartialOrd for ScheduledTask {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}

impl Ord for ScheduledTask {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        // 反转比较，使最早的任务排在前面（最小堆）
        other.next_run.cmp(&self.next_run).then_with(|| self.id.cmp(&other.id))
    }
}

/// Cron 任务结构体
struct CronTask {
    cron_running: bool,
    canceled: bool,
    expr: String,
    task: Arc<dyn Fn() + Send + Sync>,
}

impl CronTask {
    fn new(expr: String, task: Arc<dyn Fn() + Send + Sync>) -> Self {
        Self {
            cron_running: false,
            canceled: false,
            expr,
            task,
        }
    }
}

/// 异步调度器
pub struct AsyncScheduler {
    task_queue: Arc<Mutex<BinaryHeap<ScheduledTask>>>,
    cron_tasks: Arc<Mutex<std::collections::HashMap<TaskId, CronTask>>>,
    running: Arc<std::sync::atomic::AtomicBool>,
    next_id: Arc<std::sync::atomic::AtomicI64>,
    notify: Arc<Notify>,
    stats: Arc<Mutex<SchedulerStats>>,
    scheduler_handle: Mutex<Option<task::JoinHandle<()>>>,
}

impl AsyncScheduler {
    /// 创建新的调度器
    pub fn new() -> Self {
        let task_queue = Arc::new(Mutex::new(BinaryHeap::new()));
        let cron_tasks = Arc::new(Mutex::new(std::collections::HashMap::new()));
        let running = Arc::new(std::sync::atomic::AtomicBool::new(true));
        let next_id = Arc::new(std::sync::atomic::AtomicI64::new(1));
        let notify = Arc::new(Notify::new());
        let stats = Arc::new(Mutex::new(SchedulerStats::default()));

        // 启动调度器循环
        let handle = task::spawn(Self::scheduler_loop(
            task_queue.clone(),
            cron_tasks.clone(),
            running.clone(),
            notify.clone(),
            stats.clone(),
        ));

        Self {
            task_queue,
            cron_tasks,
            running,
            next_id,
            notify,
            stats,
            scheduler_handle: Mutex::new(Some(handle)),
        }
    }

    /// 调度 cron 任务
    pub async fn schedule_cron<F>(
        &self,
        name: String,
        cron_expr: &str,
        task: F,
    ) -> Result<TaskId, Box<dyn std::error::Error + Send + Sync>>
    where
        F: Fn() + Send + Sync + 'static,
    {
        if !self.running.load(std::sync::atomic::Ordering::Relaxed) {
            return Err("Scheduler is stopped".into());
        }

        let schedule = Schedule::from_str(cron_expr)?;
        let id = self.next_id.fetch_add(1, std::sync::atomic::Ordering::Relaxed);

        let task = Arc::new(task);
    let cron_task = CronTask::new(cron_expr.to_string(), task.clone());

        // 计算下次运行时间
        let now = Utc::now();
        let next_run = schedule.after(&now).next().unwrap_or(now);

        {
            let mut cron_tasks = self.cron_tasks.lock().await;
            cron_tasks.insert(id, cron_task);
        }

            let _tq = self.task_queue.clone();
            let _ct_map = self.cron_tasks.clone();
            let _stats_arc = self.stats.clone();
            let _running_arc = self.running.clone();
            let _notify_arc = self.notify.clone();

        // prepare separate clones to avoid moving `name` into closure
            let _closure_name = name.clone();
            let _scheduled_name = name.clone();

        // scheduled_task.task is a no-op; actual cron execution/rescheduling will be
        // handled in the scheduler loop when it detects a cron task by id.
        let scheduled_task = ScheduledTask::new(
            next_run,
            Arc::new(|| {}),
            id,
            name.clone(),
        );

        {
            let mut task_queue = self.task_queue.lock().await;
            task_queue.push(scheduled_task);
        }

        {
            let mut stats = self.stats.lock().await;
            stats.scheduled += 1;
        }

        self.notify.notify_one();

        Ok(id)
    }

    /// 取消任务
    pub async fn cancel(&self, id: TaskId) {
        let mut cron_tasks = self.cron_tasks.lock().await;
        if let Some(cron_task) = cron_tasks.get_mut(&id) {
            cron_task.canceled = true;
            let mut stats = self.stats.lock().await;
            stats.canceled += 1;
        }
        self.notify.notify_one();
    }

    /// 停止调度器
    pub async fn stop(&self) {
        if !self.running.swap(false, std::sync::atomic::Ordering::Relaxed) {
            return;
        }

        self.notify.notify_one();

        if let Some(handle) = self.scheduler_handle.lock().await.take() {
            let _ = handle.await;
        }

        // 清理资源
        let mut cron_tasks = self.cron_tasks.lock().await;
        cron_tasks.clear();

        let mut task_queue = self.task_queue.lock().await;
        while let Some(_) = task_queue.pop() {}
    }

    /// 获取统计信息
    pub async fn get_stats(&self) -> SchedulerStats {
        self.stats.lock().await.clone()
    }

    /// 调度器主循环
    async fn scheduler_loop(
        task_queue: Arc<Mutex<BinaryHeap<ScheduledTask>>>,
        cron_tasks: Arc<Mutex<std::collections::HashMap<TaskId, CronTask>>>,
        running: Arc<std::sync::atomic::AtomicBool>,
        notify: Arc<Notify>,
        stats: Arc<Mutex<SchedulerStats>>,
    ) {
        info!("scheduler_loop...start");

        while running.load(std::sync::atomic::Ordering::Relaxed) {
            let task_to_run = {
                let mut task_queue = task_queue.lock().await;

                // 等待任务或退出信号
                if task_queue.is_empty() {
                    drop(task_queue);
                    notify.notified().await;
                    continue;
                }

                let now = Utc::now();
                let top_task = task_queue.peek().unwrap();

                if now < top_task.next_run {
                    // 计算等待时间
                    let duration = (top_task.next_run - now).to_std().unwrap_or(Duration::from_secs(0));
                    let deadline = Instant::now() + duration;

                    drop(task_queue);
                    tokio::select! {
                        _ = sleep_until(deadline) => {},
                        _ = notify.notified() => {},
                    }
                    continue;
                }

                task_queue.pop().unwrap()
            };

            if !running.load(std::sync::atomic::Ordering::Relaxed) {
                break;
            }

            // 检查任务是否被取消
            {
                let cron_tasks = cron_tasks.lock().await;
                if let Some(cron_task) = cron_tasks.get(&task_to_run.id) {
                    if cron_task.canceled {
                        debug!("跳过取消任务 id={}, name={}", task_to_run.id, task_to_run.name);
                        let mut stats = stats.lock().await;
                        stats.skipped_cancel += 1;
                        continue;
                    }
                }
            }

            // 执行任务
            // If this is a cron task (exists in cron_tasks map) we run the cron execution path
            let is_cron = {
                let ct = cron_tasks.lock().await;
                ct.contains_key(&task_to_run.id)
            };

            if is_cron {
                let ct_clone = cron_tasks.clone();
                let tq_clone = task_queue.clone();
                let stats_clone = stats.clone();
                let running_clone = running.clone();
                let notify_clone = notify.clone();
                let id = task_to_run.id;
                let name = task_to_run.name.clone();
                task::spawn(async move {
                    execute_cron_task_internal(ct_clone, tq_clone, stats_clone, running_clone, notify_clone, id, name).await;
                });
            } else {
                let task = task_to_run.task.clone();
                task::spawn(async move {
                    task();
                });
            }
        }

        info!("scheduler_loop...stop");
    }

}

/// Internal helper to execute a cron task and reschedule it
async fn execute_cron_task_internal(
    cron_tasks: Arc<Mutex<HashMap<TaskId, CronTask>>>,
    task_queue: Arc<Mutex<BinaryHeap<ScheduledTask>>>,
    stats: Arc<Mutex<SchedulerStats>>,
    running: Arc<std::sync::atomic::AtomicBool>,
    notify: Arc<Notify>,
    id: TaskId,
    name: String,
) {
    // Extract task and schedule
    let (task, expr) = {
        let mut ct = cron_tasks.lock().await;
        match ct.get_mut(&id) {
            Some(ctask) => {
                if ctask.canceled {
                    return;
                }
                if ctask.cron_running {
                    let mut s = stats.lock().await;
                    s.skipped_running += 1;
                    return;
                }
                ctask.cron_running = true;
                (ctask.task.clone(), ctask.expr.clone())
            }
            None => return,
        }
    };

    // parse schedule locally so Schedule is not stored across await points
    let schedule = match Schedule::from_str(&expr) {
        Ok(s) => s,
        Err(e) => {
            error!("Invalid cron expr for task {}: {}", id, e);
            return;
        }
    };

    // execute
    let res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        (task)();
    }));
    if res.is_err() {
        error!("任务执行异常 id={}, name={}", id, name);
    }

    // update and reschedule
    {
        let mut ct = cron_tasks.lock().await;
        if let Some(ctask) = ct.get_mut(&id) {
            ctask.cron_running = false;
            if ctask.canceled || !running.load(std::sync::atomic::Ordering::Relaxed) {
                return;
            }
        } else {
            return;
        }
    }

    // increase stats
    {
        let mut s = stats.lock().await;
        s.executed += 1;
        s.rescheduled += 1;
    }

    // compute next run
    let now = Utc::now();
    if let Some(next_run) = schedule.after(&now).next() {
        let scheduled_name = name.clone();
        let scheduled_task = ScheduledTask::new(
            next_run,
            Arc::new(|| {}),
            id,
            scheduled_name,
        );

        let mut tq = task_queue.lock().await;
        tq.push(scheduled_task);
        notify.notify_one();
    }
}

impl Default for AsyncScheduler {
    fn default() -> Self {
        Self::new()
    }
}

impl Drop for AsyncScheduler {
    fn drop(&mut self) {
        // 注意：这里不能是 async 的，所以我们只是设置标志
        // 实际的清理应该由用户调用 stop() 来完成
        self.running.store(false, std::sync::atomic::Ordering::Relaxed);
        self.notify.notify_one();
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::atomic::{AtomicU32, Ordering};
    use tokio::time::{sleep, Duration};

    #[tokio::test]
    async fn test_scheduler_basic() {
        let scheduler = AsyncScheduler::new();
        let counter = Arc::new(AtomicU32::new(0));

        // 每秒执行一次的任务
        let counter_clone = counter.clone();
    let _id = scheduler.schedule_cron(
            "test_task".to_string(),
            "* * * * * *", // 每秒
            move || {
                counter_clone.fetch_add(1, Ordering::Relaxed);
            }
        ).await.unwrap();

        // 等待几秒
        sleep(Duration::from_secs(3)).await;

        // 停止调度器
        scheduler.stop().await;

        let count = counter.load(Ordering::Relaxed);
        assert!(count >= 2 && count <= 4); // 应该执行了2-4次

        let stats = scheduler.get_stats().await;
        assert_eq!(stats.scheduled, 1);
        assert!(stats.executed >= 2);
    }

    #[tokio::test]
    async fn test_scheduler_cancel() {
        let scheduler = AsyncScheduler::new();
        let counter = Arc::new(AtomicU32::new(0));

        let counter_clone = counter.clone();
        let id = scheduler.schedule_cron(
            "test_task".to_string(),
            "* * * * * *",
            move || {
                counter_clone.fetch_add(1, Ordering::Relaxed);
            }
        ).await.unwrap();

        // 等待1秒让任务执行几次
        sleep(Duration::from_secs(1)).await;

        // 取消任务
        scheduler.cancel(id).await;

        // 再等待1秒
        sleep(Duration::from_secs(1)).await;

        scheduler.stop().await;

        let count = counter.load(Ordering::Relaxed);
        let stats = scheduler.get_stats().await;

        assert!(count >= 1); // 至少执行了一次
        assert_eq!(stats.canceled, 1);
    }
}