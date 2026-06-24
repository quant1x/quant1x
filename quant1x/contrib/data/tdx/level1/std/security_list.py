# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import struct

from quant1x.data.meta import Exchange, Instrument, InstrumentType
from quant1x.data.market import detect_instrument_type_by_rule
from quant1x.log import logger

from ...command import Command
from ... import helpers
from ... import protocol

SECURITY_LIST_PRE_REQUEST_MAX = 1600  # 预请求最大数量


class SecurityListContext(protocol.BaseFrame):
    """证券列表"""
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
