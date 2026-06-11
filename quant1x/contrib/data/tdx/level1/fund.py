# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import struct, json
from enum import Enum
from typing import List
from dataclasses import dataclass, field

from ..command import Command
from .. import helpers
from .. import protocol
from quant1x.data.meta import Exchange, Instrument, InstrumentType
from quant1x.log import logger


@dataclass
class HistoricalFundFlow:
    """历史日线资金流向条目。"""
    
    year: int
    month: int
    day: int
    
    # 金额项 (单位：元)
    super_in: float
    super_out: float
    large_in: float
    large_out: float
    medium_in: float
    medium_out: float
    small_in: float
    small_out: float
    
    @property
    def main_net_inflow(self) -> float:
        """当日主力净流入。"""
        return (self.super_in + self.large_in) - (self.super_out + self.large_out)

def _pow2(exp: int) -> float:
    if exp >= 0:
        return float(1 << exp) if exp < 63 else 2.0 ** exp
    return 1.0 / (1 << (-exp)) if -exp < 63 else 2.0 ** exp


def _decode_volume(ivol: int) -> float:
    if ivol == 0:
        return 0.0

    logpoint = (ivol >> 24) & 0xFF
    hleax = (ivol >> 16) & 0xFF
    lheax = (ivol >> 8) & 0xFF
    lleax = ivol & 0xFF

    exp = logpoint * 2 - 0x7F
    base = _pow2(exp)

    exp_h = logpoint * 2 - 0x86
    if hleax > 0x80:
        hi = _pow2(exp_h) * 128 + (hleax & 0x7F) * _pow2(exp_h + 1)
    else:
        hi = _pow2(exp_h) * hleax

    mid = _pow2(logpoint * 2 - 0x8E) * lheax
    lo = _pow2(logpoint * 2 - 0x96) * lleax

    if hleax & 0x80:
        mid *= 2.0
        lo *= 2.0

    return base + hi + mid + lo

