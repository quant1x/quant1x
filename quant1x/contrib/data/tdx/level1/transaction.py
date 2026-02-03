# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import struct
from .command import (
    FLAG_UNCOMPRESSED,
    COMMAND_TRANSACTION_DATA,
    COMMAND_HISTORY_TRANSACTION_DATA,
)
from . import helpers

from quant1x.data.market import Exchange
from quant1x.data.cache import Transaction

# Constants
TICK_BUY = 0
TICK_SELL = 1
TICK_NEUTRAL = 2
TICK_UNKNOWN = 3

TICK_TRANSACTION_PER_REQUEST_MAX = 1800

class TransactionRequest:
    def __init__(self, exchange: Exchange, code: str, start: int, count: int):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = helpers.msg_sequence_id()
        self.packet_type = 0x00
        self.method = COMMAND_TRANSACTION_DATA
        
    
        self.market = helpers.exchange_to_market(exchange)
        self.code = code
        self.start = start
        self.count = count

    def serialize(self) -> bytes:
        # Header: zip_flag(1), seq_id(4), packet_type(1), pkg_len1(2), pkg_len2(2), method(2)
        # Body: Market(2), Code(6), Start(2), Count(2)
        body_len = 2 + 6 + 2 + 2
        pkg_len = 2 + body_len
        
        header = struct.pack('<B I B H H H', 
                             self.zip_flag, self.seq_id, self.packet_type, 
                             pkg_len, pkg_len, self.method)
        
        code_bytes = self.code.encode('ascii')
        if len(code_bytes) < 6:
            code_bytes = code_bytes + b'\x00' * (6 - len(code_bytes))
        else:
            code_bytes = code_bytes[:6]
            
        body = struct.pack('<H 6s H H', self.market, code_bytes, self.start, self.count)
        return header + body

class TransactionResponse:
    def __init__(self, price_precision: int, is_index: bool):
        self.count = 0
        self.list: list[Transaction] = []
        self.price_precision = price_precision
        self.is_index = is_index

    def deserialize(self, data: bytes):
        if len(data) < 2:
            return
        self.count = struct.unpack('<H', data[:2])[0]
        pos = 2
        
        base_unit = 10 ** self.price_precision
        last_price = 0
        for _ in range(self.count):
            if pos >= len(data):
                break
            
            if pos + 2 > len(data):
                break
            minutes = struct.unpack('<H', data[pos:pos+2])[0]
            pos += 2
            
            h = minutes // 60
            m = minutes % 60
            time_str = f"{h:02d}:{m:02d}"
            
            raw_price, pos = helpers.varint_decode(data, pos)
            vol, pos = helpers.varint_decode(data, pos)
            num, pos = helpers.varint_decode(data, pos)
            buy_or_sell, pos = helpers.varint_decode(data, pos)
            
            last_price += raw_price
            price = float(last_price) / base_unit
            
            amount = 0.0
            if self.is_index:
                amount = float(vol * 100)
                if price > 0:
                    vol = int(amount / price)
                else:
                    vol = 0
            else:
                vol *= 100
                amount = float(vol) * price
                
            # Skip one varint (unknown field)
            _, pos = helpers.varint_decode(data, pos)
            
            self.list.append(Transaction(time=time_str, price=price, volume=vol, num=num, amount=amount, direction=buy_or_sell))

class HistoricalTransactionRequest:
    def __init__(self, exchange: Exchange, code: str, date: int, start: int, count: int):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = helpers.msg_sequence_id()
        self.packet_type = 0x00
        self.method = COMMAND_HISTORY_TRANSACTION_DATA
        
        self.market = helpers.exchange_to_market(exchange)
        self.code = code
        self.date = date
        self.start = start
        self.count = count

    def serialize(self) -> bytes:
        # Header
        # Body: Date(4), Market(2), Code(6), Start(2), Count(2)
        body_len = 4 + 2 + 6 + 2 + 2
        pkg_len = 2 + body_len
        
        header = struct.pack('<B I B H H H', 
                             self.zip_flag, self.seq_id, self.packet_type, 
                             pkg_len, pkg_len, self.method)
        
        code_bytes = self.code.encode('ascii')
        if len(code_bytes) < 6:
            code_bytes = code_bytes + b'\x00' * (6 - len(code_bytes))
        else:
            code_bytes = code_bytes[:6]
            
        body = struct.pack('<I H 6s H H', self.date, self.market, code_bytes, self.start, self.count)
        return header + body

class HistoricalTransactionResponse:
    def __init__(self, price_precision: int, is_index: bool):
        self.count = 0
        self.list: list[Transaction] = []
        self.price_precision = price_precision
        self.is_index = is_index

    def deserialize(self, data: bytes):
        if len(data) < 2:
            return
        self.count = struct.unpack('<H', data[:2])[0]
        pos = 2
        
        base_unit = 10 ** self.price_precision
        last_price = 0
        
        # Skip 4 bytes (unknown/padding in history response?)
        # C++: bs.skip(4); // 历史分笔成交记录, 跳过4个字节
        pos += 4
        
        for _ in range(self.count):
            if pos >= len(data):
                break
            
            if pos + 2 > len(data):
                break
            minutes = struct.unpack('<H', data[pos:pos+2])[0]
            pos += 2
            
            h = minutes // 60
            m = minutes % 60
            time_str = f"{h:02d}:{m:02d}"
            
            raw_price, pos = helpers.varint_decode(data, pos)
            vol, pos = helpers.varint_decode(data, pos)
            # No num in history
            buy_or_sell, pos = helpers.varint_decode(data, pos)
            
            last_price += raw_price
            price = float(last_price) / base_unit
            
            amount = 0.0
            if self.is_index:
                amount = float(vol * 100)
                if price > 0:
                    vol = int(amount / price)
                else:
                    vol = 0
            else:
                vol *= 100
                amount = float(vol) * price
                
            _, pos = helpers.varint_decode(data, pos)
            
            self.list.append(Transaction(time=time_str, price=price, volume=vol, num=0, amount=amount, direction=buy_or_sell))
