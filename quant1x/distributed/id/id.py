# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.
"""分布式 ID 类型与位布局常量 (与 Go/Rust 语义对齐).

位布局 (64 位, 大端):
    | 41 位 physical(ms) | 22 位 payload |
    payload 又分: | worker_bits 位 node_id | seq_bits 位 seq |
"""

import base64

# 纪元起点: 2026-01-01T00:00:00Z (毫秒), 与 Go/Rust 的 EpochMs 一致
EPOCH_MS = 1767225600000
# 22 位 payload: 2^22 = 4194304, 单进程/线程每毫秒容量
PAYLOAD_BITS = 22
# 41 位 physical: 2^41 ms ≈ 69.7 年 (自纪元起)
PHYSICAL_BITS = 41
# worker 位数上限 (seq 至少保留 4 位)
MAX_WORKER_BITS = PAYLOAD_BITS - 4


class ID(int):
    """64 位分布式 ID, 语义对齐 Go 的 ID (uint64 包装类型).

    Python int 无符号溢出限制, 但本布局最大 63 位 (physical 41 + payload 22),
    与 Go 的 uint64 表示一一对应.
    """

    def __new__(cls, value: int) -> "ID":
        if value < 0 or value >= (1 << 64):
            raise ValueError(f"ID 超出 uint64 范围: {value}")
        return super().__new__(cls, value)

    @classmethod
    def from_int(cls, value: int) -> "ID":
        return cls(value)

    def to_int(self) -> int:
        return int(self)

    def physical(self) -> int:
        """自纪元起的毫秒数 (高 41 位)."""
        return int(self) >> PAYLOAD_BITS

    def node_id(self, worker_bits: int) -> int:
        """节点号 (payload 高 worker_bits 位)."""
        if worker_bits < 0 or worker_bits > PAYLOAD_BITS:
            raise ValueError(f"worker_bits 超出范围: {worker_bits}")
        return (int(self) >> (PAYLOAD_BITS - worker_bits)) & ((1 << worker_bits) - 1)

    def seq(self, worker_bits: int) -> int:
        """序列号 (payload 低 seq_bits 位)."""
        if worker_bits < 0 or worker_bits > PAYLOAD_BITS:
            raise ValueError(f"worker_bits 超出范围: {worker_bits}")
        return int(self) & ((1 << (PAYLOAD_BITS - worker_bits)) - 1)

    def to_bytes(self) -> bytes:
        """大端 8 字节编码, 对齐 Go 的 Bytes()."""
        return int(self).to_bytes(8, byteorder="big")

    def string(self) -> str:
        """URL-safe Base64 无填充 11 字符表示, 对齐 Go 的 String().

        RawURLEncoding: 无 padding, 字符集 [-_0-9A-Za-z].
        """
        return base64.urlsafe_b64encode(self.to_bytes()).rstrip(b"=").decode("ascii")

    @classmethod
    def from_bytes(cls, data: bytes) -> "ID":
        """解码大端 8 字节, 对齐 Go 的 FromBytes."""
        if len(data) != 8:
            raise ValueError(f"ID 字节长度必须为 8, 实际 {len(data)}")
        return cls(int.from_bytes(data, byteorder="big"))


def check_epoch(elapsed: int) -> int:
    """校验自纪元起的毫秒数落在 physical 41 位可表示范围内, 对齐 Go 的 checkEpoch."""
    if elapsed < 0 or elapsed >= (1 << PHYSICAL_BITS):
        raise ValueError(
            f"自纪元起毫秒数超出 {PHYSICAL_BITS} 位可表示范围: {elapsed}"
        )
    return elapsed
