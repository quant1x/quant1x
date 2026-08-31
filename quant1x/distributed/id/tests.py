# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.
"""分布式 ID 测试 (对齐 Go id_test.go / Rust tests.rs 语义).

运行: python -m unittest quant1x.distributed.id.tests
"""

import os
import struct
import tempfile
import threading
import time
import unittest
import zlib

from .generator import CanceledError, Generator
from .hlc import HLC
from .id import ID, PAYLOAD_BITS
from .option import with_clock, with_node_count, with_seq_seed, with_state_file, with_state_strict
from .queue import Queue, QueueClosedError, QueueEmptyError, QueueFullError
from .state_store import PERSISTENT_STATE_RECORD_SIZE


class TestGenerator(unittest.TestCase):
    def test_default_generator(self):
        generator = Generator(0, HLC())
        prev = None
        for _ in range(4096):
            identifier = generator.next()
            if prev is not None:
                self.assertGreater(identifier.to_int(), prev)
            prev = identifier.to_int()
        self.assertEqual(generator.next().node_id(0), 0)

    def test_generator_node_range(self):
        with self.assertRaises(ValueError):
            Generator(1 << 11, HLC())  # 默认 11 位 seq → 11 位 worker, 2048 个节点

    def test_hlc_rollback_monotonic(self):
        current = {"value": 1000}
        hlc = HLC(with_clock(lambda: current["value"]), with_seq_seed(9))
        first_p, first_s = hlc.now()
        current["value"] = 999  # 物理时钟回拨
        second_p, second_s = hlc.now()
        # 单调性: 物理不回退; 同物理时 seq 递增 (对齐 Rust 测试语义)
        self.assertTrue(
            second_p > first_p or (second_p == first_p and second_s > first_s),
            f"rollback broke monotonicity: ({first_p},{first_s}) -> ({second_p},{second_s})",
        )
        current["value"] = 1100
        third_p, third_s = hlc.now()
        self.assertGreater(third_p, second_p)
        self.assertEqual(third_s, 0)  # 物理推进, seq 重置
        self.assertEqual(hlc.timestamp(), third_p)

    def test_node_count_derivation(self):
        self.assertEqual(HLC(with_node_count(3)).seq_bits, 20)
        self.assertEqual(HLC(with_node_count(1024)).seq_bits, 11)
        self.assertEqual(HLC(with_node_count(5000)).seq_bits, 9)
        with self.assertRaises(ValueError):
            HLC(with_node_count(262144))  # 需要至少 4 位 seq

    def test_id_fields_and_encoding(self):
        identifier = ID((123 << PAYLOAD_BITS) | (7 << 11) | 42)
        self.assertEqual(identifier.physical(), 123)
        self.assertEqual(identifier.node_id(11), 7)
        self.assertEqual(identifier.seq(11), 42)
        self.assertEqual(ID.from_bytes(identifier.to_bytes()), identifier)
        # URL-safe Base64 无填充: 8 字节 → 11 字符
        self.assertEqual(len(identifier.string()), 11)

    def test_concurrent_unique(self):
        generator = Generator(1, HLC())
        threads = 8
        per_thread = 1250
        results = [[] for _ in range(threads)]
        barrier = threading.Barrier(threads)

        def worker(idx):
            barrier.wait()
            for _ in range(per_thread):
                results[idx].append(generator.next().to_int())

        ts = [threading.Thread(target=worker, args=(i,)) for i in range(threads)]
        for t in ts:
            t.start()
        for t in ts:
            t.join()
        all_ids = [v for row in results for v in row]
        self.assertEqual(len(all_ids), threads * per_thread)
        self.assertEqual(len(set(all_ids)), len(all_ids))


class TestQueue(unittest.TestCase):
    def test_queue_semantics(self):
        q = Queue(4)
        self.assertEqual(q.cap(), 4)  # 已是 2 的幂, 不变
        for i in range(4):
            q.try_push(i)
        with self.assertRaises(QueueFullError):
            q.try_push(5)
        self.assertEqual(q.len(), 4)
        self.assertFalse(q.is_empty())
        self.assertTrue(q.is_full())
        for i in range(4):
            self.assertEqual(q.try_pop(), i)
        with self.assertRaises(QueueEmptyError):
            q.try_pop()
        q.close()
        with self.assertRaises(QueueClosedError):
            q.try_push(1)
        with self.assertRaises(QueueClosedError):
            q.try_pop()
        q.close()  # 幂等

    def test_queue_capacity_rounds_up(self):
        self.assertEqual(Queue(5).cap(), 8)
        self.assertEqual(Queue(1023).cap(), 1024)
        self.assertEqual(Queue(1).cap(), 1)
        with self.assertRaises(ValueError):
            Queue(0)


