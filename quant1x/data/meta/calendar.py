# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import os, time, csv, bisect, requests
import pandas as pd
from typing import List, Optional

from quant1x.std import ThreadSafeSingletonABC, ThreadSafeStrategy
from quant1x.runtime.once import RollingOnce
from quant1x.config import config
from quant1x.std import filesystem as fs
from quant1x.log import logger

from quant1x.data import cache
from .sina.decoder import FinanceDecoder

from . import layout, Timestamp, tradinghours

# 新浪财经交易日历URL
SINA_CALENDAR_URL = "https://finance.sina.com.cn/realstock/company/klc_td_sh.txt"
CALENDAR_MISSING_DATE = "1992-05-04"


# in-memory caches (match Go implementation)
globalCalendarsString = []
globalCalendarsTimestamp = []

calendarRollingOnce = RollingOnce(name='calendar_init', cron=tradinghours.cn_cron_expr_daily_init)

def _get_calendar_filename() -> str:
    return os.path.join(config.meta_path, "calendar")

def _get_calendar_marker_filename() -> str:
    return os.path.join(config.meta_path, "calendar.updated")


def _preprocess_sina_text(text: str) -> str:
    """预处理JS-like响应文本(去除赋值, 尾部分号和引号)"""
    s = text
    if "=" in s:
        pos = s.find("=")
        s = s[pos + 1:]
    if ";" in s:
        pos = s.find(";")
        s = s[:pos]
    s = s.replace('"', '')
    return s

def _decode_sina_text(text: str) -> List[str]:
    """解码日历数据"""
    pre = _preprocess_sina_text(text)
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

def _update_calendar():
    """下载交易日历数据并缓存到磁盘"""
    fn = _get_calendar_filename()
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
            # 未修改, 直接返回
            return
        resp.raise_for_status()

        body = resp.text
        dates = _decode_sina_text(body)
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
            marker = _get_calendar_marker_filename()
            os.makedirs(os.path.dirname(marker), exist_ok=True)
            # create or truncate marker file
            fs.update_file_mtime(marker, now_ts)
        except Exception:
            pass
    except Exception as e:
        # match Go behaviour: let caller decide; raise to caller
        raise RuntimeError(f"更新交易日历失败: {e}")


def lazy_load_calendar():
    marker = _get_calendar_marker_filename()
    now_time = Timestamp.now()
    mod_time = cache.get_filename_modified_time(marker)
    today_init_time = now_time.get_pre_market_time()
    if now_time > today_init_time and mod_time < today_init_time:
        logger.debug("交易日历缓存文件过期, 执行更新")
        try:
            _update_calendar()
            fs.update_file_mtime(marker)
        except Exception:
            pass
    else:
        logger.debug("交易日历缓存文件未过期, 跳过更新")
    logger.debug("加载交易日历缓存文件到内存")
    
    fn = _get_calendar_filename()
    if not os.path.exists(fn):
        logger.debug("交易日历缓存文件不存在, 跳过加载")
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
        logger.error("加载交易日历缓存文件失败, 跳过加载")
        return

    global globalCalendarsString, globalCalendarsTimestamp
    globalCalendarsString = ss
    globalCalendarsTimestamp = ts

