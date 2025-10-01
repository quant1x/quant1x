use std::env;
use std::io::Error;
use q1x::{self};

use signal_hook::{consts::{SIGINT, SIGTERM}, low_level};
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use std::thread;
use std::time::Duration;
use log::{info, error};

fn main() -> Result<(), Error> {
    // 打印当前工作目录
    let cwd = env::current_dir().unwrap();
    info!("当前工作目录: {:?}", cwd);
    log4rs::init_file("engine/config/log4rs.yaml", Default::default()).unwrap();

    let ts = q1x::base::time::now();
    info!("Hello, world!");
    info!("{}", ts);

    let running = Arc::new(AtomicBool::new(true));
    // 为每个闭包单独克隆 Arc
    let r_term = running.clone();
    let r_int = running.clone();

    // 注册 SIGTERM 处理
    unsafe {
        low_level::register(SIGTERM, move || {
            r_term.store(false, Ordering::SeqCst);
            info!("捕获到 SIGTERM（系统重启/关闭）");
        })?;

        // 注册 SIGINT 处理
        low_level::register(SIGINT, move || {
            r_int.store(false, Ordering::SeqCst);
            info!("捕获到 SIGINT（Ctrl+C）");
        })?;
    }

    // 主循环
    while running.load(Ordering::SeqCst) {
        error!("程序运行中...");
        thread::sleep(Duration::from_secs(1));
    }

    // 清理资源
    println!("正在清理资源...");
    Ok(())
}
