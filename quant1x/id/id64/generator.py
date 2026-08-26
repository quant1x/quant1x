# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

from typing import TYPE_CHECKING

from .hlc import EPOCH_MS, PAYLOAD_BITS, PHYSICAL_BITS

if TYPE_CHECKING:
    from .hlc import HLC


class Generator:
    """将 HLC 推进结果与 nodeID 组装为 64 位 ID。"""

    def __init__(self, node_id: int, hlc: "HLC") -> None:
        if hlc is None:
            raise ValueError("id64: nil HLC")
        self.hlc = hlc
        self.seq_bits = hlc.seq_bits
        self.worker_bits = PAYLOAD_BITS - self.seq_bits
        if node_id < 0 or node_id >= (1 << self.worker_bits):
            raise ValueError(f"id64: nodeID {node_id} 超出 {self.worker_bits} 位节点位宽")
        self.node_id = node_id
        self.node_mask = (1 << self.worker_bits) - 1

    def next(self) -> int:
        """返回下一个 64 位 ID（int 位模式）。"""
        physical, seq = self.hlc.now()
        elapsed = physical - EPOCH_MS
        if elapsed < 0:
            raise ValueError(f"id64: 时钟早于 epoch，elapsed={elapsed}")
        if elapsed >= (1 << PHYSICAL_BITS):
            raise ValueError(f"id64: 时钟超出 41 位容量，elapsed={elapsed}")
        return (
            (elapsed << PAYLOAD_BITS)
            | ((self.node_id & self.node_mask) << self.seq_bits)
            | (seq & ((1 << self.seq_bits) - 1))
        )
