# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import struct
from enum import Enum
from typing import List

from quant1x.data.meta import Exchange
from quant1x.data.schema import Bar

from ...command import Command
from ... import helpers
from ... import protocol

SECURITY_BARS_PRE_REQUEST_MAX = 700  # 800


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
    FundFlow = 22

    @staticmethod
    def to_string(ktype: 'KLineType') -> str:
        return ktype.name


class SecurityBars(protocol.BaseMessage):
    """K线数据"""
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
