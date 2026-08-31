# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.
"""混合逻辑时钟 HLC (与 Go 的 hlc.go / Rust 的 hlc.rs 语义对齐).

约定:
    now()      推进并返回 (physical_ms, seq)
    timestamp() 返回 physical_ms (不推进)
    close()    刷新状态文件 (幂等)

物理时钟回拨时通过递增 seq 保证单调; seq 溢出时物理时钟 +1 并重置 seq.
"""

import os
import struct
import threading
import time

from .option import default_sync_every_value
from .state_store import FileStateStore, PersistentState


def random_uint16() -> int:
    """进程级随机种子 (对齐 Go 的 randUint16: crypto/rand, 失败时退化)."""
    try:
        return struct.unpack(">H", os.urandom(2))[0]
    except NotImplementedError:
        return (int(time.time_ns()) ^ (os.getpid() << 16)) & 0xFFFF


class HLC:
    """混合逻辑时钟: 41 位物理毫秒 + 22 位 payload (worker + seq)."""

    def __init__(self, *opts):
        self._lock = threading.Lock()
        self._clock = lambda: int(time.time() * 1000)
        self.physical = 0
        self.seq = 0
        self.seed = random_uint16()
        self.seq_bits = 11  # payloadBits(22) - 默认 2^11 节点
        self.sync_every = default_sync_every_value()
        self.strict = False
        self.state_file = None
        self.store = None

        for opt in opts:
            opt(self)

        if self.state_file is not None:
            self.store = FileStateStore(self.state_file, self.sync_every, self.strict)

        restored = None
        ok = False
        if self.store is not None:
            restored, ok = self.store.load()
        if ok:
            self.physical = restored.physical
            self.seq = restored.seq
        else:
            self.physical = self._clock()
            self.seq = self.seed & self.seq_mask

    @property
    def seq_mask(self) -> int:
        return (1 << self.seq_bits) - 1

    def now(self):
        """推进并返回 (physical, seq), 对齐 Go 的 Now()."""
        with self._lock:
            now = self._clock()
            if self.store is not None:
                next_state = self.store.next(
                    PersistentState(self.physical, self.seq), now, self.seq_bits
                )
                self.physical = next_state.physical
                self.seq = next_state.seq
                return self.physical, self.seq

            if now > self.physical:
                self.physical = now
                self.seq = 0
            elif self.seq >= self.seq_mask:
                self.physical += 1
                self.seq = 0
            else:
                self.seq += 1
            return self.physical, self.seq

    def timestamp(self) -> int:
        """返回物理毫秒, 对齐 Go 的 Timestamp()."""
        return self.physical

    def close(self) -> None:
        """刷新状态文件并释放映射 (幂等, 对齐 Go 的 Close())."""
        with self._lock:
            if self.store is None:
                return
            first_err = None
            try:
                self.store.flush()
            except Exception as e:
                first_err = e
            try:
                self.store.close()
            except Exception as e:
                if first_err is None:
                    first_err = e
            if first_err is not None:
                raise first_err
