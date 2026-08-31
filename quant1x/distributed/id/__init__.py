# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.
"""分布式 ID (Python 探索层).

与 Go/Rust 实现语义对齐: HLC 混合逻辑时钟 + mmap 双槽 checkpoint 持久化
+ 锁字跨进程互斥 + 有界 MPMC 队列.
"""

from .generator import CanceledError, Generator
from .hlc import HLC
from .id import EPOCH_MS, PAYLOAD_BITS, PHYSICAL_BITS, ID, check_epoch
from .option import (
    with_clock,
    with_node_count,
    with_seq_bits,
    with_seq_seed,
    with_state_file,
    with_state_strict,
    with_state_sync_every,
)
from .queue import Queue, QueueClosedError, QueueEmptyError, QueueFullError
from .state_store import FileStateStore, PersistentState, StateFileError

__all__ = [
    "CanceledError",
    "EPOCH_MS",
    "FileStateStore",
    "Generator",
    "HLC",
    "ID",
    "PAYLOAD_BITS",
    "PHYSICAL_BITS",
    "PersistentState",
    "Queue",
    "QueueClosedError",
    "QueueEmptyError",
    "QueueFullError",
    "StateFileError",
    "check_epoch",
    "with_clock",
    "with_node_count",
    "with_seq_bits",
    "with_seq_seed",
    "with_state_file",
    "with_state_strict",
    "with_state_sync_every",
]
