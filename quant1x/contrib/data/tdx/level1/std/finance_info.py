# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import struct
from dataclasses import dataclass

from quant1x.data.meta import Exchange

from ...command import Command
from ... import helpers
from ... import protocol


@dataclass
class FinanceInfo:
    """财务信息数据结构"""
    code: str = ''
    liu_tong_gu_ben: float = 0.0
    province: int = 0
    industry: int = 0
    updated_date: int = 0
    ipo_date: int = 0
    zong_gu_ben: float = 0.0

    def is_delisting(self) -> bool:
        return self.ipo_date == 0 and self.zong_gu_ben == 0 and self.liu_tong_gu_ben == 0


class FinanceInfoRequest(protocol.BaseMessage):
    """财务信息请求/响应"""

    def __init__(self, exchange: Exchange, ticker: str):
        super().__init__(Command.STD_FINANCE_INFO)
        self._market = helpers.exchange_to_market(exchange)
        self._ticker = ticker

        self.count = 0
        self.info: FinanceInfo = FinanceInfo()

    def serialize_request_body(self) -> bytes:
        code_bytes = self._ticker.encode('ascii')
        if len(code_bytes) < 6:
            code_bytes = code_bytes + b'\x00' * (6 - len(code_bytes))
        else:
            code_bytes = code_bytes[:6]
        return struct.pack('<H B 6s', 1, self._market, code_bytes)

    def deserialize_response_body(self, data: bytes) -> None:
        if len(data) < 2:
            return
        self.count = struct.unpack('<H', data[:2])[0]
        if self.count == 0:
            return

        offset = 2
        # struct: < B 6s f H H I I 30f  (143 bytes per record)
        fmt = '< B 6s f H H I I ' + 'f' * 30
        struct_size = struct.calcsize(fmt)
        if len(data) < offset + struct_size:
            return

        unpacked = struct.unpack(fmt, data[offset:offset + struct_size])
        raw_market = unpacked[0]
        raw_code = unpacked[1]
        raw_liu_tong_gu_ben = unpacked[2]
        raw_province = unpacked[3]
        raw_industry = unpacked[4]
        raw_updated_date = unpacked[5]
        raw_ipo_date = unpacked[6]
        raw_floats = unpacked[7:]

        base_unit = 10000.0
        self.info.code = raw_code.decode('utf-8').rstrip('\x00')
        if raw_market == 0:
            self.info.code = f"sz{self.info.code}"
        elif raw_market == 1:
            self.info.code = f"sh{self.info.code}"
        self.info.liu_tong_gu_ben = raw_liu_tong_gu_ben * base_unit
        self.info.province = raw_province
        self.info.industry = raw_industry
        self.info.updated_date = raw_updated_date
        self.info.ipo_date = raw_ipo_date
        self.info.zong_gu_ben = raw_floats[0] * base_unit
