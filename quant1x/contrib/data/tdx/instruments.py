# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations
import os, csv
from typing import Optional, List

from quant1x.data import config, market, status
from quant1x.data.market import Exchange, Instrument, InstrumentType
from quant1x.runtime.once import RollingOnce
from quant1x.log import logger
from . import config as tdx_config, client, protocol
from .level1 import SecurityListRequest, SecurityListResponse, SECURITY_LIST_PRE_REQUEST_MAX

# in-memory cache and synchronization
_SECURITY_MAP = {}
_SECURITY_ONCE = RollingOnce(name="tdx_security_init", cron=tdx_config.cron_expr_server_init)

def _get_security_filename() -> str:
    return os.path.join(config.meta_path, "securities.csv")


def _load_securities() -> bool:
    global _SECURITY_MAP
    fname = _get_security_filename()
    logger.debug(f"Loading securities from {fname}")
    _SECURITY_MAP.clear()
    # attempt to load CSV into memory
    try:
        with open(fname, newline='', encoding='utf-8') as fh:
            reader = csv.DictReader(fh)
            for row in reader:
                tmp = row.get('exchange') or 'sh'
                exchange = Exchange(tmp)  # 假设 Exchange 是 Enum
                tmp = row.get('type') or 'unknown'
                type = InstrumentType.from_string(tmp)
                code = row.get('code') or ''
                name = row.get('name') or ''
                lot_size = row.get('lot_size') or 100
                price_precision = row.get('price_precision') or 2
                inst = Instrument(exchange=exchange, type=type, ticker=code, name=name, lot_size=lot_size, price_precision=price_precision)
                symbol = str(inst)
                _SECURITY_MAP[symbol] = inst
    except FileNotFoundError:
        # file not present: leave map empty but record load time to avoid hot-loop
        return False
    except Exception:
        # ignore parse errors; don't raise to callers
        return False
    if len(_SECURITY_MAP) > 0:
        return True
    return False

def fetch_security_list(exchange: market.Exchange, start: int, count: int) -> List[Instrument]:
    """从 level1 服务器获取一页 SECURITY_LIST。

    返回一个字典列表，字典包含字段：`Code`（6 字符字符串）、`VolUnit`（整数）、
    `DecimalPoint`（整数）、`Name`（字符串）、`PreClose`（浮点）。出现错误时返回 `None`。
    """
    try:
        conn = client.get_std_conn()
        req = SecurityListRequest(exchange, start, count)
        resp = SecurityListResponse(exchange)
        protocol.process(conn, req, resp)
        return resp.list
    except Exception:
        logger.exception('fetch_security_list failed')
        return []
    
def init_securities():
    global _SECURITY_MAP
    fname = _get_security_filename()
    ensure_updated = status.should_initialize_file(fname)
    if not ensure_updated:
        ensure_updated = _load_securities() is False
    logger.debug(f"init_securities ensure_updated={ensure_updated}")
    if ensure_updated:
        instruments: List[Instrument] = []
        markets = [Exchange.SSE, Exchange.SZSE, Exchange.BSE]
        for m in markets:
            start = 0
            rows = []
            while True:
                try:
                    page = fetch_security_list(m, start, SECURITY_LIST_PRE_REQUEST_MAX)
                except Exception:
                    logger.exception('fetch_security_list failed')
                    page = None

                if page is None:
                    break
                if not page:
                    break
                rows.extend(page)
                if len(page) < SECURITY_LIST_PRE_REQUEST_MAX:
                    break
                start += SECURITY_LIST_PRE_REQUEST_MAX
            # 相同市场按照代码排序
            rows.sort(key=lambda x: x.ticker)
            # 合并市场
            instruments.extend(rows)
        # write CSV if we have instruments
        if instruments:
            try:
                os.makedirs(os.path.dirname(fname), exist_ok=True)
                with open(fname, 'w', newline='', encoding='utf-8') as fh:
                    writer = csv.writer(fh)
                    writer.writerow(['exchange','type','code','name','lot_size','price_precision'])
                    for r in instruments:
                        writer.writerow([
                            r.exchange.value,      # 假设 Exchange 是 Enum
                            r.type,          # 假设 InstrumentType 是 Enum
                            r.ticker,
                            r.name,
                            r.lot_size,
                            r.price_precision
                        ])
            except Exception:
                pass
        _ = _load_securities()


def get_instrument_info(security_code: str) -> Optional[Instrument]:
    _SECURITY_ONCE.do(init_securities)
    code = market.correct_security_code(security_code)
    return _SECURITY_MAP.get(code)


__all__ = [
    "get_instrument_info",
]


if __name__ == '__main__':
    # Minimal required test (as you requested): print security info for sh000001
    code = "000001.SH"
    info = get_instrument_info(code)
    print(f"Security info for {code}: {info}")
    if info is not None:
        print(f"Name: {info.name}, Lot Size: {info.lot_size}, Price Precision: {info.price_precision}")
    else:
        print("No security info found for", code)
