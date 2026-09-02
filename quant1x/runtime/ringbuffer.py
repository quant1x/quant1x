# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.
"""Vyukov 风格的 Python 原生环形缓冲区.

该实现保留 Go/Rust/C++ 侧的核心语义：
- 有界容量，向上取整到最小 2 的幂
- 非阻塞 try_push / try_pop 与阻塞 push / pop
- close() 幂等，生产者在关闭后立即拒绝写入，消费者在清空队列后收到 Closed
- wait_for_close() 等待所有已入队数据被消费完成

它是 Python 侧的 spec 参照实现，不依赖任何第三方库。
"""

from __future__ import annotations

import threading
from typing import Generic, TypeVar

T = TypeVar("T")


class QueueError(RuntimeError):
    """队列错误基类。"""


class QueueFullError(QueueError):
    """队列已满。"""


class QueueEmptyError(QueueError):
    """队列已空。"""


class QueueClosedError(QueueError):
    """队列已关闭。"""


ErrQueueFull = QueueFullError()
ErrQueueEmpty = QueueEmptyError()
ErrClosed = QueueClosedError()
ErrInvalidSize = ValueError("size must be positive")


class RingBuffer(Generic[T]):
    """有界 MPMC 语义的原生环形缓冲区.

    这个实现使用 Python 原生线程同步来保持与 Go/Rust 精确契约一致，
    重点是语义而不是无锁性能。
    """

    def __init__(self, capacity: int):
        if capacity < 1:
            raise ValueError(f"capacity must be positive, got {capacity}")

        cap = 1
        while cap < capacity:
            cap <<= 1

        self._capacity = cap
        self._mask = cap - 1
        self._slots = [None] * cap
        self._head = 0
        self._tail = 0
        self._size = 0
        self._closed = False

        self._lock = threading.Lock()
        self._not_empty = threading.Condition(self._lock)
        self._not_full = threading.Condition(self._lock)

    def __len__(self) -> int:
        return self.len()

    def __repr__(self) -> str:
        return f"RingBuffer(capacity={self._capacity}, size={self._size}, closed={self._closed})"

    def try_push(self, value: T) -> None:
        """非阻塞入队。满 / 关闭 => 抛出对应异常。"""
        with self._lock:
            if self._closed:
                raise QueueClosedError()
            if self._size >= self._capacity:
                raise QueueFullError()
            index = self._head & self._mask
            self._slots[index] = value
            self._head += 1
            self._size += 1
            self._not_empty.notify()

    def TryWrite(self, value: T) -> None:
        self.try_push(value)

    def TryPush(self, value: T) -> None:
        self.try_push(value)

    def push(self, value: T) -> None:
        """阻塞入队，直到有空位或队列关闭。"""
        with self._not_full:
            while self._size >= self._capacity and not self._closed:
                self._not_full.wait()
            if self._closed:
                raise QueueClosedError()
            index = self._head & self._mask
            self._slots[index] = value
            self._head += 1
            self._size += 1
            self._not_empty.notify()

    def Write(self, value: T) -> None:
        self.push(value)

    def Push(self, value: T) -> None:
        self.push(value)

    def try_pop(self):
        """非阻塞出队。空 / 关闭但空 => 抛出对应异常。"""
        with self._lock:
            if self._size > 0:
                index = self._tail & self._mask
                value = self._slots[index]
                self._slots[index] = None
                self._tail += 1
                self._size -= 1
                self._not_full.notify()
                return value
            if self._closed:
                raise QueueClosedError()
            raise QueueEmptyError()

    def TryRead(self):
        return self.try_pop()

    def TryPop(self):
        return self.try_pop()

    def pop(self):
        """阻塞出队，直到有元素可读取或队列关闭且为空。"""
        with self._not_empty:
            while self._size == 0 and not self._closed:
                self._not_empty.wait()
            if self._size > 0:
                index = self._tail & self._mask
                value = self._slots[index]
                self._slots[index] = None
                self._tail += 1
                self._size -= 1
                self._not_full.notify()
                return value
            raise QueueClosedError()

    def Read(self):
        return self.pop()

    def Pop(self):
        return self.pop()

    def len(self) -> int:
        with self._lock:
            return self._size

    def cap(self) -> int:
        return self._capacity

    def is_empty(self) -> bool:
        return self.len() == 0

    def is_full(self) -> bool:
        return self.len() >= self._capacity

    def close(self) -> None:
        """关闭队列。该操作幂等，并唤醒所有等待中的生产者/消费者。"""
        with self._lock:
            if self._closed:
                return
            self._closed = True
            self._not_full.notify_all()
            self._not_empty.notify_all()

    def Close(self) -> None:
        self.close()

    def is_closed(self) -> bool:
        with self._lock:
            return self._closed

    def IsClosed(self) -> bool:
        return self.is_closed()

    def wait_for_close(self) -> None:
        """等待已入队数据全部消费完毕，然后返回。"""
        with self._not_empty:
            while self._size > 0:
                self._not_empty.wait()

    def WaitForClose(self) -> None:
        self.wait_for_close()


__all__ = [
    "QueueError",
    "QueueFullError",
    "QueueEmptyError",
    "QueueClosedError",
    "ErrQueueFull",
    "ErrQueueEmpty",
    "ErrClosed",
    "ErrInvalidSize",
    "RingBuffer",
]
