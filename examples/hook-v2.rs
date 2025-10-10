use std::collections::VecDeque;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Arc, Condvar, Mutex};
use std::thread;

pub struct Runtime {
    callbacks: Arc<Mutex<VecDeque<Box<dyn Fn() + Send + Sync>>>>,
    exit_flag: Arc<AtomicBool>,
    condvar: Arc<(Mutex<bool>, Condvar)>,
}

impl Runtime {
    pub fn new() -> Result<Self, ctrlc::Error> {
        let callbacks = Arc::new(Mutex::new(VecDeque::new()));
        let exit_flag = Arc::new(AtomicBool::new(false));
        let condvar = Arc::new((Mutex::new(false), Condvar::new()));

        // 设置跨平台信号处理
        let exit_clone = exit_flag.clone();
        let condvar_clone = condvar.clone();
        ctrlc::set_handler(move || {
            exit_clone.store(true, Ordering::SeqCst);
            let (lock, cvar) = &*condvar_clone;
            let mut started = lock.lock().unwrap();
            *started = true;
            cvar.notify_one();
        })?;

        Ok(Self {
            callbacks,
            exit_flag,
            condvar,
        })
    }

    /// 注册退出回调（FILO顺序）
    pub fn register_shutdown_hook<F>(&self, f: F)
    where
        F: Fn() + Send + Sync + 'static,
    {
        self.callbacks.lock().unwrap().push_front(Box::new(f));
    }

    /// 阻塞等待退出信号
    pub fn wait_for_exit(&self) {
        let (lock, cvar) = &*self.condvar;
        let mut started = lock.lock().unwrap();

        while !*started {
            started = cvar.wait(started).unwrap();
        }

        // 执行所有回调（FILO顺序）
        while let Some(callback) = self.callbacks.lock().unwrap().pop_front() {
            (callback)();
        }
    }
}

// 使用示例
fn main() -> Result<(), Box<dyn std::error::Error>> {
    let rt = Runtime::new()?;

    // 注册资源清理函数
    rt.register_shutdown_hook(|| {
        println!("Executing database cleanup...");
    });

    rt.register_shutdown_hook(|| {
        println!("Flushing metrics to disk...");
    });

    println!("Application started. Press CTRL+C to exit.");
    rt.wait_for_exit();
    println!("Shutdown completed.");
    Ok(())
}
