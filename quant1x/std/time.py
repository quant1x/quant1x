# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import time
from datetime import datetime, timezone
from typing import Tuple
import pandas as pd

def to_rfc1123(dt=None):
    """
    将 datetime 对象转换为 RFC1123 格式的字符串
    
    Args:
        dt (datetime, optional): 要转换的 datetime 对象。如果为None，则使用当前时间
        
    Returns:
        str: RFC1123 格式的日期时间字符串，如 "Mon, 16 Feb 2026 08:00:00 GMT"
    """
    if dt is None:
        timestamp = time.time()
    else:
        timestamp = dt.timestamp()
    return time.strftime("%a, %d %b %Y %H:%M:%S GMT", time.gmtime(timestamp))

def from_rfc1123(rfc1123_str):
    """
    将 RFC1123 格式的字符串转换为 datetime 对象
    
    Args:
        rfc1123_str (str): RFC1123 格式的时间字符串，例如 "Mon, 16 Feb 2026 07:55:26 GMT"
    
    Returns:
        datetime: 转换后的 datetime 对象，带有时区信息 (UTC)
    
    Raises:
        ValueError: 如果输入字符串不符合 RFC1123 格式
    """
    import calendar
    time_tuple = time.strptime(rfc1123_str, "%a, %d %b %Y %H:%M:%S %Z")
    timestamp = calendar.timegm(time_tuple)
    return datetime.fromtimestamp(timestamp, tz=timezone.utc)

def get_quarter_by_date(date_str: str, diff_quarters: int = 0) -> Tuple[str, str, str]:
    """
    根据日期获取季度信息，支持季度偏移计算
    
    Args:
        date_str (str): 日期字符串，格式应为可被解析的日期格式
        diff_quarters (int, optional): 季度偏移量，默认为0表示当前季度，正数表示未来季度，负数表示过去季度
    
    Returns:
        Tuple[str, str, str]: 返回包含季度字符串、季度第一天和最后一天的元组，格式为("YYYYQN", "YYYY-MM-DD", "YYYY-MM-DD")
    
    Raises:
        ValueError: 如果date_str无法被解析为有效日期，将使用当前日期代替
    """
    try:
        dt = pd.to_datetime(date_str)
    except:
        dt = datetime.now()
    
    # Calculate total months and subtract
    total_months = dt.year * 12 + dt.month - 1
    target_months = total_months - (3 * diff_quarters)
    
    year = target_months // 12
    month = (target_months % 12) + 1
    
    if 1 <= month <= 3:
        quarter = f"{year}Q1"
        first_day = f"{year}-01-01"
        last_day = f"{year}-03-31"
    elif 4 <= month <= 6:
        quarter = f"{year}Q2"
        first_day = f"{year}-04-01"
        last_day = f"{year}-06-30"
    elif 7 <= month <= 9:
        quarter = f"{year}Q3"
        first_day = f"{year}-07-01"
        last_day = f"{year}-09-30"
    else:
        quarter = f"{year}Q4"
        first_day = f"{year}-10-01"
        last_day = f"{year}-12-31"
        
    return quarter, first_day, last_day
