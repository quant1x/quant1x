# -*- coding: UTF-8 -*-
import os
import pandas as pd
from dataclasses import dataclass, field
from typing import List, Optional, Any
import logging

from quant1x.exchange import Timestamp
from quant1x.level1 import protocol, client
from quant1x.level1.security_bars import SecurityBarsRequest, SecurityBarsResponse, KLineType, SecurityBar, SECURITY_BARS_MAX
from quant1x.factors import base as factors
from quant1x.level1.xdxr_info import XdxrInfo
from quant1x.datasets import xdxr
from quant1x.config import config

logger = logging.getLogger(__name__)

MAX_KLINE_LOOKBACK_DAYS = 1

@dataclass
class KLine:
    Date: str = ""
    Open: float = 0.0
    Close: float = 0.0
    High: float = 0.0
    Low: float = 0.0
    Volume: float = 0.0
    Amount: float = 0.0
    Up: int = 0
    Down: int = 0
    Datetime: str = ""
    AdjustmentCount: int = 0

    def adjust(self, adj: factors.CumulativeAdjustment):
        self.Open = self.Open * adj.m + adj.a
        self.Close = self.Close * adj.m + adj.a
        self.High = self.High * adj.m + adj.a
        self.Low = self.Low * adj.m + adj.a
        
        # 成交量复权
        if self.Volume != 0:
            # 1. 计算均价
            ap = self.Amount / self.Volume
            # 2. 均价复权
            ap_adjusted = ap * adj.m + adj.a
            # 3. 成交量复权
            self.Volume *= (1 + adj.share_adjustment_ratio)
            # 4. 以新成交量*均价计算成交额
            self.Amount = self.Volume * ap_adjusted
        
        # 5. 更新除权除息次数
        self.AdjustmentCount = adj.no

    @staticmethod
    def headers() -> List[str]:
        return ["Date", "Open", "Close", "High", "Low", "Volume", "Amount", "Up", "Down", "Datetime", "AdjustmentCount"]

def save_kline(filename: str, values: List[KLine]):
    if not values:
        return
        
    dirname = os.path.dirname(filename)
    if dirname:
        os.makedirs(dirname, exist_ok=True)
    
    data = [
        {
            "Date": v.Date,
            "Open": v.Open,
            "Close": v.Close,
            "High": v.High,
            "Low": v.Low,
            "Volume": v.Volume,
            "Amount": v.Amount,
            "Up": v.Up,
            "Down": v.Down,
            "Datetime": v.Datetime,
            "AdjustmentCount": v.AdjustmentCount
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
                Date=str(row['Date']),
                Open=float(row['Open']),
                Close=float(row['Close']),
                High=float(row['High']),
                Low=float(row['Low']),
                Volume=float(row['Volume']),
                Amount=float(row['Amount']),
                Up=int(row['Up']),
                Down=int(row['Down']),
                Datetime=str(row['Datetime']),
                AdjustmentCount=int(row['AdjustmentCount'])
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
        with client.client() as conn:
            req = SecurityBarsRequest(code, kline_type.value, start, count)
            # Determine is_index from request logic or pass it explicitly if needed
            # SecurityBarsRequest constructor determines is_index
            
            resp = SecurityBarsResponse(req.is_index, kline_type.value)
            protocol.process(conn.socket, req, resp)
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
    last_day = klines[-1].Date
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
                if kline.Date >= info.Date:
                    break
                
                if kline.Date < info.Date:
                    kline.Open = kline.Open * m + a
                    kline.Close = kline.Close * m + a
                    kline.High = kline.High * m + a
                    kline.Low = kline.Low * m + a
                    
                    if kline.Volume != 0:
                        ap = kline.Amount / kline.Volume
                        ap_adjusted = ap * m + a
                        kline.Volume *= (1 + share_ratio)
                        kline.Amount = kline.Volume * ap_adjusted
                    
                    kline.AdjustmentCount += 1
        
        times -= 1

class DataKLine:
    def update(self, code: str, date: Timestamp):
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
            current_start_date = Timestamp.parse(kline.Date)
            adjust_times = kline.AdjustmentCount
            
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
                    Date=date_time.only_date(),
                    Open=row.Open,
                    Close=row.Close,
                    High=row.High,
                    Low=row.Low,
                    Volume=row.Vol * 100, # Convert to shares
                    Amount=row.Amount,
                    Up=row.UpCount,
                    Down=row.DownCount,
                    Datetime=row.DateTime,
                    AdjustmentCount=0
                )
                incremental_klines.append(kx)
                
        # 6. Adjustment logic
        is_fresh_fetch_require_adjustment = (adjust_times == 1)
        dividends = xdxr.load_xdxr(code)
        
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

