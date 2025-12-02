# -*- coding: utf-8 -*-
import os
import time
from functools import lru_cache
from .. import config, timestamp
import numpy as np
import pandas as pd

exchange_start_time = '09:15:00'
exchange_end_time = '15:00:00'
# time_range = "09:15:00~11:30:00,13:00:00~15:00:00"
trade_session = timestamp.TimeRange(f'{exchange_start_time}~{exchange_end_time}')


@lru_cache(maxsize=None)
def __calendar() -> pd.Series:
    """
    交易日历
    """
    fn = os.path.join(config.meta_path, "calendar")
    df = pd.read_csv(fn)
    return df['date']


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
    date = time.strftime(timestamp.FORMAT_ONLY_DATE)
    return date


def is_session_pre() -> bool:
    """
    是否盘前
    """
    now = time.strftime(timestamp.FORMAT_ONLY_TIME)
    return now < exchange_start_time


def is_session_reg() -> bool:
    """
    是否盘中
    """
    now = time.strftime(timestamp.FORMAT_ONLY_TIME)
    return trade_session.is_trading(now)


def is_session_post() -> bool:
    """
    是否盘后
    """
    now = time.strftime(timestamp.FORMAT_ONLY_TIME)
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


if __name__ == '__main__':
    a1 = last_trade_date()
    print(type(a1))
    print(a1)
    d1 = last_trade_date()
    print(d1)
    print(front_trade_date())
    print(next_trade_date())