class Calendar(ThreadSafeSingletonABC):
    """
    交易日历单例管理器

    功能: 
    1. 从网络或缓存加载交易日历
    2. 提供日期查询功能
    3. 线程安全的单例实现
    """

    _thread_safe_strategy = ThreadSafeStrategy.DOUBLE_CHECKED

    def initialize(self) -> None:
        """初始化交易日历"""
        self._calendars_string: List[str] = []
        self._calendars_timestamp: List[Timestamp] = []
        self._loaded = False
        logger.debug("交易日历单例已初始化")

    def cleanup(self) -> None:
        """清理资源"""
        self._calendars_string.clear()
        self._calendars_timestamp.clear()
        self._loaded = False
        logger.debug("交易日历资源已清理")

    def _load_calendar_data(self) -> None:
        """加载交易日历数据到内存"""
        marker = _get_calendar_marker_filename()
        now_time = Timestamp.now()
        mod_time = cache.get_filename_modified_time(marker)
        today_init_time = now_time.get_pre_market_time()

        if now_time > today_init_time and mod_time < today_init_time:
            logger.debug("交易日历缓存文件过期, 执行更新")
            try:
                _update_calendar()
                fs.update_file_mtime(marker)
            except Exception:
                pass
        else:
            logger.debug("交易日历缓存文件未过期, 跳过更新")

        logger.debug("加载交易日历缓存文件到内存")
        fn = _get_calendar_filename()
        if not os.path.exists(fn):
            logger.debug("交易日历缓存文件不存在, 跳过加载")
            return

        try:
            with open(fn, newline='', encoding='utf-8') as f:
                reader = csv.reader(f)
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

            self._calendars_string = ss
            self._calendars_timestamp = ts
            self._loaded = True
            logger.debug(f"交易日历加载完成, 共 {len(ss)} 个交易日")
        except Exception as e:
            logger.error(f"加载交易日历缓存文件失败: {e}")

    def _ensure_loaded(self) -> None:
        """确保日历数据已加载"""
        calendarRollingOnce.do(self._load_calendar_data)
        if not self._calendars_string:
            raise RuntimeError("exchange calendar is empty")

    def get_calendar_list(self) -> pd.Series:
        """获取全部的交易日期"""
        self._ensure_loaded()
        return pd.Series(self._calendars_string)

    def get_calendar_timestamps(self) -> List[Timestamp]:
        """获取交易日历时间戳列表"""
        self._ensure_loaded()
        return self._calendars_timestamp.copy()

    def last_trading_day(self, date: Optional[Timestamp] = None, debug_timestamp: Optional[Timestamp] = None) -> Timestamp:
        """获取最近一个交易日"""
        trade_dates = self.get_calendar_timestamps()
        if not trade_dates:
            return Timestamp.zero()

        if date is None:
            date = Timestamp.now().get_pre_market_time()

        idx = bisect.bisect_right(trade_dates, date)
        if idx > 0:
            idx -= 1

        last_ts = trade_dates[idx]
        current_ts = debug_timestamp if debug_timestamp is not None else Timestamp.now()

        if current_ts < last_ts and idx > 0:
            idx -= 1

        return trade_dates[idx]

    def prev_trading_day(self, date: Optional[Timestamp] = None, debug_timestamp: Optional[Timestamp] = None) -> Timestamp:
        """获取上一个交易日"""
        trade_dates = self.get_calendar_timestamps()
        if not trade_dates:
            return Timestamp.zero()

        if date is None:
            date = Timestamp.now().get_pre_market_time()

        idx = bisect.bisect_left(trade_dates, date)
        if idx > 0:
            idx -= 1

        last_ts = trade_dates[idx]
        current_ts = debug_timestamp if debug_timestamp is not None else Timestamp.now()

        if current_ts < last_ts and idx > 0:
            idx -= 1

        return trade_dates[idx]

    def next_trading_day(self, date: Optional[Timestamp] = None, debug_timestamp: Optional[Timestamp] = None) -> Timestamp:
        """获取下一个交易日"""
        trade_dates = self.get_calendar_timestamps()
        if not trade_dates:
            return Timestamp.zero()

        if date is None:
            date = Timestamp.now().get_pre_market_time()

        current_time = debug_timestamp if debug_timestamp is not None else Timestamp.now()
        idx = bisect.bisect_left(trade_dates, date)

        if idx >= len(trade_dates):
            if trade_dates:
                return trade_dates[-1]
            return Timestamp.zero()

        candidate_day = trade_dates[idx]

        if current_time >= candidate_day and idx < len(trade_dates):
            idx += 1
            if idx >= len(trade_dates):
                return trade_dates[-1]
            return trade_dates[idx]

        return candidate_day

    def date_range(self, begin: Timestamp, end: Optional[Timestamp] = None, skip_today: bool = False) -> List[Timestamp]:
        """获取日期范围"""
        if end is None:
            end = Timestamp.now()

        if begin > end:
            return []

        trade_dates = self.get_calendar_timestamps()
        if not trade_dates:
            return []

        lower = bisect.bisect_left(trade_dates, begin)
        upper = bisect.bisect_right(trade_dates, end)

        if skip_today and upper > 0:
            today = Timestamp.now().get_pre_market_time()
            last_in_range = trade_dates[upper - 1]
            if last_in_range > today or last_in_range > end:
                upper -= 1

        if lower >= upper:
            return []

        return trade_dates[lower:upper]

    def get_date_range(self, begin: str, end: str, skip_today: bool = False) -> List[str]:
        """获取日期范围(字符串版本)"""
        if begin > end:
            return []

        trade_dates = self.get_calendar_list().tolist()
        if not trade_dates:
            return []

        it_start = bisect.bisect_left(trade_dates, begin)
        it_end = bisect.bisect_right(trade_dates, end)

        if skip_today:
            if it_end > 0:
                today_str = get_today()
                last_day = trade_dates[it_end - 1]
                if last_day > today_str or last_day > end:
                    it_end -= 1

        if it_start >= it_end:
            return []

        return trade_dates[it_start:it_end]


