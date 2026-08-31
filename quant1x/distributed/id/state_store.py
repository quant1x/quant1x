# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.
"""跨进程持久化状态存储 (与 Go/Rust 语义对齐).

布局: 128 字节定长文件, 内存映射.
    [0, 64)    双槽 checkpoint 区 (每槽 32 字节, 槽 0/1 交替写入)
    [64, 72)   锁字 (uint64: 高 32 位 pid, 低 32 位时间戳秒)

checkpoint 槽格式 (32 字节, 大端):
    [0,8)  generation  | [8,16) physical(int64) | [16,20) seq(uint32) | [20,24) crc32(IEEE, 覆盖 [0,20)) | 其余保留

兼容旧版 18 字节追加式日志 (id64): 文件大小 != 128 时按旧格式从尾部向前
扫描最近一条 CRC 校验通过的记录并迁移.

锁字抢占: 持有者 pid 进程已死 或 时间戳超过 30 秒 视为 stale, 可接管.
注意: Python 的 mmap 无原子 CAS, 锁字读改写以进程内互斥 + 写后校验收敛,
跨进程存在极小竞争窗口 (探索层折衷, 语义结构对齐 Go/Rust).
"""

import mmap
import os
import struct
import sys
import threading
import time
import zlib
from dataclasses import dataclass

STATE_FILE_SIZE = 128
CHECKPOINT_SLOT_SIZE = 32
CHECKPOINT_SLOT_COUNT = 2
CHECKPOINT_AREA_SIZE = 64
STATE_LOCK_OFFSET = CHECKPOINT_AREA_SIZE
LEGACY_RECORD_SIZE = 18
PERSISTENT_STATE_RECORD_SIZE = LEGACY_RECORD_SIZE
LOCK_TAKEOVER_AFTER_SECONDS = 30
LOCK_BACKOFF_MAX_SLEEP_US = 1024
MAX_PID = (1 << 32) - 1
MAX_STAMP = (1 << 32) - 1

# 进程内串行化锁字读改写 (Python 无 mmap 原子 CAS 的折衷)
_LOCK_WORD_MUTEX = threading.Lock()


class StateFileError(Exception):
    """状态文件读写/映射/锁字相关错误 (对应 Rust 的 Error::StateFile)."""


@dataclass
class PersistentState:
    """持久化水位: 物理时间(ms) + 序列号."""
    physical: int = 0
    seq: int = 0


def compare_persistent_state(a: PersistentState, b: PersistentState) -> int:
    """比较两个持久化状态, 返回 -1/0/1 (对齐 Go 的 comparePersistentState)."""
    if a.physical != b.physical:
        return 1 if a.physical > b.physical else -1
    if a.seq != b.seq:
        return 1 if a.seq > b.seq else -1
    return 0


def advance_persistent_state(
    prev: PersistentState, now: int, seq_bits: int
) -> PersistentState:
    """推进水位 (对齐 Go 的 advancePersistentState)."""
    seq_mask = (1 << seq_bits) - 1
    if now > prev.physical:
        return PersistentState(physical=now, seq=0)
    if prev.seq >= seq_mask:
        return PersistentState(physical=prev.physical + 1, seq=0)
    return PersistentState(physical=prev.physical, seq=prev.seq + 1)


def lock_stamp_now() -> int:
    return int(time.time())


def encode_lock_word(pid: int, stamp: int) -> int:
    return ((pid & MAX_PID) << 32) | (stamp & MAX_STAMP)


def decode_lock_word(word: int) -> tuple:
    return (word >> 32) & MAX_PID, word & MAX_STAMP


def lock_holder_stale(pid: int, stamp: int, now: int = None) -> bool:
    """锁字持有者是否 stale (对齐 Go 的 lockHolderStale)."""
    if stamp == 0:
        return True
    if now is None:
        now = lock_stamp_now()
    if now < stamp:
        return False
    return now - stamp > LOCK_TAKEOVER_AFTER_SECONDS


