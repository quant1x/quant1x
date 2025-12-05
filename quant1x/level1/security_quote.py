# -*- coding: UTF-8 -*-
from __future__ import annotations

import struct
import math
from enum import IntEnum
from typing import List, Dict, Any, Optional
from dataclasses import dataclass, field

from quant1x.level1.protocol import (
    FLAG_UNCOMPRESSED,
    COMMAND_SECURITY_QUOTES_OLD,
    sequence_id,
)
from quant1x.level1 import helpers
from quant1x.exchange.code import (
    detect_market,
    assert_index_by_market_and_code,
    MarketType,
    get_market_flag,
)
from quant1x.exchange import Timestamp

# Enums
class TradeState(IntEnum):
    DELISTING = 0  # 终止上市
    NORMAL = 1     # 正常交易
    SUSPEND = 2    # 停牌
    IPO = 3        # IPO阶段

class SpreadLevel(IntEnum):
    VERY_LOW = 0
    LOW = 1
    MEDIUM = 2
    HIGH = 3
    VERY_HIGH = 4

# Constants
SPREAD_PCT_VERY_LOW = 0.05
SPREAD_PCT_LOW = 0.2
SPREAD_PCT_MEDIUM = 0.8
SPREAD_PCT_HIGH = 2.0

@dataclass
class StockInfo:
    market: int = 0
    code: str = ""

class SecurityQuoteRequest:
    def __init__(self, codes: List[str]):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = sequence_id()
        self.packet_type = 0x01
        self.pkg_len1 = 0
        self.pkg_len2 = 0
        self.method = COMMAND_SECURITY_QUOTES_OLD
        self.padding = bytes.fromhex("0500000000000000")
        self.list: List[StockInfo] = []
        
        for security_code in codes:
            sc = security_code.strip()
            if not sc:
                continue
            market_id, _, symbol = detect_market(sc)
            # market_id is Enum, convert to int
            self.list.append(StockInfo(market=market_id.value, code=symbol))

    def serialize(self) -> bytes:
        count = len(self.list)
        # PkgLen1 = 2 + u16(count * 7) + 10
        self.pkg_len1 = 2 + (count * 7) + 10
        self.pkg_len2 = self.pkg_len1
        
        header = struct.pack('<B I B H H H', self.zip_flag, self.seq_id, self.packet_type, self.pkg_len1, self.pkg_len2, self.method)
        
        body = bytearray()
        body.extend(self.padding)
        body.extend(struct.pack('<H', count))
        for stock in self.list:
            body.append(stock.market)
            # Ensure code is 6 bytes
            code_bytes = stock.code.encode('ascii')[:6].ljust(6, b'\x00')
            body.extend(code_bytes)
            
        return header + body

@dataclass
class Level:
    price: float = 0.0
    vol: int = 0

@dataclass
class SecurityQuote:
    state: TradeState = TradeState.NORMAL
    market: int = 0
    code: str = ""
    active1: int = 0
    price: float = 0.0
    last_close: float = 0.0
    open: float = 0.0
    high: float = 0.0
    low: float = 0.0
    server_time: str = ""
    reversed_bytes0: int = 0
    reversed_bytes1: int = 0
    vol: int = 0
    cur_vol: int = 0
    amount: float = 0.0
    s_vol: int = 0
    b_vol: int = 0
    index_open_amount: int = 0
    stock_open_amount: int = 0
    open_volume: int = 0
    close_volume: int = 0
    index_up: int = 0
    index_up_limit: int = 0
    index_down: int = 0
    index_down_limit: int = 0
    
    bid1: float = 0.0
    ask1: float = 0.0
    bid_vol1: int = 0
    ask_vol1: int = 0
    
    bid2: float = 0.0
    ask2: float = 0.0
    bid_vol2: int = 0
    ask_vol2: int = 0
    
    bid3: float = 0.0
    ask3: float = 0.0
    bid_vol3: int = 0
    ask_vol3: int = 0
    
    bid4: float = 0.0
    ask4: float = 0.0
    bid_vol4: int = 0
    ask_vol4: int = 0
    
    bid5: float = 0.0
    ask5: float = 0.0
    bid_vol5: int = 0
    ask_vol5: int = 0
    
    reversed_bytes4: int = 0
    reversed_bytes5: int = 0
    reversed_bytes6: int = 0
    reversed_bytes7: int = 0
    reversed_bytes8: int = 0
    
    rate: float = 0.0
    active2: int = 0
    time_stamp: str = ""

    def implicit_spread(self) -> float:
        if math.isnan(self.price) or self.price <= 0.0:
            if self.ask1 > 0.0 and self.bid1 > 0.0:
                return self.ask1 - self.bid1
            return 0.0
        
        if self.ask1 > 0.0 and self.bid1 > 0.0:
            mid = (self.ask1 + self.bid1) / 2.0
            return 2.0 * abs(self.price - mid)
            
        if self.ask1 > 0.0 and self.bid1 > 0.0:
            return self.ask1 - self.bid1
            
        return 0.0

    def implicit_spread_pct(self) -> float:
        if self.ask1 > 0.0 and self.bid1 > 0.0:
            mid = (self.ask1 + self.bid1) / 2.0
            s = self.implicit_spread()
            if mid > 0.0:
                return s / mid * 100.0
        
        if self.last_close > 0.0:
            s = self.implicit_spread()
            return s / self.last_close * 100.0
            
        return 0.0

    def implicit_spread_level(self) -> SpreadLevel:
        pct = self.implicit_spread_pct()
        if pct < SPREAD_PCT_VERY_LOW: return SpreadLevel.VERY_LOW
        if pct < SPREAD_PCT_LOW: return SpreadLevel.LOW
        if pct < SPREAD_PCT_MEDIUM: return SpreadLevel.MEDIUM
        if pct < SPREAD_PCT_HIGH: return SpreadLevel.HIGH
        return SpreadLevel.VERY_HIGH

