# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import struct
from enum import Enum
from typing import List

from .command import FLAG_UNCOMPRESSED
from .command import Command
from . import helpers

from .. import protocol

class Synchronize1(protocol.BaseMessage):
    """第一次协议握手（合并Request和Response）"""
    def __init__(self):
        super().__init__(Command.STD_SYNCHRONIZE1)
        self.info = ""
        self._padding = bytes.fromhex("01")

    def serialize_request_body(self) -> bytes:
        return self._padding

    def deserialize_response_body(self, data: bytes) -> None:
        offset = 68
        if len(data) >= offset:
            info_bytes = data[offset:]
            try:
                self.info = info_bytes.decode('gbk', errors='ignore').rstrip('\x00')
            except Exception:
                self.info = info_bytes.decode('utf-8', errors='ignore')






class Synchronize2(protocol.BaseMessage):
    """第二次协议握手（合并Request和Response）"""
    def __init__(self):
        super().__init__(Command.STD_SYNCHRONIZE2)
        self.info = ""
        self._padding = bytes.fromhex("d5d0c9ccd6a4a8af0000008fc22540130000d500c9ccbdf0d7ea00000002")

    def serialize_request_body(self) -> bytes:
        return self._padding

    def deserialize_response_body(self, data: bytes) -> None:
        offset = 58
        if len(data) >= offset:
            info_bytes = data[offset:]
            try:
                self.info = info_bytes.decode('gbk', errors='ignore').rstrip('\x00')
            except Exception:
                self.info = info_bytes.decode('utf-8', errors='ignore')





class Heartbeat(protocol.BaseMessage):
    """心跳（合并Request和Response）"""
    def __init__(self):
        super().__init__(Command.STD_HEARTBEAT)
        self.info = ""
        self.request_header.packet_type = 0x02

    def serialize_request_body(self) -> bytes:
        return b""

    def deserialize_response_body(self, data: bytes) -> None:
        if len(data) >= 10:
            s = data[:10]
        else:
            s = data
        try:
            self.info = s.decode('gbk', errors='ignore').split('\x00', 1)[0]
        except Exception:
            self.info = s.decode('utf-8', errors='ignore').split('\x00', 1)[0]





from quant1x.data.meta import Exchange, Instrument
from quant1x.data.market import detect_instrument_type_by_rule
from quant1x.log import logger

SECURITY_LIST_PRE_REQUEST_MAX = 1600 # 预请求最大数量

class SecurityList(protocol.BaseMessage):
    """证券列表（合并Request和Response）"""
    def __init__(self, exchange: Exchange, start: int = 0, count: int = 0):
        super().__init__(Command.STD_SECURITY_LIST)
        self.exchange = exchange
        self.start = start
        self.count = count
        self.list: list[Instrument] = []

    def serialize_request_body(self) -> bytes:
        market_id = helpers.exchange_to_market(self.exchange)
        return struct.pack('<H I I I',
                           int(market_id) & 0xFFFF,
                           int(self.start) & 0xFFFFFFFF,
                           int(self.count) & 0xFFFFFFFF,
                           0)

    def deserialize_response_body(self, data: bytes) -> None:
        self.list.clear()
        if not data:
            return

        offset = 0
        if len(data) < 2:
            return
        (cnt,) = struct.unpack_from('<H', data, offset)
        offset += 2
        for _ in range(cnt):
            if offset + 25 > len(data):
                logger.warning('Insufficient data when parsing SECURITY_LIST payload')
                break
            code_bytes = data[offset:offset+6]
            offset += 6
            (vol_unit,) = struct.unpack_from('<H', data, offset)
            offset += 2
            name_buf = data[offset:offset+16]
            offset += 16
            offset += 4  # 保留字段
            (decimal_point,) = struct.unpack_from('<B', data, offset)
            offset += 1
            (tmp_u32,) = struct.unpack_from('<I', data, offset)
            offset += 4
            offset += 4  # 保留/未使用

            try:
                code = code_bytes.decode('ascii', errors='ignore').rstrip('\x00')
            except Exception:
                code = code_bytes.decode('utf-8', errors='ignore').rstrip('\x00')
            try:
                nul_pos = name_buf.index(0)
            except ValueError:
                nul_pos = len(name_buf)
            try:
                name = name_buf[:nul_pos].decode('gbk', errors='ignore')
            except Exception:
                name = name_buf[:nul_pos].decode('utf-8', errors='ignore')

            pre_close = helpers.int_to_float64(tmp_u32)
            _ = pre_close
            typ_ = detect_instrument_type_by_rule(self.exchange, code)
            inst = Instrument(exchange=self.exchange, type=typ_, ticker=code, name=name,
                              lot_size=vol_unit, price_precision=decimal_point,
                              ext_market=helpers.exchange_to_market(self.exchange),
                              ext_category=typ_.value)
            self.list.append(inst)

        logger.debug('security_list fetched market={} start={} count={} parsed={}',
                     self.exchange, 0, cnt, len(self.list))




