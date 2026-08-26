# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

import base64
from dataclasses import dataclass

PAYLOAD_BITS = 22


@dataclass(frozen=True)
class ID:
    """64 位可排序标识（8 字节 BigEndian）。

    位布局（动态位宽）：

    | 1bit 符号(恒 0) | Physical(41bit, epoch 相对毫秒) | NodeID(workerBits) | Seq(seqBits) |

    workerBits / seqBits 由节点总数推导（见 ``with_node_count``），
    因此 ``node_id`` / ``seq`` 解析需要传入对应的 ``worker_bits``。
    """

    raw: bytes

    def __post_init__(self) -> None:
        if len(self.raw) != 8:
            raise ValueError("ID expects exactly 8 bytes")

    @classmethod
    def from_int(cls, value: int) -> "ID":
        return cls(value.to_bytes(8, "big", signed=False))

    @classmethod
    def from_bytes(cls, raw: bytes) -> "ID":
        return cls(raw)

    def bytes(self) -> bytes:
        return self.raw

    def to_int(self) -> int:
        return int.from_bytes(self.raw, "big", signed=False)

    def __str__(self) -> str:
        return base64.urlsafe_b64encode(self.raw).rstrip(b"=").decode("ascii")

    def physical(self) -> int:
        """返回 epoch 相对毫秒（高 41 位）。"""
        return self.to_int() >> PAYLOAD_BITS

    def node_id(self, worker_bits: int) -> int:
        shift = PAYLOAD_BITS - worker_bits
        return (self.to_int() >> shift) & ((1 << worker_bits) - 1)

    def seq(self, worker_bits: int) -> int:
        shift = PAYLOAD_BITS - worker_bits
        return self.to_int() & ((1 << shift) - 1)
