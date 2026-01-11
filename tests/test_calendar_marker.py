import os
import sys
import importlib
import time


def test_calendar_marker_real(tmp_path, monkeypatch):
    # isolate meta path to a temp directory
    monkeypatch.setattr('quant1x.config.meta_path', str(tmp_path))

    # force-reload exchange.calendar to pick up patched meta_path
    for m in list(sys.modules.keys()):
        if m.startswith('quant1x.exchange'):
            sys.modules.pop(m, None)

    cal = importlib.import_module('quant1x.exchange.calendar')

    fn = cal.calendar_file_path()
    marker = cal._calendar_marker_path()

    # ensure clean state
    if os.path.exists(fn):
        os.remove(fn)
    if os.path.exists(marker):
        os.remove(marker)

    # run the real update (per project convention this may hit the real upstream)
    cal.__update_calendar()

    assert os.path.exists(fn), "calendar cache file not created"
    assert os.path.exists(marker), "calendar.updated marker not created"

    cache_mtime = os.path.getmtime(fn)
    marker_mtime = os.path.getmtime(marker)

    # print timestamps for debugging: epoch and human-readable
    print(f"cache mtime:  {cache_mtime} -> {time.ctime(cache_mtime)}")
    print(f"marker mtime: {marker_mtime} -> {time.ctime(marker_mtime)}")

    # marker.mtime should reflect local completion time and be recent.
    now = time.time()
    # allow a small window for test execution and filesystem timestamp granularity
    assert abs(now - marker_mtime) < 5, f"marker mtime not recent: now={now}, marker={marker_mtime}"