def lock_backoff(retries: int) -> None:
    """指数退避 (对齐 Go 的 lockBackoff): 1us..1024us, 24 次后不再睡眠."""
    if retries < 24:
        backoff = 1 << (retries // 4)
        if backoff > LOCK_BACKOFF_MAX_SLEEP_US:
            backoff = LOCK_BACKOFF_MAX_SLEEP_US
        time.sleep(backoff / 1_000_000)


def process_alive(pid: int) -> bool:
    """进程是否存活 (对齐 Go 分平台的 processAlive)."""
    if pid <= 0:
        return False
    if sys.platform.startswith("win"):
        return _process_alive_windows(pid)
    if os.name == "posix":
        return _process_alive_posix(pid)
    # 未知平台: 保守判定存活, 由锁字 TTL 抢占兜底
    return True


def _process_alive_windows(pid: int) -> bool:
    try:
        import ctypes
    except ImportError:
        return True
    kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
    # 与 Go 一致: PROCESS_QUERY_INFORMATION = 0x0400
    PROCESS_QUERY_INFORMATION = 0x0400
    handle = kernel32.OpenProcess(PROCESS_QUERY_INFORMATION, False, pid)
    if not handle:
        # ERROR_ACCESS_DENIED(5): 进程存在但无查询权限
        return ctypes.get_last_error() == 5
    try:
        result = kernel32.WaitForSingleObject(handle, 0)
        # WAIT_OBJECT_0(0)/WAIT_ABANDONED(0x80): 进程已退出
        # WAIT_TIMEOUT(0x102): 进程存活
        return result == 0x102
    finally:
        kernel32.CloseHandle(handle)


def _process_alive_posix(pid: int) -> bool:
    try:
        os.kill(pid, 0)
    except ProcessLookupError:
        return False
    except PermissionError:
        return True
    except OSError:
        return True
    return True


class FileStateStore:
    """定长 mmap 状态文件存储 (对齐 Go 的 fileStateStore / Rust 的 FileStateStore)."""

    def __init__(self, path: str, sync_every: int = None, strict: bool = False):
        from .option import default_sync_every_value

        self.path = path
        self.sync_every = sync_every if sync_every and sync_every > 0 else default_sync_every_value()
        self.strict = strict
        self.unsynced = 0
        self.generation = 0
        self.latest = PersistentState()
        self._file = None
        self.mapped = None

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
        return False

    def load(self):
        """读取持久化状态: 优先 mmap checkpoint, 兼容旧版追加式日志 (对齐 Go)."""
        legacy = PersistentState()
        ok = False
        try:
            size = os.path.getsize(self.path)
        except FileNotFoundError:
            size = None
        if size is not None and size != STATE_FILE_SIZE:
            # 旧版追加式日志格式, 读取最近一条有效记录并迁移
            legacy, ok = self.load_latest_state()
        self.open_mapped()
        mapped_state, mapped_ok = self.load_checkpoint()
        if mapped_ok and (not ok or compare_persistent_state(mapped_state, legacy) > 0):
            legacy, ok = mapped_state, True
        if ok:
            self.latest = legacy
        return legacy, ok

    def open_mapped(self) -> None:
        """打开 (必要时创建) 定长状态文件并映射到内存 (对齐 Go 的 openMapped)."""
        if self.mapped is not None:
            return
        # 创建文件 (不覆盖已有内容); Windows 要求映射长度不超过文件大小
        f = open(self.path, "a+b")
        try:
            f.seek(0, os.SEEK_END)
            if f.tell() < STATE_FILE_SIZE:
                f.truncate(STATE_FILE_SIZE)
        except OSError as e:
            f.close()
            raise StateFileError(f"初始化 state 文件 {self.path!r} 失败: {e}") from e
        self._file = f  # mmap 存活期间句柄必须保持打开
        try:
            self.mapped = mmap.mmap(f.fileno(), STATE_FILE_SIZE, access=mmap.ACCESS_WRITE)
        except (OSError, ValueError) as e:
            f.close()
            self._file = None
            raise StateFileError(f"mmap state 文件 {self.path!r} 失败: {e}") from e

    def load_checkpoint(self):
        """扫描双槽 checkpoint, 返回最新有效水位 (对齐 Go 的 loadCheckpoint)."""
        if self.mapped is None:
            self.open_mapped()
        best = PersistentState()
        best_generation = 0
        valid = False
        for slot in range(CHECKPOINT_SLOT_COUNT):
            base = slot * CHECKPOINT_SLOT_SIZE
            record = bytes(self.mapped[base:base + CHECKPOINT_SLOT_SIZE])
            generation = struct.unpack(">Q", record[0:8])[0]
            if generation == 0:
                continue
            checksum = struct.unpack(">I", record[20:24])[0]
            if (zlib.crc32(record[0:20]) & 0xFFFFFFFF) != checksum:
                continue
            physical = struct.unpack(">q", record[8:16])[0]
            seq = struct.unpack(">I", record[16:20])[0]
            if not valid or generation > best_generation:
                best_generation = generation
                best = PersistentState(physical=physical, seq=seq)
                valid = True
        if valid:
            self.generation = best_generation
            self.latest = best
        return best, valid

    def checkpoint(self, state: PersistentState, flush: bool) -> None:
        """写入一个 checkpoint 槽 (对齐 Go 的 checkpoint)."""
        self.open_mapped()
        self.generation += 1
        slot = (self.generation % CHECKPOINT_SLOT_COUNT) * CHECKPOINT_SLOT_SIZE
        record = bytearray(CHECKPOINT_SLOT_SIZE)
        struct.pack_into(">Q", record, 0, self.generation)
        struct.pack_into(">q", record, 8, state.physical)
        struct.pack_into(">I", record, 16, state.seq)
        checksum = zlib.crc32(bytes(record[0:20])) & 0xFFFFFFFF
        struct.pack_into(">I", record, 20, checksum)
        self.mapped[slot:slot + CHECKPOINT_SLOT_SIZE] = bytes(record)
        # 写入成功后同步内存水位, 保证 Flush()/Close() 不会用旧水位覆盖新 checkpoint.
        # 四种语言统一在 checkpoint 内同步 (Python 为 Spec 锚点, 此修正已同步到 Go/Rust/C++).
        self.latest = state
        if flush:
            self.mapped.flush()

    def load_latest_state(self):
        """旧版 18 字节追加式日志迁移: 尾部向前扫描最近有效记录并截断 (对齐 Go)."""
        try:
            f = open(self.path, "rb")
        except FileNotFoundError:
            return PersistentState(), False
        with f:
            f.seek(0, os.SEEK_END)
            size = f.tell()
            end = size - size % PERSISTENT_STATE_RECORD_SIZE
            if end == 0:
                return PersistentState(), False
            offset = end - PERSISTENT_STATE_RECORD_SIZE
            while offset >= 0:
                f.seek(offset)
                record = f.read(PERSISTENT_STATE_RECORD_SIZE)
                if len(record) == PERSISTENT_STATE_RECORD_SIZE:
                    st, valid = self.decode_state_record(record)
                    if valid:
                        if size > offset + PERSISTENT_STATE_RECORD_SIZE:
                            with open(self.path, "r+b") as f2:
                                f2.truncate(offset + PERSISTENT_STATE_RECORD_SIZE)
                        return st, True
                offset -= PERSISTENT_STATE_RECORD_SIZE
        return PersistentState(), False

    @staticmethod
    def decode_state_record(record: bytes):
        """旧版记录格式: [0,8) physical(int64) | [8,10) 保留 | [10,14) seq(uint32) | [14,18) crc32(覆盖 [0,14))."""
        if len(record) != PERSISTENT_STATE_RECORD_SIZE:
            return PersistentState(), False
        checksum = struct.unpack(">I", record[14:18])[0]
        if (zlib.crc32(record[0:14]) & 0xFFFFFFFF) != checksum:
            return PersistentState(), False
        physical = struct.unpack(">q", record[0:8])[0]
        seq = struct.unpack(">I", record[10:14])[0]
        return PersistentState(physical=physical, seq=seq), True

    def next(self, local: PersistentState, now: int, seq_bits: int) -> PersistentState:
        """推进并落盘水位 (对齐 Go 的 next / Rust 的 next).

        非严格模式: 内存推进, 累计 unsynced 达 sync_every 才 checkpoint.
        严格模式: 每次先抢跨进程锁, 以文件内最新水位为基准推进并立即 checkpoint.
        """
        if not self.strict:
            next_state = advance_persistent_state(local, now, seq_bits)
            self.latest = next_state
            self.unsynced += 1
            if self.unsynced >= self.sync_every:
                self.checkpoint(next_state, True)
                self.unsynced = 0
            return next_state

        guard = self.lock_mapped()
        try:
            base = local
            latest, ok = self.load_checkpoint()
            if ok and compare_persistent_state(latest, base) > 0:
                base = latest
            next_state = advance_persistent_state(base, now, seq_bits)
            # 始终以最新推进水位为准; checkpoint() 内也会同步 latest, 两处一致.
            # (Go/Rust 早期实现在严格模式下不更新 latest, 导致 close() 的 flush()
            #  用旧水位覆盖新 checkpoint 造成回退, 该缺陷已在四种语言中统一修正.)
            self.latest = next_state
            flush = self.unsynced + 1 >= self.sync_every
            self.checkpoint(next_state, flush)
            self.unsynced += 1
            if self.unsynced >= self.sync_every:
                self.unsynced = 0
            return next_state
        finally:
            guard.release()

    def flush(self) -> None:
        """立即把尚未 checkpoint 的水位写入映射并 msync (对齐 Go 的 flush)."""
        if self.unsynced == 0:
            return
        guard = self.lock_mapped()
        try:
            self.checkpoint(self.latest, True)
            self.unsynced = 0
        finally:
            guard.release()

    def close(self) -> None:
        """刷新未落盘水位并释放共享映射 (幂等, 对齐 Go 的 close)."""
        first_err = None
        try:
            self.flush()
        except Exception as e:
            first_err = e
        if self.mapped is not None:
            try:
                self.mapped.flush()
            except Exception as e:
                if first_err is None:
                    first_err = e
            try:
                self.mapped.close()
            except Exception as e:
                if first_err is None:
                    first_err = e
            self.mapped = None
        if self._file is not None:
            try:
                self._file.close()
            except Exception as e:
                if first_err is None:
                    first_err = e
            self._file = None
        if first_err is not None:
            raise first_err

    def lock_mapped(self):
        """获取跨进程锁 (返回释放守卫, 对齐 Go 的 lockMapped)."""
        self.open_mapped()
        self_pid = os.getpid()
        mine = encode_lock_word(self_pid, lock_stamp_now())
        retries = 0
        while True:
            current = self._load_lock_word()
            if current == 0:
                if self._cas_lock_word(0, mine):
                    return _LockGuard(self, mine)
                continue
            pid, stamp = decode_lock_word(current)
            if not lock_holder_stale(pid, stamp):
                lock_backoff(retries)
                retries += 1
                continue
            # 持有者已死亡或超时, 尝试接管
            if self._cas_lock_word(current, mine):
                return _LockGuard(self, mine)
            retries = 0

    def _load_lock_word(self) -> int:
        with _LOCK_WORD_MUTEX:
            return struct.unpack(
                ">Q", bytes(self.mapped[STATE_LOCK_OFFSET:STATE_LOCK_OFFSET + 8])
            )[0]

    def _cas_lock_word(self, expected: int, desired: int) -> bool:
        """进程内原子 CAS; 跨进程存在极小竞争窗口 (探索层折衷, 见模块 docstring)."""
        with _LOCK_WORD_MUTEX:
            current = struct.unpack(
                ">Q", bytes(self.mapped[STATE_LOCK_OFFSET:STATE_LOCK_OFFSET + 8])
            )[0]
            if current != expected:
                return False
            self.mapped[STATE_LOCK_OFFSET:STATE_LOCK_OFFSET + 8] = struct.pack(">Q", desired)
            self.mapped.flush()
            return True

    def _unlock_lock_word(self, mine: int) -> None:
        """仅当锁字仍为自己持有时清零 (对齐 Go 的 unlock: CAS 失败即放弃)."""
        with _LOCK_WORD_MUTEX:
            current = struct.unpack(
                ">Q", bytes(self.mapped[STATE_LOCK_OFFSET:STATE_LOCK_OFFSET + 8])
            )[0]
            if current == mine:
                self.mapped[STATE_LOCK_OFFSET:STATE_LOCK_OFFSET + 8] = struct.pack(">Q", 0)
                self.mapped.flush()


class _LockGuard:
    """跨进程锁守卫, release 时按 mine 释放锁字."""
    __slots__ = ("store", "mine")

    def __init__(self, store: FileStateStore, mine: int):
        self.store = store
        self.mine = mine

    def release(self) -> None:
        self.store._unlock_lock_word(self.mine)