from quant1x.data.meta import Exchange
from quant1x.data.schema import Bar

SECURITY_BARS_PRE_REQUEST_MAX = 700#800

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

class SecurityBars(protocol.BaseMessage):
    """K线数据（合并Request和Response）"""
    def __init__(self, exchange: Exchange, code: str, category: KLineType, start: int, count: int, is_index: bool = False):
        super().__init__(Command.STD_SECURITY_BARS)
        self.request_header.packet_type = 0x00
        self._category = category
        self._i = 1
        self._start = start
        self._count = count
        self._market = helpers.exchange_to_market(exchange)
        self._code = code
        self._padding = bytes.fromhex("00000000000000000000")
        self._is_index = is_index

        self.count = 0
        self.list: List[Bar] = []

    def serialize_request_body(self) -> bytes:
        code_bytes = self._code.encode('ascii')
        if len(code_bytes) < 6:
            code_bytes = code_bytes + b'\x00' * (6 - len(code_bytes))
        else:
            code_bytes = code_bytes[:6]
        body = struct.pack('<H 6s H H H H',
                           self._market, code_bytes, self._category.value,
                           self._i, self._start, self._count)
        return body + self._padding

    def deserialize_response_body(self, data: bytes) -> None:
        self.list.clear()
        if len(data) < 2:
            return
        self.count = struct.unpack('<H', data[:2])[0]
        pos = 2
        pre_diff_base = 0
        cat = self._category.value

        for _ in range(self.count):
            if pos >= len(data):
                break
            e = Bar()
            year = month = day = 0
            hour = 15
            minute = 0

            if cat < 4 or cat == 7 or cat == 8:
                if pos + 4 > len(data): break
                zipday = struct.unpack('<H', data[pos:pos+2])[0]
                pos += 2
                tminutes = struct.unpack('<H', data[pos:pos+2])[0]
                pos += 2
                year, month, day, hour, minute = helpers.get_datetime_from_uint32(cat, zipday, tminutes)
            else:
                if pos + 4 > len(data): break
                zipday = struct.unpack('<I', data[pos:pos+4])[0]
                pos += 4
                year = int(zipday / 10000)
                month = int((zipday % 10000) / 100)
                day = int(zipday % 100)

            e.date = f"{year:04d}-{month:02d}-{day:02d}"
            e.timestamp = f"{year:04d}-{month:02d}-{day:02d} {hour:02d}:{minute:02d}:00"

            price_open_diff, pos = helpers.varint_decode(data, pos)
            price_close_diff, pos = helpers.varint_decode(data, pos)
            price_high_diff, pos = helpers.varint_decode(data, pos)
            price_low_diff, pos = helpers.varint_decode(data, pos)

            if pos + 8 > len(data): break
            ivol = struct.unpack('<I', data[pos:pos+4])[0]
            pos += 4
            e.volume = helpers.int_to_float64(ivol)
            dbvol = struct.unpack('<I', data[pos:pos+4])[0]
            pos += 4
            e.amount = helpers.int_to_float64(dbvol)

            e.open = float(price_open_diff + pre_diff_base) / 1000.0
            price_open_diff += pre_diff_base
            e.close = float(price_open_diff + price_close_diff) / 1000.0
            e.high = float(price_open_diff + price_high_diff) / 1000.0
            e.low = float(price_open_diff + price_low_diff) / 1000.0
            pre_diff_base = price_open_diff + price_close_diff

            if self._is_index:
                if pos + 4 > len(data): break
                e.up = struct.unpack('<H', data[pos:pos+2])[0]
                pos += 2
                e.down = struct.unpack('<H', data[pos:pos+2])[0]
                pos += 2

            self.list.append(e)

