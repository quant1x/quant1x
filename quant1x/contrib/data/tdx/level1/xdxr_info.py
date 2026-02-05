# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations
import struct
from enum import Enum
from typing import List, Tuple, Dict, Any
from dataclasses import dataclass

from .command import (
    FLAG_UNCOMPRESSED,
    COMMAND_XDXR_INFO
)
from . import helpers

from quant1x.data.market import Exchange
from quant1x.data.schema import XdxrInfo, XdxrCategory

class XdxrInfoRequest:
    def __init__(self, exchange: Exchange, code: str):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = helpers.msg_sequence_id()
        self.packet_type = 0x01
        self.method = COMMAND_XDXR_INFO
        
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
