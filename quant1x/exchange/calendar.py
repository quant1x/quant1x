# -*- coding: utf-8 -*-
import os
import time
from functools import lru_cache
from .. import config
from . import session
from .timestamp import Timestamp
import numpy as np
import pandas as pd
from typing import List, Optional
import bisect

exchange_start_time = '09:15:00'
exchange_end_time = '15:00:00'
# time_range = "09:15:00~11:30:00,13:00:00~15:00:00"
trade_session = session.TimeRange(f'{exchange_start_time}~{exchange_end_time}')


@lru_cache(maxsize=None)
def __calendar() -> pd.Series:
    """
    交易日历
    """
    fn = os.path.join(config.meta_path, "calendar")
    df = pd.read_csv(fn)
    return df['date']

@lru_cache(maxsize=None)
def __calendar_timestamps() -> List[Timestamp]:
    """
    交易日历 (Timestamp对象列表)
    """
    dates = __calendar()
    # 转换为 Timestamp 对象，并设置为当天的盘前时间 (09:00:00)
    return [Timestamp.parse(d).get_pre_market_time() for d in dates]

def calendar() -> pd.Series:
    """
    获取全部的交易日期
    Returns:
        pd.Series
    """
    return __calendar()


def fix_trade_date(date_str: str, fmt: str = "%Y-%m-%d") -> str:
    """强制将日期字符串转换为指定格式

    参数:
        date_str: 输入日期字符串
        fmt: 目标格式（默认%Y-%m-%d）

    返回:
        统一格式的日期字符串

    示例:
        >>> fix_trade_date("2023/12/25")
        "2023-12-25"
    """
    from datetime import datetime
    return datetime.strptime(date_str, "%Y-%m-%d").strftime(fmt) if date_str else date_str


def get_today() -> str:
    """
    获取当前日期
    """
    date = time.strftime(session.FORMAT_ONLY_DATE)
    return date


def is_session_pre() -> bool:
    """
    是否盘前
    """
    now = time.strftime(session.FORMAT_ONLY_TIME)
    return now < exchange_start_time


def is_session_reg() -> bool:
    """
    是否盘中
    """
    now = time.strftime(session.FORMAT_ONLY_TIME)
    return trade_session.is_trading(now)


def is_session_post() -> bool:
    """
    是否盘后
    """
    now = time.strftime(session.FORMAT_ONLY_TIME)
    return now > exchange_end_time


def last_trade_date(base_date: str = None) -> str:
    """
    获取基准日期之前最近的一个交易日（若未指定基准日期则使用当天）

    参数:
        base_date: 基准日期字符串（格式：YYYY-MM-DD），可选

    返回:
        最近交易日的日期字符串（格式：YYYY-MM-DD）
    """
    calendar_series = __calendar()  # 获取交易日历
    ref_date = base_date if base_date is not None else get_today()  # 确定基准日期
    session_pre = trade_session.is_session_pre() if base_date is None else False  # 仅对当天检查盘前

    # 查找基准日期的位置
    idx = calendar_series.searchsorted(ref_date)
    if isinstance(idx, np.ndarray):  # 处理可能的数组返回
        idx = idx[0]

    # 获取候选日期
    date = calendar_series.iloc[idx]

    # 逻辑判断
    if (str(date) > ref_date) or (str(date) == ref_date and session_pre):
        date = calendar_series.iloc[idx - 1]

    # 确保返回字符串
    return str(date) if not isinstance(date, str) else date


def front_trade_date(n: int = 1, base_date: str = None) -> str:
    """获取基准日期前N个交易日

    参数:
        n: 向前追溯的交易日的数量（默认1）
        base_date: 基准日期（可选），格式为YYYY-MM-DD
    """
    dates = __calendar()
    ref_date = base_date if base_date is not None else last_trade_date()

    # 处理searchsorted返回值（兼容所有类型）
    idx = dates.searchsorted(ref_date)
    idx = idx[0] if hasattr(idx, '__iter__') else idx

    return str(dates.iat[max(0, int(idx) - n)])


def next_trade_date(base_date: str = None) -> str:
    """获取基准日期后的下一个交易日

    参数:
        base_date: 基准日期（可选），格式为YYYY-MM-DD
    """
    dates = __calendar()
    ref_date = base_date if base_date is not None else last_trade_date()

    # 使用searchsorted高效查找
    idx = dates.searchsorted(ref_date, side='right')
    idx = idx[0] if hasattr(idx, '__iter__') else idx

    return str(dates.iloc[min(int(idx), len(dates) - 1)])


def last_trading_day(date: Optional[Timestamp] = None, debug_timestamp: Optional[Timestamp] = None) -> Timestamp:
    """
    获取最近一个交易日 (Timestamp版本)
    """
    trade_dates = __calendar_timestamps()
    if not trade_dates:
        return Timestamp.zero()

    if date is None:
        # 默认使用今天
        date = Timestamp.now().get_pre_market_time()

    # 查找 date 在 trade_dates 中的位置 (upper_bound)
    # bisect_right 相当于 C++ 的 upper_bound
    idx = bisect.bisect_right(trade_dates, date)
    
    if idx > 0:
        idx -= 1
    
    # 判断是否盘前
    last_ts = trade_dates[idx]
    current_ts = debug_timestamp if debug_timestamp is not None else Timestamp.now()
    
    if current_ts < last_ts and idx > 0:
        idx -= 1
        
    return trade_dates[idx]


