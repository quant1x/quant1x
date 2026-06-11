# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from ...command import Command
from ... import protocol


class Heartbeat(protocol.BaseMessage):
    """心跳（合并Request和Response）"""
    def __init__(self):
        super().__init__(Command.STD_HEARTBEAT)
        self.info = ""
        self.request_header.packet_type = 0x02

    def serialize_request_body(self) -> bytes:
        return b""

    def deserialize_response_body(self, data: bytes) -> None:
        if len(data) >= 10:
            s = data[:10]
        else:
            s = data
        try:
            self.info = s.decode('gbk', errors='ignore').split('\x00', 1)[0]
        except Exception:
            self.info = s.decode('utf-8', errors='ignore').split('\x00', 1)[0]
