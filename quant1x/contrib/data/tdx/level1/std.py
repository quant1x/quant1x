# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import struct
from enum import Enum
from typing import List

from .command import FLAG_UNCOMPRESSED
from .command import Command
from . import helpers

class Synchronize1Request:
    """
    第一次协议握手请求
    """
    def __init__(self):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = helpers.msg_sequence_id()
        self.packet_type = 0x01
        self.pkg_len1 = 0
        self.pkg_len2 = 0
        self.method = Command.STD_SYNCHRONIZE1.value
        self.padding = bytes.fromhex("01")

    def serialize(self) -> bytes:
        self.pkg_len1 = 2 + len(self.padding)
        self.pkg_len2 = self.pkg_len1
        
        header = struct.pack('<B I B H H H', self.zip_flag, self.seq_id, self.packet_type, self.pkg_len1, self.pkg_len2, self.method)
        return header + self.padding

class Synchronize1Response:
    """
    第一次协议握手响应
    """
    def __init__(self):
        self.info = ""

    def deserialize(self, data: bytes) -> None:
        offset = 68
        if len(data) >= offset:
            info_bytes = data[offset:]
            try:
                self.info = info_bytes.decode('gbk', errors='ignore').rstrip('\x00')
            except Exception:
                self.info = info_bytes.decode('utf-8', errors='ignore')



class Synchronize2Request:
    """
    第二次协议握手请求
    """
    def __init__(self):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = helpers.msg_sequence_id()
        self.packet_type = 0x01
        self.pkg_len1 = 0
        self.pkg_len2 = 0
        self.method = Command.STD_SYNCHRONIZE2.value
        self.padding = bytes.fromhex("d5d0c9ccd6a4a8af0000008fc22540130000d500c9ccbdf0d7ea00000002")
        # padding = bytearray()
        # padding.extend(bytes.fromhex("d5d0c9ccd6a4a8af0000008fc22540130000d500c9ccbdf0d7ea00000002"))

    def serialize(self) -> bytes:
        self.pkg_len1 = 2 + len(self.padding)
        self.pkg_len2 = self.pkg_len1
        
        header = struct.pack('<B I B H H H', self.zip_flag, self.seq_id, self.packet_type, self.pkg_len1, self.pkg_len2, self.method)
        return header + self.padding

class Synchronize2Response:
    """
    第二次协议握手响应
    """
    def __init__(self):
        self.info = ""

    def deserialize(self, data: bytes) -> None:
        offset = 58
        if len(data) >= offset:
            info_bytes = data[offset:]
            try:
                self.info = info_bytes.decode('gbk', errors='ignore').rstrip('\x00')
            except Exception:
                self.info = info_bytes.decode('utf-8', errors='ignore')


class HeartbeatRequest:
    """
    心跳请求
    """
    def __init__(self):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = helpers.msg_sequence_id()
        self.packet_type = 0x02
        self.pkg_len1 = 0
        self.pkg_len2 = 0
        self.method = Command.STD_HEARTBEAT.value

    def serialize(self) -> bytes:
        self.pkg_len1 = 2
        self.pkg_len2 = 2
        
        header = struct.pack('<B I B H H H', self.zip_flag, self.seq_id, self.packet_type, self.pkg_len1, self.pkg_len2, self.method)
        return header

class HeartbeatResponse:
    """
    心跳响应
    """
    def __init__(self):
        self.info = ""

    def deserialize(self, data: bytes) -> None:
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

class SecurityListRequest:
    def __init__(self, exchange: Exchange, start, count):
        self.market_id = helpers.exchange_to_market(exchange)
        self.start = start
        self.count = count
    
    def serialize(self):
        payload = struct.pack('<H I I I', int(self.market_id) & 0xFFFF, int(self.start) & 0xFFFFFFFF, int(self.count) & 0xFFFFFFFF, 0)
        zip_flag = FLAG_UNCOMPRESSED
        seq_id = helpers.msg_sequence_id()
        packet_type = 0x01
        pkg_len1 = 2 + len(payload)
        pkg_len2 = pkg_len1
        method = Command.STD_SECURITY_LIST.value
        header = struct.pack('<B I B H H H', zip_flag, seq_id, packet_type, pkg_len1, pkg_len2, method)
        return header + payload

class SecurityListResponse:
    def __init__(self, exchange: Exchange):
        self.exchange = exchange
        self.list: list[Instrument] = []
    
    def deserialize(self, data):
        if not data:
            # 响应体为空 -> 表示没有证券记录
            return

        # 解析：先读取 u16 的计数，然后依次解析记录
        offset = 0
        if len(data) < 2:
            return
        (cnt,) = struct.unpack_from('<H', data, offset)
        offset += 2
        # 每条记录至少为 25 字节（与 Rust 实现一致）
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
            # 跳过 4 字节（保留字段）
            offset += 4
            (decimal_point,) = struct.unpack_from('<B', data, offset)
            offset += 1
            (tmp_u32,) = struct.unpack_from('<I', data, offset)
            offset += 4
            # 跳过最后 4 字节（保留/未使用）
            offset += 4

            # 解码代码和名称字段
            try:
                code = code_bytes.decode('ascii', errors='ignore').rstrip('\x00')
            except Exception:
                code = code_bytes.decode('utf-8', errors='ignore').rstrip('\x00')
            # 名称使用 GBK 编码，直到第一个 NUL 字节为止
            try:
                nul_pos = name_buf.index(0)
            except ValueError:
                nul_pos = len(name_buf)
            try:
                name = name_buf[:nul_pos].decode('gbk', errors='ignore')
            except Exception:
                name = name_buf[:nul_pos].decode('utf-8', errors='ignore')

            # 解码前收盘价
            pre_close = helpers.int_to_float64(tmp_u32)
            _ = pre_close # 避免未使用警告
            typ_ = detect_instrument_type_by_rule(self.exchange, code)
            inst = Instrument(exchange=self.exchange, type=typ_, ticker=code, name=name, lot_size=vol_unit, price_precision=decimal_point, ext_market=helpers.exchange_to_market(self.exchange), ext_category=typ_.value)
            self.list.append(inst)

        logger.debug('security_list fetched market={} start={} count={} parsed={}', self.exchange, 0, cnt, len(self.list))

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

