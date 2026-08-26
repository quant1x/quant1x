# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

import os
import time
import zlib
from dataclasses import dataclass
from typing import Protocol, Tuple

# 状态文件单条记录大小（与 id128 一致）：
# physical(8B) + logical(2B, 恒 0) + seq(4B) + crc32(4B) = 18B。
RECORD_SIZE = 18

# 默认落盘间隔：快速路径下状态记录先在内存批量缓冲中累积，
# 每攒满 N 条才一次性落盘（带跨进程锁 + fsync）。
# 可用环境变量 QUANT1X_ID64_SYNC_EVERY 覆盖（显式 with_state_sync_every 优先级最高）。
# 默认 1000：大多数请求不碰磁盘；进程异常退出最多丢失最近 1000 条进度
# （这些 ID 重启后可能重复），优雅退出前调用 close() 可零丢失。
DEFAULT_SYNC_EVERY = 1000


def default_sync_every() -> int:
    """返回默认落盘间隔（环境变量 QUANT1X_ID64_SYNC_EVERY，未设置或非法时为 1000）。"""
    raw = os.environ.get("QUANT1X_ID64_SYNC_EVERY")
    if raw:
        try:
            value = int(raw)
            if value > 0:
                return value
        except ValueError:
            pass
    return DEFAULT_SYNC_EVERY


@dataclass(frozen=True)
class PersistentState:
    physical: int
    seq: int


class StateStore(Protocol):
    def load(self) -> Tuple[PersistentState, bool]:
        ...

    def next_state(self, local: PersistentState, now_ms: int, seq_bits: int) -> PersistentState:
        ...


def compare_persistent_state(left: PersistentState, right: PersistentState) -> int:
    if left.physical < right.physical:
        return -1
    if left.physical > right.physical:
        return 1
    if left.seq < right.seq:
        return -1
    if left.seq > right.seq:
        return 1
    return 0


def encode_state(state: PersistentState) -> bytes:
    body = (
        int(state.physical).to_bytes(8, "big", signed=False)
        + b"\x00\x00"  # logical 恒 0（兼容 id128 的 18B 记录格式）
        + int(state.seq).to_bytes(4, "big", signed=False)
    )
    checksum = zlib.crc32(body) & 0xFFFFFFFF
    return body + checksum.to_bytes(4, "big", signed=False)


def decode_state(record: bytes) -> PersistentState:
    if len(record) != RECORD_SIZE:
        raise ValueError("invalid record size")
    checksum = int.from_bytes(record[14:18], "big", signed=False)
    if (zlib.crc32(record[:14]) & 0xFFFFFFFF) != checksum:
        raise ValueError("invalid record checksum")
    return PersistentState(
        physical=int.from_bytes(record[0:8], "big", signed=False),
        seq=int.from_bytes(record[10:14], "big", signed=False),
    )


class _ProcessFileLock:
    def __init__(self, path: str) -> None:
        self._path = path
        self._file = None
        self._remove_on_release = False

    def __enter__(self) -> "_ProcessFileLock":
        if os.name == "nt":
            import msvcrt

            self._file = open(self._path, "a+b")
            self._file.seek(0)
            if self._file.tell() == 0:
                self._file.write(b"0")
                self._file.flush()
            self._file.seek(0)
            msvcrt.locking(self._file.fileno(), msvcrt.LK_LOCK, 1)
            return self

        try:
            import fcntl
        except ImportError:
            deadline = time.time() + 10.0
            while True:
                try:
                    fd = os.open(self._path, os.O_CREAT | os.O_EXCL | os.O_RDWR, 0o644)
                    self._file = os.fdopen(fd, "w+b")
                    self._remove_on_release = True
                    return self
                except FileExistsError:
                    if time.time() >= deadline:
                        raise TimeoutError("获取兼容锁超时")
                    time.sleep(0.01)
        else:
            self._file = open(self._path, "a+b")
            fcntl.flock(self._file.fileno(), fcntl.LOCK_EX)
            return self

    def __exit__(self, exc_type, exc, tb) -> None:
        if self._file is None:
            return

        if os.name == "nt":
            import msvcrt

            self._file.seek(0)
            msvcrt.locking(self._file.fileno(), msvcrt.LK_UNLCK, 1)
        else:
            try:
                import fcntl
            except ImportError:
                pass
            else:
                fcntl.flock(self._file.fileno(), fcntl.LOCK_UN)

        self._file.close()
        if self._remove_on_release:
            try:
                os.remove(self._path)
            except FileNotFoundError:
                pass


