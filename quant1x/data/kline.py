# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import os
import pandas as pd
from dataclasses import dataclass, field
from typing import List, Optional, Any
import logging

from quant1x.exchange import Timestamp
from quant1x.level1 import protocol
from quant1x.level1.client import get_std_conn
from quant1x.level1.security_bars import SecurityBarsRequest, SecurityBarsResponse, KLineType, SecurityBar, SECURITY_BARS_MAX
# from quant1x.factors import base as factors  # 延迟导入以避免循环导入
from quant1x.level1.xdxr_info import XdxrInfo
import quant1x.data.xdxr as xdxr_module
from quant1x.config import config
from quant1x.data import adapter
from quant1x.data.adapter import DataAdapter, PLUGIN_MASK_BASE_DATA, register, DEFAULT_DATA_PROVIDER
from quant1x.data.base import BASE_KLINE

logger = logging.getLogger(__name__)

MAX_KLINE_LOOKBACK_DAYS = 1

@dataclass
class KLine:
    date: str = ""
    open: float = 0.0
    close: float = 0.0
    high: float = 0.0
    low: float = 0.0
    volume: float = 0.0
    amount: float = 0.0
    up: int = 0
    down: int = 0
    datetime: str = ""
    adjustment_count: int = 0

    def adjust(self, adj):
        # 延迟导入以避免循环导入
        from quant1x.factors import base as factors
        adj = factors.CumulativeAdjustment(
            timestamp=adj.timestamp,
            m=adj.m,
            a=adj.a,
            monetary_adjustment=adj.monetary_adjustment,
            share_adjustment_ratio=adj.share_adjustment_ratio,
            no=adj.no
        )
        
        self.open = self.open * adj.m + adj.a
        self.close = self.close * adj.m + adj.a
        self.high = self.high * adj.m + adj.a
        self.low = self.low * adj.m + adj.a
        
        # 成交量复权
        if self.volume != 0:
            # 1. 计算均价
            ap = self.amount / self.volume
            # 2. 均价复权
            ap_adjusted = ap * adj.m + adj.a
            # 3. 成交量复权
            self.volume *= (1 + adj.share_adjustment_ratio)
            # 4. 以新成交量*均价计算成交额
            self.amount = self.volume * ap_adjusted
        
        # 5. 更新除权除息次数
        self.adjustment_count = adj.no

    @staticmethod
    def headers() -> List[str]:
        return ["date", "open", "close", "high", "low", "volume", "amount", "up", "down", "datetime", "adjustment_count"]

def save_kline(filename: str, values: List[KLine]):
    if not values:
        return
        
    dirname = os.path.dirname(filename)
    if dirname:
        os.makedirs(dirname, exist_ok=True)
    
    data = [
        {
            "date": v.date,
            "open": v.open,
            "close": v.close,
            "high": v.high,
            "low": v.low,
            "volume": v.volume,
            "amount": v.amount,
            "up": v.up,
            "down": v.down,
            "datetime": v.datetime,
            "adjustment_count": v.adjustment_count
        }
        for v in values
    ]
    
    df = pd.DataFrame(data, columns=KLine.headers())
    df.to_csv(filename, index=False)

def read_kline_from_csv(filename: str) -> List[KLine]:
    klines = []
    if not os.path.exists(filename):
        return klines
        
    try:
        df = pd.read_csv(filename)
        # Ensure columns exist
        required_cols = KLine.headers()
        if not all(col in df.columns for col in required_cols):
            return klines
            
        for _, row in df.iterrows():
            kline = KLine(
                date=str(row['date']),
                open=float(row['open']),
                close=float(row['close']),
                high=float(row['high']),
                low=float(row['low']),
                volume=float(row['volume']),
                amount=float(row['amount']),
                up=int(row['up']),
                down=int(row['down']),
                datetime=str(row['datetime']),
                adjustment_count=int(row['adjustment_count'])
            )
            klines.append(kline)
    except Exception as e:
        logger.error(f"Failed to read kline csv {filename}: {e}")
        
    return klines

def load_kline(code: str) -> List[KLine]:
    filename = config.get_kline_filename(code)
    logger.debug(f"[dataset::KLine] kline file: {filename}")
    return read_kline_from_csv(filename)

def fetch_kline(code: str, start: int, count: int, kline_type: KLineType = KLineType.DAILY) -> List[SecurityBar]:
    try:
        with get_std_conn() as conn:
            req = SecurityBarsRequest(code, kline_type.value, start, count)
            # Determine is_index from request logic or pass it explicitly if needed
            # SecurityBarsRequest constructor determines is_index
            
            resp = SecurityBarsResponse(req.is_index, kline_type.value)
            protocol.process(conn, req, resp)
            return resp.list
    except Exception as e:
        logger.error(f"[dataset::KLine] fetch_kline error: {e}")
        return []

