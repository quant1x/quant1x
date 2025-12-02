use std::sync::mpsc;
use std::thread;
use std::time::Duration;
use tokio::time;

pub struct TaskScheduler {
    sender: mpsc::Sender<Box<dyn Fn() + Send + 'static>>,
}

impl TaskScheduler {
    pub fn new() -> Self {
        let (sender, receiver) = mpsc::channel();

        // 创建专用线程运行Tokio运行时
        let _handle = thread::spawn(move || {
            let rt = tokio::runtime::Runtime::new().unwrap();
            rt.block_on(async {
                while let Ok(task) = receiver.recv() {
                    tokio::spawn(wrap_task(task));
                }
            });
        });

        TaskScheduler { sender }
    }

    // 添加新任务（间隔单位：秒）
    pub fn add_task<F>(&self, interval_seconds: u64, task: F)
    where
        F: Fn() + Send + 'static + Clone,
    {
        let _sender_clone = self.sender.clone();
        let interval = Duration::from_secs(interval_seconds);

        // 发送任务到处理线程
        self.sender
            .send(Box::new(move || {
                let task_clone = task.clone();
                let interval_clone = interval.clone();

                // 在Tokio运行时中执行定时任务
                tokio::spawn(async move {
                    let mut timer = time::interval(interval_clone);
                    loop {
                        timer.tick().await;
                        task_clone();
                    }
                });
            }))
            .unwrap();
    }
}

// 将任务包装为Tokio任务
async fn wrap_task<F>(task: F)
where
    F: Fn() + Send + 'static,
{
    (task)();
}

fn main() {
    let scheduler = TaskScheduler::new();

    // 添加每2秒执行的任务
    scheduler.add_task(2, || {
        println!("Task 1 executed");
    });

    // 添加每3秒执行的任务
    scheduler.add_task(3, || {
        println!("Task 2 executed");
    });

    // 保持主线程运行
    loop {
        std::thread::sleep(std::time::Duration::from_secs(1));
    }
}
