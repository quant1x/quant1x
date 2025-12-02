# -*- coding: UTF-8 -*-
import time
from datetime import datetime, timedelta
from typing import Optional, Tuple, Union

# Constants
SECONDS_PER_MINUTE = 60
SECONDS_PER_HOUR = 60 * 60
SECONDS_PER_DAY = 24 * 60 * 60
MILLISECONDS_PER_SECOND = 1000
MILLISECONDS_PER_MINUTE = 60 * 1000
MILLISECONDS_PER_HOUR = 60 * 60 * 1000
MILLISECONDS_PER_DAY = 24 * 60 * 60 * 1000

PRE_MARKET_HOUR = 9
PRE_MARKET_MINUTE = 0
PRE_MARKET_SECOND = 0

class Timestamp:
    """
    Timestamp class compatible with C++/Rust/Go implementations.
    Stores time as milliseconds since epoch (UTC), but provides methods
    to work with local time.
    """
    def __init__(self, ms: Union[int, float] = 0):
        self.ms = int(ms)

    @staticmethod
    def now() -> 'Timestamp':
        return Timestamp(time.time() * 1000)

    @staticmethod
    def zero() -> 'Timestamp':
        return Timestamp(0)

    def value(self) -> int:
        return self.ms

    def to_datetime(self) -> datetime:
        """Convert to local datetime"""
        if self.ms == 0:
            return datetime.fromtimestamp(0)
        return datetime.fromtimestamp(self.ms / 1000.0)

    @staticmethod
    def from_datetime(dt: datetime) -> 'Timestamp':
        return Timestamp(dt.timestamp() * 1000)

    def start_of_day(self) -> 'Timestamp':
        """Get the timestamp of 00:00:00 on the same day (Local time)"""
        dt = self.to_datetime()
        start = dt.replace(hour=0, minute=0, second=0, microsecond=0)
        return Timestamp(start.timestamp() * 1000)

    @staticmethod
    def midnight() -> 'Timestamp':
        """Get the timestamp of today's 00:00:00 (Local time)"""
        return Timestamp.now().start_of_day()

    def today(self, hour: int = 0, minute: int = 0, second: int = 0, millisecond: int = 0) -> 'Timestamp':
        """Get timestamp for specific time on the same day"""
        start = self.start_of_day()
        offset = (hour * MILLISECONDS_PER_HOUR + 
                  minute * MILLISECONDS_PER_MINUTE + 
                  second * MILLISECONDS_PER_SECOND + 
                  millisecond)
        return Timestamp(start.value() + offset)

    def since(self, hour: int = 0, minute: int = 0, second: int = 0, millisecond: int = 0) -> 'Timestamp':
        """Alias for today()"""
        return self.today(hour, minute, second, millisecond)

    def offset(self, hour: int = 0, minute: int = 0, second: int = 0, millisecond: int = 0) -> 'Timestamp':
        """Add offset to current timestamp"""
        offset_ms = (hour * MILLISECONDS_PER_HOUR + 
                     minute * MILLISECONDS_PER_MINUTE + 
                     second * MILLISECONDS_PER_SECOND + 
                     millisecond)
        return Timestamp(self.ms + offset_ms)

    @staticmethod
    def pre_market_time(year: int, month: int, day: int) -> 'Timestamp':
        """Construct pre-market timestamp (09:00:00) for specific date"""
        dt = datetime(year, month, day, PRE_MARKET_HOUR, PRE_MARKET_MINUTE, PRE_MARKET_SECOND)
        return Timestamp(dt.timestamp() * 1000)

    def get_pre_market_time(self) -> 'Timestamp':
        """Get pre-market timestamp for the same day"""
        return self.today(PRE_MARKET_HOUR, PRE_MARKET_MINUTE, PRE_MARKET_SECOND, 0)

    def floor(self) -> 'Timestamp':
        """Round down to nearest minute (00 seconds, 000 ms)"""
        # Note: This logic depends on whether we want to floor in UTC or Local.
        # C++ implementation usually implies flooring the representation.
        # Here we floor based on the stored value (which is UTC). 
        # If we want to floor in Local time, we should convert to datetime.
        # Assuming standard behavior: floor to minute boundary.
        # Since timezones usually have minute-aligned offsets, flooring UTC ms should work for Local too 
        # unless offset has seconds.
        return Timestamp(self.ms - (self.ms % MILLISECONDS_PER_MINUTE))

    def ceil(self) -> 'Timestamp':
        """Round up to end of minute (59 seconds, 999 ms)"""
        floored = self.ms - (self.ms % MILLISECONDS_PER_MINUTE)
        return Timestamp(floored + MILLISECONDS_PER_MINUTE - 1)

    def extract(self) -> Tuple[int, int, int]:
        """Return (year, month, day)"""
        dt = self.to_datetime()
        return dt.year, dt.month, dt.day

    def to_string(self, layout: str = "%Y-%m-%d %H:%M:%S") -> str:
        dt = self.to_datetime()
        if "%f" in layout:
            # Python's %f is microseconds (000000), we might want milliseconds
            s = dt.strftime(layout)
            return s
        return dt.strftime(layout)

    def only_date(self) -> str:
        return self.to_string("%Y-%m-%d")

    def cache_date(self) -> str:
        return self.to_string("%Y%m%d")

    def only_time(self) -> str:
        return self.to_string("%H:%M:%S")

    def is_empty(self) -> bool:
        return self.ms == 0

    def is_same_date(self, other: 'Timestamp') -> bool:
        dt1 = self.to_datetime()
        dt2 = other.to_datetime()
        return dt1.date() == dt2.date()

    @staticmethod
    def parse(time_str: str) -> 'Timestamp':
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
                # If format has no time, it defaults to 00:00:00
                return Timestamp(dt.timestamp() * 1000)
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
                return Timestamp(dt.timestamp() * 1000)
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