class HistoryFundFlowDetails_invalid(protocol.BaseMessage):
    """历史日线资金流向"""
    PRE_REQUEST_MAX = 10 # 单次请求最大数量

    def __init__(self,
                 exchange: Exchange,
                 ticker: str,
                 start: int = 0x00000000,
                 count: int = PRE_REQUEST_MAX,
                 ):
        super().__init__(Command.STD_SECURITY_BARS, flags=1)
        logger.warning(f"请求和响应正常, 无数据返回")
        self.request_header.packet_type = 0x08
        self._market = helpers.exchange_to_market(exchange)
        self._ticker = ticker
        #self._date = date
        self._start = start
        self._count = count
        #self._type = type
        #self._padding = bytes.fromhex("00000000000000000000")

        self.count = 0
        self.list: List[HistoricalFundFlow] = []

    def serialize_request_body(self) -> bytes:
        u3 = 0
        u4 = 0
        u5 = 0
        
        payload = struct.pack('<H 6s H H H H I I H',
                              self._market,
                              self._ticker.encode('ascii'),
                              22,
                              1,
                              self._start,
                              self._count,
                              u3,
                              u4,
                              u5,
        )
        # body = struct.pack('<H 6s H H H H',
        #                    self._market, self._ticker.encode('ascii'),
        #                    22,1,
        #                    self._start, self._count)
        # return body + self._padding
        
        return payload

    def deserialize_response_body(self, data: bytes) -> None:
        self.list.clear()
        response_body = data
        # 响应格式：9字节头 + 2字节数量 + 每条记录 36 字节
        if len(response_body) < 11:
            return
            
        (num,) = struct.unpack("<H", response_body[9:11])
        pos = 11
        results = []
        
        for _ in range(num):
            if len(response_body) < pos + 36:
                break
            
            # 记录格式：4字节日期 + 8个4字节自定义浮点金额
            # [0]日期, [1..4]流入(超/大/中/小), [5..8]流出(超/大/中/小)
            raw_data = struct.unpack("<IIIIIIIII", response_body[pos:pos+36])
            
            raw_date = raw_data[0]
            year = raw_date // 10000
            month = (raw_date // 100) % 100
            day = raw_date % 100
            
            results.append(HistoricalFundFlow(
                year=year, month=month, day=day,
                super_in=_decode_volume(raw_data[1]),
                large_in=_decode_volume(raw_data[2]),
                medium_in=_decode_volume(raw_data[3]),
                small_in=_decode_volume(raw_data[4]),
                super_out=_decode_volume(raw_data[5]),
                large_out=_decode_volume(raw_data[6]),
                medium_out=_decode_volume(raw_data[7]),
                small_out=_decode_volume(raw_data[8]),
            ))
            pos += 36
        self.count = num
        self.list = results

class HistoryFundFlowDetails(protocol.BaseMessage):
    """历史日线资金流向"""
    PRE_REQUEST_MAX = 10 # 单次请求最大数量
    #020000000001310031001812000030303030303100000000000000000000000000000000000053746F636B5F5A4A4C580000000000000000000000
    #......1.1.....000001..................Stock_ZJLX...........
    #020400000001310031001812000030303030303100000000000000000000000000000000000053746f636b5f5a4a4c580000000000000000000000
    
    def __init__(self,
                 exchange: Exchange,
                 ticker: str,
                 ):
        super().__init__(Command.STD_FUND_FLOW, flags=2)
        logger.warning(f"请求和响应正常, 无数据返回")
        self.request_header.packet_type = 0x01
        self._market = helpers.exchange_to_market(exchange)
        self._ticker = ticker

        self.count = 0
        self.list: List[HistoricalFundFlow] = []

    def serialize_request_body(self) -> bytes:
        payload = struct.pack("<H8s16x21s", 
                              self._market,
                              self._ticker.encode('ascii'),
                              "Stock_ZJLX".encode("ascii"),
        )
        
        return payload

    def deserialize_response_body(self, data: bytes) -> None:
        self.list.clear()
        market, query_info, ext = struct.unpack("<H12s5x8s", data[:27])

        list_raw = struct.unpack(f"<{len(data) - 27}s", data[27:])[0]
        python_list = json.loads(list_raw.decode("gbk"))

        result = {
            "data": None,
            "query_info": query_info.hex(),
            "ext": ext.hex(),
        }
        print(python_list)

        if len(python_list) >= 2:
            today_data = python_list[0]
            five_days_data = python_list[1]
            keys = [
                "今日主力流入", "今日主力流出", "今日散户流入", "今日散户流出",
                "5日主买", "5日主卖", "5日超大单净额", "5日大单净额", "5日中单净额", "5日小单净额",
            ]
            merged_data = today_data + five_days_data
            d = dict(zip(keys, merged_data))
            for k in keys:
                try:
                    d[k] = float(d[k])
                except (ValueError, TypeError):
                    pass
            d["今日主力净流入"] = d["今日主力流入"] - d["今日主力流出"]
            d["今日散户净流入"] = d["今日散户流入"] - d["今日散户流出"]
            d["5日主力净流入"] = d["5日主买"] - d["5日主卖"]
            result["data"] = d

        self.result = result

if __name__ == '__main__':
    import pandas as pd
    from ..client import get_std_conn
    conn = get_std_conn([('121.36.248.138', 7709)])
    #conn = get_std_conn()
    
    # 测试 批量auction details
    req = HistoryFundFlowDetails_invalid(exchange=Exchange.SZSE, ticker='000001')
    protocol.process_level1_new(conn, req)
    if req.list:
        print(f"history fund flow details: count={req.count}")
        df = pd.DataFrame(req.list)
        print(df)
    req = HistoryFundFlowDetails(exchange=Exchange.SZSE, ticker='000001')
    protocol.process_level1_new(conn, req)
    if req.result:
        print(f"history fund flow details: count={req.count}")
        print(req.result)
    