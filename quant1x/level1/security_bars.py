# -*- coding: UTF-8 -*-
import struct
from enum import Enum
from typing import List, Tuple
from dataclasses import dataclass

from quant1x.level1 import protocol, helpers
from quant1x.exchange import code as exchange_code

SECURITY_BARS_MAX = 800

class KLineType(Enum):
    _5MIN = 0
    _15MIN = 1
    _30MIN = 2
    _1HOUR = 3
    DAILY = 4
    WEEKLY = 5
    MONTHLY = 6
    EXHQ_1MIN = 7
    _1MIN = 8
    RI_K = 9
    _3MONTH = 10
    YEARLY = 11

    @staticmethod
    def to_string(ktype: 'KLineType') -> str:
        return ktype.name

@dataclass
class SecurityBar:
    Open: float = 0.0
    Close: float = 0.0
    High: float = 0.0
    Low: float = 0.0
    Vol: float = 0.0
    Amount: float = 0.0
    Year: int = 0
    Month: int = 0
    Day: int = 0
    Hour: int = 0
    Minute: int = 0
    DateTime: str = ""
    UpCount: int = 0
    DownCount: int = 0

    def __str__(self):
        return (f"Open: {self.Open} Close: {self.Close} High: {self.High} Low: {self.Low} "
                f"Vol: {self.Vol} Amount: {self.Amount} Year: {self.Year} Month: {self.Month} "
                f"Day: {self.Day} Hour: {self.Hour} Minute: {self.Minute} DateTime: {self.DateTime} "
                f"UpCount: {self.UpCount} DownCount: {self.DownCount}")

class SecurityBarsRequest:
    def __init__(self, security_code: str, category: int, start: int, count: int):
        self.zip_flag = protocol.FLAG_UNCOMPRESSED
        self.seq_id = protocol.sequence_id()
        self.packet_type = 0x00
        self.method = protocol.COMMAND_SECURITY_BARS
        
        self.category = category
        self.i = 1
        self.start = start
        self.count = count
        
        market_id, _, symbol = exchange_code.detect_market(security_code)
        self.market = market_id.value if hasattr(market_id, 'value') else market_id
        self.code = symbol
        self.is_index = exchange_code.assert_index_by_market_and_code(market_id, symbol)
        
        self.padding = bytes.fromhex("00000000000000000000")

    def serialize(self) -> bytes:
        # Body: Market(2) + Code(6) + Category(2) + I(2) + Start(2) + Count(2) + Padding(10)
        # Total Body = 26 bytes
        body_len = 2 + 6 + 2 + 2 + 2 + 2 + len(self.padding)
        pkg_len = body_len + 2
        
        header = struct.pack('<B I B H H H', 
                             self.zip_flag, self.seq_id, self.packet_type, 
                             pkg_len, pkg_len, self.method)
        
        code_bytes = self.code.encode('ascii')
        if len(code_bytes) < 6:
            code_bytes = code_bytes + b'\x00' * (6 - len(code_bytes))
        else:
            code_bytes = code_bytes[:6]
            
        body = struct.pack('<H 6s H H H H', 
                           self.market, code_bytes, self.category, self.i, self.start, self.count)
        return header + body + self.padding

class SecurityBarsResponse:
    def __init__(self, is_index: bool, category: int):
        self.count = 0
        self.list: List[SecurityBar] = []
        self.is_index = is_index
        self.category = category

    def deserialize(self, data: bytes):
        if len(data) < 2:
            return
            
        self.count = struct.unpack('<H', data[:2])[0]
        pos = 2
        
        pre_diff_base = 0
        
        for _ in range(self.count):
            if pos >= len(data):
                break
                
            e = SecurityBar()
            year = 0
            month = 0
            day = 0
            hour = 15
            minute = 0
            
            if self.category < 4 or self.category == 7 or self.category == 8:
                if pos + 4 > len(data): break
                zipday = struct.unpack('<H', data[pos:pos+2])[0]
                pos += 2
                tminutes = struct.unpack('<H', data[pos:pos+2])[0]
                pos += 2
                
                year, month, day, hour, minute = helpers.get_datetime_from_uint32(self.category, zipday, tminutes)
            else:
                if pos + 4 > len(data): break
                zipday = struct.unpack('<I', data[pos:pos+4])[0]
                pos += 4
                year = int(zipday / 10000)
                month = int((zipday % 10000) / 100)
                day = int(zipday % 100)
                
            e.Year = year
            e.Month = month
            e.Day = day
            e.Hour = hour
            e.Minute = minute
            e.DateTime = f"{year:04d}-{month:02d}-{day:02d} {hour:02d}:{minute:02d}:00"
            
            price_open_diff, pos = helpers.varint_decode(data, pos)
            price_close_diff, pos = helpers.varint_decode(data, pos)
            price_high_diff, pos = helpers.varint_decode(data, pos)
            price_low_diff, pos = helpers.varint_decode(data, pos)
            
            if pos + 8 > len(data): break
            ivol = struct.unpack('<I', data[pos:pos+4])[0]
            pos += 4
            e.Vol = helpers.int_to_float64(ivol)
            
            dbvol = struct.unpack('<I', data[pos:pos+4])[0]
            pos += 4
            e.Amount = helpers.int_to_float64(dbvol)
            
            e.Open = float(price_open_diff + pre_diff_base) / 1000.0
            price_open_diff += pre_diff_base
            
            e.Close = float(price_open_diff + price_close_diff) / 1000.0
            e.High = float(price_open_diff + price_high_diff) / 1000.0
            e.Low = float(price_open_diff + price_low_diff) / 1000.0
            
            pre_diff_base = price_open_diff + price_close_diff
            
            if self.is_index:
                if pos + 4 > len(data): break
                e.UpCount = struct.unpack('<H', data[pos:pos+2])[0]
                pos += 2
                e.DownCount = struct.unpack('<H', data[pos:pos+2])[0]
                pos += 2
                
            self.list.append(e)
