# -*- coding: UTF-8 -*-
import re
import time
from dataclasses import dataclass
from datetime import datetime
from typing import List

# 仅日期格式: 2022-11-28
FORMAT_ONLY_DATE = '%Y-%m-%d'
# 仅时间格式: 09:15:59
FORMAT_ONLY_TIME = '%H:%M:%S'
# 文件名中包含日期的日期格式: 20221128
FORMAT_FILE_DATE = '%Y%m%d'
# 时间戳: 2022-11-28 09:15:59
FORMAT_DATETIME = '%Y-%m-%d %H:%M:%S'
# 时间戳带毫秒数, 如果毫秒数保留前3位, 需要自己截取: 2022-11-28 09:15:59.123456
FORMAT_TIMESTAMP = '%Y-%m-%d %H:%M:%S.%f'


def seconds_to_timestamp(x: int):
    """
    秒数转时间戳字符串
    :param x:
    :return:
    """
    return time.strftime(FORMAT_DATETIME, time.localtime(x))


@dataclass
class TimeRange(object):
    """
    时间范围, 用~或-间隔HH-MM-SS
    """
    begin: str
    end: str

    def __init__(self, time_range: str):
        """
        构造
        :param time_range:
        :return:
        """
        self.begin = ''
        self.end = ''

        time_range = time_range.strip()
        list = re.split(r"[~-]\s*", time_range)
        if len(list) != 2:
            raise RuntimeError("非法的时间格式")
        # TODO：时间格式校验
        # 时间排序
        self.begin = list[0].strip()
        self.end = list[1].strip()
        if self.begin > self.end:
            self.begin, self.end = self.end, self.begin

    def is_trading(self, timestamp: str = "") -> bool:
        """
        是否交易中
        :param timestamp: %H:%M:%S
        :return:
        """
        timestamp = timestamp.strip()
        if len(timestamp) == 0:
            timestamp = time.strftime(FORMAT_ONLY_TIME)
        if self.begin <= timestamp <= self.end:
            return True
        return False

    def is_valid(self) -> bool:
        """
        时段是否有效
        :return:
        """
        return self.begin != '' and self.end != ''

    def is_session_pre(self, timestamp: str = "") -> bool:
        """
        是否盘前
        :param timestamp: %H:%M:%S
        """
        timestamp = timestamp.strip()
        if len(timestamp) == 0:
            timestamp = time.strftime(FORMAT_ONLY_TIME)
        return timestamp < self.begin

    def is_session_reg(self, timestamp: str = "") -> bool:
        """
        是否盘中
        :param timestamp: %H:%M:%S
        """
        timestamp = timestamp.strip()
        if len(timestamp) == 0:
            timestamp = time.strftime(FORMAT_ONLY_TIME)
        return self.is_trading(timestamp)

    def is_session_post(self, timestamp: str = "") -> bool:
        """
        是否盘后
        :param timestamp: %H:%M:%S
        """
        timestamp = timestamp.strip()
        if len(timestamp) == 0:
            timestamp = time.strftime(FORMAT_ONLY_TIME)
        return timestamp > self.end


@dataclass
class TradingSession:
    """
    交易时段
    """
    sessions: List

    def __init__(self, time_range: str):
        """
        构造
        :param time_range:
        """
        self.sessions = []
        time_range = time_range.strip()
        list = re.split(r",\s*", time_range)
        for v in list:
            v = v.strip()
            r = TimeRange(v)
            self.sessions.append(r)

    def is_trading(self, timestamp: str = "") -> bool:
        """
        是否交易中
        :param timestamp:
        :return:
        """
        timestamp = timestamp.strip()
        if len(timestamp) == 0:
            timestamp = time.strftime(FORMAT_ONLY_TIME)
        for item in self.sessions:
            v = item
            if v.is_trading(timestamp):
                return True
        return False

    def is_valid(self) -> bool:
        """
        时段是否有效
        :return:
        """
        for item in self.sessions:
            if not item.is_valid():
                return False
        return True


if __name__ == '__main__':
    dt = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    dt_ms = datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')
    print(dt)
    print(dt_ms)

    time_range = " 09:30:00 ~ 14:56:30 "
    time_range = " 14:56:30 - 09:30:00 "
    tr = TimeRange(time_range)
    print(tr)

    time_range = "11:30:00 ~ 09:15:00 , 15:00:00 - 13:00:00 "
    time_range = "15:00:00 - 13:00:00 "

    tr = TradingSession(time_range)
    print(tr)
    print(tr.is_trading('09:30:00'))
