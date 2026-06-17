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
    let cli = Cli::parse();

    match cli.command {
        Commands::Version {} => {
            println!("{}", "0.1.0");
            // 业务逻辑ru
            std::process::exit(0);
        }
        Commands::Update { base, features} => {
            println!("执行 update: base={}, features={}", base, features);
            // 业务逻辑
            std::process::exit(0);
        }
        Commands::Repair { number } => {
            println!("执行 repair: number={}", number);
            // 业务逻辑
            std::process::exit(0);
        }
    }
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
            info!("捕获到 SIGTERM(系统重启/关闭)");
        })?;

        // 注册 SIGINT 处理
        low_level::register(SIGINT, move || {
            r_int.store(false, Ordering::SeqCst);
            info!("捕获到 SIGINT(Ctrl+C)");
        })?;
    }

    // 主循环
    while running.load(Ordering::SeqCst) {
        info!("程序运行中...");
        thread::sleep(Duration::from_secs(1));
    }

    // 清理资源
    println!("正在清理资源...");
    Ok(())
}


use clap::{Parser, Subcommand};

/// 主命令描述
#[derive(Parser)]
#[command(name = "stock", about = "quant1x engine", version)]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

/// 子命令枚举
#[derive(Subcommand)]
enum Commands {
    /// 显示版本信息
    Version {
    },

    /// 更新最后一个交易日的数据
    Update {
        #[arg(short, long, default_value = "")]
        base: String,
        #[arg(short, long, default_value = "")]
        features:String,
    },

    /// 修复历史数据
    Repair {
        /// 数字参数
        #[arg(short, long)]
        number: u32,
    },
}