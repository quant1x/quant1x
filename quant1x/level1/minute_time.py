# -*- coding: UTF-8 -*-
from __future__ import annotations

import struct
from dataclasses import dataclass
from typing import List

from quant1x.level1.protocol import (
    FLAG_UNCOMPRESSED,
    COMMAND_HISTORY_MINUTE_DATA,
    sequence_id,
)
from quant1x.level1 import helpers
from quant1x.exchange.code import detect_market

@dataclass
class MinuteTime:
    price: float = 0.0
    vol: int = 0

class HistoryMinuteTimeRequest:
    """
    历史分时数据请求
    """
    def __init__(self, security_code: str, date: int):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = sequence_id()
        self.packet_type = 0x00  # Note: 0x00 for this request type
        self.pkg_len1 = 0
        self.pkg_len2 = 0
        self.method = COMMAND_HISTORY_MINUTE_DATA
        
        self.date = date
        market_id, _, symbol = detect_market(security_code)
        self.market = market_id.value
        self.code = symbol

    def serialize(self) -> bytes:
        # Body: Date(4) + Market(1) + Code(6) = 11 bytes
        # PkgLen = Body + 2 = 13
        self.pkg_len1 = 2 + 4 + 1 + 6
        self.pkg_len2 = self.pkg_len1
        
        header = struct.pack('<B I B H H H', self.zip_flag, self.seq_id, self.packet_type, self.pkg_len1, self.pkg_len2, self.method)
        
        # Ensure code is 6 bytes
        code_bytes = self.code.encode('ascii')[:6].ljust(6, b'\x00')
        
        body = struct.pack('<I B', self.date, self.market) + code_bytes
        return header + body

class HistoryMinuteTimeResponse:
    """
    历史分时数据响应
    """
    def __init__(self, market: int, code: str):
        self.count = 0
        self.list: List[MinuteTime] = []
        self.market = market
        self.code = code

    def deserialize(self, data: bytes) -> None:
        if len(data) < 2:
            return
            
        self.count = struct.unpack('<H', data[:2])[0]
        pos = 2
        
        base_unit = helpers.default_base_unit(self.market, self.code)
        last_price = 0
        
        # Skip 4 bytes
        pos += 4
        
        try:
            for _ in range(self.count):
                if pos >= len(data):
                    break
                    
                raw_price, pos = helpers.varint_decode(data, pos)
                _, pos = helpers.varint_decode(data, pos) # reversed1
                vol, pos = helpers.varint_decode(data, pos)
                
                last_price += raw_price
                price = float(last_price) / base_unit
                
                self.list.append(MinuteTime(price=price, vol=vol))
                
        except IndexError:
            # Handle insufficient data gracefully
            pass
