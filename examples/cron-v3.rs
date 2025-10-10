use chrono::Local;
use job_scheduler::{Job, JobScheduler};
use std::time::Duration;

fn main() {
    let mut sched = JobScheduler::new();

    // 添加一个每10秒执行的任务（cron 表达式）
    sched.add(Job::new("0/10 * * * * *".parse().unwrap(), || {
        println!("定时任务触发 - {}", Local::now());
        // 在此处添加任务逻辑
    }));

    loop {
        sched.tick(); // 检查并执行任务
        std::thread::sleep(Duration::from_millis(500)); // 降低 CPU 占用
    }
}
