# AsyncScheduler (调度器)

简介

本目录提供了一个轻量的异步 cron 风格调度器 `AsyncScheduler`，用于在运行时注册并调度周期性任务。调度器基于 `tokio` 运行时实现，支持通过 cron 表达式（`cron::Schedule`）定义执行计划。

主要特性

- 使用 cron 表达式调度周期任务（例如每秒/每分钟等）。
- 避免同一 cron 任务的并发执行：当上一次执行还在运行时，调度器会跳过本次执行并在统计中计数（`skipped_running`）。
- 提供统计信息 `SchedulerStats`（scheduled, executed, skipped_cancel, skipped_running, rescheduled, canceled）。

快速示例

```rust
use quant1x::runtime::AsyncScheduler;
use std::sync::Arc;
use std::sync::atomic::{AtomicU32, Ordering};
use tokio::time::sleep;
use std::time::Duration;

#[tokio::main]
async fn main() {
    let scheduler = AsyncScheduler::new();
    let counter = Arc::new(AtomicU32::new(0));
    let c = counter.clone();

    // 每秒执行一次
    let _id = scheduler.schedule_cron("tick".to_string(), "* * * * * *", move || {
        c.fetch_add(1, Ordering::Relaxed);
    }).await.unwrap();

    sleep(Duration::from_secs(3)).await;

    scheduler.stop().await;

    println!("stats: {:?}", scheduler.get_stats().await);
}
```

字段说明（`SchedulerStats`）

- scheduled: 成功调度（enqueue）任务的次数。
- executed: 实际执行（task callback 被调用）的次数。
- skipped_cancel: 当任务在队列中被发现已被用户取消时计数（cancel 后的跳过）。
- skipped_running: 当 cron 任务的上一轮仍在运行时，跳过当前轮并计数。
- rescheduled: cron 任务成功执行后重新排入队列的次数（表示周期性继续）。
- canceled: 用户调用 `cancel()` 对应命中的次数。

注意和最佳实践

- 任务回调签名为 `Fn()`，是同步执行的。如果你的任务会进行阻塞或耗时工作，请在回调内部 spawn 一个异步任务或将工作提交给线程池（例如 `tokio::spawn` 或 `spawn_blocking`），以免阻塞调度循环。
- 调度器的生命周期应显式通过 `stop().await` 来停止，以便正确清理内部资源。
- 测试中可能需要控制任务开始/结束的时序来验证 `skipped_running` 行为。生产中不要直接修改内部 `cron_running` 标志。

故障排查

- 如果发现周期性任务没有按预期运行，先检查 cron 表达式是否合法（`Schedule::from_str`）。
- 如果任务频繁被跳过，说明任务没有在下一次触发前完成——考虑将真实工作交给后台任务或缩短工作时间。

