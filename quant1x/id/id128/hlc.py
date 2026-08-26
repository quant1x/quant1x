# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

import os
import secrets
import threading
import time
from typing import Callable, Optional, Tuple

from .state_store import (
    FileStateStore,
    PersistentState,
    advance_persistent_state,
    default_sync_every,
)

# 进程级随机种子缓存：每个进程只生成一次（对齐 Go 的 sync.Once）。
_SEED_CACHE: Optional[int] = None


def _random_seed() -> int:
    """返回进程级随机种子（进程内只生成一次）。

    熵源不可用时退化为 UnixNano 与 PID 混洗（对齐 Go 的退化策略）。
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
    def __init__(self, *options) -> None:
        self._lock = threading.Lock()
        self.physical = 0
        self.logical = 0
        self.seq = 0
        self._now: Callable[[], int] = lambda: int(time.time() * 1000)
        self.seed = _random_seed()
        self.sync_every = default_sync_every()
        self.strict = False
        self.store: Optional[FileStateStore] = None

        for option in options:
            if option is not None:
                option(self)

        if self.store is not None:
            self.store.sync_every = self.sync_every
            self.store.strict = self.strict
            state, ok = self.load_state()
            if ok:
                self.physical = state.physical
                self.logical = state.logical
                self.seq = state.seq
            else:
                self.physical = self._now()
                self.logical = self.seed
        else:
            self.physical = self._now()
            self.logical = self.seed

    def now(self) -> Tuple[int, int]:
        with self._lock:
            current = PersistentState(self.physical, self.logical, self.seq)
            now_ms = self._now()
            if self.store is not None:
                next_state = self.store.next_state(current, now_ms, self.seed)
            else:
                next_state = advance_persistent_state(current, now_ms, self.seed)

            self.physical = next_state.physical
            self.logical = next_state.logical
            self.seq = next_state.seq

            return ((self.physical << 16) | self.logical, self.seq)

    def timestamp(self) -> int:
        with self._lock:
            return self.physical

    def load_state(self) -> Tuple[PersistentState, bool]:
        if self.store is None:
            return PersistentState(0, 0, 0), False
        return self.store.load()

    def close(self) -> None:
        """把快速路径批量缓冲中尚未落盘的状态记录写入磁盘并同步。

        启用状态文件后，进程异常退出最多丢失最近 sync_every-1 条进度
        （这些 ID 重启后可能重复）；优雅退出前调用本方法可零丢失。
        未启用状态文件时为空操作。可多次调用，幂等。
        """
        with self._lock:
            if self.store is not None:
                self.store.flush()