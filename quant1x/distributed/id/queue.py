# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.
"""有界多生产者多消费者队列 (对齐 Go runtime.RingBuffer 的语义).

API 语义:
    try_push   非阻塞, 满 -> QueueFullError, 已关闭 -> QueueClosedError
    try_pop    非阻塞, 空 -> QueueEmptyError, 已关闭且空 -> QueueClosedError
    push       阻塞直到有空位, 关闭后 -> QueueClosedError
    pop        阻塞直到有数据, 已关闭且空 -> QueueClosedError
    wait_for_close  等待所有已入队数据被消费完毕

容量向上取整为 2 的幂 (与 Go/Rust 一致), 幂等 close.
Python 探索层使用标准库线程原语实现同等语义 (无锁环缓冲为生产层实现).
"""

import collections
import threading


class QueueError(Exception):
    """队列错误基类."""


class QueueFullError(QueueError):
    """队列已满 (对应 Go 的 ErrQueueFull)."""


class QueueEmptyError(QueueError):
    """队列已空 (对应 Go 的 ErrQueueEmpty)."""


class QueueClosedError(QueueError):
    """队列已关闭 (对应 Go 的 ErrClosed)."""


class Queue:
    """有界 MPMC 队列, 容量向上取整为 2 的幂."""

    def __init__(self, capacity: int):
        if capacity < 1:
            raise ValueError(f"capacity 必须为正数, 实际 {capacity}")
        cap = 1
        while cap < capacity:
            cap <<= 1
        self._cap = cap
        self._deque = collections.deque()
        self._lock = threading.Lock()
        self._not_full = threading.Condition(self._lock)
        self._not_empty = threading.Condition(self._lock)
        self._closed = False

    def try_push(self, value) -> None:
        """非阻塞入队, 满 -> QueueFullError, 已关闭 -> QueueClosedError."""
        with self._lock:
            if self._closed:
                raise QueueClosedError()
            if len(self._deque) >= self._cap:
                raise QueueFullError()
            self._deque.append(value)
            self._not_empty.notify()

    def try_pop(self):
        """非阻塞出队, 空 -> QueueEmptyError, 已关闭且空 -> QueueClosedError."""
        with self._lock:
            if self._deque:
                value = self._deque.popleft()
                self._not_full.notify()
                return value
            if self._closed:
                raise QueueClosedError()
            raise QueueEmptyError()

    def push(self, value) -> None:
        """阻塞入队直到有空位; 队列关闭后 -> QueueClosedError."""
        with self._not_full:
            while len(self._deque) >= self._cap:
                self._not_full.wait()
            if self._closed:
                raise QueueClosedError()
            self._deque.append(value)
            self._not_empty.notify()

    def pop(self):
        """阻塞出队直到有数据; 已关闭且空 -> QueueClosedError."""
        with self._not_empty:
            while not self._deque and not self._closed:
                self._not_empty.wait()
            if self._deque:
                value = self._deque.popleft()
                self._not_full.notify()
                return value
            raise QueueClosedError()

    def len(self) -> int:
        with self._lock:
            return len(self._deque)

    def cap(self) -> int:
        return self._cap

    def is_empty(self) -> bool:
        return self.len() == 0

    def is_full(self) -> bool:
        return self.len() == self._cap

    def close(self) -> None:
        """关闭队列: 唤醒所有等待者, 幂等 (对齐 Go 的 Close)."""
        with self._lock:
            if self._closed:
                return
            self._closed = True
            self._not_full.notify_all()
            self._not_empty.notify_all()

    def is_closed(self) -> bool:
        with self._lock:
            return self._closed

    def wait_for_close(self) -> None:
        """阻塞直到所有已入队数据被消费 (对齐 Go 的 WaitForClose)."""
        with self._not_empty:
            while self._deque:
                self._not_empty.wait()
