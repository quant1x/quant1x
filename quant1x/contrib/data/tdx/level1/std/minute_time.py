# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import struct
from dataclasses import dataclass
from typing import List

from ...command import Command
from ... import helpers
from ... import protocol
from quant1x.data.market import detect_symbol


@dataclass
class MinuteTime:
    price: float = 0.0
    vol: int = 0


class HistoryMinuteTime(protocol.BaseMessage):
    """历史分时数据"""
    def __init__(self, security_code: str, date: int):
        super().__init__(Command.STD_HISTORY_MINUTE_DATA)
        self.request_header.packet_ctrl = 0x00
        self._date = date
        inst = detect_symbol(security_code)
        market_id = helpers.exchange_to_market(inst.exchange)
        symbol = inst.marker_ticker()
        self._market = market_id
        self._code = symbol

        self.count: int = 0
        self.list: List[MinuteTime] = []

    def serialize_request_body(self) -> bytes:
        # Body: Date(4) + Market(1) + Code(6)
        code_bytes = self._code.encode('ascii')[:6].ljust(6, b'\x00')
        return struct.pack('<I B', self._date, self._market) + code_bytes

    def deserialize_response_body(self, data: bytes) -> None:
        self.list.clear()
        if len(data) < 2:
            return

        self.count = struct.unpack('<H', data[:2])[0]
        pos = 2

        base_unit = helpers.default_base_unit(self._market, self._code)
        last_price = 0

        # Skip 4 bytes
        pos += 4

        try:
            for _ in range(self.count):
                if pos >= len(data):
                    break
                raw_price, pos = helpers.varint_decode(data, pos)
                _, pos = helpers.varint_decode(data, pos)  # reversed1
                vol, pos = helpers.varint_decode(data, pos)

                last_price += raw_price
                price = float(last_price) / base_unit

                self.list.append(MinuteTime(price=price, vol=vol))
        except IndexError:
            pass
