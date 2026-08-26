# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

import os
import tempfile
import threading
import unittest

from .generator import Generator
from .hlc import HLC, PAYLOAD_BITS
from .id import ID
from .option import with_clock, with_node_count, with_seq_seed, with_state_file, with_state_strict
from .state_store import PersistentState


class Id64Tests(unittest.TestCase):
    def test_hlc_rollback_monotonic(self) -> None:
        current = {"value": 1000}
        hlc = HLC(with_clock(lambda: current["value"]), with_seq_seed(9))

        prev_physical, prev_seq = hlc.now()
        current["value"] = 500  # 时钟回拨
        cur_physical, cur_seq = hlc.now()

        self.assertTrue(
            cur_physical > prev_physical
            or (cur_physical == prev_physical and cur_seq > prev_seq)
        )

    def test_uses_options_at_construction(self) -> None:
        current = {"value": 4321}
        hlc = HLC(with_clock(lambda: current["value"]), with_seq_seed(9))

        self.assertEqual(hlc.timestamp(), 4321)
        self.assertEqual(hlc.seq, 9)

    def test_node_count_derivation(self) -> None:
        cases = [
            (1024, 11, 11),
            (5000, 13, 9),
            (3, 2, 20),
            (131072, 18, 4),
        ]
        for count, worker_bits, seq_bits in cases:
            hlc = HLC(with_node_count(count))
            self.assertEqual(hlc.seq_bits, seq_bits, f"count={count}")
            gen = Generator(0, hlc)
            self.assertEqual(gen.worker_bits, worker_bits, f"count={count}")

    def test_node_count_too_large(self) -> None:
        with self.assertRaises(ValueError):
            HLC(with_node_count(262144))  # seqBits = 3 < 4

    def test_id_field_decoding(self) -> None:
        elapsed = 0x123456789A
        worker_bits = 11
        seq_bits = 11
        node_id = 0x1F
        seq = 0x2A

        value = (elapsed << PAYLOAD_BITS) | (node_id << seq_bits) | seq
        identifier = ID.from_int(value)

        self.assertEqual(identifier.physical(), elapsed)
        self.assertEqual(identifier.node_id(worker_bits), node_id)
        self.assertEqual(identifier.seq(worker_bits), seq)
        self.assertEqual(identifier.to_int(), value)

    def test_persistent_state_across_restart(self) -> None:
        current = {"value": 1000}
        with tempfile.TemporaryDirectory() as tmp:
            path = os.path.join(tmp, "state.bin")
            hlc1 = HLC(with_clock(lambda: current["value"]), with_state_file(path), with_seq_seed(9))
            p1, s1 = hlc1.now()
            hlc1.close()  # 快速路径为批量缓冲：优雅退出前刷盘

            hlc2 = HLC(with_clock(lambda: current["value"]), with_state_file(path), with_seq_seed(9))
            p2, s2 = hlc2.now()

        self.assertTrue(p2 > p1 or (p2 == p1 and s2 > s1))

    def test_shared_state_file(self) -> None:
        current = {"value": 1000}
        with tempfile.TemporaryDirectory() as tmp:
            path = os.path.join(tmp, "state.bin")
            # 多写者活跃共享：必须显式开启严格模式（每次发号读盘取 max）
            hlc1 = HLC(with_clock(lambda: current["value"]), with_state_file(path), with_seq_seed(9), with_state_strict())
            hlc2 = HLC(with_clock(lambda: current["value"]), with_state_file(path), with_seq_seed(9), with_state_strict())

            prev_p, prev_s = hlc1.now()
            for i in range(1000):
                if i % 2 == 0:
                    p, s = hlc2.now()
                else:
                    p, s = hlc1.now()
                self.assertTrue(p > prev_p or (p == prev_p and s > prev_s))
                prev_p, prev_s = p, s

    def test_corrupted_tail(self) -> None:
        current = {"value": 1000}
        with tempfile.TemporaryDirectory() as tmp:
            path = os.path.join(tmp, "state.bin")
            hlc = HLC(with_clock(lambda: current["value"]), with_state_file(path))
            hlc.now()
            hlc.close()  # 快速路径为批量缓冲：先落盘有效记录

            with open(path, "ab") as f:
                f.write(b"ABCDEFGHIJKLMNOPQR")  # 18 字节 CRC 不匹配的坏记录

            hlc2 = HLC(with_clock(lambda: current["value"]), with_state_file(path))
            hlc2.now()  # 坏损尾部被跳过，继续工作

    def test_node_id_out_of_range(self) -> None:
        hlc = HLC(with_node_count(3))  # workerBits=2，nodeID 上限 3
        with self.assertRaises(ValueError):
            Generator(4, hlc)

    def test_generator_monotonic(self) -> None:
        hlc = HLC()
        gen = Generator(1, hlc)
        prev = gen.next()
        for _ in range(1000):
            cur = gen.next()
            self.assertGreater(cur, prev)
            prev = cur

    def test_generator_concurrent_unique(self) -> None:
        hlc = HLC()
        gen = Generator(1, hlc)
        n = 10000
        ids = []
        lock = threading.Lock()

        def worker() -> None:
            for _ in range(n // 8):
                value = gen.next()
                with lock:
                    ids.append(value)

        threads = [threading.Thread(target=worker) for _ in range(8)]
        for t in threads:
            t.start()
        for t in threads:
            t.join()

        self.assertEqual(len(ids), n)
        self.assertEqual(len(set(ids)), n)

    def test_generator_fields(self) -> None:
        current = {"value": 1767225600123}
        hlc = HLC(with_clock(lambda: current["value"]), with_seq_seed(9))
        gen = Generator(7, hlc)

        value = gen.next()
        identifier = ID.from_int(value)
        self.assertEqual(identifier.node_id(gen.worker_bits), 7)


if __name__ == "__main__":
    unittest.main()
