import csv
import threading
from dataclasses import dataclass
import os
from typing import Optional
import time

from quant1x import config
from quant1x.exchange.code import correct_security_code
from quant1x.level1 import security_list as _l1_security_list
from quant1x.exchange.code import MarketType


@dataclass
class SecurityInfo:
    code: str
    name: str
    lot_size: int
    price_precision: int


# in-memory cache and synchronization
_SECURITY_MAP = {}
_SECURITY_LOCK = threading.Lock()
_SECURITY_INIT = False
_SECURITY_FILE_MTIME = 0.0

# Pre-market time defaults (match C++ runtime/config defaults)
PRE_MARKET_HOUR = 9
PRE_MARKET_MINUTE = 0
PRE_MARKET_SECOND = 0



def _get_security_filename() -> str:
    return os.path.join(config.meta_path, "securities.csv")


def _file_mtime(path: str) -> float:
    try:
        return os.path.getmtime(path)
    except Exception:
        return 0.0


def _pre_market_epoch_for_today() -> float:
    """Return epoch seconds for today's pre-market time (local time)."""
    t = time.localtime()
    try:
        pre = time.mktime((t.tm_year, t.tm_mon, t.tm_mday,
                           PRE_MARKET_HOUR, PRE_MARKET_MINUTE, PRE_MARKET_SECOND,
                           t.tm_wday, t.tm_yday, t.tm_isdst))
    except Exception:
        pre = 0.0
    return pre


def _is_stale() -> bool:
    """Return True when cache should be refreshed following C++ logic:

    - If cache not initialized
    - If cache file missing
    - If current time >= today's pre-market time AND the cache file mtime is older than that pre-market time
    """
    global _SECURITY_FILE_MTIME
    if not _SECURITY_INIT:
        return True
    filename = _get_security_filename()
    mtime = _file_mtime(filename)
    # missing file -> stale
    if mtime == 0.0:
        return True

    now = time.time()
    pre_ts = _pre_market_epoch_for_today()
    # If we've passed today's pre-market checkpoint and the file timestamp is older than that
    if pre_ts and now >= pre_ts and mtime < pre_ts:
        return True

    # Otherwise consider it fresh (we also detect mtime changes in init_securities under lock)
    return False


def init_securities(force: bool = False) -> None:
    """Initialize or refresh the securities cache.

    If force is True, always reload. Otherwise reload only when not initialized or stale.
    """
    global _SECURITY_INIT, _SECURITY_FILE_MTIME
    if not force and not _is_stale():
        return
    with _SECURITY_LOCK:
        # double-check under lock
        if not force and not _is_stale():
            return
        filename = _get_security_filename()
        # If file missing or stale, attempt to download via level1 and overwrite CSV
        # Mirror C++ behaviour: for each market, fetch pages until page is smaller
        # than PRE_REQUEST_MAX and then write CSV with header matching native code.
        try:
            need_download = False
            file_mtime = _file_mtime(filename)
            now = time.time()
            pre_ts = _pre_market_epoch_for_today()
            if file_mtime == 0.0:
                need_download = True
            elif pre_ts and now >= pre_ts and file_mtime < pre_ts:
                need_download = True
            if need_download:
                rows = []
                markets = [MarketType.SHANGHAI, MarketType.SHENZHEN, MarketType.BEIJING]
                for m in markets:
                    start = 0
                    while True:
                        # call the level1.security_list.fetch_security_list helper
                        try:
                            page = _l1_security_list.fetch_security_list(m.value, start, getattr(_l1_security_list, 'PRE_REQUEST_MAX', 1000))
                        except Exception:
                            page = None

                        if page is None:
                            # on error abort download and fallback to reading existing file
                            rows = []
                            break
                        if not page:
                            break
                        # prefix codes with market flag (sh/sz/bj)
                        prefix = 'sh' if m == MarketType.SHANGHAI else ('sz' if m == MarketType.SHENZHEN else 'bj')
                        for it in page:
                            code = (it.get('Code') or '').strip()
                            code_pref = prefix + code
                            rows.append((code_pref, it.get('VolUnit', 0), it.get('DecimalPoint', 0), it.get('Name', ''), it.get('PreClose', 0.0)))
                        if len(page) < getattr(_l1_security_list, 'PRE_REQUEST_MAX', 1000):
                            break
                        start += getattr(_l1_security_list, 'PRE_REQUEST_MAX', 1000)
                # write CSV if we have rows
                if rows:
                    try:
                        os.makedirs(os.path.dirname(filename), exist_ok=True)
                        with open(filename, 'w', newline='', encoding='utf-8') as fh:
                            writer = csv.writer(fh)
                            writer.writerow(['Code', 'VolUnit', 'DecimalPoint', 'Name', 'PreClose'])
                            for r in rows:
                                writer.writerow(r)
                        # update observed file mtime
                        file_mtime = _file_mtime(filename)
                    except Exception:
                        # ignore write errors; we'll try to load existing file below
                        file_mtime = _file_mtime(filename)
        except Exception:
            # If any unexpected error in download path, continue to attempt reading file
            file_mtime = _file_mtime(filename)
        _SECURITY_MAP.clear()
        # attempt to load CSV into memory
        try:
            with open(filename, newline='', encoding='utf-8') as fh:
                reader = csv.DictReader(fh)
                for row in reader:
                    code = (row.get('Code') or '').strip()
                    if not code:
                        continue
                    try:
                        lot = int(row.get('VolUnit') or 0)
                    except Exception:
                        lot = 0
                    try:
                        prec = int(row.get('DecimalPoint') or 0)
                    except Exception:
                        prec = 0
                    name = (row.get('Name') or '').strip()
                    code_fixed = correct_security_code(code)
                    _SECURITY_MAP[code_fixed] = SecurityInfo(code=code_fixed, name=name, lot_size=lot, price_precision=prec)
        except FileNotFoundError:
            # file not present: leave map empty but record load time to avoid hot-loop
            pass
        except Exception:
            # ignore parse errors; don't raise to callers
            pass
    # record file mtime observed when loading
    _SECURITY_FILE_MTIME = file_mtime
    _SECURITY_INIT = True


def refresh_securities() -> None:
    """Public API to force refresh the securities cache immediately."""
    init_securities(force=True)


def get_security_info(security_code: str) -> Optional[SecurityInfo]:
    # ensure cache is fresh (will reload if stale)
    init_securities()
    code = correct_security_code(security_code)
    return _SECURITY_MAP.get(code)


__all__ = [
    "SecurityInfo",
    "init_securities",
    "refresh_securities",
    "get_security_info",
    "correct_security_code",
]


if __name__ == '__main__':
    # Minimal required test (as you requested): print security info for sh000001
    code = "sh000001"
    info = get_security_info(code)
    print(f"Security info for {code}: {info}")
    if info is not None:
        print(f"Name: {info.name}, Lot Size: {info.lot_size}, Price Precision: {info.price_precision}")
    else:
        print("No security info found for", code)