class TestServe(unittest.TestCase):
    @staticmethod
    def _pop_with_retry(q, deadline=5.0):
        """非阻塞消费, 队列短暂为空时重试 (对齐 Go TryPop 语义的测试辅助)."""
        end = time.time() + deadline
        while True:
            try:
                return q.try_pop()
            except QueueEmptyError:
                if time.time() > end:
                    raise
                time.sleep(0.001)

    def test_generator_serve_feeds_queue(self):
        generator = Generator(0, HLC())
        q = Queue(1024)
        cancel = threading.Event()
        result = {}
        serve_done = threading.Event()

        def run():
            try:
                generator.serve(q, cancel)
            except CanceledError:
                result["err"] = CanceledError
            finally:
                serve_done.set()

        t = threading.Thread(target=run)
        t.start()
        try:
            prev = None
            for _ in range(4096):
                identifier = self._pop_with_retry(q)
                if prev is not None:
                    self.assertGreater(identifier.to_int(), prev)
                prev = identifier.to_int()
            self.assertIsNone(result.get("err"))
            cancel.set()
            self.assertTrue(serve_done.wait(timeout=5))
            self.assertIs(result["err"], CanceledError)
        finally:
            cancel.set()
            q.close()
            t.join(timeout=5)

    def test_generator_serve_stops_on_closed_queue(self):
        generator = Generator(0, HLC())
        q = Queue(1024)
        q.close()
        self.assertIsNone(generator.serve(q))

    def test_generator_serve_drain_after_cancel(self):
        generator = Generator(0, HLC())
        q = Queue(8)  # 容量 8, 生产快于消费, serve 将阻塞在满队列重试
        cancel = threading.Event()
        result = {}

        def run():
            try:
                generator.serve(q, cancel)
            except CanceledError:
                result["err"] = CanceledError

        t = threading.Thread(target=run)
        t.start()
        try:
            while q.len() < 8:
                time.sleep(0.001)
            cancel.set()  # 取消 → serve 必须退出 (不阻塞在满队列上)
            t.join(timeout=5)
            self.assertFalse(t.is_alive())
            self.assertIs(result["err"], CanceledError)
            q.close()  # 关闭并排空, 验证无 ID 丢失
            drained = []
            while True:
                try:
                    drained.append(q.try_pop())
                except QueueClosedError:
                    break
            self.assertEqual(len(drained), 8)
        finally:
            cancel.set()
            q.close()
            t.join(timeout=5)


class TestStateFile(unittest.TestCase):
    def test_state_file_across_restart(self):
        with tempfile.TemporaryDirectory() as tmp:
            state_file = os.path.join(tmp, "id.state")
            # 固定物理时钟: seq 承担单调递增, 便于确定性断言
            clock = lambda: 1000
            hlc1 = None
            hlc2 = None
            try:
                # 第一次运行: 无持久化记录 → 种子初始化 seq=9
                hlc1 = HLC(with_clock(clock), with_state_file(state_file), with_seq_seed(9))
                self.assertEqual(hlc1.seq, 9)
                self.assertEqual(hlc1.now(), (1000, 10))  # 构造时物理已=1000, seq 递增
                self.assertEqual(hlc1.now(), (1000, 11))
                hlc1.close()
                hlc1 = None

                # 第二次运行: 从状态文件恢复, 继续严格单调
                hlc2 = HLC(with_clock(clock), with_state_file(state_file), with_seq_seed(9))
                self.assertGreater(hlc2.seq, 0)  # 从文件恢复, 而非种子
                self.assertEqual(hlc2.now(), (1000, 12))
                hlc2.close()
                hlc2 = None
            finally:
                if hlc2 is not None:
                    hlc2.close()
                if hlc1 is not None:
                    hlc1.close()

    def test_state_file_strict(self):
        with tempfile.TemporaryDirectory() as tmp:
            state_file = os.path.join(tmp, "id.state")
            clock = lambda: 1000
            hlc1 = None
            hlc2 = None
            try:
                hlc1 = HLC(
                    with_clock(clock),
                    with_state_file(state_file),
                    with_state_strict(),
                    with_seq_seed(9),
                )
                self.assertEqual(hlc1.now(), (1000, 10))
                self.assertEqual(hlc1.now(), (1000, 11))
                hlc1.close()
                hlc1 = None

                hlc2 = HLC(
                    with_clock(clock),
                    with_state_file(state_file),
                    with_state_strict(),
                    with_seq_seed(9),
                )
                self.assertGreater(hlc2.seq, 0)
                self.assertEqual(hlc2.now(), (1000, 12))
                hlc2.close()
                hlc2 = None
            finally:
                if hlc2 is not None:
                    hlc2.close()
                if hlc1 is not None:
                    hlc1.close()

    def test_corrupted_tail(self):
        """旧版 18B 追加式日志: 尾部 CRC 损坏应被跳过并截断 (迁移逻辑)."""
        with tempfile.TemporaryDirectory() as tmp:
            state_file = os.path.join(tmp, "id.state")
            good = bytearray(PERSISTENT_STATE_RECORD_SIZE)
            struct.pack_into(">q", good, 0, 1234)
            struct.pack_into(">I", good, 10, 7)
            checksum = zlib.crc32(bytes(good[:14])) & 0xFFFFFFFF
            struct.pack_into(">I", good, 14, checksum)

            bad = bytearray(good)
            struct.pack_into(">q", bad, 0, 9999)
            struct.pack_into(">I", bad, 14, 0)  # 故意损坏 CRC

            with open(state_file, "wb") as f:
                f.write(bytes(good))
                f.write(bytes(bad))

            hlc = None
            try:
                hlc = HLC(with_state_file(state_file), with_seq_seed(9))
                self.assertEqual(hlc.physical, 1234)  # 跳过损坏尾部
                self.assertEqual(hlc.seq, 7)
                hlc.close()
                hlc = None
            finally:
                if hlc is not None:
                    hlc.close()


if __name__ == "__main__":
    unittest.main()
