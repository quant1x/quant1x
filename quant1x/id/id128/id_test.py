# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

import os
import tempfile
import threading
import unittest

from .generator import Generator
from .hlc import HLC
from .id import ID
from .option import with_clock, with_logical_seed, with_state_file
from .state_store import FileStateStore, PersistentState
from .uint128 import Uint128


class IdTests(unittest.TestCase):
    def test_hlc_rollback_monotonic(self) -> None:
        current = {"value": 1000}
        hlc = HLC(with_clock(lambda: current["value"]), with_logical_seed(7))

        prev_hlc, prev_seq = hlc.now()
        current["value"] = 500
        cur_hlc, cur_seq = hlc.now()

        self.assertTrue(cur_hlc > prev_hlc or (cur_hlc == prev_hlc and cur_seq > prev_seq))

    def test_id_field_decoding(self) -> None:
        hlc_value = 0x0102030405060708
        node_id = 0x11223344
        seq = 0xAABBCCDD
        raw = Uint128(hlc_value, (node_id << 32) | seq)
        identifier = ID.from_uint128(raw)

        self.assertEqual(identifier.hlc(), hlc_value)
        self.assertEqual(identifier.node_id(), node_id)
        self.assertEqual(identifier.seq(), seq)

    def test_persistent_state_across_restart(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            state_file = os.path.join(temp_dir, "id.state")
            current = {"value": 1000}
            options = (
                with_clock(lambda: current["value"]),
                with_logical_seed(7),
                with_state_file(state_file),
            )

            first = Generator(1, HLC(*options)).next()
            second = Generator(1, HLC(*options)).next()
            self.assertLess(first, second)

    def test_load_ignores_corrupted_tail(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            state_file = os.path.join(temp_dir, "id.state")
            store = FileStateStore(state_file)
            want = PersistentState(1234, 7, 99)
            store._append_state(want)

            with open(state_file, "ab") as file:
                file.write(bytes((0xDE, 0xAD, 0xBE, 0xEF)))

            got, ok = store.load()
            self.assertTrue(ok)
            self.assertEqual(got, want)

    def test_generator_concurrent(self) -> None:
        generator = Generator(1, HLC())
        total = 20000
        ids = [None] * total

        def worker(index: int) -> None:
            ids[index] = generator.next()

        threads = [threading.Thread(target=worker, args=(i,)) for i in range(total)]
        for thread in threads:
            thread.start()
        for thread in threads:
            thread.join()

        self.assertEqual(len(set(ids)), total)
        sorted_ids = sorted(ids)
        for i in range(1, total):
            self.assertLess(sorted_ids[i - 1], sorted_ids[i])


if __name__ == "__main__":
    unittest.main()
