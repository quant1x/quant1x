//! ID 发号器 (Rust 移植, 对应 Go quant1x/distributed/id)
//!
//! 提供 64 位可排序 ID, 基于混合逻辑时钟 (HLC) 与可选的状态持久化
//! (mmap 双槽 checkpoint + 锁字跨进程互斥). 语义与 Go 实现对齐.

pub mod error;
pub mod generator;
pub mod hlc;
pub mod id;
pub mod queue;
pub mod state_store;

mod crc32;

pub use error::Error;
pub use generator::Generator;
pub use hlc::{Hlc, HlcBuilder};
pub use id::{Id, EPOCH_MS};
pub use queue::Queue;
pub use state_store::PersistentState;

#[cfg(test)]
mod tests;
