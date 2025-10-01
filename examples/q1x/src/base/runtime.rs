//! runtime 模块
//! 收录和运行时相关的基础功能

use std::collections::VecDeque;
use std::sync::{Once, OnceLock};
use log::info;
use std::sync::Arc;
use std::sync::atomic::{AtomicBool, Ordering};
use std::thread;
use std::time::Duration;
use signal_hook::consts::{SIGINT, SIGTERM};
use signal_hook::low_level;
use tokio_cron_scheduler::{JobScheduler, Job};

use lazy_static::lazy_static;
use std::sync::Mutex;

lazy_static! {
    static ref global_signal_running :Arc<AtomicBool> = Arc::new(AtomicBool::new(true));
}

/// 初始化全局信号监听
fn global_signal_init() {
    // 显式创建多线程运行时
    let r_term = global_signal_running.clone();
    let r_int = global_signal_running.clone();

    // 注册 SIGTERM 处理
    unsafe {
        low_level::register(SIGTERM, move || {
            r_term.store(false, Ordering::SeqCst);
            info!("捕获到 SIGTERM（系统重启/关闭）");
        }).unwrap();

        // 注册 SIGINT 处理
        low_level::register(SIGINT, move || {
            r_int.store(false, Ordering::SeqCst);
            info!("捕获到 SIGINT（Ctrl+C）");
        }).unwrap();
    }
}

#[allow(non_upper_case_globals)]
static global_runtime_init_once: Once = Once::new();

lazy_static!{
    /// 全局的运行时应用退出回调函数列表, 先进后出(FILO)
    static ref global_runtime_shutdown_hooks: Arc<Mutex<VecDeque<Box<dyn Fn() + Send + Sync>>>> = Arc::new(Mutex::new(VecDeque::new()));
}

/// 注册shutdown hook
pub fn register_shutdown_hook<F>(f: F)
where F: Fn() + Send + Sync + 'static,
{
    global_runtime_init_once.call_once(|| {
        global_signal_init();
    });
    let mut hooks = global_runtime_shutdown_hooks.lock().unwrap();
    hooks.push_front(Box::new(f));
}


/// 等待应用退出信号
pub fn wait_for_exit() {
    while global_signal_running.load(Ordering::SeqCst) {
        info!("程序运行中...");
        thread::sleep(Duration::from_secs(1));
    }
    let mut hooks = global_runtime_shutdown_hooks.lock().unwrap();
    // 执行所有回调（FILO顺序）
    while let Some(callback) = hooks.pop_front() {
        (callback)();
    }
}

#[allow(non_upper_case_globals)]
static global_runtime_scheduler: OnceLock<RuntimeCronScheduler> = OnceLock::new();

fn get_runtime_scheduler() -> &'static RuntimeCronScheduler {
    let scheduler = global_runtime_scheduler.get_or_init(|| {
        RuntimeCronScheduler::new()
    });
    scheduler
}

/// 添加定时任务, 此函数为运行时提供全局的定时任务调度
pub fn add_task<F>(spec: &str, f: F)
where
    F: Fn() + Send + Sync + 'static,
{
    let scheduler_guard = get_runtime_scheduler();
    scheduler_guard.add_local_cron_job(spec, f);
}

struct RuntimeCronScheduler {
    scheduler: Arc<JobScheduler>,
    runtime: tokio::runtime::Runtime,
}

impl RuntimeCronScheduler {
    fn new() -> Self {
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

    /// 添加本地时间Cron任务
    fn add_local_cron_job<F>(&self, cron_expr: &str, job: F)
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

// pub struct RollingOnce<T> {
//     once: std::sync::Once,
//     lock: std::sync::OnceLock<T>,
// }
//
// impl<T> RollingOnce<T> {
//     pub const fn new() -> Self {
//         let once = Once::new();
//         let mut lock = OnceLock::new();
//
//         Self{ once, lock }
//     }
//     pub fn get_or_init<F>(&self, init: F) -> &T
//     where
//     F: Fn() + Send + Sync + 'static,
//     {
//         self.lock.get_or_init(||{init})
//     }
//     pub fn reset(&mut self) {
//         add_task("0 0 9 * * *", || {
//             println!("reset RollingOnce");
//             self.lock.take();
//         });
//     }
// }

// pub struct RollingOnce<T> {
//     lock: OnceLock<T>,
// }
//
// impl<T> RollingOnce<T> {
//     pub const fn new() -> Self {
//         Self {
//             lock: OnceLock::new(),
//         }
//     }
//
//     pub fn get_or_init<F>(&mut self, init: F) -> &T
//     where
//         F: FnOnce() -> T + Send + Sync + 'static,
//     {
//         add_task("0 0 9 * * *", || {
//                         println!("reset RollingOnce");
//                         self.lock.take();
//                     });
//         self.lock.get_or_init(init)
//     }
//
//     pub fn reset(&mut self) {
//         self.lock.take();
//     }
// }
//
// static mut ROLLING_ONCE: RollingOnce<String> = RollingOnce::new();
//
// pub fn test_get() -> &'static String {
//     ROLLING_ONCE.get_or_init(|| {
//         "Hello World!".to_string()
//     })
// }
