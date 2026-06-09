# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

import struct

from quant1x.data.meta import Timestamp
from quant1x.level1.protocol import (
    FLAG_UNCOMPRESSED,
    COMMAND_SECURITY_COUNT,
    sequence_id,
)

class SecurityCountRequest:
    """
    证券统计请求
    """
    def __init__(self, market: int = 0):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = sequence_id()
        self.packet_type = 0x01
        self.body_wire_len = 0
        self.body_raw_len = 0
        self.method = COMMAND_SECURITY_COUNT
        self.market = market
        yyyymmdd = Timestamp.now().yyyymmdd() # 当日日期，格式为 YYYYMMDD
        self.padding = struct.pack('<I', yyyymmdd) # 4字节日期填充

    def serialize(self) -> bytes:
        # Body: Market(2) + Padding(4) = 6 bytes
        # PkgLen = Body + 2 = 8
        self.body_wire_len = 2 + 2 + len(self.padding)
        self.body_raw_len = self.body_wire_len
        
        header = struct.pack('<B I B H H H', self.zip_flag, self.seq_id, self.packet_type, self.body_wire_len, self.body_raw_len, self.method)
        
        body = struct.pack('<H', self.market) + self.padding
        return header + body

class SecurityCountResponse:
    """
    证券统计响应
    """
    def __init__(self):
        self.count = 0

    def deserialize(self, data: bytes) -> None:
        if len(data) >= 2:
            self.count = struct.unpack('<H', data[:2])[0]
