# -*- coding: utf-8 -*-
# Copyright (c) 2026 Quant1X. All rights reserved.
# Author: wangfeng <wangfengxy@sina.cn>
# SPDX-License-Identifier: MIT

from __future__ import annotations

import os
import time
import zlib
from dataclasses import dataclass
from typing import Protocol, Tuple

RECORD_SIZE = 18


@dataclass(frozen=True)
class PersistentState:
    physical: int
    logical: int
    seq: int


class StateStore(Protocol):
    def load(self) -> Tuple[PersistentState, bool]:
        ...

    def next_state(self, local: PersistentState, now_ms: int, seed: int) -> PersistentState:
        ...


def compare_persistent_state(left: PersistentState, right: PersistentState) -> int:
    if left.physical < right.physical:
        return -1
    if left.physical > right.physical:
        return 1
    if left.logical < right.logical:
        return -1
    if left.logical > right.logical:
        return 1
    if left.seq < right.seq:
        return -1
    if left.seq > right.seq:
        return 1
    return 0


def encode_state(state: PersistentState) -> bytes:
    body = (
        int(state.physical).to_bytes(8, "big", signed=False)
        + int(state.logical).to_bytes(2, "big", signed=False)
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
        logical=int.from_bytes(record[8:10], "big", signed=False),
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
    def __init__(self, path: str, sync_every: int = 1) -> None:
        self.path = path
        self.lock_path = path + ".lock"
        self.sync_every = max(1, int(sync_every))
        self.unsynced = 0

    def load(self) -> Tuple[PersistentState, bool]:
        return self._load_latest_state()

    def next_state(self, local: PersistentState, now_ms: int, seed: int) -> PersistentState:
        with _ProcessFileLock(self.lock_path):
            latest, ok = self._load_latest_state()
            base = latest if ok and compare_persistent_state(latest, local) > 0 else local
            next_state = advance_persistent_state(base, now_ms, seed)
            self._append_state(next_state)
            return next_state

    def _load_latest_state(self) -> Tuple[PersistentState, bool]:
        if not os.path.exists(self.path):
            return PersistentState(0, 0, 0), False

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
        os.makedirs(os.path.dirname(self.path) or ".", exist_ok=True)
        record = encode_state(state)
        with open(self.path, "ab") as file:
            file.write(record)
            self.unsynced += 1
            if self.unsynced >= self.sync_every:
                file.flush()
                os.fsync(file.fileno())
                self.unsynced = 0


def advance_persistent_state(state: PersistentState, now_ms: int, seed: int) -> PersistentState:
    physical = state.physical
    logical = state.logical
    seq = state.seq

    if now_ms > physical:
        return PersistentState(now_ms, seed & 0xFFFF, 0)

    seq = (seq + 1) & 0xFFFFFFFF
    if seq == 0:
        logical = (logical + 1) & 0xFFFF
        if logical == 0:
            physical += 1
            logical = seed & 0xFFFF

    return PersistentState(physical, logical, seq)