# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

import os
import secrets
import threading
import time
from typing import Callable, Tuple

from .state_store import PersistentState, advance_persistent_state, default_sync_every

# EPOCH_MS 是 ID 时间戳的起点（2026-01-01T00:00:00Z，毫秒）。
# 41 位毫秒时间戳可覆盖约 69.7 年（至 2095 年）。
EPOCH_MS = 1767225600000

# 布局：1 位符号(恒 0) + 41 + workerBits + seqBits = 64。
PHYSICAL_BITS = 41
PAYLOAD_BITS = 22

# 进程级随机种子缓存：每个进程只生成一次（对齐 Go 的 sync.Once）。
_SEED_CACHE: int | None = None


def _random_seed() -> int:
    """返回进程级随机种子（进程内只生成一次）。

    熵源不可用时退化为 UnixNano 与 PID 混洗（对齐 Go / id128 的退化策略）。
    """
    global _SEED_CACHE
    if _SEED_CACHE is None:
        try:
            _SEED_CACHE = secrets.randbits(16)
        except OSError:
            fallback = int(time.time() * 1_000_000_000) ^ (os.getpid() << 16)
            _SEED_CACHE = fallback & 0xFFFF
    return _SEED_CACHE


class HLC:
    """管理物理时间与序列号的单调推进。

    内部维护 (physical, seq) 二元组：
    - physical：绝对毫秒时间戳（epoch 相对值在组装 ID 时换算）
    - seq：序列号，达到 seqBits 容量时进位 physical+1（时钟回拨时保持单调）
    """

    def __init__(self, *options: Callable[["HLC"], None]) -> None:
        self._lock = threading.Lock()
        self.physical = 0
        self.seq = 0
        self._now: Callable[[], int] = lambda: int(time.time() * 1000)
        self.seed = _random_seed()
        # 默认节点总数 1024（workerBits=11, seqBits=11）
        self.seq_bits = PAYLOAD_BITS - _seq_bits_from_node_count(1024)
        self.sync_every = default_sync_every()
        self.strict = False
        self.store = None

        for option in options:
            if option is not None:
                option(self)

        if self.store is not None:
            restored, ok = self.store.load()
            if ok:
                self.physical = restored.physical
                self.seq = restored.seq
            else:
                self.physical = self._now()
                self.seq = self.seed & self._seq_mask()
        else:
            self.physical = self._now()
            self.seq = self.seed & self._seq_mask()

    def _seq_mask(self) -> int:
        return (1 << self.seq_bits) - 1

    def now(self) -> Tuple[int, int]:
        """返回严格单调递增的 (physical 绝对毫秒, seq)。"""
        with self._lock:
            current = PersistentState(self.physical, self.seq)
            now_ms = self._now()
            if self.store is not None:
                next_state = self.store.next_state(current, now_ms, self.seq_bits)
            else:
                next_state = advance_persistent_state(current, now_ms, self.seq_bits)
            self.physical = next_state.physical
            self.seq = next_state.seq
            return self.physical, self.seq

    def timestamp(self) -> int:
        """返回当前物理时间（绝对毫秒）。"""
        return self.physical

    def close(self) -> None:
        """把快速路径批量缓冲中尚未落盘的状态记录写入磁盘并同步。

        启用状态文件后，进程异常退出最多丢失最近 sync_every-1 条进度
        （这些 ID 重启后可能重复）；优雅退出前调用本方法可零丢失。
        未启用状态文件时为空操作。可多次调用，幂等。
        """
        with self._lock:
            if self.store is not None:
                self.store.flush()


def _seq_bits_from_node_count(count: int) -> int:
    """与 with_node_count 的推导公式一致（用于默认值）。"""
    if count < 1:
        count = 1
    return PAYLOAD_BITS - count.bit_length()
