# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

import struct
from quant1x.level1.protocol import (
    FLAG_UNCOMPRESSED,
    COMMAND_LOGIN2,
    sequence_id,
)

class Synchronize2Request:
    """
    第二次协议握手请求
    """
    def __init__(self):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = sequence_id()
        self.packet_type = 0x01
        self.body_wire_len = 0
        self.body_raw_len = 0
        self.method = COMMAND_LOGIN2
        self.padding = bytes.fromhex("d5d0c9ccd6a4a8af0000008fc22540130000d500c9ccbdf0d7ea00000002")

    def serialize(self) -> bytes:
        self.body_wire_len = 2 + len(self.padding)
        self.body_raw_len = self.body_wire_len
        
        header = struct.pack('<B I B H H H', self.zip_flag, self.seq_id, self.packet_type, self.body_wire_len, self.body_raw_len, self.method)
        return header + self.padding

class Synchronize2Response:
    """
    第二次协议握手响应
    """
    def __init__(self):
        self.info = ""

    def deserialize(self, data: bytes) -> None:
        offset = 58
        if len(data) >= offset:
            info_bytes = data[offset:]
            try:
                self.info = info_bytes.decode('gbk', errors='ignore').rstrip('\x00')
            except Exception:
                self.info = info_bytes.decode('utf-8', errors='ignore')
