# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

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

            hlc.store = FileStateStore(path, sync_every=hlc.sync_every)

    return apply


def with_state_sync_every(every: int) -> Option:
    def apply(hlc: "HLC") -> None:
        hlc.sync_every = max(1, int(every))
        if hlc.store is not None:
            hlc.store.sync_every = hlc.sync_every

    return apply