from quant1x.data.meta import Exchange
from quant1x.data.schema import Transaction as TransactionRecord

# Constants
TICK_BUY = 0
TICK_SELL = 1
TICK_NEUTRAL = 2
TICK_UNKNOWN = 3

TICK_TRANSACTION_PER_REQUEST_MAX = 1800

class Transaction(protocol.BaseMessage):
    """逐笔成交数据（合并Request和Response）"""
    def __init__(self, exchange: Exchange, code: str, start: int, count: int,
                 price_precision: int = 2, is_index: bool = False):
        super().__init__(Command.STD_TRANSACTION_DATA)
        self.request_header.packet_type = 0x00
        self._market = helpers.exchange_to_market(exchange)
        self._code = code
        self._start = start
        self._count = count
        self._price_precision = price_precision
        self._is_index = is_index

        self.count = 0
        self.list: list[TransactionRecord] = []

    def serialize_request_body(self) -> bytes:
        code_bytes = self._code.encode('ascii')
        if len(code_bytes) < 6:
            code_bytes = code_bytes + b'\x00' * (6 - len(code_bytes))
        else:
            code_bytes = code_bytes[:6]
        return struct.pack('<H 6s H H', self._market, code_bytes, self._start, self._count)

    def deserialize_response_body(self, data: bytes) -> None:
        self.list.clear()
        if len(data) < 2:
            return
        self.count = struct.unpack('<H', data[:2])[0]
        pos = 2
        base_unit = 10 ** self._price_precision
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
            if self._is_index:
                amount = float(vol * 100)
                vol = int(amount / price) if price > 0 else 0
            else:
                vol *= 100
                amount = float(vol) * price
            _, pos = helpers.varint_decode(data, pos)
            self.list.append(TransactionRecord(time=time_str, price=price, volume=vol, num=num, amount=amount, direction=buy_or_sell))

class HistoricalTransaction(protocol.BaseMessage):
    """历史逐笔成交数据（合并Request和Response）"""
    def __init__(self, exchange: Exchange, code: str, date: int, start: int, count: int,
                 price_precision: int = 2, is_index: bool = False):
        super().__init__(Command.STD_HISTORY_TRANSACTION_DATA)
        self.request_header.packet_type = 0x00
        self._market = helpers.exchange_to_market(exchange)
        self._code = code
        self._date = date
        self._start = start
        self._count = count
        self._price_precision = price_precision
        self._is_index = is_index

        self.count = 0
        self.list: list[TransactionRecord] = []

    def serialize_request_body(self) -> bytes:
        code_bytes = self._code.encode('ascii')
        if len(code_bytes) < 6:
            code_bytes = code_bytes + b'\x00' * (6 - len(code_bytes))
        else:
            code_bytes = code_bytes[:6]
        return struct.pack('<I H 6s H H', self._date, self._market, code_bytes, self._start, self._count)

    def deserialize_response_body(self, data: bytes) -> None:
        self.list.clear()
        if len(data) < 2:
            return
        self.count = struct.unpack('<H', data[:2])[0]
        pos = 2
        pos += 4  # C++: bs.skip(4); 历史分笔成交记录, 跳过4个字节
        base_unit = 10 ** self._price_precision
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
            buy_or_sell, pos = helpers.varint_decode(data, pos)
            last_price += raw_price
            price = float(last_price) / base_unit
            amount = 0.0
            if self._is_index:
                amount = float(vol * 100)
                vol = int(amount / price) if price > 0 else 0
            else:
                vol *= 100
                amount = float(vol) * price
            _, pos = helpers.varint_decode(data, pos)
            self.list.append(TransactionRecord(time=time_str, price=price, volume=vol, num=0, amount=amount, direction=buy_or_sell))


