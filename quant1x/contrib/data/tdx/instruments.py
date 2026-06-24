# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations
from math import log
import os, csv
from typing import Optional, List

from quant1x.config import config
from quant1x.contrib.data.tdx.level1 import ext
from quant1x.data import status
from quant1x.data import market
from quant1x.data.meta import Exchange, Instrument, InstrumentType
from quant1x.runtime.once import RollingOnce
from quant1x.log import logger
from . import client, config as tdx_config, protocol
from .level1 import SecurityListContext, SECURITY_LIST_PRE_REQUEST_MAX


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
                tmp = row.get('exchange') or 'unknown'
                exchange = Exchange.parse(tmp)  # 假设 Exchange 是 Enum
                tmp = row.get('type') or 'unknown'
                type = InstrumentType.from_string(tmp)
                code = row.get('code') or ''
                name = row.get('name') or ''
                lot_size = int(row.get('lot_size') or '100')
                price_precision = int(row.get('price_precision') or '2')
                tmp = row.get('ext_market') or ''
                ext_market = int(tmp) if tmp.isdigit() else 0
                tmp = row.get('ext_category') or ''
                ext_category = int(tmp) if tmp.isdigit() else 0
                tmp = row.get('alias_ticker') or ''
                alias_ticker = tmp
                
                code = code.lower()
                inst = Instrument(exchange=exchange,
                                  type=type,
                                  ticker=code,
                                  name=name,
                                  lot_size=lot_size,
                                  price_precision=price_precision,
                                  ext_market=ext_market,
                                  ext_category=ext_category,
                                  alias_ticker=alias_ticker)
                symbol = inst.symbol()
                # if code == 'hsi':
                #     print(f"{symbol} -> {inst}")
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

def fetch_security_list(exchange: Exchange, start: int, count: int) -> List[Instrument]:
    """从 level1 服务器获取一页 SECURITY_LIST.

    返回一个字典列表, 字典包含字段: `Code`(6 字符字符串), `VolUnit`(整数),
    `DecimalPoint`(整数), `Name`(字符串), `PreClose`(浮点). 出现错误时返回 `None`.
    """
    try:
        conn = client.get_std_conn()
        msg = SecurityListContext(exchange, start, count)
        protocol.transact_message_sync(conn, msg)
        return msg.list
    except Exception:
        logger.exception('fetch_security_list failed')
        return []
    
def init_securities():
    global _SECURITY_MAP
    fname = _get_security_filename()
    create_or_update = status.should_initialize_file(fname)
    if not create_or_update:
        create_or_update = _load_securities() is False
    logger.debug(f"init_securities create_or_update={create_or_update}")
    if create_or_update:
        instruments: List[Instrument] = []
        # 1. 标准行情: A股
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
        # 2. 扩展行情: 港股等
        from .level1.ext import InstrumentInfo
        markets = [Exchange.HKEX]
        offset = InstrumentInfo.PRE_REQUEST_MAX
        for m in markets:
            start = 0
            rows = []
            conn = client.get_ext_conn()
            while True:
                page = []
                fetch_count = 0
                try:
                    ii = InstrumentInfo(start, offset)
                    protocol.transact_message_sync(conn, ii)
                    fetch_count = ii.reply.get('count', 0)
                    if ii.reply['count'] > 0:
                        page = ii.reply['list']
                except Exception:
                    logger.exception('fetch_security_list failed')
                    break
                rows.extend(page)
                if fetch_count < offset:
                    break
                start += offset
            # 相同市场按照代码排序
            rows.sort(key=lambda x: (
                (x.ext_market is None, x.ext_market),
                (x.ext_category is None, x.ext_category),
                (x.ticker is None, x.ticker or '')
                )
            )
            logger.debug(f"init_securities rows[ext]={rows}")
            # 合并市场
            instruments.extend(rows)
        
        # write CSV if we have instruments
        if instruments:
            try:
                os.makedirs(os.path.dirname(fname), exist_ok=True)
                with open(fname, 'w', newline='', encoding='utf-8') as fh:
                    writer = csv.writer(fh)
                    headers = Instrument.headers()
                    writer.writerow(headers)
                    for r in instruments:
                        # writer.writerow([
                        #     r.exchange.identifier,      # 假设 Exchange 是 Enum
                        #     r.type,          # 假设 InstrumentType 是 Enum
                        #     r.ticker,
                        #     r.name,
                        #     r.lot_size,
                        #     r.price_precision,
                        #     r.ext_market,
                        #     r.ext_category
                        # ])
                        writer.writerow(r.to_iterable())
            except Exception:
                pass
        _ = _load_securities()


def get_instrument_info(symbol: str) -> Optional[Instrument]:
    _SECURITY_ONCE.do(init_securities)
    security_code = market.correct_security_code(symbol)
    logger.debug(f"get_instrument_info: symbol={symbol}, security_code={security_code}")
    return _SECURITY_MAP.get(security_code)


__all__ = [
    "get_instrument_info",
]


if __name__ == '__main__':
    # Minimal required test (as you requested): print security info for sh000001
    code = "sz000737"
    code = "hsi.hk"
    code = 'ixic.us'
    code = 'US0487.us'
    code = 'aapl.us'
    code = '880005'
    info = get_instrument_info(code)
    print(f"Security info for {code}: {info}")
    if info is not None:
        print(f"Name: {info.name}, Lot Size: {info.lot_size}, Price Precision: {info.price_precision}, ext_market: {info.ext_market}, ext_category: {info.ext_category}, alias_ticker: {info.alias_ticker}")
    else:
        print("No security info found for", code)