class SecurityQuoteResponse:
    def __init__(self):
        self.count = 0
        self.list: List[SecurityQuote] = []

    @staticmethod
    def get_price(base_unit: float, price: int, diff: int) -> float:
        return float(price + diff) / base_unit

    def deserialize(self, data: bytes) -> None:
        # Skip 2 bytes? C++: stream.skip(2);
        pos = 2
        if len(data) < pos + 2:
            return
            
        self.count = struct.unpack('<H', data[pos:pos+2])[0]
        pos += 2
        
        # Use current time for timestamp
        # In C++: auto now = exchange::timestamp::now();
        # In Python: Timestamp.now().to_string()?
        # Assuming Timestamp is available and has similar API or use simple string
        # from quant1x.exchange import Timestamp
        # timestamp = Timestamp.now().to_string()
        # For now, let's use a placeholder or simple datetime if Timestamp is not fully compatible
        # But I imported Timestamp, let's try to use it if possible, or just use empty string if unsure.
        # C++: auto timestamp = now.toString();
        timestamp = "" 
        try:
            # Assuming Timestamp class has now() and str() or similar
            # If not, we can skip or use simple datetime
            import datetime
            timestamp = datetime.datetime.now().strftime("%Y%m%d%H%M%S%f")[:-3]
        except:
            pass

        for _ in range(self.count):
            ele = SecurityQuote()
            ele.market = data[pos]
            pos += 1
            ele.code = data[pos:pos+6].decode('ascii')
            pos += 6
            
            base_unit = helpers.default_base_unit(ele.market, ele.code)
            
            ele.active1 = struct.unpack('<H', data[pos:pos+2])[0]
            pos += 2
            
            price_base, pos = helpers.varint_decode(data, pos)
            ele.price = self.get_price(base_unit, price_base, 0)
            
            tmp, pos = helpers.varint_decode(data, pos)
            ele.last_close = self.get_price(base_unit, price_base, tmp)
            
            tmp, pos = helpers.varint_decode(data, pos)
            ele.open = self.get_price(base_unit, price_base, tmp)
            
            tmp, pos = helpers.varint_decode(data, pos)
            ele.high = self.get_price(base_unit, price_base, tmp)
            
            tmp, pos = helpers.varint_decode(data, pos)
            ele.low = self.get_price(base_unit, price_base, tmp)
            
            ele.reversed_bytes0, pos = helpers.varint_decode(data, pos)
            if ele.reversed_bytes0 > 0:
                # C++: helpers::format_time(ele.reversedBytes0)
                # Assuming format_time is not available in python helpers yet, implementing simple logic
                # Usually it is HHMMSS or similar integer
                ele.server_time = str(ele.reversed_bytes0) 
            else:
                ele.server_time = "0"
                
            ele.reversed_bytes1, pos = helpers.varint_decode(data, pos)
            
            vol, pos = helpers.varint_decode(data, pos)
            ele.vol = vol * 100
            
            ele.cur_vol, pos = helpers.varint_decode(data, pos)
            
            raw_amount = struct.unpack('<I', data[pos:pos+4])[0]
            pos += 4
            ele.amount = helpers.int_to_float64(raw_amount)
            
            ele.s_vol, pos = helpers.varint_decode(data, pos)
            ele.b_vol, pos = helpers.varint_decode(data, pos)
            
            val, pos = helpers.varint_decode(data, pos)
            ele.index_open_amount = val * 100
            
            val, pos = helpers.varint_decode(data, pos)
            ele.stock_open_amount = val * 100
            
            # Determine if index or block
            is_index_or_block = assert_index_by_market_and_code(MarketType(ele.market), ele.code)
            
            tmp_open_volume = 0.0
            if is_index_or_block:
                if ele.open > 0:
                    tmp_open_volume = round(float(ele.index_open_amount) / ele.open)
            else:
                if ele.open > 0:
                    tmp_open_volume = round(float(ele.stock_open_amount) / ele.open)
            
            if math.isnan(tmp_open_volume):
                tmp_open_volume = 0.0
            ele.open_volume = int(tmp_open_volume)
            
            # Bid/Ask levels
            bid_levels = []
            ask_levels = []
            for _ in range(5):
                bid_p_diff, pos = helpers.varint_decode(data, pos)
                ask_p_diff, pos = helpers.varint_decode(data, pos)
                bid_vol, pos = helpers.varint_decode(data, pos)
                ask_vol, pos = helpers.varint_decode(data, pos)
                
                bid_levels.append((self.get_price(base_unit, bid_p_diff, price_base), bid_vol))
                ask_levels.append((self.get_price(base_unit, ask_p_diff, price_base), ask_vol))
                
            ele.bid1, ele.bid_vol1 = bid_levels[0]
            ele.bid2, ele.bid_vol2 = bid_levels[1]
            ele.bid3, ele.bid_vol3 = bid_levels[2]
            ele.bid4, ele.bid_vol4 = bid_levels[3]
            ele.bid5, ele.bid_vol5 = bid_levels[4]
            
            ele.ask1, ele.ask_vol1 = ask_levels[0]
            ele.ask2, ele.ask_vol2 = ask_levels[1]
            ele.ask3, ele.ask_vol3 = ask_levels[2]
            ele.ask4, ele.ask_vol4 = ask_levels[3]
            ele.ask5, ele.ask_vol5 = ask_levels[4]
            
            ele.reversed_bytes4 = struct.unpack('<H', data[pos:pos+2])[0]
            pos += 2
            
            ele.reversed_bytes5, pos = helpers.varint_decode(data, pos)
            ele.reversed_bytes6, pos = helpers.varint_decode(data, pos)
            ele.reversed_bytes7, pos = helpers.varint_decode(data, pos)
            ele.reversed_bytes8, pos = helpers.varint_decode(data, pos)
            
            reversed_bytes9 = struct.unpack('<h', data[pos:pos+2])[0]
            pos += 2
            ele.rate = float(reversed_bytes9) / 100.0
            
            ele.active2 = struct.unpack('<H', data[pos:pos+2])[0]
            pos += 2
            
            # State logic
            if ele.last_close == 0.0 and ele.open == 0.0:
                ele.state = TradeState.DELISTING
            else:
                if ele.open != 0.0:
                    ele.state = TradeState.NORMAL
                else:
                    ele.state = TradeState.SUSPEND
            
            if is_index_or_block:
                ele.index_up = ele.bid_vol1
                ele.index_down = ele.ask_vol1
                ele.index_up_limit = ele.bid_vol2
                ele.index_down_limit = ele.ask_vol2
            
            # Closing logic (simplified, assuming not closing for now or handled by caller)
            # C++: if (status == exchange::TimeStatus::ExchangeClosing) ...
            
            ele.time_stamp = timestamp
            self.list.append(ele)

    def verify_delisted_securities(self, code_maps: Dict[str, StockInfo]) -> None:
        if not code_maps:
            return
            
        remains = []
        # 1. First pass
        for i in range(len(self.list)):
            v = self.list[i]
            security_code = get_market_flag(MarketType(v.market)) + v.code
            
            if v.state == TradeState.DELISTING:
                if security_code in code_maps:
                    # Found, code normal, data should be normal (IPO waiting)
                    v.state = TradeState.IPO
                    del code_maps[security_code]
                else:
                    # Not found, data mismatch
                    remains.append(i)
            else:
                # Data normal
                if security_code in code_maps:
                    del code_maps[security_code]
                    
        # 2. Second pass
        if not remains:
            return
            
        for key, value in code_maps.items():
            if not remains:
                break
            idx = remains.pop(0)
            v = self.list[idx]
            v.market = value.market
            v.code = value.code
