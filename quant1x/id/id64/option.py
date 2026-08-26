# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

from typing import Callable, TYPE_CHECKING

from .hlc import PAYLOAD_BITS

if TYPE_CHECKING:
    from .hlc import HLC

Option = Callable[["HLC"], None]


def with_clock(now: Callable[[], int]) -> Option:
    """覆盖默认时钟（返回绝对毫秒），测试用。"""

    def apply(hlc: "HLC") -> None:
        if now is not None:
            hlc._now = now

    return apply


def with_seq_seed(seed: int) -> Option:
    """设置序列号启动种子（默认随机）。

    种子用于无状态文件时随机化初始 seq，降低重启碰撞概率。
    """

    def apply(hlc: "HLC") -> None:
        hlc.seed = seed & 0xFFFF

    return apply


def with_state_file(path: str) -> Option:
    """启用状态文件持久化，跨进程/重启恢复高水位。"""

    def apply(hlc: "HLC") -> None:
        if path:
            from .state_store import FileStateStore

            hlc.store = FileStateStore(path, sync_every=hlc.sync_every, strict=hlc.strict)

    return apply


def with_state_sync_every(every: int) -> Option:
    """设置状态文件落盘间隔（每 N 次生成落盘一次）。"""

    def apply(hlc: "HLC") -> None:
        hlc.sync_every = max(1, int(every))
        if hlc.store is not None:
            hlc.store.sync_every = hlc.sync_every

    return apply


def with_state_strict() -> Option:
    """启用严格模式：每次发号前从磁盘读取最新状态并取 max。

    默认关闭（快速路径）：构造时从状态文件恢复一次高水位，运行期只追加不读盘，
    热路径仅一次写入。适用于单写者，以及多进程顺序接管（failover）场景——
    新进程构造时读到前任写者的最新水位，保证跨重启不重复。

    当多个进程（或同进程多个 HLC 实例）活跃共享同一状态文件、且都期望严格唯一时，
    必须开启严格模式：它以每次发号增加一次磁盘读为代价，保证各写者水位同步。
    """

    def apply(hlc: "HLC") -> None:
        hlc.strict = True
        if hlc.store is not None:
            hlc.store.strict = True

    return apply


def with_node_count(count: int) -> Option:
    """设置预期的节点总数，据此动态推导节点位宽与序列号位宽：

    workerBits = bit_length(count)
    seqBits    = 64 - 1 - 41 - workerBits

    当 seqBits < 4（节点数 > 2^18）时抛出 ValueError。
    """

    def apply(hlc: "HLC") -> None:
        node_count = max(1, int(count))
        worker_bits = node_count.bit_length()
        hlc.seq_bits = PAYLOAD_BITS - worker_bits
        if hlc.seq_bits < 4:
            raise ValueError("id64: 节点数过多，无法为序列号保留足够的位宽")

    return apply


def with_seq_bits(seq_bits: int) -> Option:
    """直接设置序列号位宽（底层选项，通常用 with_node_count 代替）。"""

    def apply(hlc: "HLC") -> None:
        if seq_bits < 4 or seq_bits > PAYLOAD_BITS - 1:
            raise ValueError("id64: seqBits 超出有效范围 [4, 21]")
        hlc.seq_bits = int(seq_bits)

    return apply
