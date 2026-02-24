# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import os
from datetime import datetime
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
        # 可能因权限、竞争条件（文件被删除）等导致 stat 失败
        return Timestamp.zero()


MaxCachedDaysToDropOnIncrementalUpdate = 1
"""
    是增量更新缓存清理的最大天数。
    该机制确保在 A 股除权除息日等场景下，当日数据能被正确覆盖。
    由于 A 股的复权处理以交易日为单位，且同一天内可能多次更新数据，
    因此需先删除缓存中已有的当日记录，再插入最新增量数据。
"""

def get_period_name(period: str = 'D') -> str:
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