# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import struct
from typing import List

from quant1x.data.meta import Exchange, Instrument
from quant1x.data.schema import XdxrInfo, XdxrEntry, XdxrCategory

from ...command import Command
from ... import helpers
from ... import protocol


class Xdxr(protocol.BaseMessage):
    """除权除息信息"""
    def __init__(self, exchange: Exchange, ticker: str):
        super().__init__(Command.STD_XDXR_INFO)
        self._market = helpers.exchange_to_market(exchange)
        self._ticker = ticker
        self._padding = bytes.fromhex('0100')

        self.count = 0
        self.list: List[XdxrInfo] = []

    def serialize_request_body(self) -> bytes:
        code_bytes = self._ticker.encode('ascii')
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


class XdxrBatch(protocol.BaseMessage):
    """批量获取除权除息信息"""
    def __init__(self, insts: List[Instrument]):
        super().__init__(Command.STD_XDXR_INFO)
        self._insts = insts

        self.count = 0
        self.list: List[XdxrEntry] = []

    def serialize_request_body(self) -> bytes:
        inst_count = len(self._insts)
        data = struct.pack('<H', inst_count)
        for inst in self._insts:
            market = helpers.exchange_to_market(inst.exchange)
            code_bytes = inst.ticker.encode('ascii')
            if len(code_bytes) < 6:
                code_bytes = code_bytes + b'\x00' * (6 - len(code_bytes))
            else:
                code_bytes = code_bytes[:6]
            data += struct.pack('<B 6s', market, code_bytes)
        return data

    def deserialize_response_body(self, data: bytes) -> None:
        self.list.clear()
        pos = 0
        body_len = len(data)
        if body_len < 2:
            return
        
        self.count = struct.unpack('<H', data[0:2])[0]
        pos += 2
        for _ in range(self.count):
            (market_id_, ticker_, count_) = struct.unpack('<B6sH', data[pos:pos+9])
            exchange_ = helpers.market_to_exchange(market_id_)
            pos += 9
            entry = XdxrEntry(exchange_, ticker_.decode('ascii'))
            for _ in range(count_):
                if pos + 29 > body_len:
                    break
                (market_, code_, u1) = struct.unpack('<B 6s B', data[pos:pos+8])
                assert market_ == market_id_ and code_ == ticker_
                pos += 8
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
                entry.list.append(info)
            self.list.append(entry)

    @staticmethod
    def _get_v(v: int) -> float:
        if v == 0:
            return 0.0
        return helpers.int_to_float64(v)
