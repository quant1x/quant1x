# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import os
import time
from turtle import up
#from functools import lru_cache
import numpy as np
import pandas as pd
from typing import List, Optional
import bisect
import requests
import csv
from .sina.decoder import FinanceDecoder
from quant1x.runtime.once import RollingOnce
from quant1x.data import market, layout, cache, config
from quant1x.data.timestamp import Timestamp
from quant1x.std import filesystem as fs
from quant1x.log import logger


# 新浪财经交易日历URL
SINA_CALENDAR_URL = "https://finance.sina.com.cn/realstock/company/klc_td_sh.txt"
CALENDAR_MISSING_DATE = "1992-05-04"


# in-memory caches (match Go implementation)
globalCalendarsString = []
globalCalendarsTimestamp = []

calendarRollingOnce = RollingOnce(name='calendar_init', cron=market.cn_cron_expr_daily_init)

def get_calendar_filename() -> str:
    return os.path.join(config.meta_path, "calendar")

def get_calendar_marker_filename() -> str:
    return os.path.join(config.meta_path, "calendar.updated")


def __preprocess(text: str) -> str:
    """预处理JS-like响应文本（去除赋值、尾部分号和引号）"""
    s = text
    if "=" in s:
        pos = s.find("=")
        s = s[pos + 1:]
    if ";" in s:
        pos = s.find(";")
        s = s[:pos]
    s = s.replace('"', '')
    return s

def __decode(text: str) -> List[str]:
    """解码日历数据"""
    pre = __preprocess(text)
    decoder = FinanceDecoder(pre)
    raw = decoder.decode()

    dates = []
    if isinstance(raw, list):
        for item in raw:
            if isinstance(item, dict) and "date" in item:
                date = item["date"].strip()
                if date:
                    dates.append(date)
            elif isinstance(item, str):
                dates.append(item.strip())

    return dates if dates else []

def update_calendar():
    """下载交易日历数据并缓存到磁盘"""
    fn = get_calendar_filename()
    logger.warning(f"更新交易日历: 下载并缓存到 {fn}")
    try:
        # conditional GET using file mtime to avoid unnecessary downloads
        headers = {}
        if os.path.exists(fn):
            try:
                mtime = os.path.getmtime(fn)
                # format in RFC1123
                from email.utils import formatdate

                headers['If-Modified-Since'] = formatdate(mtime, usegmt=True)
            except Exception:
                pass

        resp = requests.get(SINA_CALENDAR_URL, timeout=15, headers=headers)
        if resp.status_code == 304:
            # 未修改，直接返回
            return
        resp.raise_for_status()

        body = resp.text
        dates = __decode(body)
        if not dates:
            raise ValueError("解码交易日历数据失败")

        # Ensure missing date present, insert preserving order (do not resort)
        idx = bisect.bisect_left(dates, CALENDAR_MISSING_DATE)
        if idx == len(dates) or dates[idx] != CALENDAR_MISSING_DATE:
            dates.insert(idx, CALENDAR_MISSING_DATE)

        # write CSV cache
        os.makedirs(os.path.dirname(fn), exist_ok=True)
        with open(fn, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(["date", "source"])
            for date in dates:
                writer.writerow([date, "sina"])

        # set file mtime: prefer HTTP Last-Modified when present; fall back to now.
        now_ts = time.time()
        lm = resp.headers.get('Last-Modified')
        if lm:
            try:
                from email.utils import parsedate_to_datetime

                dt = parsedate_to_datetime(lm)
                secs = int(dt.timestamp())
                os.utime(fn, (secs, secs))
            except Exception:
                try:
                    secs = int(now_ts)
                    os.utime(fn, (secs, secs))
                except Exception:
                    pass
        else:
            try:
                secs = int(now_ts)
                os.utime(fn, (secs, secs))
            except Exception:
                pass

        # update the calendar marker next to the cache so RollingOnce and
        # other processes can decide when the last successful update occurred.
        try:
            marker = get_calendar_marker_filename()
            os.makedirs(os.path.dirname(marker), exist_ok=True)
            # create or truncate marker file
            fs.update_file_mtime(marker, now_ts)
        except Exception:
            pass
    except Exception as e:
        # match Go behaviour: let caller decide; raise to caller
        raise RuntimeError(f"更新交易日历失败: {e}")


def lazy_load_calendar():
    marker = get_calendar_marker_filename()
    now_time = Timestamp.now()
    mod_time = cache.get_filename_modified_time(marker)
    today_init_time = now_time.get_pre_market_time()
    if now_time > today_init_time and mod_time < today_init_time:
        logger.debug("交易日历缓存文件过期，执行更新")
        try:
            update_calendar()
            fs.update_file_mtime(marker)
        except Exception:
            pass
    else:
        logger.debug("交易日历缓存文件未过期，跳过更新")
    logger.debug("加载交易日历缓存文件到内存")
    #ensure_updated = False
    fn = get_calendar_filename()
    if not os.path.exists(fn):
        logger.debug("交易日历缓存文件不存在，跳过加载")
        return

    try:
        with open(fn, newline='', encoding='utf-8') as f:
            reader = csv.reader(f)
            # skip header
            try:
                next(reader)
            except StopIteration:
                return
            ss = []
            ts = []
            for rec in reader:
                if not rec:
                    continue
                date = rec[0].strip()
                if not date:
                    continue
                ss.append(date)
                try:
                    t = Timestamp.parse(date)
                    ts.append(t.get_pre_market_time())
                except Exception:
                    pass
    except Exception:
        logger.error("加载交易日历缓存文件失败，跳过加载")
        return

    global globalCalendarsString, globalCalendarsTimestamp
    globalCalendarsString = ss
    globalCalendarsTimestamp = ts

#@lru_cache(maxsize=None)
def load_calendar() -> pd.Series:
    """
    交易日历
    """
    # Ensure in-memory cache is loaded using the persistent rolling once (do -> swallow exceptions)
    calendarRollingOnce.do(lazy_load_calendar)
    if not globalCalendarsString:
        raise RuntimeError("exchange calendar is empty")
    # return a pandas Series for compatibility
    return pd.Series(globalCalendarsString)

#@lru_cache(maxsize=None)
def __calendar_timestamps() -> List[Timestamp]:
    """
    交易日历 (Timestamp对象列表)
    """
    dates = load_calendar()
    # 转换为 Timestamp 对象，并设置为当天的盘前时间 (09:00:00)
    return [Timestamp.parse(d).get_pre_market_time() for d in dates]

def calendar() -> pd.Series:
    """
    获取全部的交易日期
    Returns:
        pd.Series
    """
    return load_calendar()


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
    date = time.strftime(layout.FORMAT_ONLY_DATE)
    return date

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
        
    trade_dates = load_calendar().tolist() # Convert Series to list for bisect
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
    a1 = last_trading_day()
    print(type(a1))
    print(a1)
