use chrono::Local;
use tokio_cron_scheduler::{Job, JobScheduler};

#[tokio::main]
async fn main() {
    let sched = JobScheduler::new().await.unwrap();

    // 添加每10秒执行的任务
    sched
        .add(
            Job::new_async("0/5 * * * * *", |_uuid, _l| {
                Box::pin(async {
                    println!("Async 定时任务 - {}", Local::now());
                    // 异步任务代码
                })
            })
            .unwrap(),
        )
        .await
        .unwrap();

    sched.start().await.unwrap(); // 启动调度器
    println!("start");

    // 保持主线程运行
    loop {
        tokio::time::sleep(tokio::time::Duration::from_secs(60)).await;
    }
}
