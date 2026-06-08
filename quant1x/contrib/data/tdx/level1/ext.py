# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import struct
from typing import Dict, List, Tuple
from collections import OrderedDict
from datetime import datetime

from quant1x.log import logger
from .. import protocol
from ..helpers import get_datetime
from ..command import Command, QuoteType
from quant1x.data.meta import Exchange, Instrument, InstrumentType
from quant1x.data import detect_instrument_type_by_rule
from ..market import find_exchange_by_market_and_category, find_market_by_exchange_and_asset_class
from quant1x.data.meta.ticker_rules.market_usa import usa_code_to_ticker
from .. import helpers

class Synchronize(protocol.BaseMessage):
    """
    协议握手
    """
    def __init__(self):
        super().__init__(Command.EXT_SYNCHRONIZE, flags=0x01)
        self.info = ''
        self.success: bool = False
    
    def serialize_request_body(self) -> bytes:
        #padding = bytes.fromhex("1f 32 c6 e5 d5 3d fb 41 1f 32 c6 e5 d5 3d fb 41 1f 32 c6 e5 d5 3d fb 41 1f 32 c6 e5 d5 3d fb 41 1f 32 c6 e5 d5 3d fb 41 1f 32 c6 e5 d5 3d fb 41 1f 32 c6 e5 d5 3d fb 41 1f 32 c6 e5 d5 3d fb 41 cc e1 6d ff d5 ba 3f b8 cb c5 7a 05 4f 77 48 ea")
        padding = bytearray.fromhex('' \
        'e5bb1c2fafe52594' \
        '1f32c6e5d53dfb41' \
        '5b734cc9cdbf0ac9' \
        '2021bfdd1eb06d22' \
        'd008884c1611cb13' \
        '78f6abd824d899d2' \
        '1f32c6e5d53dfb41' \
        '1f32c6e5d53dfb41' \
        'a9325ac935dc0837' \
        '335a16e4ce17c1bb')
        return padding
        
    def deserialize_response_body(self, data: bytes) -> None:
        logger.debug(f"Synchronize.deserialize_response_body: {data.hex()}")
        result_code, result_message, year, month, day, minute, hour, ms, second, server_name, u1, u2, u3, u4, u5, desc, u6, u7, u8, ip = struct.unpack('<B52sHBBBBBB21sfBHHH151sBBB52s', data)
        
        result_message = result_message.decode('gbk', errors='ignore').rstrip('\x00')
        date_time = datetime(year, month, day, hour, minute, second).strftime('%Y-%m-%d %H:%M:%S')
        server_name = server_name.decode('gbk').replace('\x00', '')
        desc = desc.decode('gbk').replace('\x00', '')
        ip = ip.decode('gbk').replace('\x00', '')
        unknown = [u1, u2, u3, u4, u5, u6, u7, u8, ms]
        logger.debug(f"Synchronize response: result_code={result_code}, result_message={result_message}, date_time={date_time}, server_name={server_name}, desc={desc}, ip={ip}, unknown={unknown}")
        
        # offset = 0
        # if len(data) >= offset:
        #     info_bytes = data[offset:]
        #     try:
        #         self.info = info_bytes.decode('gbk', errors='ignore').replace('\x00', '')
        #     except Exception:
        #         self.info = info_bytes.decode('utf-8', errors='ignore').replace('\x00', '')
        #     logger.debug("Synchronize.deserialize_response_body info={}", self.info)
        self.success = result_code>0