class SecurityBarsRequest:
    def __init__(self, exchange: Exchange, code: str, category: KLineType, start: int, count: int):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = helpers.msg_sequence_id()
        self.packet_type = 0x00
        self.method = Command.STD_SECURITY_BARS.value
        
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

from quant1x.data.meta import Exchange
from quant1x.data.schema import Transaction

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
        self.method = Command.STD_TRANSACTION_DATA.value
        
    
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
        self.method = Command.STD_HISTORY_TRANSACTION_DATA.value
        
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


from quant1x.data.meta import Exchange
from quant1x.data.schema import XdxrInfo, XdxrCategory

class XdxrInfoRequest:
    def __init__(self, exchange: Exchange, code: str):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = helpers.msg_sequence_id()
        self.packet_type = 0x01
        self.method = Command.STD_XDXR_INFO.value
        
        self.market = helpers.exchange_to_market(exchange)
        self.code = code
        self.padding = bytes.fromhex('0100')

    def serialize(self) -> bytes:
        # Body: padding(2) + Market(1) + Code(6) = 9 bytes
        # PkgLen = BodyLen + 2 = 11
        body_len = 2 + 1 + 6
        pkg_len = body_len + 2
        
        header = struct.pack('<B I B H H H', 
                             self.zip_flag, self.seq_id, self.packet_type, 
                             pkg_len, pkg_len, self.method)
        
        code_bytes = self.code.encode('ascii')
        if len(code_bytes) < 6:
            code_bytes = code_bytes + b'\x00' * (6 - len(code_bytes))
        else:
            code_bytes = code_bytes[:6]
            
        body = struct.pack('<2s B 6s', self.padding, self.market, code_bytes)
        return header + body

class XdxrInfoResponse:
    def __init__(self):
        self.count = 0
        self.list: List[XdxrInfo] = []

    def deserialize(self, data: bytes):
        if len(data) < 9:
            return
            
        pos = 9
        if pos + 2 > len(data):
            return
            
        self.count = struct.unpack('<H', data[pos:pos+2])[0]
        pos += 2
        
        for _ in range(self.count):
            if pos + 29 > len(data): # 1+6+1+4+1+16 = 29 bytes per record
                break
                
            # Market(1), Code(6), Unknown(1), Date(4), Category(1), Data(16)
            pos += 1 # Market
            pos += 6 # Code
            pos += 1 # Unknown
            
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
            
            if category == 1: # 除权除息
                info.FenHong = struct.unpack('<f', record_data[0:4])[0]
                info.PeiGuJia = struct.unpack('<f', record_data[4:8])[0]
                info.SongZhuanGu = struct.unpack('<f', record_data[8:12])[0]
                info.PeiGu = struct.unpack('<f', record_data[12:16])[0]
            elif category in [11, 12]:
                # Skip 8 bytes
                info.SuoGu = struct.unpack('<f', record_data[8:12])[0]
            elif category in [13, 14]:
                info.XingQuanJia = struct.unpack('<f', record_data[0:4])[0]
                # Skip 8 bytes (4-12)
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

    def _get_v(self, v: int) -> float:
        if v == 0:
            return 0.0
        return helpers.int_to_float64(v)



BLOCK_CHUNKS_SIZE = 0x7530

class BlockInfoRequest:
    """
    板块数据请求
    """
    def __init__(self, filename: str, offset: int):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = helpers.msg_sequence_id()
        self.packet_type = 0x01
        self.pkg_len1 = 0
        self.pkg_len2 = 0
        self.method = Command.STD_BLOCK_DATA.value
        
        self.start = offset
        self.size = BLOCK_CHUNKS_SIZE
        self.block_filename = filename

    def serialize(self) -> bytes:
        # Body: Start(4) + Size(4) + BlockFilename(100) = 108 bytes
        # PkgLen = Body + 2 = 110 (0x6E)
        self.pkg_len1 = 0x6E
        self.pkg_len2 = 0x6E
        
        header = struct.pack('<B I B H H H', self.zip_flag, self.seq_id, self.packet_type, self.pkg_len1, self.pkg_len2, self.method)
        
        # Ensure filename is 100 bytes
        filename_bytes = self.block_filename.encode('ascii')[:100].ljust(100, b'\x00')
        
        body = struct.pack('<I I', self.start, self.size) + filename_bytes
        return header + body

class BlockInfoResponse:
    """
    板块数据响应
    """
    def __init__(self):
        self.size = 0
        self.data = bytearray()

    def deserialize(self, data: bytes) -> None:
        if len(data) < 4:
            return
            
        self.size = struct.unpack('<I', data[:4])[0]
        if self.size > 0:
            # The rest is data
            self.data = bytearray(data[4:])

