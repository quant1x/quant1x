# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations
import os, csv, threading
from typing import Optional, List, Tuple

from quant1x.config import config
from quant1x.data import adapter
from quant1x.data.adapter import DataAdapter, DEFAULT_DATA_PROVIDER
from quant1x.data.base import BASEDATA_TRANSACTION
from .client import get_std_conn
from . import protocol
from .level1 import (
    TICK_TRANSACTION_PER_REQUEST_MAX,
    Transaction, HistoricalTransaction
)
from quant1x.data.schema import Transaction
from quant1x.data.meta.calendar import last_trading_day
from quant1x.data.meta import Timestamp, Instrument, InstrumentType
from quant1x.data.market import detect_symbol
from quant1x.log import logger

# Constants
_trains_begin_date = "2024-10-01"
_historical_transaction_data_first_time = "09:25"
_historical_transaction_data_start_time = "09:30"
_historical_transaction_data_final_bidding_time = "14:57"
_historical_transaction_data_last_time = "15:00"

# Global state for historical trading data begin date
_historical_trading_data_once = False
_historical_trading_data_mutex = threading.Lock()
_historical_trading_data_begin_date = Timestamp.parse(_trains_begin_date)

def lazy_init_historical_trading_data():
    """
    初始化历史交易数据的开始日期.

    该函数将全局变量 _historical_trading_data_begin_date 设置为从 trains_begin_date 解析得到的时间戳.

    Note:
        这是一个惰性初始化函数, 只在首次调用时执行初始化.
    """
    global _historical_trading_data_begin_date
    _historical_trading_data_begin_date = Timestamp.parse(_trains_begin_date)

def get_begin_date_of_historical_trading_data() -> Timestamp:
    """
    获取历史交易数据的起始日期.

    该函数使用双重检查锁模式确保历史交易数据只被初始化一次,
    并返回初始化后的起始日期.

    Returns:
        Timestamp: 历史交易数据的起始日期.
    """
    global _historical_trading_data_once
    if not _historical_trading_data_once:
        with _historical_trading_data_mutex:
            if not _historical_trading_data_once:
                lazy_init_historical_trading_data()
                _historical_trading_data_once = True
    return _historical_trading_data_begin_date

def get_historical_trade_filename(inst: Instrument, date: str) -> str:
    """
    获取历史成交记录文件路径.
    目录结构: ${trans}/${YYYY}/${YYYYMMDD}/${SecurityCode}.csv
    """
    date_str = date.replace('-', '').replace('/', '')
    year = date_str[:4]
    base_path = os.path.join(config.data_path, 'trans', inst.cache_dir())
    code = inst.symbol()
    return os.path.join(base_path, year, date_str, f"{code}.csv")

def load_transaction_data_from_cache(inst: Instrument, feature_date: Timestamp, ignore_previous_data: bool) -> Tuple[List[Transaction], str]:
    """
    从缓存文件加载指定证券代码在特定日期的逐笔交易数据.

    Args:
        corrected_code (str): 证券代码(已校正).
        feature_date (Timestamp): 查询日期.
        ignore_previous_data (bool): 是否忽略历史数据.

    Returns:
        Tuple[List[TickTransaction], str]:
            返回两个值:
            1. 交易数据列表, 每个元素为TickTransaction对象.
            2. 数据起始时间字符串(用于增量更新).

    Raises:
        不直接抛出异常, 但会在日志中记录错误信息.
    """
    trade_date = feature_date.cache_date()
    corrected_code = inst.symbol()
    if ignore_previous_data:
        start_date = get_begin_date_of_historical_trading_data()
        if feature_date.cache_date() < start_date.cache_date():
            logger.error(f"[dataset::trans] code={corrected_code}, trade-date={trade_date}, start-date={start_date.to_string()}, 没有数据")
            return [], _historical_transaction_data_first_time

    start_time = _historical_transaction_data_first_time
    filename = get_historical_trade_filename(inst, feature_date.only_date())

    data_list = []

    if os.path.exists(filename):
        try:
            with open(filename, 'r', encoding='utf-8') as f:
                reader = csv.DictReader(f)
                # Expected headers: time, price, volume, num, amount, direction
                for row in reader:
                    t = Transaction(
                        time=row['time'],
                        price=float(row['price']),
                        volume=int(row['volume']),
                        num=int(row['num']),
                        amount=float(row['amount']),
                        direction=int(row['direction'])
                    )
                    data_list.append(t)

            if data_list:
                last_time = data_list[-1].time
                if last_time == _historical_transaction_data_last_time:
                    return data_list, start_time

                first_time = ""
                skip_count = 0
                cache_length = len(data_list)
                for i in range(1, cache_length + 1):
                    tm = data_list[cache_length - i].time
                    if not first_time:
                        first_time = tm
                        start_time = first_time
                        skip_count += 1
                        continue

                    if tm < first_time:
                        start_time = first_time
                        break
                    else:
                        skip_count += 1

                if skip_count > 0:
                    data_list = data_list[:-skip_count]

        except Exception:
            logger.exception(f"[dataset::trans] read cache failed")

    return data_list, start_time

