import struct
from typing import Dict, List, Tuple
from collections import OrderedDict
from datetime import datetime

from quant1x.log import logger
from .. import protocol
from .helpers import get_datetime
from .command import Command
from quant1x.data.meta import Exchange, Instrument, InstrumentType
from quant1x.data import detect_instrument_type_by_rule
from ..market import find_exchange_by_market_and_category, find_market_by_exchange_and_asset_class


class Synchronize(protocol.BaseMessage):
    """
    协议握手
    """
    def __init__(self):
        super().__init__(Command.EXT_SYNCHRONIZE)
        self.info = ''
        self.success: bool = False
    
    def serialize_request_body(self) -> bytes:
        padding = bytes.fromhex("1f 32 c6 e5 d5 3d fb 41 1f 32 c6 e5 d5 3d fb 41 1f 32 c6 e5 d5 3d fb 41 1f 32 c6 e5 d5 3d fb 41 1f 32 c6 e5 d5 3d fb 41 1f 32 c6 e5 d5 3d fb 41 1f 32 c6 e5 d5 3d fb 41 1f 32 c6 e5 d5 3d fb 41 cc e1 6d ff d5 ba 3f b8 cb c5 7a 05 4f 77 48 ea")
        return padding
        
    def deserialize_response_body(self, data: bytes) -> None:
        logger.debug(f"Synchronize.deserialize_response_body: {data.hex()}")
        _, _, year, month, day, minute, hour, ms, second, server_name, u1, u2, u3, u4, u5, desc, u6, u7, u8, ip = struct.unpack('<B52sHBBBBBB21sfBHHH151sBBB52s', data)
        # print({
        #     "date_time": datetime(year, month, day, hour, minute, second).strftime('%Y-%m-%d %H:%M:%S'),
        #     "server_name": server_name.decode('gbk').replace('\x00', ''),
        #     "desc": desc.decode('gbk').replace('\x00', ''),
        #     "ip": ip.decode('gbk').replace('\x00', ''),
        #     "unknown": [u1, u2, u3, u4, u5, u6, u7, u8]
        # })
        ip = ip.decode('gbk').replace('\x00', '')
        offset = 0
        if len(data) >= offset:
            info_bytes = data[offset:]
            try:
                self.info = info_bytes.decode('gbk', errors='ignore').rstrip('\x00')
            except Exception:
                self.info = info_bytes.decode('utf-8', errors='ignore')
            logger.debug("ExtSynchronizeResponse info={}", self.info)
        self.success = len(ip)>0

class MarketList(protocol.BaseMessage):
    """
    市场信息列表
    """
    def __init__(self):
        super().__init__(Command.EXT_MARKET_LIST)
        self.reply = []

    def serialize_request_body(self) -> bytes:
        padding = bytes()
        return padding

    def deserialize_response_body(self, data: bytes) -> None:
        logger.debug(f"MarketList.deserialize_response_body: {data.hex()}")
        pos = 0
        (cnt, ) = struct.unpack("<H", data[pos: pos + 2])
        pos += 2

        result = []
        for i in range(cnt):
            # 64byte for one
            (category, raw_name, market, raw_short_name, ignore_bytes, unknown_bytes) = struct.unpack("<B32sB2s26s2s", data[pos: pos+64])
            pos += 64

            name = raw_name.decode("gbk")
            short_name = raw_short_name.decode("gbk")

            result.append(OrderedDict(
                [
                    ("market", market),
                    ("category", category),
                    ("name", name.rstrip("\x00")),
                    ("short_name", short_name.rstrip("\x00")),
                    #('ignore_bytes', ignore_bytes),
                    #('unknown_bytes', unknown_bytes)
                ]
            ))
        self.reply = result
        logger.debug("MarketList.deserialize_response_body reply: {}", self.reply)


class InstrumentCount(protocol.BaseMessage):
    """
    市场数量请求
    """
    def __init__(self):
        super().__init__(Command.EXT_INSTRUMENT_COUNT)
        self.reply = 0

    def serialize_request_body(self) -> bytes:
        return b''

    def deserialize_response_body(self, data: bytes) -> None:
        logger.debug(f"[InstrumentCount] deserialize: {data.hex()}")
        pos = 0
        logger.debug(f"[InstrumentCount] deserialize: {data[19: 19+4].hex()}")
        (num,) = struct.unpack("<I", data[19: 19+4])
        logger.debug(f"[InstrumentCount] deserialize: num={num}")
        
        self.reply = num
        logger.debug("[InstrumentCount] reply: {}", self.reply)