class Synchronize2(protocol.BaseMessage):
    """
    协议握手
    """
    def __init__(self):
        super().__init__(Command.EXT_SYNCHRONIZE2, flags=0x01)
        self.reply = ''
        self.success: bool = False
    
    def serialize_request_body(self) -> bytes:
        padding = b''
        return padding
        
    def deserialize_response_body(self, data: bytes) -> None:
        logger.debug(f"Synchronize2.deserialize_response_body: {data.hex()}")
        maybe_delay, u2, u3, u4, info, version = struct.unpack('<4I25s29s', data[:70])
        u5, u6, u7, u8, u9, date_now, time_now, f1, f2, u15, u16, u17, u18, date2, date3, date4, u22 = struct.unpack('<HHHHHIIffHHHBIIIH', data[70:117])
        server_sign, maybe_switch = struct.unpack('<13sB', data[117:131])
        
        name, = struct.unpack('<30s', data[159:189])
        a, u23, date5, s0, u24, date6, s1, u25, date7, date8 = struct.unpack('<18s5IB3I', data[189:240])
        server_sign2, = struct.unpack('<13s', data[240:253])
        u26, date9, date10, s2, date11, date12, date13, s3, u28, s4, u29 = struct.unpack('<IIIBIIIBfBH', data[253:286])
        date14, u30, date15, u31 = struct.unpack('<IfIf', data[311:327])
        
        time_now = datetime(date_now // 10000, date_now % 10000 // 100, date_now % 100, time_now // 10000, time_now % 10000 // 100, time_now % 100)
        print({
            "delay": maybe_delay,
            "info": info.decode('gbk').replace('\x00', ''),
            "version": version.decode('gbk').replace('\x00', ''),
            "server_sign": server_sign.decode('gbk').replace('\x00', ''),
            "time_now": time_now.strftime('%Y-%m-%d %H:%M:%S'),
            "server_sign2": server_sign2.decode('gbk').replace('\x00', ''),
            "name": name.decode('gbk').replace('\x00', '')
        })
        result_code = 0  # 需要定义result_code变量
        offset = 0
        if len(data) >= offset:
            info_bytes = data[offset:]
            try:
                self.info = info_bytes.decode('gbk', errors='ignore').replace('\x00', '')
            except Exception:
                self.info = info_bytes.decode('utf-8', errors='ignore').replace('\x00', '')
            logger.debug("ExtSynchronizeResponse info={}", self.info)
        self.success = result_code>0


class MarketList(protocol.BaseMessage):
    """
    市场信息列表
    """
    def __init__(self):
        super().__init__(Command.EXT_MARKET_LIST, flags=0x01)
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
        super().__init__(Command.EXT_INSTRUMENT_COUNT, flags=0x01)
        self.reply = {}

    def serialize_request_body(self) -> bytes:
        return b''

    def deserialize_response_body(self, data: bytes) -> None:
        # 31个字节
        logger.debug(f"[InstrumentCount] deserialize: len={len(data)}, data={data.hex()}")
        # pos = 19
        # logger.debug(f"[InstrumentCount] deserialize: {data[pos:pos+4].hex()}")
        # (num,) = struct.unpack("<I", data[pos: pos+4])
        # logger.debug(f"[InstrumentCount] deserialize: num={num}")
        (name, reversed1, reversed2, num, reversed3, reversed4) = struct.unpack("<11s5I", data[:31])
        name = name.decode("gbk").rstrip("\x00")
        logger.debug(f"[InstrumentCount] deserialize: name={name}, num={num}, ignore={reversed1}, {reversed2}, {reversed3}, {reversed4}")
        
        self.reply = {
            "source": name,
            "count": num
        }
        logger.debug("[InstrumentCount] reply: {}", self.reply)

class InstrumentInfo(protocol.BaseMessage):
    """
    instrument 信息
    """
    PRE_REQUEST_MAX = 1021
    
    def __init__(self, start: int=0, count=PRE_REQUEST_MAX):
        super().__init__(Command.EXT_INSTRUMENT_INFO, flags=0x01)
        
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
            
            code = code.replace('\x00', '')
            name = name.replace('\x00', '')
            desc = desc.replace('\x00', '')
            
            try:
                exchange, typ_ = find_exchange_by_market_and_category(market, category)
                ticker = ''
                if market == 12 and category == 5:
                    ticker = usa_code_to_ticker(code)
                inst = Instrument(exchange=exchange,
                                  type=typ_,
                                  ticker=code if ticker == '' else ticker,
                                  name=name,
                                  lot_size=lot_size,
                                  price_precision=price_precision,
                                  ext_market=market,
                                  ext_category=category,
                                  alias_ticker=code)
                result.append(inst)
            except Exception as e:
                logger.exception(f"Error processing instrument: {e}, code={code}, name={name}, desc={desc}")
            if unused_bytes != 0:
                logger.warning(f"InstrumentInfo.deserialize_response_body: unused_bytes is not zero: {unused_bytes}, code={code}, name={name}, desc={desc}, lot_size={lot_size}, price_precision={price_precision}, market={market}, category={category}")
            
            pos += 64
            
        self.reply = {'count': count, 'list': result}
        #logger.debug("[InstrumentInfo] reply: {}", self.reply)

class InstrumentQuote1(protocol.BaseMessage):
    """即时行情"""
    def __init__(self, market, ticker: str):
        super().__init__(Command.EXT_INSTRUMENT_QUOTE_X1, flags=0x01)
        self.market = market
        self.ticker = ticker
        self.reply = []
        
    def serialize_request_body(self) -> bytes:
        pkg = bytearray()
        code = self.ticker.encode("utf-8")
        pkg.extend(struct.pack('<B9s', self.market, code))
        return pkg

    def deserialize_response_body(self, data: bytes) -> None:
        if (len(data) < 20):
            return

        pos = 0
        market, code = struct.unpack('<B9s', data[pos: pos+10])
        pos += 10

        # jump 4
        pos += 4

        ## 持仓 ((13340,), 66),

        (pre_close, open_price, high, low, price, kaicang, _,
         zongliang, xianliang, _ , neipan, waipai,
         _, chicang,
         b1, b2, b3, b4, b5,
         bv1, bv2, bv3, bv4, bv5,
         a1, a2, a3, a4, a5,
         av1, av2, av3, av4, av5
         ) = struct.unpack('<fffffIIIIIIIIIfffffIIIIIfffffIIIII', data[pos: pos+136])


        one = {
            'market': market,
            'code': code.decode("utf-8").rstrip('\x00'),
            'pre_close': pre_close,
            'open': open_price,
            'high': high,
            'low': low,
            'price': price,
            'kaicang': kaicang,
            'zongliang': zongliang,
            'xianliang': xianliang,
            'neipan': neipan,
            'waipan': waipai,
            'chicang': chicang,
            'bid1': b1,
            'bid2': b2,
            'bid3': b3,
            'bid4': b4,
            'bid5': b5,
            'bid_vol1': bv1,
            'bid_vol2': bv2,
            'bid_vol3': bv3,
            'bid_vol4': bv4,
            'bid_vol5': bv5,
            'ask1': a1,
            'ask2': a2,
            'ask3': a3,
            'ask4': a4,
            'ask5': a5,
            'ask_vol1': av1,
            'ask_vol2': av2,
            'ask_vol3': av3,
            'ask_vol4': av4,
            'ask_vol5': av5,
        }
        logger.debug("[InstrumentQuote1] deserialize: {}", one)
        self.reply.append(one)

class InstrumentQuote2(protocol.BaseMessage):
    """即时行情"""
    def __init__(self, futures: list[tuple[int, str]]):
        super().__init__(Command.EXT_INSTRUMENT_QUOTE_X2, flags=0x01)
        self.reply = []
        self.futures = futures
        length = len(futures)
        if length <= 0:
            raise Exception("futures count must > 0")

    def serialize_request_body(self) -> bytes:
        length = len(self.futures)
        if length <= 0:
            raise Exception("futures count must > 0")
        body = bytearray(struct.pack('<HHHHH', 0, 0, 0, 0, length))
        
        for future in self.futures:
            category, code = future
            logger.debug(f"[Futures_Quotes] serialize: category={category}, code={code}")
            code = code.encode("gbk")
            body.extend(struct.pack('<B23s', category, code))
        return body
    
    def deserialize_response_body(self, data: bytes) -> None:
        logger.debug(f"[Futures_Quotes] deserialize: {data.hex()}")
        pos = 0
        (reserved1, count) = struct.unpack('<8sH', data[pos:pos+10])
        logger.debug(f"[Futures_Quotes] deserialize: reserved1={reserved1}, count={count}")
        pos += 10
        step = 314
        for _ in range(count):
            info_bytes = data[pos:pos+314]
            try:
                self.info = info_bytes.decode('gbk', errors='ignore').replace('\x00', '')
            except Exception:
                self.info = info_bytes.decode('utf-8', errors='ignore').replace('\x00', '')
            #logger.debug(f"[Futures_Quotes] info={self.info}")
            one = unpack_futures(info_bytes)
            self.reply.append(one)
            pos += step
    

from quant1x.data.schema import Bar
class InstrumentBars(protocol.BaseMessage):
    """
    K线数据
    """
    PRE_REQUEST_MAX = 700
    
    def __init__(self, category, market, ticker, start: int=0, count=PRE_REQUEST_MAX):
        super().__init__(Command.EXT_INSTRUMENT_BARS, flags=0x01)
        
        self.market = market
        self.ticker = ticker
        self.category = category
        self.frequency = 1
        """通过实验发现, 频率为 1 时, 返回的数据是按照category设定的K线周期连续的数据, 大于1时, 返回的数据是则是在category的基础再聚合的数据"""
        self.start = start
        self.count = count
        self.reply : List[Bar] = []
    
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
            # one = OrderedDict([
            #     ("date","%d-%02d-%02d" % (year, month, day)),
            #     ("open", open_price),
            #     ("close", close),
            #     ("high", high),
            #     ("low", low),
            #     ("position", position),
            #     ("volume", volume),
            #     ("price", price),
            #     #("year", year),
            #     #("month", month),
            #     #("day", day),
            #     #("hour", hour),
            #     #("minute", minute),
            #     ("amount", amount),
            #     ("timestamp", "%d-%02d-%02d %02d:%02d:%02d" % (year, month, day, hour, minute, second)),
            # ])
            one = Bar(
                date="%d-%02d-%02d" % (year, month, day),
                open=open_price,
                close=close,
                high=high,
                low=low,
                volume=volume,
                amount=amount,
                timestamp="%d-%02d-%02d %02d:%02d:%02d" % (year, month, day, hour, minute, second)
            )
            result.append(one)
        
        self.reply = result
        #logger.debug("[ExtInstrumentBarResponse] reply: {}", self.reply)

from quant1x.data.schema import Transaction
from datetime import date as datetime_date, time as datetime_time
class TransactionData(protocol.BaseMessage):
    PRE_REQUEST_MAX = 1800
    """
    获取最新的(最后一个交易日)成交数据
    """
    def __init__(self, market, ticker: str, offset: int=0, size: int=PRE_REQUEST_MAX):
        super().__init__(Command.EXT_TRANSACTION_DATA, flags=0x01)
        self.market = market
        self.ticker = ticker
        self.offset = offset
        self.size = size
        self.reply = []
    
    def serialize_request_body(self) -> bytes:
        ticker = self.ticker.encode("utf-8")
        body = struct.pack('<B9siH', self.market, ticker, self.offset, self.size)
        logger.debug(f"[TransactionData] serialize: market={self.market}, ticker={self.ticker}, offset={self.offset}, size={self.size}")
        return body
    
    def deserialize_response_body(self, data: bytes) -> None:
        logger.debug(f"[TransactionData] deserialize: {data.hex()}")
        pos = 0
        market, code, _, num = struct.unpack('<B9s4sH', data[pos: pos + 16])
        pos += 16
        result = []
        for i in range(num):

            (raw_time, price, volume, zengcang, direction) = struct.unpack("<HIIiH", data[pos: pos + 16])

            pos += 16
            hour = raw_time // 60
            minute = raw_time % 60
            second = direction % 10000
            nature = direction ### 保持老接口的兼容性

            if second > 59:
                second = 0

            date = datetime.combine(datetime_date.today(), datetime_time(hour,minute,second))

            value = direction // 10000
            nature_name = ""
            if value == 0:
                direction = 1
                if zengcang > 0:
                    if volume > zengcang:
                        nature_name = "多开"
                    elif volume == zengcang:
                        nature_name = "双开"
                elif zengcang == 0:
                    nature_name = "多换"
                else:
                    if volume == -zengcang:
                        nature_name = "双平"
                    else:
                        nature_name = "空平"
            elif value == 1:
                direction = -1
                if zengcang > 0:
                    if volume > zengcang:
                        nature_name = "空开"
                    elif volume == zengcang:
                        nature_name = "双开"
                elif zengcang == 0:
                    nature_name = "空换"
                else:
                    if volume == -zengcang:
                        nature_name = "双平"
                    else:
                        nature_name = "多平"
            else:
                direction = 0
                if zengcang > 0:
                    if volume > zengcang:
                        nature_name = "开仓"
                    elif volume == zengcang:
                        nature_name = "双开"
                elif zengcang < 0:
                    if volume > -zengcang:
                        nature_name = "平仓"
                    elif volume == -zengcang:
                        nature_name = "双平"
                else:
                    nature_name = "换手"

            if market in [31,48]:
                if nature == 0:
                    direction = 1
                    nature_name = 'B'
                elif nature == 256:
                    direction = -1
                    nature_name = 'S'
                else: #512
                    direction = 0
                    nature_name = ''


            result.append(OrderedDict([
                ("date", date),
                ("hour", hour),
                ("minute", minute),
                ("second", second),
                ("price", price),
                ("volume", volume),
                ("zengcang", zengcang),
                ("nature", nature),
                ("nature_mark", nature // 10000),
                ("nature_value", nature % 10000),
                ("nature_name", nature_name),
                ("direction", direction),
            ]))
        self.reply = result

class DailyTransactionData(protocol.BaseMessage):
    PRE_REQUEST_MAX = 1800
    """
    获取某日的成交数据
    """
    def __init__(self, market, ticker: str, date: int, offset: int=0, size: int=PRE_REQUEST_MAX):
        super().__init__(Command.EXT_DAILY_TRANSACTION_DATA, flags=0x01)
        self.market = market
        self.ticker = ticker
        self.date = date
        self.offset = offset
        self.size = size
        self.reply = []
    
    def serialize_request_body(self) -> bytes:
        ticker = self.ticker.encode("utf-8")
        body = struct.pack('<IB9siH', self.date, self.market, ticker, self.offset, self.size)
        logger.debug(f"[DailyTransactionData] serialize: market={self.market}, ticker={self.ticker}, offset={self.offset}, size={self.size}")
        return body
    
    def deserialize_response_body(self, data: bytes) -> None:
        logger.debug(f"[DailyTransactionData] deserialize: {data.hex()}")
        pos = 0
        market, code, _, num = struct.unpack('<B9s4sH', data[pos: pos + 16])
        pos += 16
        result = []
        for i in range(num):

            (raw_time, price, volume, zengcang, direction) = struct.unpack("<HIIiH", data[pos: pos + 16])

            pos += 16
            year = self.date // 10000
            month = self.date % 10000 // 100
            day = self.date % 100
            hour = raw_time // 60
            minute = raw_time % 60
            second = direction % 10000
            nature = direction #### 为了老用户接口的兼容性，已经转换为使用 nature_value
            value = direction // 10000
            nature_name = '换手'
            # 对于大于59秒的值，属于无效数值
            if second > 59:
                second = 0
            date =datetime(year, month, day, hour, minute, second)

            if value == 0:
                direction = 1
                if zengcang > 0:
                    if volume > zengcang:
                        nature_name = "多开"
                    elif volume == zengcang:
                        nature_name = "双开"
                elif zengcang == 0:
                    nature_name = "多换"
                else:
                    if volume == -zengcang:
                        nature_name = "双平"
                    else:
                        nature_name = "空平"
            elif value == 1:
                direction = -1
                if zengcang > 0:
                    if volume > zengcang:
                        nature_name = "空开"
                    elif volume == zengcang:
                        nature_name = "双开"
                elif zengcang == 0:
                    nature_name = "空换"
                else:
                    if volume == -zengcang:
                        nature_name = "双平"
                    else:
                        nature_name = "多平"
            else:
                direction = 0
                if zengcang > 0:
                    if volume > zengcang:
                        nature_name = "开仓"
                    elif volume == zengcang:
                        nature_name = "双开"
                elif zengcang < 0:
                    if volume > -zengcang:
                        nature_name = "平仓"
                    elif volume == -zengcang:
                        nature_name = "双平"
                else:
                    nature_name = "换手"

            if market in [31,48]:
                if nature == 0:
                    direction = 1
                    nature_name = 'B'
                elif nature == 256:
                    direction = -1
                    nature_name = 'S'
                else: #512
                    direction = 0
                    nature_name = ''

            result.append(OrderedDict([
                ("date", date),
                ("hour", hour),
                ("minute", minute),
                ("price", price),
                ("volume", volume),
                ("zengcang", zengcang),
                ("natrue_name", nature_name),
                ("nature_name", nature_name), #修正了nature_name的拼写错误(natrue), 为了保持兼容性，原有的natrue_name还会保留一段时间
                ("direction", direction),
                ("nature", nature),

            ]))
        self.reply = result


class TodoCmd0X2459(protocol.BaseMessage):
    """
    获取股票的公告信息, html格式
    """
    def __init__(self, market, ticker: str):
        super().__init__(Command.EXT_TODO_2459, flags=0x01)
        self.ticker = ticker
        self.reply = []

    def serialize_request_body(self) -> bytes:
        ticker = self.ticker.encode("utf-8")
        body = struct.pack('<6s', ticker)
        return bytearray.fromhex('00 00 00 00 60 EA 00 00 69 77 73 68 6F 70 5F 68 6B 2F 30 39 39 38 38 2E 68 74 6D 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00')
    def deserialize_response_body(self, data: bytes) -> None:
        logger.debug(f"[TodoCmd0X2459] deserialize: {data.hex()}")
        offset = 4
        if len(data) >= offset:
            info_bytes = data[offset:]
            try:
                self.info = info_bytes.decode('utf-8', errors='ignore').replace('\x00', '')
            except Exception:
                self.info = info_bytes.decode('latin-1', errors='ignore').replace('\x00', '')
            logger.debug(f"[TodoCmd0X2459] info={self.info}")

from quant1x.data.schema.company import CompanyInfoChunk
class CompanyInfoCategories(protocol.BaseMessage):
    """
    基础F0 数据文件的块信息, 编码格式为gbk
    """
    def __init__(self, market, ticker: str):
        super().__init__(Command.EXT_COMPANY_INFO_CATEGORIES, flags=0x01)
        self.market = market
        self.ticker = ticker
        self.reply = []

    def serialize_request_body(self) -> bytes:
        ticker = self.ticker.encode("utf-8")
        body = struct.pack('<BB6s', self.market, 0, ticker)
        padding = bytes.fromhex('00000000')
        return body + padding
    
    def deserialize_response_body(self, data: bytes) -> None:
        logger.debug(f"[TodoCmd0X24B8] deserialize: size={len(data)}, data={data.hex()}")
        
        pos = 0
        count = struct.unpack('<H', data[pos:pos+2])[0] # 记录数
        logger.debug(f"[TodoCmd0X24B8] deserialize: count={count}")
        pos += 2
        step = 152
        for _ in range(count):
            record_data = data[pos:pos+step]
            #offset, length, chucksun, title, filename = struct.unpack('<II8s56s80s', record_data)
            title, filename, offset, length = struct.unpack('<64s80sII', record_data)
            title = title.decode('gbk', errors='ignore').replace('\x00', '')
            filename = filename.decode('gbk', errors='ignore').replace('\x00', '')
            #logger.debug(f"[TodoCmd0X24B8] deserialize: title={title} filename={filename} offset={offset} length={length}")
            pos += step
            e = CompanyInfoChunk(title=title, filename=filename, offset=offset, size=length)
            self.reply.append(e)
            

class CompanyInfoContent(protocol.BaseMessage):
    """
    基础F0 数据文件的块信息, 编码格式为gbk
    """
    def __init__(self, market, ticker: str, filename: str, offset: int, size: int):
        super().__init__(Command.EXT_COMPANY_INFO_CONTENT, flags=0x01)
        self.market = market
        self.ticker = ticker
        self.filename = filename
        self.offset = offset
        self.size = size
        self.reply = ''

    def serialize_request_body(self) -> bytes:
        body = struct.pack('<BB6s', self.market, 0, self.ticker.encode("utf-8"))
        padding = bytes.fromhex('0000')
        file = struct.pack('<80sIII', self.filename.encode("gbk"), self.offset, self.size, 0)
        return body + padding + file
    
    def deserialize_response_body(self, data: bytes) -> None:
        logger.debug(f"[CompanyInfoContent] deserialize: {data.hex()}")
        (market, ticker, reserved, length) = struct.unpack('<H6sHH', data[:12])
        ticker = ticker.decode('utf-8').replace('\x00', '')
        logger.debug(f"[CompanyInfoContent] deserialize: market={market} ticker={ticker} reserved={reserved} length={length}")
        pos = 12
        offset = pos
        if len(data) >= offset:
            info_bytes = data[offset:]
            try:
                info = info_bytes.decode('gbk', errors='ignore')#.replace('\x00', '')
            except Exception:
                info = info_bytes.decode('utf-8', errors='ignore')#.replace('\x00', '')
            logger.debug(f"[CompanyInfoContent] info={info}")
            self.reply = info


from quant1x.data.schema import XdxrInfo, XdxrCategory
class TodoCmd0X2488(protocol.BaseMessage):
    """
    待确认命令, 0x2488, 可能是与当日行情有关系的数据
    """
    def __init__(self, ticker: str):
        super().__init__(Command.EXT_XDXR_INFO, flags=0x01)
        self.ticker = ticker
        self.reply = []

    def serialize_request_body(self) -> bytes:
        return bytearray.fromhex('1F 30 39 39 38 38 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 37 00 00 00 00 00 00 00 00 00')
    
    def deserialize_response_body(self, data: bytes) -> None:
        logger.debug(f"[TodoCmd0X2488] deserialize: {data.hex()}")
        
        pos = 36
        count = struct.unpack('<H', data[pos:pos+2])[0]
        logger.debug(f"[TodoCmd0X2488] deserialize: count={count}")
        pos += 2
        
        for _ in range(count):
            # if pos + 29 > len(data): # 1+6+1+4+1+16 = 29 bytes per record
            #     break
                
            # # Market(1), Code(6), Unknown(1), Date(4), Category(1), Data(16)
            # pos += 1 # Market
            # pos += 6 # Code
            # pos += 1 # Unknown
            year, month, day, hour, minute, pos = helpers.get_datetime(9, data, pos)
            # date_int = struct.unpack('<I', data[pos:pos+4])[0]
            # pos += 4
            
            category = struct.unpack('<B', data[pos:pos+1])[0]
            pos += 1
            
            record_data = data[pos:pos+11]
            pos += 11
            
            # year, month, day, _, _ = helpers.get_datetime_from_uint32(9, date_int, 0)
            
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
                info.QianLiuTong = helpers.int_to_float64(v1)
                v2 = struct.unpack('<I', record_data[4:8])[0]
                info.QianZongGuBen = helpers.int_to_float64(v2)
                v3 = struct.unpack('<I', record_data[8:12])[0]
                info.HouLiuTong = helpers.int_to_float64(v3)
                v4 = struct.unpack('<I', record_data[12:16])[0]
                info.HouZongGuBen = helpers.int_to_float64(v4)
                
            self.reply.append(info)

class TodoCmd0X2489(protocol.BaseMessage):
    """
    K线数据
    """
    def __init__(self, market:int, ticker: str):
        super().__init__(Command.EXT_TODO_2489, flags=0x01)
        self.market = market
        self.ticker = ticker
        self.bar_type = 4
        self.unknown1 = 1
        self.start = 0
        self.size = 240
        self.reply = []

    def serialize_request_body(self) -> bytes:
        # 52个字节
        ticker = self.ticker.encode("utf-8")
        symbol = struct.pack('<B23s', self.market, ticker) # 24: 证券代码
        type_ = struct.pack('<HH', self.bar_type, self.unknown1) # 4: 2，K线类型，2，未知
        range_ = struct.pack('<IH', self.start, self.size) # 6: 4，起始时间，2，数量
        padding= bytearray.fromhex('00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00') # 18: 未知填充
        return symbol + type_ + range_ + padding
    
    def _parse_date(self, num):
        year = num // 2048 + 2004
        month = (num % 2048) // 100
        day = (num % 2048) % 100

        return year, month, day

    def _parse_time(self, num):
        return (num // 60) , (num % 60)
    
    def deserialize_response_body(self, data: bytes) -> None:
        logger.debug(f"[TodoCmd0X2489] deserialize: len={len(data)}, data={data.hex()}")
        
        pos = 0
        (market, ticker, reserved1, num, num2, reserved2) = struct.unpack('<B6s17sHH12s', data[pos:pos+40])
        ticker = ticker.decode('gbk').replace('\x00', '')
        logger.debug(f"[TodoCmd0X2489] deserialize: market={market}, ticker={ticker}, reserved1={reserved1.hex()}, num={num}, num2={num2}, reserved2={reserved2.hex()}")
        pos += 40
        (count,) = struct.unpack('<H', data[pos:pos+2])
        logger.debug(f"[TodoCmd0X2489] deserialize: count={count}")
        pos += 2
        for _ in range(count):
            (year, month, day, hour, minute, pos) = helpers.get_datetime(9, data, pos)
            logger.debug(f"[TodoCmd0X2489] deserialize: year={year}, month={month}, day={day}, hour={hour}, minute={minute}")
            (open_price, high, low, close, position, trade, settlementprice) = struct.unpack("<ffffIIf", data[pos:pos+28])  
            #print(raw_li[0])
            pos += 28
            one = OrderedDict([
                ("datetime", "%d-%02d-%02d %02d:%02d" % (year, month, day, hour, minute)),
                ("year", year),
                ("month", month),
                ("day", day),
                ("hour", hour),
                ("minute", minute),
                ("open", open_price),
                ("high", high),
                ("low", low),
                ("close", close),
                ("position", position),
                ("trade", trade),
                ("settlementprice", settlementprice), # 抛空量？
            ])
            logger.debug(f"[TodoCmd0X2489] deserialize: one={one}")
            self.reply.append(one)

def date_from_int(date: int) -> tuple[int,int,int]:
    """把日期转换成年/月/日"""
    year = date // 10000
    tmp = date - year * 10000
    month = tmp // 100
    day = tmp % 100
    return year, month, day

def seconds_to_hhmmss(secs: int) -> tuple[int,int,int]:
    """把秒数转换成时/分/秒"""
    h = secs // 3600
    m = (secs % 3600) // 60
    s = secs % 60
    return h, m, s

def unpack_futures(data, code_len: int = 23):
    if len(data) == 292 + code_len:
        raise Exception('')
    category, code = struct.unpack(f'<B{code_len}s', data[:1 + code_len])
    active, pre_close, open, high, low, current, open_position, add_position, vol, curr_vol, amount, in_vol, ex_vol, u14, hold_position = struct.unpack(f'<I5f4If4I', data[1 + code_len: 61 + code_len])
    pending_list = struct.unpack('<5f5I5f5I', data[61 + code_len: 141 + code_len])
    pending = {
        'bids': [{'price': pending_list[i], 'vol': pending_list[i + 5]} for i in range(5)],
        'asks': [{'price': pending_list[i], 'vol': pending_list[i + 5]} for i in range(10, 15)]
    }
    u1, settlement_price, u2, average_price, pre_settlement_price, u3, u4, u5, u6, pre_close_price  = struct.unpack('<HfIffIIIIf', data[141 + code_len: 179 + code_len])
    s1, pre_vol, u7, s2, u8, day3_raise, s3, settlement_price2, date, u9, raise_speed, u10, s4, u11, u12 = struct.unpack('<12sff12sff25sfIIff24sHB', data[179 + code_len: 291 + code_len])
    
    code = code.decode('gbk').replace('\x00', '')
    secs = u9
    year, month, day = date_from_int(date)
    hour, minute, second = seconds_to_hhmmss(secs)
    
    snapshot = {
            'category': category, 
            'code': code, 
            'active': active, 
            'pre_close': pre_close, 
            'open': open, 
            'high': high, 
            'low': low, 
            'current': current, 
            'open_position': open_position, 
            'add_position': add_position, 
            'vol': vol, 
            'curr_vol': curr_vol, 
            'amount': amount, 
            'in_vol': in_vol, 
            'ex_vol': ex_vol, 
            'u14': u14, 
            'hold_position': hold_position,
            'pending': pending,
            'settlement_price': settlement_price,
            'average_price': average_price,
            'pre_settlement_price': pre_settlement_price,
            'pre_close_price': pre_close_price,
            'pre_vol': pre_vol,
            'day3_raise': day3_raise,
            'settlement_price2': settlement_price2,
            'date': date,
            'timestamp': f"{year:04d}-{month:02d}-{day:02d} {hour:02d}:{minute:02d}:{second:02d}",
            'raise_speed': raise_speed,
            'u1': u1,
            'u2': u2,
            'u3': [u3, u4, u5, u6],
        }
    logger.debug(f"[Futures_Quotes] deserialize: snapshot={snapshot}, u9={u9}, u10={u10}, u11={u11}, u12={u12}")
    return snapshot

class Futures_Quotes(protocol.BaseMessage):
    """期货行情"""
    def __init__(self, futures: list[tuple[int, str]]):
        super().__init__(Command.EXT_FUTURES_QUOTES, flags=0x01)
        self.reply = []
        self.futures = futures
        length = len(futures)
        if length <= 0:
            raise Exception("futures count must > 0")

    def serialize_request_body(self) -> bytes:
        length = len(self.futures)
        if length <= 0:
            raise Exception("futures count must > 0")
        body = bytearray(struct.pack('<HHHHH', 5, 0, 0, 0, length))
        
        for future in self.futures:
            category, code = future
            logger.debug(f"[Futures_Quotes] serialize: category={category}, code={code}")
            code = code.encode("gbk")
            body.extend(struct.pack('<B23s', category, code))
        return body
    
    def deserialize_response_body(self, data: bytes) -> None:
        logger.debug(f"[Futures_Quotes] deserialize: {data.hex()}")
        pos = 0
        (reserved1, count) = struct.unpack('<8sH', data[pos:pos+10])
        logger.debug(f"[Futures_Quotes] deserialize: reserved1={reserved1}, count={count}")
        pos += 10
        step = 314
        for _ in range(count):
            info_bytes = data[pos:pos+314]
            try:
                self.info = info_bytes.decode('gbk', errors='ignore').replace('\x00', '')
            except Exception:
                self.info = info_bytes.decode('utf-8', errors='ignore').replace('\x00', '')
            #logger.debug(f"[Futures_Quotes] info={self.info}")
            one = unpack_futures(info_bytes)
            self.reply.append(one)
            pos += step
    
class IntradayChartSampling(protocol.BaseMessage):
    """
    当日分时简图
    """
    def __init__(self, market, ticker: str):
        super().__init__(Command.EXT_INTRADAY_CHART_SAMPLING, flags=0x01)
        self.market = market
        self.ticker = ticker
        self.reply = []

    def serialize_request_body(self) -> bytes:
        ticker = self.ticker.encode("utf-8")
        symbol = struct.pack('<H22s', self.market, ticker) # 24: 证券代码
        padding= bytearray.fromhex('01001400000000000000000000') # 13: 未知填充
        return symbol + padding
    
    def deserialize_response_body(self, data: bytes) -> None:
        data_len = len(data)
        logger.debug(f"[IntradayChartSampling] deserialize: len={data_len}, data={data.hex()}")
        (market, ticker) = struct.unpack('<H22s', data[:24])
        logger.debug(f"[IntradayChartSampling] deserialize: market={market}, ticker={ticker.decode('gbk')}")
        pos = 24
        (unknown1, width, height, unknown2) = struct.unpack('<HBBH', data[pos:pos+6])
        logger.debug(f"[IntradayChartSampling] deserialize: unknown1={unknown1}, width={width}, height={height}, unknown2={unknown2}")
        pos += 6
        # year, month, day, hour, minute, pos = helpers.get_datetime(category, data, pos)
        # logger.debug(f"[IntradayChartSampling] deserialize: year={year}, month={month}, day={day}, hour={hour}, minute={minute}")
        logger.debug(f"[IntradayChartSampling] deserialize: pos={pos}, data={data[pos:pos+4].hex()}")
        pos += 4
        (count, pre_close, unknown1) = struct.unpack('<HfH', data[pos:pos+8])
        logger.debug(f"[IntradayChartSampling] deserialize: count={count}, pre_close={pre_close}, unknown1={unknown1}")
        pos += 8
        for _ in range(count):
            (f1,) = struct.unpack('<f', data[pos:pos+4])
            #logger.debug(f"[IntradayChartSampling] deserialize: f1={f1}, ")
            self.reply.append(f1)
            pos += 4
        logger.debug(f"[IntradayChartSampling] deserialize: pos={pos}, data={data[pos:].hex()}")

class TodoCmdUnknown(protocol.BaseMessage):
    """
    获取股票的公告信息, html格式
    """
    def __init__(self, command:int, market, ticker: str):
        custom = Command.from_parts(QuoteType.EXTENSION, command & 0xFFFF, "自定义指令")
        super().__init__(custom, flags=0x01)
        self.market = market
        self.ticker = ticker
        self.reply = []

    def serialize_request_body(self) -> bytes:
        ticker = self.ticker.encode("utf-8")
        symbol = struct.pack('<H22s', self.market, ticker) # 24: 证券代码
        padding= bytearray.fromhex('01001400000000000000000000') # 13: 未知填充
        return symbol + padding
    
    def deserialize_response_body(self, data: bytes) -> None:
        data_len = len(data)
        logger.debug(f"[TodoCmdUnknown] deserialize: len={data_len}, data={data.hex()}")
        (market, ticker) = struct.unpack('<H22s', data[:24])
        logger.debug(f"[TodoCmdUnknown] deserialize: market={market}, ticker={ticker.decode('gbk')}")
        pos = 24
        (unknown1, width, height, unknown2) = struct.unpack('<HBBH', data[pos:pos+6])
        logger.debug(f"[TodoCmdUnknown] deserialize: unknown1={unknown1}, width={width}, height={height}, unknown2={unknown2}")
        pos += 6
        # year, month, day, hour, minute, pos = helpers.get_datetime(category, data, pos)
        # logger.debug(f"[TodoCmdUnknown] deserialize: year={year}, month={month}, day={day}, hour={hour}, minute={minute}")
        logger.debug(f"[TodoCmdUnknown] deserialize: pos={pos}, data={data[pos:pos+4].hex()}")
        pos += 4
        (count, pre_close, unknown1) = struct.unpack('<HfH', data[pos:pos+8])
        logger.debug(f"[TodoCmdUnknown] deserialize: count={count}, pre_close={pre_close}, unknown1={unknown1}")
        pos += 8
        for _ in range(count):
            (f1,) = struct.unpack('<f', data[pos:pos+4])
            #logger.debug(f"[TodoCmdUnknown] deserialize: f1={f1}, ")
            self.reply.append(f1)
            pos += 4
        logger.debug(f"[TodoCmdUnknown] deserialize: pos={pos}, data={data[pos:].hex()}")

