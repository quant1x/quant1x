mod adapter;
mod adapters;
mod scheduler;
mod market;

use anyhow::{Context, Result};
use tokio::signal;

#[tokio::main]
async fn main() -> Result<()> {
    // 初始化日志等基础设施
    println!("=== Adapter System Starting ===");

    // 启动调度器
    scheduler::start_scheduler()
        .await
        .context("Failed to start scheduler")?;

    // 等待终止信号
    signal::ctrl_c().await?;
    println!("=== Shutting down ===");

    Ok(())
}