class InstrumentInfo(protocol.BaseMessage):
    """
    instrument 信息
    """
    PRE_REQUEST_MAX = 1021
    
    def __init__(self, start: int=0, count=PRE_REQUEST_MAX):
        super().__init__(Command.EXT_INSTRUMENT_INFO)
        
        self.start = start
        self.count = count
        self.reply = {}

    def serialize_request_body(self) -> bytes:
        logger.debug("[InstrumentInfo] start={}, count={}", self.start, self.count)
        body = struct.pack('<I H', self.start, self.count)
        return body
    
    def deserialize_response_body(self, data: bytes) -> None:
        #logger.debug(f"[ExtInstrumentInfoResponse] deserialize: {data.hex()}")
        pos = 0
        start, count = struct.unpack("<IH", data[:6])
        logger.debug(f"[InstrumentInfo] deserialize: start={start}, count={count}")
        pos += 6
        result : List[Instrument] = []
        for i in range(count):
            (category, market, price_precision, lot_size, unused_bytes, code_raw, name_raw, desc_raw) = struct.unpack("<BBBBB9s17s9s", data[pos: pos+40])

            code = code_raw.decode("gbk", 'ignore')
            name = name_raw.decode("gbk", 'ignore')
            desc = desc_raw.decode("gbk", 'ignore')
            
            code = code.rstrip("\x00")
            name = name.rstrip("\x00")
            desc = desc.rstrip("\x00")
            
            try:
                exchange, typ_ = find_exchange_by_market_and_category(market, category)
                inst = Instrument(exchange=exchange, type=typ_, ticker=code, name=name, lot_size=lot_size, price_precision=price_precision, ext_market=market, ext_category=category)
                result.append(inst)
            except Exception as e:
                logger.exception(f"Error processing instrument: {e}, code={code}, name={name}, desc={desc}")
            if unused_bytes != 0:
                logger.warning(f"InstrumentInfo.deserialize_response_body: unused_bytes is not zero: {unused_bytes}, code={code}, name={name}, desc={desc}, lot_size={lot_size}, price_precision={price_precision}, market={market}, category={category}")
            
            pos += 64
            
        self.reply = {'count': count, 'list': result}
        #logger.debug("[InstrumentInfo] reply: {}", self.reply)


class InstrumentBars(protocol.BaseMessage):
    """
    K线数据
    """
    PRE_REQUEST_MAX = 700
    
    def __init__(self, category, market, ticker, start: int=0, count=PRE_REQUEST_MAX):
        super().__init__(Command.EXT_INSTRUMENT_BARS)
        
        self.market = market
        self.ticker = ticker
        self.category = category
        self.frequency = 1
        """通过实验发现, 频率为 1 时, 返回的数据是按照category设定的K线周期连续的数据, 大于1时, 返回的数据是则是在category的基础再聚合的数据"""
        self.start = start
        self.count = count
        self.reply = []
    
    def serialize_request_body(self) -> bytes:
        ticker = self.ticker.encode("utf-8")
        body = struct.pack('<B9sHHIH', self.market, ticker, self.category, self.frequency, self.start, self.count)
        logger.debug(f"[InstrumentBars] serialize: market={self.market}, ticker={self.ticker}, category={self.category}, frequency={self.frequency}, start={self.start}, count={self.count}")
        return body
    
    def deserialize_response_body(self, data: bytes) -> None:
        logger.debug(f"[InstrumentBars] deserialize: {data[:14].hex()}")
        pos = 0
        pos += 14
        start, count = struct.unpack("<IH", data[pos:pos+6])
        logger.debug(f"[InstrumentBars] deserialize: start={start}, count={count}")
        pos += 6
        result = []
        for i in range(count):
            year, month, day, hour, minute, pos = get_datetime(self.category, data, pos)
            second = 0
            (open_price, high, low, close, position, volume, price) = struct.unpack("<ffffIIf", data[pos: pos+28])
            (amount, ) = struct.unpack("f", data[pos+16: pos+16+4])

            pos += 28
            one = OrderedDict([
                ("date","%d-%02d-%02d" % (year, month, day)),
                ("open", open_price),
                ("close", close),
                ("high", high),
                ("low", low),
                ("position", position),
                ("volume", volume),
                ("price", price),
                #("year", year),
                #("month", month),
                #("day", day),
                #("hour", hour),
                #("minute", minute),
                ("amount", amount),
                ("timestamp", "%d-%02d-%02d %02d:%02d:%02d" % (year, month, day, hour, minute, second)),
            ])
            result.append(one)
        
        self.reply = result
        #logger.debug("[ExtInstrumentBarResponse] reply: {}", self.reply)