from quant1x.data.meta import Exchange
from quant1x.data.schema import XdxrInfo, XdxrCategory

class Xdxr(protocol.BaseMessage):
    """除权除息信息（合并Request和Response）"""
    def __init__(self, exchange: Exchange, code: str):
        super().__init__(Command.STD_XDXR_INFO)
        self._market = helpers.exchange_to_market(exchange)
        self._code = code
        self._padding = bytes.fromhex('0100')

        self.count = 0
        self.list: List[XdxrInfo] = []

    def serialize_request_body(self) -> bytes:
        code_bytes = self._code.encode('ascii')
        if len(code_bytes) < 6:
            code_bytes = code_bytes + b'\x00' * (6 - len(code_bytes))
        else:
            code_bytes = code_bytes[:6]
        return struct.pack('<2s B 6s', self._padding, self._market, code_bytes)

    def deserialize_response_body(self, data: bytes) -> None:
        self.list.clear()
        if len(data) < 9:
            return
        pos = 9
        if pos + 2 > len(data):
            return
        self.count = struct.unpack('<H', data[pos:pos+2])[0]
        pos += 2
        for _ in range(self.count):
            if pos + 29 > len(data):
                break
            pos += 1  # Market
            pos += 6  # Code
            pos += 1  # Unknown
            date_int = struct.unpack('<I', data[pos:pos+4])[0]
            pos += 4
            category = struct.unpack('<B', data[pos:pos+1])[0]
            pos += 1
            record_data = data[pos:pos+16]
            pos += 16
            year, month, day, _, _ = helpers.get_datetime_from_uint32(9, date_int, 0)
            info = XdxrInfo()
            info.Category = category
            info.Date = f"{year:04d}-{month:02d}-{day:02d}"
            info.Name = XdxrCategory.to_string(category)
            if category == 1:
                info.FenHong = struct.unpack('<f', record_data[0:4])[0]
                info.PeiGuJia = struct.unpack('<f', record_data[4:8])[0]
                info.SongZhuanGu = struct.unpack('<f', record_data[8:12])[0]
                info.PeiGu = struct.unpack('<f', record_data[12:16])[0]
            elif category in [11, 12]:
                info.SuoGu = struct.unpack('<f', record_data[8:12])[0]
            elif category in [13, 14]:
                info.XingQuanJia = struct.unpack('<f', record_data[0:4])[0]
                info.FenShu = struct.unpack('<f', record_data[12:16])[0]
            else:
                v1 = struct.unpack('<I', record_data[0:4])[0]
                info.QianLiuTong = self._get_v(v1)
                v2 = struct.unpack('<I', record_data[4:8])[0]
                info.QianZongGuBen = self._get_v(v2)
                v3 = struct.unpack('<I', record_data[8:12])[0]
                info.HouLiuTong = self._get_v(v3)
                v4 = struct.unpack('<I', record_data[12:16])[0]
                info.HouZongGuBen = self._get_v(v4)
            self.list.append(info)

    @staticmethod
    def _get_v(v: int) -> float:
        if v == 0:
            return 0.0
        return helpers.int_to_float64(v)



BLOCK_CHUNKS_SIZE = 0x7530

class BlockInfo(protocol.BaseMessage):
    """板块数据（合并Request和Response）"""
    def __init__(self, filename: str, offset: int):
        super().__init__(Command.STD_BLOCK_DATA)
        self._filename = filename
        self._offset = offset
        self._chunk_size = BLOCK_CHUNKS_SIZE

        self.size = 0
        self.data = bytearray()

    def serialize_request_body(self) -> bytes:
        filename_bytes = self._filename.encode('ascii')[:100].ljust(100, b'\x00')
        return struct.pack('<I I', self._offset, self._chunk_size) + filename_bytes

    def deserialize_response_body(self, data: bytes) -> None:
        self.data = bytearray()
        if len(data) < 4:
            return
        self.size = struct.unpack('<I', data[:4])[0]
        if self.size > 0:
            self.data = bytearray(data[4:])

