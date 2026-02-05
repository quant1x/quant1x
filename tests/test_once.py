# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import os
import threading
import time
import tempfile
import pytest

from quant1x.runtime.once import RollingOnce


def test_do_once_persists_and_reset(tmp_path):
    marker = str(tmp_path / "calendar.updated")
    ro = RollingOnce(marker=marker)

    r = ro.do_once_try(lambda: 7)
    assert r == 7
    assert os.path.exists(marker)

    r2 = ro.do_once_try(lambda: 8)
    assert r2 is None

    ro.reset()
    r3 = ro.do_once_try(lambda: 9)
    assert r3 == 9


def test_do_once_concurrent_runs_once(tmp_path):
    marker = str(tmp_path / "calendar2.updated")
    ro = RollingOnce(marker=marker)

    counter = {"v": 0}
    lock = threading.Lock()

    def worker():
        def inc():
            with lock:
                counter["v"] += 1

        try:
            ro.do_once_try(inc)
        except Exception:
            pass

    threads = [threading.Thread(target=worker) for _ in range(16)]
    for t in threads:
        t.start()
    for t in threads:
        t.join()

    assert counter["v"] == 1


def test_exception_marks_done(tmp_path):
    marker = str(tmp_path / "calendar3.updated")
    ro = RollingOnce(marker=marker)

    def raise_err():
        raise ValueError("boom")

    with pytest.raises(ValueError):
        ro.do_once_try(raise_err)

    # subsequent calls should be skipped
    assert ro.do_once_try(lambda: 1) is None


def test_seconds_reset(tmp_path):
    marker = str(tmp_path / "calendar4.updated")
    ro = RollingOnce.seconds(1, marker=marker)
    try:
        r = ro.do_once_try(lambda: 1)
        assert r == 1
        assert ro.do_once_try(lambda: 2) is None

        time.sleep(1.3)

        # after the seconds reset, do_once_try should run again
        r2 = ro.do_once_try(lambda: 3)
        assert r2 == 3
    finally:
        ro.close()


def test_mark_run_and_reset_removes_marker(tmp_path):
    marker = str(tmp_path / "calendar5.updated")
    ro = RollingOnce(marker=marker)
    ro.mark_run()
    assert os.path.exists(marker)
    ro.reset()
    assert not os.path.exists(marker)
