# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

import struct
from quant1x.level1.protocol import (
    FLAG_UNCOMPRESSED,
    COMMAND_LOGIN1,
    sequence_id,
)

class Synchronize1Request:
    """
    第一次协议握手请求
    """
    def __init__(self):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = sequence_id()
        self.packet_type = 0x01
        self.body_wire_len = 0
        self.body_raw_len = 0
        self.method = COMMAND_LOGIN1
        self.padding = bytes.fromhex("01")

    def serialize(self) -> bytes:
        self.body_wire_len = 2 + len(self.padding)
        self.body_raw_len = self.body_wire_len
        
        header = struct.pack('<B I B H H H', self.zip_flag, self.seq_id, self.packet_type, self.body_wire_len, self.body_raw_len, self.method)
        return header + self.padding

class Synchronize1Response:
    """
    第一次协议握手响应
    """
    def __init__(self):
        self.info = ""

    def deserialize(self, data: bytes) -> None:
        offset = 68
        if len(data) >= offset:
            info_bytes = data[offset:]
            try:
                self.info = info_bytes.decode('gbk', errors='ignore').rstrip('\x00')
            except Exception:
                self.info = info_bytes.decode('utf-8', errors='ignore')
