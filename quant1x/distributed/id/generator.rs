//! ID 生成器 (对应 Go Generator)

use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;

use super::id::{Id, EPOCH_MS, PAYLOAD_BITS};
use super::{Error, Hlc, Queue};

/// ID 生成器: 由 HLC 提供 (physical, seq), 拼装节点编号生成 64 位 ID
pub struct Generator {
    hlc: Arc<Hlc>,
    node_id: u32,
    worker_bits: u8,
    seq_bits: u8,
}

impl Generator {
    /// 创建生成器; 节点编号超出 worker 位可表示范围返回错误
    pub fn new(node_id: u32, hlc: Arc<Hlc>) -> Result<Generator, Error> {
        let worker_bits = PAYLOAD_BITS - hlc.seq_bits();
        if (node_id as u64) >= (1u64 << worker_bits) {
            return Err(Error::NodeIdOutOfRange(node_id));
        }
        let seq_bits = hlc.seq_bits();
        Ok(Generator {
            hlc,
            node_id,
            worker_bits,
            seq_bits,
        })
    }

    /// worker 位数
    pub fn worker_bits(&self) -> u8 {
        self.worker_bits
    }

    /// 生成下一个 ID
    pub fn next(&self) -> Result<Id, Error> {
        let (physical, sequence) = self.hlc.now()?;
        let elapsed = Id::check_epoch(physical - EPOCH_MS)?;
        Ok(Id(
            (elapsed as u64) << PAYLOAD_BITS
                | (self.node_id as u64) << self.seq_bits
                | (sequence as u64) & ((1u64 << self.seq_bits) - 1),
        ))
    }

    /// 把发号器接入 ID 队列: 持续发号并写入 `queue`, 队列满时阻塞等待消费腾位,
    /// 直到 `cancel` 置位 (对应 Go 的 ctx 取消) 或队列关闭.
    ///
    /// 返回: 队列已关闭返回 `Ok(())` (未消费的存量 ID 仍可由消费者排空);
    /// 取消返回 `Error::Canceled`.
    pub fn serve(&self, queue: &Queue, cancel: &AtomicBool) -> Result<(), Error> {
        loop {
            if cancel.load(Ordering::Acquire) {
                return Err(Error::Canceled);
            }
            if queue.is_closed() {
                return Ok(());
            }
            let id = self.next()?;
            // 队列满时重试同一个 ID (非阻塞 try_push), 每轮回到循环顶部检查取消/关闭,
            // 避免阻塞在满队列自旋上无法响应 cancel (对齐 Go 测试期望的 graceful drain)
            loop {
                if cancel.load(Ordering::Acquire) {
                    return Err(Error::Canceled);
                }
                if queue.is_closed() {
                    return Ok(());
                }
                match queue.try_push(id) {
                    Ok(()) => break,
                    Err(Error::Closed) => return Ok(()),
                    Err(Error::QueueFull) => std::thread::yield_now(),
                    Err(e) => return Err(e),
                }
            }
        }
    }
}
