# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

import struct
from quant1x.level1.protocol import (
    FLAG_UNCOMPRESSED,
    COMMAND_HEARTBEAT,
    sequence_id,
)

class HeartbeatRequest:
    """
    心跳请求
    """
    def __init__(self):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = sequence_id()
        self.packet_type = 0x02
        self.body_wire_len = 0
        self.body_raw_len = 0
        self.method = COMMAND_HEARTBEAT

    def serialize(self) -> bytes:
        self.body_wire_len = 2
        self.body_raw_len = 2
        
        header = struct.pack('<B I B H H H', self.zip_flag, self.seq_id, self.packet_type, self.body_wire_len, self.body_raw_len, self.method)
        return header

class HeartbeatResponse:
    """
    心跳响应
    """
    def __init__(self):
        self.info = ""

    def deserialize(self, data: bytes) -> None:
        # get_string(10)
        if len(data) >= 10:
            s = data[:10]
        else:
            s = data
            
        try:
            self.info = s.decode('gbk', errors='ignore').split('\x00', 1)[0]
        except Exception:
            self.info = s.decode('utf-8', errors='ignore').split('\x00', 1)[0]