class FileStateStore:
    """文件状态存储。

    默认快速路径（strict=False）：构造时恢复一次水位，运行期纯内存推进，
    状态记录先累积在批量缓冲中，攒满 sync_every 条才一次性落盘（带锁 + fsync），
    热路径零系统调用。适合单写者（含多进程顺序接管 / failover）场景——
    新写者构造时读到前任写者最近一次落盘的水位，保证跨进程、跨重启不重复。
    开启严格模式（strict=True）后每次 next_state 都读盘取 max，保证多写者活跃共享唯一。
    """

    def __init__(self, path: str, sync_every: int | None = None, strict: bool = False) -> None:
        self.path = path
        self.lock_path = path + ".lock"
        self.sync_every = max(1, int(sync_every if sync_every is not None else default_sync_every()))
        self.strict = strict
        self.unsynced = 0
        self._dir_ready = False
        self._pending = bytearray()

    def load(self) -> Tuple[PersistentState, bool]:
        return self._load_latest_state()

    def next_state(self, local: PersistentState, now_ms: int, seq_bits: int) -> PersistentState:
        if not self.strict:
            # 快速路径：纯内存推进，记录先入批量缓冲；攒满 sync_every 条才落盘一次。
            # 进程异常退出最多丢失最近 sync_every-1 条进度（这些 ID 重启后可能重复），
            # 优雅退出前调用 close() 可把缓冲完整刷盘、零丢失。
            next_state = advance_persistent_state(local, now_ms, seq_bits)
            self._buffer_state(next_state)
            return next_state

        with _ProcessFileLock(self.lock_path):
            # 严格模式：以磁盘最新状态为基准（多写者活跃共享唯一性）。
            base = local
            latest, ok = self._load_latest_state()
            if ok and compare_persistent_state(latest, base) > 0:
                base = latest
            next_state = advance_persistent_state(base, now_ms, seq_bits)
            self._append_state(next_state)
            return next_state

    def _buffer_state(self, state: PersistentState) -> None:
        self._pending += encode_state(state)
        if len(self._pending) >= self.sync_every * RECORD_SIZE:
            self._flush_pending()

    def _flush_pending(self) -> None:
        if not self._pending:
            return
        with _ProcessFileLock(self.lock_path):
            # 目录已就绪时跳过 makedirs（避免热路径上的 stat 开销）
            if not self._dir_ready:
                os.makedirs(os.path.dirname(self.path) or ".", exist_ok=True)
                self._dir_ready = True
            with open(self.path, "ab") as file:
                file.write(bytes(self._pending))
                file.flush()
                os.fsync(file.fileno())
            self._pending.clear()

    def flush(self) -> None:
        """立即把批量缓冲中的记录写入状态文件（带锁 + fsync）。
        优雅退出前调用，可避免重启后重复最近尚未落盘的 ID。"""
        self._flush_pending()

    def _load_latest_state(self) -> Tuple[PersistentState, bool]:
        if not os.path.exists(self.path):
            return PersistentState(0, 0), False

        size = os.path.getsize(self.path)
        if size < RECORD_SIZE:
            raise ValueError(f"状态文件长度非法: {size}")

        end = size - (size % RECORD_SIZE)
        if end == 0:
            raise ValueError(f"状态文件长度非法: {size}")

        with open(self.path, "rb") as file:
            offset = end - RECORD_SIZE
            while offset >= 0:
                file.seek(offset)
                record = file.read(RECORD_SIZE)
                if len(record) == RECORD_SIZE:
                    try:
                        return decode_state(record), True
                    except ValueError:
                        pass
                offset -= RECORD_SIZE

        raise ValueError("状态文件中没有有效记录")

    def _append_state(self, state: PersistentState) -> None:
        # 目录已就绪时跳过 makedirs（避免热路径上的 stat 开销）
        if not self._dir_ready:
            os.makedirs(os.path.dirname(self.path) or ".", exist_ok=True)
            self._dir_ready = True
        record = encode_state(state)
        with open(self.path, "ab") as file:
            file.write(record)
            self.unsynced += 1
            if self.unsynced >= self.sync_every:
                file.flush()
                os.fsync(file.fileno())
                self.unsynced = 0


def advance_persistent_state(state: PersistentState, now_ms: int, seq_bits: int) -> PersistentState:
    """在共享状态上推进 (physical, seq)：

    - 物理时间前进：重置 seq 为 0
    - 否则 seq+1；seq 达容量时进位 physical+1 并重置 seq（保持单调，不等待墙钟追平）
    """
    if now_ms > state.physical:
        return PersistentState(now_ms, 0)

    mask = (1 << seq_bits) - 1
    if state.seq >= mask:
        return PersistentState(state.physical + 1, 0)
    return PersistentState(state.physical, state.seq + 1)
