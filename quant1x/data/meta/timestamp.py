# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import time
from datetime import datetime, timedelta
from typing import Optional, Tuple, Union
# Pre-market time constants
from .tradinghours import PRE_MARKET_HOUR, PRE_MARKET_MINUTE, PRE_MARKET_SECOND

# Constants
SECONDS_PER_MINUTE = 60
SECONDS_PER_HOUR = 60 * 60
SECONDS_PER_DAY = 24 * 60 * 60
MILLISECONDS_PER_SECOND = 1000
MILLISECONDS_PER_MINUTE = 60 * 1000
MILLISECONDS_PER_HOUR = 60 * 60 * 1000
MILLISECONDS_PER_DAY = 24 * 60 * 60 * 1000


class Timestamp:
    """
    Timestamp class compatible with C++/Rust/Go implementations.
    Stores time as LOCAL milliseconds (not UTC!), similar to C++ implementation.
    Strings are parsed as local time. When converting to/from UTC,
    use the conversion functions.
    """
    def __init__(self, ms: Union[int, float] = 0):
        self.ms = int(ms)

    @staticmethod
    def now() -> 'Timestamp':
        """Get current timestamp (LOCAL milliseconds)"""
        # Get current local datetime, then convert to local milliseconds
        dt_local = datetime.now()
        return Timestamp.from_datetime(dt_local)

    @staticmethod
    def zero() -> 'Timestamp':
        return Timestamp(0)

    def value(self) -> int:
        return self.ms

    def to_datetime(self) -> datetime:
        """Convert local milliseconds to local datetime (naive)"""
        seconds = self.ms / 1000.0
        # epoch = 1970-01-01 00:00:00 LOCAL time
        epoch = datetime(1970, 1, 1)
        return epoch + timedelta(seconds=seconds)

    @staticmethod
    def from_datetime(dt: datetime) -> 'Timestamp':
        """Convert local datetime to local milliseconds"""
        # dt is assumed to be local time (naive)
        # We want to store as LOCAL milliseconds, not UTC
        # epoch = 1970-01-01 00:00:00 LOCAL time
        epoch = datetime(1970, 1, 1)
        local_seconds = (dt - epoch).total_seconds()
        return Timestamp(local_seconds * 1000)

    def start_of_day(self) -> 'Timestamp':
        """Get the timestamp of 00:00:00 on the same day (Local time)"""
        return Timestamp(self.ms - (self.ms % MILLISECONDS_PER_DAY))

    @staticmethod
    def midnight() -> 'Timestamp':
        """Get the timestamp of today's 00:00:00 (Local time)"""
        ts = Timestamp.now()
        return Timestamp(ts.ms - (ts.ms % MILLISECONDS_PER_DAY))

    def today(self, hour: int = 0, minute: int = 0, second: int = 0, millisecond: int = 0) -> 'Timestamp':
        """Get timestamp for specific time on the same day (Local time)"""
        ts = self.start_of_day().value()
        ts += hour * MILLISECONDS_PER_HOUR
        ts += minute * MILLISECONDS_PER_MINUTE
        ts += second * MILLISECONDS_PER_SECOND
        ts += millisecond
        return Timestamp(ts)

    def since(self, hour: int = 0, minute: int = 0, second: int = 0, millisecond: int = 0) -> 'Timestamp':
        """Alias for today()"""
        return self.today(hour, minute, second, millisecond)

    def offset(self, hour: int = 0, minute: int = 0, second: int = 0, millisecond: int = 0) -> 'Timestamp':
        """Add offset to current timestamp (in milliseconds)"""
        ts = self.value()
        ts += hour * MILLISECONDS_PER_HOUR
        ts += minute * MILLISECONDS_PER_MINUTE
        ts += second * MILLISECONDS_PER_SECOND
        ts += millisecond
        return Timestamp(ts)

    @staticmethod
    def pre_market_time(year: int, month: int, day: int) -> 'Timestamp':
        """Construct pre-market timestamp (09:00:00) for specific date"""
        dt = datetime(year, month, day, PRE_MARKET_HOUR, PRE_MARKET_MINUTE, PRE_MARKET_SECOND)
        return Timestamp.from_datetime(dt)

    def get_pre_market_time(self) -> 'Timestamp':
        """Get pre-market timestamp for the same day"""
        return self.today(PRE_MARKET_HOUR, PRE_MARKET_MINUTE, PRE_MARKET_SECOND, 0)

    def floor(self) -> 'Timestamp':
        """Round down to nearest minute (00 seconds, 000 ms)"""
        ts = self.value()
        ts -= (ts % MILLISECONDS_PER_MINUTE)
        return Timestamp(ts)

    def ceil(self) -> 'Timestamp':
        """Round up to end of minute (59 seconds, 999 ms)"""
        ts = self.value()
        ts = ts - (ts % MILLISECONDS_PER_MINUTE) + (MILLISECONDS_PER_MINUTE - 1)
        return Timestamp(ts)

    def extract(self) -> Tuple[int, int, int]:
        """Return (year, month, day) in local time"""
        dt = self.to_datetime()
        return dt.year, dt.month, dt.day

    def to_string(self, layout: str = "%Y-%m-%d %H:%M:%S") -> str:
        """Convert to string in local time"""
        dt = self.to_datetime()
        return dt.strftime(layout)

    def only_date(self) -> str:
        return self.to_string("%Y-%m-%d")

    def cache_date(self) -> str:
        return self.to_string("%Y%m%d")

    def only_time(self) -> str:
        """Return time only, truncated to seconds"""
        dt = self.to_datetime()
        return dt.strftime("%H:%M:%S")
    
    def yyyymmdd(self) -> int:
        """返回日期部分的整数表示，格式为 YYYYMMDD"""
        dt = self.to_datetime()
        return dt.year * 10000 + dt.month * 100 + dt.day

    def is_empty(self) -> bool:
        return self.ms == 0

    def is_same_date(self, other: 'Timestamp') -> bool:
        """Check if two timestamps are on the same day (local time)"""
        day1 = self.ms // MILLISECONDS_PER_DAY
        day2 = other.ms // MILLISECONDS_PER_DAY
        return day1 == day2

    @staticmethod
    def parse(time_str: str) -> 'Timestamp':
        """Parse time string as local time"""
        formats = [
            "%Y-%m-%d %H:%M:%S.%f",
            "%Y-%m-%d %H:%M:%S",
            "%Y-%m-%d",
            "%Y%m%d",
            "%Y/%m/%d %H:%M:%S",
            "%Y/%m/%d",
            "%m/%d/%Y %H:%M:%S",
        ]
        for fmt in formats:
            try:
                dt = datetime.strptime(time_str, fmt)
                return Timestamp.from_datetime(dt)
            except ValueError:
                continue
        raise ValueError(f"Unable to parse timestamp: {time_str}")

    @staticmethod
    def parse_time(time_str: str) -> 'Timestamp':
        """Parse time string, assuming today's date if date is missing"""
        # Try full formats first
        try:
            return Timestamp.parse(time_str)
        except ValueError:
            pass

        # Try time-only formats
        time_formats = [
            "%H:%M:%S.%f",
            "%H:%M:%S",
            "%H:%M",
            "%H%M%S",
            "%H%M",
        ]

        now = datetime.now()
        for fmt in time_formats:
            try:
                t = datetime.strptime(time_str, fmt).time()
                dt = datetime.combine(now.date(), t)
                return Timestamp.from_datetime(dt)
            except ValueError:
                continue
        raise ValueError(f"Unable to parse time string: {time_str}")

    # Comparison operators
    def __eq__(self, other):
        if isinstance(other, Timestamp):
            return self.ms == other.ms
        return False

    def __lt__(self, other):
        if isinstance(other, Timestamp):
            return self.ms < other.ms
        return NotImplemented

    def __le__(self, other):
        if isinstance(other, Timestamp):
            return self.ms <= other.ms
        return NotImplemented

    def __gt__(self, other):
        if isinstance(other, Timestamp):
            return self.ms > other.ms
        return NotImplemented

    def __ge__(self, other):
        if isinstance(other, Timestamp):
            return self.ms >= other.ms
        return NotImplemented

    def __repr__(self):
        return f"Timestamp({self.ms}, '{self.to_string()}')"

    def __str__(self):
        return self.to_string()


if __name__ == "__main__":
    ts = Timestamp.parse("1970-01-01")
    print(ts)

    ts = Timestamp.parse("1900-01-01")
    print(ts)

    ts = Timestamp.parse("2024-01-01 09:30:00")
    print(ts)

    ts = Timestamp.now()
    print(f"Now: {ts}")
    
    ts = Timestamp.zero()
    print(f"Zero: {ts}")
