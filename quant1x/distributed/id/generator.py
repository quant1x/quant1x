# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.
"""分布式 ID 生成器 (与 Go/Rust 语义对齐).

Next() 组合 HLC 水位: (physical - epoch) << 22 | node_id << seq_bits | seq.
Serve() 持续生产并入队, 响应 cancel 事件与队列关闭 (对齐 Rust 修复后的语义).
"""

import threading
import time

from .id import EPOCH_MS, PAYLOAD_BITS, ID, check_epoch
from .queue import Queue, QueueClosedError, QueueFullError


class CanceledError(Exception):
    """Serve 被取消 (对应 Go 的 context.Canceled)."""


class Generator:
    """按 node_id + HLC 生成全局唯一 ID."""

    def __init__(self, node_id: int, hlc):
        if hlc is None:
            raise ValueError("hlc 不能为 None")
        self.hlc = hlc
        self.seq_bits = hlc.seq_bits
        self.worker_bits = PAYLOAD_BITS - self.seq_bits
        if node_id < 0 or node_id >= (1 << self.worker_bits):
            raise ValueError(
                f"node_id {node_id} 超出 {self.worker_bits} 位 worker 范围 "
                f"({1 << self.worker_bits} 个节点)"
            )
        self.node_id = node_id

    def next(self) -> ID:
        """生成下一个 ID (对齐 Go 的 Next)."""
        physical, sequence = self.hlc.now()
        elapsed = check_epoch(physical - EPOCH_MS)
        return ID(
            (elapsed << PAYLOAD_BITS)
            | (self.node_id << self.seq_bits)
            | (sequence & ((1 << self.seq_bits) - 1))
        )

    def serve(self, queue: Queue, cancel: threading.Event = None) -> None:
        """持续生产并入队, 直到取消或队列关闭 (对齐 Go 的 Serve).

        cancel 为 threading.Event, set 后返回并抛 CanceledError.
        队列关闭时正常返回 None; 队列满时重试同一 ID, 每轮检查取消/关闭,
        避免阻塞在满队列上无法响应取消.
        """
        if queue is None:
            raise ValueError("queue 不能为 None")
        while True:
            if cancel is not None and cancel.is_set():
                raise CanceledError()
            if queue.is_closed():
                return
            identifier = self.next()
            # 队列满时重试同一个 ID, 每轮回到循环顶部检查取消/关闭
            while True:
                if cancel is not None and cancel.is_set():
                    raise CanceledError()
                if queue.is_closed():
                    return
                try:
                    queue.try_push(identifier)
                    break
                except QueueFullError:
                    time.sleep(0)  # 让出 GIL, 对齐 Rust 的 yield_now
                except QueueClosedError:
                    return