def update_transaction_data(inst: Instrument, feature_date: Timestamp, start_time: str):
    """
    更新指定证券代码在特定日期的交易数据, 并将其保存到CSV文件中.

    Args:
        inst (Instrument): 证券 instruments.
        feature_date (Timestamp): 交易日期.
        start_time (str): 开始时间(格式为HH:MM:SS).

    Raises:
        Exception: 当网络请求或文件操作失败时记录错误日志.
    """
    trade_date_str = feature_date.cache_date()
    trade_date_int = int(trade_date_str)

    # Check if today is last trading day
    today_is_last_trading_date = feature_date.is_same_date(last_trading_day())

    offset = TICK_TRANSACTION_PER_REQUEST_MAX
    start = 0
    history: List[Transaction] = []
    hs: List[List[Transaction]] = []

    exchange = inst.exchange
    code = inst.ticker
    price_precision = inst.price_precision
    print(f"price_precision: {price_precision}")
    is_index = inst.type.is_index()

    conn = get_std_conn()
    while True:
        try:
            if today_is_last_trading_date:
                msg = Transaction(exchange, code, start, offset, price_precision, is_index)
            else:
                msg = HistoricalTransaction(exchange, code, trade_date_int, start, offset, price_precision, is_index)
            protocol.process_level1_new(conn, msg)

            if msg.count == 0 or not msg.list:
                break

            tmp = []
            tmp_list = list(msg.list)
            tmp_list.reverse()

            for td in tmp_list:
                if td.time >= start_time:
                    tmp.append(td)

            tmp.reverse()
            hs.append(tmp)

            if len(tmp) < offset:
                break

            start += offset
        except Exception:
            logger.exception(f"[dataset::trans] code={inst.symbol()}, tradeDate={trade_date_str}")
            break
    # Reverse hs
    hs.reverse()
    for v in hs:
        history.extend(v)

    if not history:
        return

    # Load existing and merge
    #corrected_code = inst.symbol()
    existing_list, _ = load_transaction_data_from_cache(inst, feature_date, False)
    existing_list.extend(history)

    # Write to CSV
    filename = get_historical_trade_filename(inst, feature_date.only_date())
    tmp_filename = filename + ".tmp"

    try:
        os.makedirs(os.path.dirname(filename), exist_ok=True)
        with open(tmp_filename, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(["time", "price", "volume", "num", "amount", "direction"])
            for rec in existing_list:
                writer.writerow([rec.time, rec.price, rec.volume, rec.num, rec.amount, rec.direction])

        os.replace(tmp_filename, filename)
    except Exception:
        logger.exception(f"[dataset::trans] rename failed: {tmp_filename} -> {filename}")
        if os.path.exists(tmp_filename):
            os.remove(tmp_filename)

def ensure_transaction_data_updated(inst: Instrument, feature_date: Timestamp, ignore_previous_data: bool):
    """
    确保指定证券代码在特定日期的交易数据是最新的.

    Args:
        inst (Instrument): 证券代码(已修正格式).
        feature_date (Timestamp): 需要检查的日期.
        ignore_previous_data (bool): 是否忽略之前缓存的数据.

    Raises:
        DataUpdateError: 当数据更新失败时抛出.
    """
    data_list, start_time = load_transaction_data_from_cache(inst, feature_date, ignore_previous_data)
    needs_update = not data_list or (data_list[-1].time != _historical_transaction_data_last_time)
    if needs_update:
        update_transaction_data(inst, feature_date, start_time)

class DataTrans(DataAdapter):
    def kind(self) -> int:
        return BASEDATA_TRANSACTION
        
    def owner(self):
        return DEFAULT_DATA_PROVIDER
        
    def key(self):
        return "trans"
        
    def name(self):
        return "历史成交"
        
    def usage(self):
        return "历史成交"
        
    def print(self, inst: Instrument, date: Optional[Timestamp] = None) -> None:
        pass
        
    def update(self, inst: Instrument, date: Optional[Timestamp] = None) -> None:
        if date is None:
            # 如果没有提供日期，使用当前日期或其他默认逻辑
            # 这里可能需要根据业务逻辑决定如何处理
            raise ValueError("Date is required for transaction data update")
        # corrected_code = inst.symbol()
        # ensure_transaction_data_updated(corrected_code, date, False)
        ensure_transaction_data_updated(inst, date, False)

# 注册插件
_data_trans_plugin = adapter.register(DataTrans)

def checkout_transaction_data(inst: Instrument, feature_date: Timestamp, ignore_previous_data: bool) -> List[Transaction]:
    """
    获取指定证券在特定日期的逐笔交易数据.

    Args:
        inst (Instrument): 证券 instrument.
        feature_date (Timestamp): 目标日期.
        ignore_previous_data (bool): 是否忽略已有缓存数据, 强制重新获取.

    Returns:
        List[transaction.TickTransaction]: 返回指定证券在目标日期的逐笔交易数据列表.

    Raises:
        ValueError: 如果证券代码无效或日期格式不正确.
        DataNotAvailableError: 如果请求的数据不可用.
    """
    if inst.is_valid() is False:
        raise ValueError(f"Invalid instrument: {inst.symbol()}")
    ensure_transaction_data_updated(inst, feature_date, ignore_previous_data)
    data_list, _ = load_transaction_data_from_cache(inst, feature_date, ignore_previous_data)
    return data_list

if __name__ == "__main__":
    # Example usage
    from .instruments import get_instrument_info
    code = "sh510050"
    date = Timestamp.parse("2023-01-04")
    inst = get_instrument_info(code)
    if inst is None:
        print(f"Instrument not found: {code}")
        exit(1)
    transactions = checkout_transaction_data(inst, date, False)
    #print(transactions)