# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import os
from datetime import datetime
from dateutil import parser
import pandas as pd
from .meta import Timestamp

def get_today_initialized_time() -> Timestamp:
    """获取今天初始化时间"""
    now = Timestamp.now()
    return now.get_pre_market_time()

def get_filename_modified_time(fname: str) -> Timestamp:
    """获取文件最后修改时间"""
    if not os.path.exists(fname):
        return Timestamp.zero()
    try:
        info = os.lstat(fname)
        dt = datetime.fromtimestamp(info.st_mtime)
        return Timestamp.from_datetime(dt)
    except OSError:
        # 可能因权限, 竞争条件(文件被删除)等导致 stat 失败
        return Timestamp.zero()


MaxCachedDaysToDropOnIncrementalUpdate = 1
"""
    是增量更新缓存清理的最大天数. 
    该机制确保在 A 股除权除息日等场景下, 当日数据能被正确覆盖. 
    由于 A 股的复权处理以交易日为单位, 且同一天内可能多次更新数据, 
    因此需先删除缓存中已有的当日记录, 再插入最新增量数据. 
"""

_default_bar_period = 'D'

def get_period_name(period: str = _default_bar_period) -> str:
    """
    根据周期标识返回中文名称

    Parameters:
    period (str): 周期标识 'W', 'M', 'Q', 'Y'

    Returns:
    str: 中文周期名称
    """
    period_names = {
        'W': '周',
        'M': '月',
        'Q': '季',
        'Y': '年',
        'D': '日'
    }
    period = period.upper()
    return period_names.get(period, period)

def convert_bars_trading(bars, period='D'):
    """
    基于实际交易日的K线转换函数

    Parameters:
    bars (pd.DataFrame): 日线数据
    period (str): 目标周期
        'W' - 周线
        'M' - 月线
        'Q' - 季度线
        'Y' - 年线

    Returns:
    pd.DataFrame: 转换后的K线数据, date字段表示实际交易日
    """
    if bars.empty:
        return bars.copy()

    df = bars.copy()
    df['date'] = pd.to_datetime(df['date'])
    df = df.sort_values('date').reset_index(drop=True)

    # 直接使用简化的周期标识
    period = period.upper()
    if period not in ['W', 'M', 'Q', 'Y']:
        return df

    # 根据周期分组
    groups = df['date'].dt.to_period(period)

    # 聚合数据, date字段保留实际的交易日
    result = df.groupby(groups).agg({
        'date': 'last',  # 实际最后一个交易日
        'open': 'first',
        'high': 'max',
        'low': 'min',
        'close': 'last',
        'volume': 'sum',
        'amount': 'sum'
    }).reset_index(drop=True)

    return result

def date_format(date: str, layout: str = '%Y-%m-%d') -> str:
    dt = parser.parse(date)  # 自动识别各种常见日期格式
    return dt.strftime(layout)


if __name__ == "__main__":
    # 测试日期格式化函数
    test_dates = [
        "2024-06-01",
        "2024/06/01",
        "June 1, 2024",
        "2024.06.01",
    ]
    for d in test_dates:
        print(f"Original: {d} -> Formatted: {date_format(d)}")