def prev_trading_day(date: Optional[Timestamp] = None, debug_timestamp: Optional[Timestamp] = None) -> Timestamp:
    """
    获取上一个交易日 (Timestamp版本)
    """
    trade_dates = __calendar_timestamps()
    if not trade_dates:
        return Timestamp.zero()

    if date is None:
        date = Timestamp.now().get_pre_market_time()

    # 查找 date 在 trade_dates 中的位置 (lower_bound)
    # bisect_left 相当于 C++ 的 lower_bound
    idx = bisect.bisect_left(trade_dates, date)
    
    if idx > 0:
        idx -= 1
        
    # 判断是否盘前
    last_ts = trade_dates[idx]
    current_ts = debug_timestamp if debug_timestamp is not None else Timestamp.now()
    
    if current_ts < last_ts and idx > 0:
        idx -= 1
        
    return trade_dates[idx]


def next_trading_day_ts(date: Optional[Timestamp] = None, debug_timestamp: Optional[Timestamp] = None) -> Timestamp:
    """
    获取下一个交易日 (Timestamp版本)
    """
    trade_dates = __calendar_timestamps()
    if not trade_dates:
        return Timestamp.zero()

    if date is None:
        date = Timestamp.now().get_pre_market_time()
        
    current_time = debug_timestamp if debug_timestamp is not None else Timestamp.now()
    
    # 找到第一个大于等于 date 的交易日 (lower_bound)
    idx = bisect.bisect_left(trade_dates, date)
    
    if idx >= len(trade_dates):
        if trade_dates:
            return trade_dates[-1]
        return Timestamp.zero()
        
    candidate_day = trade_dates[idx]
    
    # 如果当前时间已经过了候选交易日的盘前时间，则取下一个
    if current_time >= candidate_day and idx < len(trade_dates):
        idx += 1
        if idx >= len(trade_dates):
            return trade_dates[-1]
        return trade_dates[idx]
        
    return candidate_day


def date_range(begin: Timestamp, end: Optional[Timestamp] = None, skip_today: bool = False) -> List[Timestamp]:
    """
    获取日期范围 (Timestamp版本)
    """
    if end is None:
        end = Timestamp.now()
        
    if begin > end:
        return []
        
    trade_dates = __calendar_timestamps()
    if not trade_dates:
        return []
        
    # 查找范围边界
    lower = bisect.bisect_left(trade_dates, begin)
    upper = bisect.bisect_right(trade_dates, end)
    
    # 处理 skip_today 逻辑
    if skip_today and upper > 0:
        today = Timestamp.now().get_pre_market_time()
        last_in_range = trade_dates[upper - 1]
        if last_in_range > today or last_in_range > end:
            upper -= 1
    else:
        # 调整 upper 到最后一个 <= end 的日期
        # bisect_right 返回的是第一个 > end 的位置，所以 upper-1 就是 <= end 的位置
        # 这里不需要额外调整，切片 [lower:upper] 刚好包含 lower 到 upper-1
        pass
        
    if lower >= upper:
        return []
        
    return trade_dates[lower:upper]


def get_date_range(begin: str, end: str, skip_today: bool = False) -> List[str]:
    """
    获取日期范围 (字符串版本)
    """
    if begin > end:
        return []
        
    trade_dates = __calendar().tolist() # Convert Series to list for bisect
    if not trade_dates:
        return []
        
    # 查找起始索引
    it_start = bisect.bisect_left(trade_dates, begin)
    
    # 查找结束索引
    it_end = bisect.bisect_left(trade_dates, end)
    
    # bisect_left 返回的是第一个 >= end 的位置
    # 如果 trade_dates[it_end] == end, 我们需要包含它，所以切片应该是 it_end + 1
    # 如果 trade_dates[it_end] > end, 我们不需要包含它，切片应该是 it_end
    
    # 为了匹配 C++ lower_bound 逻辑:
    # C++: lower_bound(begin), lower_bound(end)
    # range: [itStart, itEnd) if *itEnd >= end? No, C++ logic is complex.
    # Let's simplify: find all dates d such that begin <= d <= end
    
    # Re-implement using simple filtering or bisect_right for end
    
    it_start = bisect.bisect_left(trade_dates, begin)
    it_end = bisect.bisect_right(trade_dates, end) # first element > end
    
    if skip_today:
        if it_end > 0:
            today_str = get_today()
            last_day = trade_dates[it_end - 1]
            if last_day > today_str or last_day > end:
                it_end -= 1
                
    if it_start >= it_end:
        return []
        
    return trade_dates[it_start:it_end]


if __name__ == '__main__':
    a1 = last_trade_date()
    print(type(a1))
    print(a1)
    d1 = last_trade_date()
    print(d1)
    print(front_trade_date())
    print(next_trade_date())
