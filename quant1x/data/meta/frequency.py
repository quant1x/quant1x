# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import re
from enum import Enum
from typing import NamedTuple, Union

__all__ = [
    "TimeUnit",
    "Frequency",
    "parse_frequency_string",
    "is_fixed_duration",
    "to_total_seconds",
]


class TimeUnit(Enum):
    """
    标准化的时间单位枚举, 覆盖 pandas 常见别名. 
    所有单位均为固定长度(不包括月, 年等日历单位). 
    """
    NANOSECOND = "ns"
    MICROSECOND = "us"
    MILLISECOND = "ms"
    SECOND = "s"
    MINUTE = "min"
    HOUR = "h"
    DAY = "d"
    WEEK = "w"
    MONTH = "m"
    YEAR = "y"

    @property
    def seconds_per_unit(self) -> float:
        """每单位对应的秒数(float 支持纳秒)"""
        mapping = {
            TimeUnit.NANOSECOND: 1e-9,
            TimeUnit.MICROSECOND: 1e-6,
            TimeUnit.MILLISECOND: 1e-3,
            TimeUnit.SECOND: 1.0,
            TimeUnit.MINUTE: 60.0,
            TimeUnit.HOUR: 3600.0,
            TimeUnit.DAY: 86400.0,
            TimeUnit.WEEK: 604800.0,
            TimeUnit.MONTH: 2592000.0,
            TimeUnit.YEAR: 31536000.0,
        }
        return mapping[self]


class Frequency(NamedTuple):
    """
    表示一个标准化的频率值: num x unit. 
    例如: 5 分钟 → Frequency(num=5, unit=TimeUnit.MINUTE)
    """
    num: int
    unit: TimeUnit

    def to_total_seconds(self) -> float:
        """返回总秒数(可用于比较, 排序, 计算)"""
        return self.num * self.unit.seconds_per_unit

    def __str__(self) -> str:
        return f"{self.num}{self.unit.value}"
    
    def cache_key(self) -> str:
        if self.unit == TimeUnit.DAY:
            return 'day'
        return f"{self.num}{self.unit.value}"

# 日线频率的常量
FREQ_DAILY = Frequency(num=1, unit=TimeUnit.DAY)
"""日线"""
FREQ_WEEKLY = Frequency(num=1, unit=TimeUnit.WEEK)
"""周线"""
FREQ_MONTHLY = Frequency(num=1, unit=TimeUnit.MONTH)
"""月线"""
FREQ_YEARLY = Frequency(num=1, unit=TimeUnit.YEAR)
"""年线"""

# pandas 单位别名映射表(只读)
_PANDAS_UNIT_ALIASES = {
    # nanosecond
    "N": TimeUnit.NANOSECOND, "ns": TimeUnit.NANOSECOND,
    # microsecond
    "U": TimeUnit.MICROSECOND, "us": TimeUnit.MICROSECOND, "µs": TimeUnit.MICROSECOND,
    # millisecond
    "L": TimeUnit.MILLISECOND, "ms": TimeUnit.MILLISECOND,
    # second
    "S": TimeUnit.SECOND, "s": TimeUnit.SECOND,
    # minute
    "T": TimeUnit.MINUTE, "min": TimeUnit.MINUTE,
    # hour
    "H": TimeUnit.HOUR, "h": TimeUnit.HOUR,
    # day
    "D": TimeUnit.DAY, "d": TimeUnit.DAY,
    # week
    "W": TimeUnit.WEEK, "w": TimeUnit.WEEK,
    # month
    "M": TimeUnit.MONTH, "m": TimeUnit.MONTH,
    # year
    "Y": TimeUnit.YEAR, "y": TimeUnit.YEAR,
}

def parse_frequency_string(freq: str) -> Frequency:
    """
    解析 pandas 风格的频率字符串(如 '5T', '1H', '30s')为标准化 FrequencyValue. 
    
    Args:
        freq: 频率字符串, 如 "5T", "1h", "90s"
    
    Returns:
        Frequency(count=5, unit=TimeUnit.MINUTE)
    
    Raises:
        ValueError: 无效格式或不支持的单位
    """
    s = freq.strip()
    if not s:
        raise ValueError("frequency string is empty")

    # 正则提取数字前缀和单位后缀
    match = re.match(r'^(\d*)(.*)$', s)
    if not match:
        raise ValueError(f"invalid frequency format: {s}")

    num_str, unit_str = match.groups()
    num = int(num_str) if num_str else 1

    if not unit_str:
        raise ValueError("missing unit in frequency string")

    unit = _PANDAS_UNIT_ALIASES.get(unit_str)
    if unit is None:
        raise ValueError(f"unsupported or unknown frequency unit: {unit_str!r}")

    return Frequency(num=num, unit=unit)


def to_total_seconds(freq: Union[str, Frequency]) -> float:
    """便捷函数: 将频率转为总秒数"""
    if isinstance(freq, str):
        freq = parse_frequency_string(freq)
    return freq.to_total_seconds()


def is_fixed_duration(freq: Union[str, Frequency]) -> bool:
    """
    判断是否为固定时长(所有当前支持的单位都是固定的). 
    未来若加入 'M'(月), 'Y'(年), 此处需调整. 
    """
    return True  # 当前所有单位均为固定长度

if __name__ == "__main__":
    f1 = parse_frequency_string("5T")
    print(f1)  # 5min
    print(f1.count, f1.unit)  # 5 TimeUnit.MINUTE
    print(f1.to_total_seconds())  # 300.0

    f2 = parse_frequency_string("1h")
    print(f2.to_total_seconds())  # 3600.0

    # 可用于比较
    assert f1.to_total_seconds() < f2.to_total_seconds()