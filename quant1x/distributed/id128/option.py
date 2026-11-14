# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X Team
# SPDX-License-Identifier: MIT

from __future__ import annotations

from typing import Callable, TYPE_CHECKING

if TYPE_CHECKING:
    from .hlc import HLC

Option = Callable[["HLC"], None]


def with_clock(now: Callable[[], int]) -> Option:
    def apply(hlc: "HLC") -> None:
        if now is not None:
            hlc._now = now

    return apply


def with_logical_seed(seed: int) -> Option:
    def apply(hlc: "HLC") -> None:
        hlc.seed = seed & 0xFFFF

    return apply


def with_state_file(path: str) -> Option:
    def apply(hlc: "HLC") -> None:
        if path:
            from .state_store import FileStateStore

            hlc.store = FileStateStore(path, sync_every=hlc.sync_every, strict=hlc.strict)

    return apply


def with_state_sync_every(every: int) -> Option:
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