# 保持向后兼容的函数接口
def get_calendar_instance() -> Calendar:
    """获取 Calendar 单例实例"""
    instance = Calendar.get_instance()
    return instance  # type: ignore


def fix_trade_date(date_str: str, fmt: str = "%Y-%m-%d") -> str:
    """强制将日期字符串转换为指定格式

    参数:
        date_str: 输入日期字符串
        fmt: 目标格式(默认%Y-%m-%d)

    返回:
        统一格式的日期字符串

    示例:
        >>> fix_trade_date("2023/12/25")
        "2023-12-25"
    """
    from datetime import datetime
    return datetime.strptime(date_str, "%Y-%m-%d").strftime(fmt) if date_str else date_str


def get_today() -> str:
    """获取当前日期"""
    date = time.strftime(layout.FORMAT_ONLY_DATE)
    return date


def load_calendar_series() -> pd.Series:
    """加载交易所日历数据并返回为pandas Series格式"""
    return get_calendar_instance().get_calendar_list()


def load_calendar() -> List[Timestamp]:
    """加载交易日历"""
    return get_calendar_instance().get_calendar_timestamps()


def get_calendar_list() -> pd.Series:
    """获取全部的交易日期"""
    return get_calendar_instance().get_calendar_list()


def last_trading_day(date: Optional[Timestamp] = None, debug_timestamp: Optional[Timestamp] = None) -> Timestamp:
    """获取最近一个交易日"""
    return get_calendar_instance().last_trading_day(date, debug_timestamp)


def prev_trading_day(date: Optional[Timestamp] = None, debug_timestamp: Optional[Timestamp] = None) -> Timestamp:
    """获取上一个交易日"""
    return get_calendar_instance().prev_trading_day(date, debug_timestamp)


def next_trading_day(date: Optional[Timestamp] = None, debug_timestamp: Optional[Timestamp] = None) -> Timestamp:
    """获取下一个交易日"""
    return get_calendar_instance().next_trading_day(date, debug_timestamp)


def date_range(begin: Timestamp, end: Optional[Timestamp] = None, skip_today: bool = False) -> List[Timestamp]:
    """获取日期范围"""
    return get_calendar_instance().date_range(begin, end, skip_today)


def get_date_range(begin: str, end: str, skip_today: bool = False) -> List[str]:
    """获取日期范围(字符串版本)"""
    return get_calendar_instance().get_date_range(begin, end, skip_today)

if __name__ == '__main__':
    a1 = last_trading_day()
    print(type(a1))
    print(a1)
    calendar = Calendar()
    print(calendar.get_calendar_list())
