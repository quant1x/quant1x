# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.
"""HLC 构造选项 (与 Go 的 option.go / Rust 的 option 语义对齐).

每个 with_* 返回一个可调用对象, 在 HLC 构造时按其语义应用.
"""

import os

from .id import PAYLOAD_BITS

# 默认每 N 次 Now 落盘一次 checkpoint
DEFAULT_SYNC_EVERY = 1000
# seq 保留位数下限 (worker 位上限 18)
MIN_SEQ_BITS = 4


def default_sync_every_value() -> int:
    """读取环境变量 QUANT1X_ID64_SYNC_EVERY, 非法/缺失时回退默认值, 对齐 Go."""
    raw = os.environ.get("QUANT1X_ID64_SYNC_EVERY")
    if raw is not None:
        try:
            value = int(raw)
            if value > 0:
                return value
        except ValueError:
            pass
    return DEFAULT_SYNC_EVERY


def with_clock(now):
    """自定义毫秒时钟函数 (默认 time.time() * 1000), 对齐 Go 的 WithClock."""
    return lambda hlc: setattr(hlc, "_clock", now)


def with_seq_seed(seed: int) -> object:
    """初始化 seq 的种子值, 对齐 Go 的 WithSeqSeed."""
    if seed < 0 or seed > 0xFFFF:
        raise ValueError(f"seq seed 必须为 uint16, 实际 {seed}")
    return lambda hlc: setattr(hlc, "seed", seed)


def with_state_file(path) -> object:
    """启用跨进程持久化状态文件, 对齐 Go 的 WithStateFile."""
    if not path:
        raise ValueError("state 文件路径不能为空")
    return lambda hlc: setattr(hlc, "state_file", path)


def with_state_sync_every(every: int) -> object:
    """设置 checkpoint 落盘间隔, 对齐 Go 的 WithStateSyncEvery."""
    if every <= 0:
        raise ValueError(f"sync_every 必须为正数, 实际 {every}")
    return lambda hlc: setattr(hlc, "sync_every", every)


def with_state_strict() -> object:
    """严格模式: 每次 Now 先抢跨进程锁再 checkpooint, 对齐 Go 的 WithStateStrict."""
    return lambda hlc: setattr(hlc, "strict", True)


def with_node_count(count: int) -> object:
    """按节点数推导 seq 位数, 对齐 Go 的 WithNodeCount.

    seq_bits = 22 - ceil(log2(count)); count < 2^18 时 seq_bits >= 4.
    """
    if count < 1:
        count = 1
    seq_bits = PAYLOAD_BITS - count.bit_length()
    if seq_bits < MIN_SEQ_BITS:
        raise ValueError(f"节点数 {count} 过大: 需至少保留 {MIN_SEQ_BITS} 位序列号")
    return lambda hlc: setattr(hlc, "seq_bits", seq_bits)


def with_seq_bits(seq_bits: int) -> object:
    """直接指定 seq 位数 (4..21), 对齐 Go 的 WithSeqBits."""
    if seq_bits < MIN_SEQ_BITS or seq_bits > PAYLOAD_BITS - 1:
        raise ValueError(f"seq_bits 必须在 {MIN_SEQ_BITS}..{PAYLOAD_BITS - 1}, 实际 {seq_bits}")
    return lambda hlc: setattr(hlc, "seq_bits", seq_bits)
