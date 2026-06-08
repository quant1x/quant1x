import struct
from enum import Enum
from typing import List
from dataclasses import dataclass, field

from ..command import Command
from .. import helpers
from .. import protocol
from quant1x.data.meta import Exchange, Instrument, InstrumentType
from quant1x.log import logger


@dataclass
class AuctionInfo:
    time: str # 时间
    price: float # 价格
    matched: int # 匹配量
    unmatched: int # 未匹配量
    u: int # 未知

class AuctionDetails(protocol.BaseMessage):
    """集合竞价详情"""
    PRE_REQUEST_MAX = 500 # 单次请求最大数量

    def __init__(self,
                 exchange: Exchange,
                 ticker: str,
                 date: int = 0x00000000, # 0: 当日, 非0: 指定日期
                 type: int = 3, # 0: 早盘集合竞价, 非0: 早盘和尾盘集合竞价. 注意: 官方填充3
                 start: int = 0x00000000,
                 count: int = PRE_REQUEST_MAX,
                 ):
        super().__init__(Command.STD_AUCTION_INFO)
        self._market = helpers.exchange_to_market(exchange)
        self._ticker = ticker
        self._date = date
        self._start = start
        self._count = count
        self._type = type

        self.count = 0
        self.list: List[AuctionInfo] = []

    def serialize_request_body(self) -> bytes:
        u1 = 0x00000000
        u2 = 0x00000000
        payload = struct.pack('<H 6s I I I I I',
                              self._market,
                              self._ticker.encode('ascii'),
                              self._date,
                              self._type,
                              u2,
                              self._start,
                              self._count)
        return payload

    def deserialize_response_body(self, data: bytes) -> None:
        self.list.clear()
        response_body = data
        #response_body_len = len(response_body)
        pos = 0
        (count,) = struct.unpack('<H', response_body[pos:pos+2])
        pos += 2
        result = []
        for i in range(count):
            #print(f"[response]body: {i}, pos={pos}, data={response_body[pos:pos+16].hex()}")
            time_raw, price, matched, unmatched, u, second = struct.unpack('<HfIiBB', response_body[pos:pos+16])
            hour= time_raw // 60
            hour = hour % 24
            minute = time_raw % 60
            second = second % 60
            #price = helpers.int_to_float64(price)
            #print(f"time: time_raw={time_raw}, hour={hour}, minute={minute}, second={second}")
            tm_str = f"{hour:02d}:{minute:02d}:{second:02d}"
            # e ={
            #     'time': tm_str, # 时间
            #     'price': price, # 价格
            #     'matched': matched, # 匹配量
            #     'unmatched': unmatched, # 未匹配量
            #     'u': u, # 未知
            # }
            e = AuctionInfo(tm_str,
                            price,
                            matched,
                            unmatched,
                            u,
            )
            result.append(e)
            pos += 16
        self.count = count
        self.list = result

if __name__ == '__main__':
    import pandas as pd
    from ..client import get_std_conn
    conn = get_std_conn()
    
    # # 测试 单个xdxr
    # req = Xdxr(exchange=Exchange.SZSE, ticker='000001')
    # protocol.process_level1_new(conn, req)
    # if req.list:
    #     print(f"xdxr: count={req.count}")
    #     df = pd.DataFrame(req.list)
    #     print(df)
    
    # 测试 批量auction details
    req = AuctionDetails(exchange=Exchange.SZSE, ticker='000001', type=3, date=20260423)
    protocol.process_level1_new(conn, req)
    if req.list:
        print(f"auction details: count={req.count}")
        df = pd.DataFrame(req.list)
        print(df)
    