def apply_forward_adjustment_for_event(klines: List[KLine], 
                                       current_start_date: Timestamp, 
                                       dividends: List[XdxrInfo]):
    if not klines:
        return
        
    # 最后一根K线的日期
    last_day = klines[-1].date
    # 转成时间戳且对齐时间
    ts_last_day = Timestamp.parse(last_day).get_pre_market_time()
    # 计算最后一根K线的下一个交易日的日期
    # TODO: Implement next_trading_day properly. For now, use +1 day approximation or just rely on date comparison
    last_day_next = ts_last_day.offset(hour=24).only_date() # Approximation
    start_date_str = current_start_date.only_date()
    
    # Filter dividends
    xdxr_infos = [
        x for x in dividends 
        if x.Date <= last_day_next and x.Category == 1
    ]
    
    # Sort by date? C++ uses std::views::filter, order depends on input.
    # Assuming dividends are sorted by date.
    
    times = len(xdxr_infos)
    
    for info in xdxr_infos:
        if info.Date <= start_date_str:
            # IPO check logic in C++ is commented out or specific?
            # "除权除息数据在日线第一条数据之前... continue" is commented out in C++.
            pass
        else:
            m, a = info.adjust_factor()
            share_ratio = info.compute_share_adjustment_ratio()
            
            for kline in klines:
                if kline.date >= info.Date:
                    break
                
                if kline.date < info.Date:
                    kline.open = kline.open * m + a
                    kline.close = kline.close * m + a
                    kline.high = kline.high * m + a
                    kline.low = kline.low * m + a
                    
                    if kline.volume != 0:
                        ap = kline.amount / kline.volume
                        ap_adjusted = ap * m + a
                        kline.volume *= (1 + share_ratio)
                        kline.amount = kline.volume * ap_adjusted
                    
                    kline.adjustment_count += 1
        
        times -= 1

class DataKLine(DataAdapter):
    def kind(self) -> int:
        return BASE_KLINE
        
    def owner(self) -> str:
        return DEFAULT_DATA_PROVIDER
        
    def key(self) -> str:
        return "kline"
        
    def name(self) -> str:
        return "前复权K线"
        
    def usage(self) -> str:
        return "前复权K线数据"
        
    def print(self, code: str, dates: Optional[List[Timestamp]] = None) -> None:
        pass
        
    def update(self, code: str, date: Optional[Timestamp] = None) -> None:
        # 1. Determine start date from local cache
        current_start_date = Timestamp.parse("1990-12-19") # market_first_date
        cache_filename = config.get_kline_filename(code)
        cache_klines = read_kline_from_csv(cache_filename)
        
        klines_length = len(cache_klines)
        klines_offset_days = MAX_KLINE_LOOKBACK_DAYS
        adjust_times = 0
        
        if klines_length > 0:
            if klines_offset_days > klines_length:
                klines_offset_days = klines_length
            
            kline = cache_klines[klines_length - klines_offset_days]
            current_start_date = Timestamp.parse(kline.date)
            adjust_times = kline.adjustment_count
            
        # 2. Determine end date
        current_end_date = Timestamp.now().get_pre_market_time()
        logger.debug(f"[dataset::KLine] [{code}]: from {current_start_date.only_date()} to {current_end_date.only_date()}")
        
        step = SECURITY_BARS_MAX
        start = 0
        hs: List[List[SecurityBar]] = []
        element_count = 0
        
        while True:
            count = step
            reply = fetch_kline(code, start, count)
            if not reply:
                break
                
            element_count += len(reply)
            hs.append(reply)
            
            last_bar = reply[-1]
            last_bar_date = Timestamp.parse(f"{last_bar.Year}-{last_bar.Month:02d}-{last_bar.Day:02d}").get_pre_market_time()
            
            if last_bar_date < current_start_date:
                break
                
            if len(reply) < count:
                break
                
            start += count
            
        hs.reverse()
        
        incremental_klines: List[KLine] = []
        
        for vec in hs:
            for row in vec:
                date_time = Timestamp.parse(f"{row.Year}-{row.Month:02d}-{row.Day:02d}").get_pre_market_time()
                
                if date_time < current_start_date or date_time > current_end_date:
                    continue
                    
                kx = KLine(
                    date=date_time.only_date(),
                    open=row.Open,
                    close=row.Close,
                    high=row.High,
                    low=row.Low,
                    volume=row.Vol * 100, # Convert to shares
                    amount=row.Amount,
                    up=row.UpCount,
                    down=row.DownCount,
                    datetime=row.DateTime,
                    adjustment_count=0
                )
                incremental_klines.append(kx)
                
        # 6. Adjustment logic
        is_fresh_fetch_require_adjustment = (adjust_times == 1)
        dividends = xdxr_module.load_xdxr(code)
        
        if is_fresh_fetch_require_adjustment:
            apply_forward_adjustment_for_event(incremental_klines, current_start_date, dividends)
            
        # 7. Merge
        klines = []
        if klines_length > klines_offset_days:
            klines.extend(cache_klines[:klines_length - klines_offset_days])
            
        klines.extend(incremental_klines)
        
        # 8. Forward adjust
        if not is_fresh_fetch_require_adjustment:
            apply_forward_adjustment_for_event(klines, current_start_date, dividends)
            
        # 9. Save
        save_kline(cache_filename, klines)


# 注册插件
_data_kline_plugin = adapter.register(DataKLine)

