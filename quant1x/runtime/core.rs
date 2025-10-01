use crate::runtime::scheduler::{AsyncScheduler, TaskId};
use once_cell::sync::OnceCell;
use std::sync::Arc;

static GLOBAL_SCHEDULER: OnceCell<Arc<AsyncScheduler>> = OnceCell::new();

/// Ensure the global scheduler is initialized and return a clone of the Arc.
pub fn init_global_scheduler() -> Arc<AsyncScheduler> {
    GLOBAL_SCHEDULER
        .get_or_init(|| Arc::new(AsyncScheduler::new()))
        .clone()
}

/// Schedule a cron task on the global scheduler.
/// Returns the task id on success.
pub async fn add_task<F>(name: &str, cron_expr: &str, task: F) -> Result<TaskId, Box<dyn std::error::Error + Send + Sync>>
where
    F: Fn() + Send + Sync + 'static,
{
    let sched = init_global_scheduler();
    sched.schedule_cron(name.to_string(), cron_expr, task).await
}

/// Cancel a task by id. This will spawn a background task to perform the cancel
/// in case it's invoked from synchronous code.
pub fn cancel_task(id: TaskId) {
    if let Some(s) = GLOBAL_SCHEDULER.get() {
        let s = s.clone();
        // fire-and-forget cancel on the scheduler
        tokio::spawn(async move { s.cancel(id).await });
    }
}
