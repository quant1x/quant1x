//! ID 队列 (对应 Go Queue, 底层复用 runtime::Queue 即 Vyukov 有界 MPMC 队列)
//!
//! 语义对齐 Go:
//! - `try_push` / `try_pop` 非阻塞;
//! - `push` / `pop` 阻塞 (忙等退避);
//! - `close` 后进入只读排空: 存量 ID 仍可消费, 队空后再返回 `Error::Closed`.

use std::sync::atomic::{AtomicBool, Ordering};

use crate::runtime;

use super::{Error, Id};

/// ID 队列
pub struct Queue {
    inner: runtime::Queue<Id>,
    closed: AtomicBool,
}

impl Queue {
    /// 创建队列, 容量向上取整到 2 的幂; 容量 0 返回错误
    pub fn new(capacity: usize) -> Result<Queue, Error> {
        if capacity == 0 {
            return Err(Error::InvalidSize);
        }
        Ok(Queue {
            inner: runtime::Queue::new(capacity),
            closed: AtomicBool::new(false),
        })
    }

    /// 非阻塞推入; 队列已满返回 `Error::QueueFull`, 已关闭返回 `Error::Closed`
    pub fn try_push(&self, value: Id) -> Result<(), Error> {
        if self.closed.load(Ordering::Acquire) {
            return Err(Error::Closed);
        }
        self.inner.try_push(value).map_err(|_| Error::QueueFull)
    }

    /// 非阻塞弹出; 队列为空返回 `Error::QueueEmpty`, 已关闭且空返回 `Error::Closed`
    pub fn try_pop(&self) -> Result<Id, Error> {
        match self.inner.try_pop() {
            Ok(value) => Ok(value),
            Err(()) => {
                if self.closed.load(Ordering::Acquire) {
                    Err(Error::Closed)
                } else {
                    Err(Error::QueueEmpty)
                }
            }
        }
    }

    /// 阻塞推入: 队列满时忙等腾位, 直到成功或队列关闭
    pub fn push(&self, value: Id) -> Result<(), Error> {
        loop {
            if self.closed.load(Ordering::Acquire) {
                return Err(Error::Closed);
            }
            if self.inner.push(value).is_ok() {
                return Ok(());
            }
            // 队满, 让出后重试 (对齐 Go Push 的阻塞语义)
            std::thread::yield_now();
        }
    }

    /// 阻塞弹出: 队列空时忙等数据, 直到成功; 已关闭且空返回 `Error::Closed`
    pub fn pop(&self) -> Result<Id, Error> {
        loop {
            if self.inner.is_empty() {
                if self.closed.load(Ordering::Acquire) {
                    return Err(Error::Closed);
                }
                std::thread::yield_now();
                continue;
            }
            if let Ok(value) = self.inner.pop() {
                return Ok(value);
            }
            std::thread::yield_now();
        }
    }

    /// 当前队列长度 (近似值)
    pub fn len(&self) -> usize {
        self.inner.len()
    }

    /// 队列容量 (2 的幂)
    pub fn cap(&self) -> usize {
        self.inner.cap()
    }

    /// 关闭队列: 进入只读排空模式
    pub fn close(&self) {
        self.closed.store(true, Ordering::Release);
    }

    /// 队列是否已关闭
    pub fn is_closed(&self) -> bool {
        self.closed.load(Ordering::Acquire)
    }

    /// 阻塞等待队列排空 (存量消费完毕)
    pub fn wait_for_close(&self) {
        while !self.inner.is_empty() {
            std::thread::yield_now();
        }
    }
}
