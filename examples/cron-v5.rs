use std::sync::Arc;
use std::thread;
use tokio_cron_scheduler::{Job, JobScheduler};

pub struct LocalCronScheduler {
    scheduler: Arc<JobScheduler>,
    runtime: tokio::runtime::Runtime,
}

impl LocalCronScheduler {
    pub fn new() -> Self {
        let runtime = tokio::runtime::Runtime::new().unwrap();

        // 初始化调度器
        let scheduler = runtime.block_on(async {
            JobScheduler::new().await.unwrap()
        });
        let scheduler = Arc::new(scheduler);

        // 启动调度器线程
        let scheduler_clone = scheduler.clone();
        let runtime_handle = runtime.handle().clone();
        thread::spawn(move || {
            runtime_handle.block_on(async {
                scheduler_clone.start().await.unwrap();
            });
        });

        Self { scheduler, runtime }
    }

    // 添加本地时间Cron任务
    pub fn add_local_cron_job<F>(&self, cron_expr: &str, job: F)
    where
        F: Fn() + Send + Sync + 'static,
    {
        // 直接创建Job并设置时区
        let job = Job::new_tz(cron_expr.to_string(), chrono::Local,move |_uuid, _lock| {
            job();
        }).unwrap();
        // 使用运行时处理异步添加
        self.runtime.block_on(async {
            self.scheduler.add(job).await.unwrap()
        });
    }
}

// 示例用法
fn main() {
    let scheduler = LocalCronScheduler::new();

    // 每天本地时间8:00执行
    scheduler.add_local_cron_job("0 8 10  * * *", || {
        println!("每日任务执行 - 本地时间: {}", chrono::Local::now());
    });

    // 每分钟第0秒执行（测试用）
    scheduler.add_local_cron_job("*/5 * * * * *", || {
        println!("每分钟任务 - 本地时间: {}", chrono::Local::now());
    });

    // 保持主线程运行
    loop {
        std::thread::sleep(std::time::Duration::from_secs(1));
    }
}