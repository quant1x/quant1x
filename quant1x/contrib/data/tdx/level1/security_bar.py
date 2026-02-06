# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import struct
from enum import Enum
from typing import List
from dataclasses import dataclass
from .command import (
    FLAG_UNCOMPRESSED,
    COMMAND_SECURITY_BARS,
)
from . import helpers
from quant1x.data.meta import Exchange
from quant1x.data.schema import Bar

SECURITY_BARS_PRE_REQUEST_MAX = 800

class KLineType(Enum):
    """K线类型"""
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

class SecurityBarsRequest:
    def __init__(self, exchange: Exchange, code: str, category: KLineType, start: int, count: int):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = helpers.msg_sequence_id()
        self.packet_type = 0x00
        self.method = COMMAND_SECURITY_BARS
        
        self.category = category.value
        self.i = 1
        self.start = start
        self.count = count
        
        self.market = helpers.exchange_to_market(exchange)
        self.code = code
        
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
    def __init__(self, is_index: bool, category: KLineType):
        self.count = 0
        self.list: List[Bar] = []
        self.is_index = is_index
        self.category = category.value

    def deserialize(self, data: bytes):
        if len(data) < 2:
            return
            
        self.count = struct.unpack('<H', data[:2])[0]
        pos = 2
        
        pre_diff_base = 0
        
        for _ in range(self.count):
            if pos >= len(data):
                break
                
            e = Bar()
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
            # 日期
            e.date = f"{year:04d}-{month:02d}-{day:02d}"
            # TODO: 处理时间戳
            e.timestamp = f"{year:04d}-{month:02d}-{day:02d} {hour:02d}:{minute:02d}:00"
            
            price_open_diff, pos = helpers.varint_decode(data, pos)
            price_close_diff, pos = helpers.varint_decode(data, pos)
            price_high_diff, pos = helpers.varint_decode(data, pos)
            price_low_diff, pos = helpers.varint_decode(data, pos)
            
            if pos + 8 > len(data): break
            ivol = struct.unpack('<I', data[pos:pos+4])[0]
            pos += 4
            # 成交量
            e.volume = helpers.int_to_float64(ivol)
            
            dbvol = struct.unpack('<I', data[pos:pos+4])[0]
            pos += 4
            # 成交额
            e.amount = helpers.int_to_float64(dbvol)
            
            # 开盘价
            e.open = float(price_open_diff + pre_diff_base) / 1000.0
            price_open_diff += pre_diff_base
            
            # 收盘价
            e.close = float(price_open_diff + price_close_diff) / 1000.0
            # 最高价
            e.high = float(price_open_diff + price_high_diff) / 1000.0
            # 最低价
            e.low = float(price_open_diff + price_low_diff) / 1000.0
            
            pre_diff_base = price_open_diff + price_close_diff
            
            # 指数数据
            if self.is_index:
                if pos + 4 > len(data): break
                # 上涨家数
                e.up = struct.unpack('<H', data[pos:pos+2])[0]
                pos += 2
                # 下跌家数
                e.down = struct.unpack('<H', data[pos:pos+2])[0]
                pos += 2
                
            self.list.append(e)
