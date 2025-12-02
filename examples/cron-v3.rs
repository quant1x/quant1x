use chrono::Local;
use tokio_cron_scheduler::{Job, JobScheduler};
use std::time::Duration;

#[tokio::main]
async fn main() {
    let sched = JobScheduler::new().await.unwrap();

    // 添加一个每10秒执行的任务（cron 表达式）
    sched.add(Job::new("0/10 * * * * *", |_uuid, _l| {
        println!("定时任务触发 - {}", Local::now());
        // 在此处添加任务逻辑
    }).unwrap()).await.unwrap();

    sched.start().await.unwrap();

    loop {
        tokio::time::sleep(Duration::from_millis(500)).await;
    }
}
