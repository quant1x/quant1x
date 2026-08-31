//! 统一的错误类型 (对应 Go 包内的 error 使用)

use std::fmt;

/// distributed/id 的运行时错误
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum Error {
    /// 队列容量无效 (0)
    InvalidSize,
    /// 队列已满 (TryPush)
    QueueFull,
    /// 队列为空 (TryPop)
    QueueEmpty,
    /// 队列已关闭
    Closed,
    /// 操作被取消 (Serve 收到停止信号)
    Canceled,
    /// 时间戳超出 41 位可表示范围
    EpochElapsedOutOfRange(i64),
    /// 节点编号超出可用 worker 位范围
    NodeIdOutOfRange(u32),
    /// seq 位数不在 [4, 21] 范围内
    InvalidSeqBits(u8),
    /// 节点数过大, 推导出的 seq 位数小于 4
    NodeCountTooLarge(u32),
    /// 状态文件操作失败 (IO / mmap / 损坏)
    StateFile(String),
    /// 内部互斥锁中毒
    LockPoisoned,
    /// ID 字符串解析失败
    ParseId(String),
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Error::InvalidSize => write!(f, "invalid queue size"),
            Error::QueueFull => write!(f, "queue full"),
            Error::QueueEmpty => write!(f, "queue empty"),
            Error::Closed => write!(f, "closed"),
            Error::Canceled => write!(f, "canceled"),
            Error::EpochElapsedOutOfRange(v) => {
                write!(f, "epoch elapsed out of range: {v}")
            }
            Error::NodeIdOutOfRange(v) => write!(f, "node id out of range: {v}"),
            Error::InvalidSeqBits(v) => write!(f, "invalid seq bits: {v}"),
            Error::NodeCountTooLarge(v) => write!(f, "node count too large: {v}"),
            Error::StateFile(msg) => write!(f, "state file: {msg}"),
            Error::LockPoisoned => write!(f, "mutex poisoned"),
            Error::ParseId(msg) => write!(f, "parse id: {msg}"),
        }
    }
}

impl std::error::Error for Error {}
