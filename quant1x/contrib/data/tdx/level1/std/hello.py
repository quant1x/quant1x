# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import struct
from datetime import datetime

from ...command import Command
from ... import protocol


class StdLoginContext(protocol.BaseFrame):
    """第一次协议握手: login"""
    def __init__(self):
        super().__init__(Command.STD_SYNCHRONIZE1)
        self.info = ""
        self._padding = bytes.fromhex("01")

    def serialize_request_body(self) -> bytes:
        return self._padding

    def deserialize_response_body(self, data: bytes) -> None:
        (
            _, year, day, month, minute, hour, _, second,
            unknown1, unknown2, unknown3,
            date, a1, b1, date2, a2, b2,
            unknown4, unknown5, unknown6,
            server_name, web_site, unknown7, category,
        ) = struct.unpack('<BHBBBBBB16s16sBIHHIHHHH5s22s64s6s30s', data)

        info = {
            "date_time": datetime(year, month, day, hour, minute, second).strftime('%Y-%m-%d %H:%M:%S'),
            "server_name": server_name.decode('gbk').replace('\x00', ''),
            "web_site": web_site.decode('gbk').replace('\x00', ''),
            "category": category.decode('gbk').replace('\x00', ''),
            "b1": b1, "b2": b2, "a1": a1, "a2": a2,
            "date": date, "date2": date2,
            "unknown1": unknown1, "unknown2": unknown2, "unknown3": unknown3,
            "unknown4": unknown4, "unknown5": unknown5, "unknown6": unknown6,
            "unknown7": unknown7,
        }
        self.reply = info


class UpgradeTipContext(protocol.BaseFrame):
    """第二次协议握手"""
    def __init__(self):
        super().__init__(Command.STD_SYNCHRONIZE2)
        self.info = ""
        self._padding = bytes.fromhex("d5d0c9ccd6a4a8af0000008fc22540130000d500c9ccbdf0d7ea00000002")

    def serialize_request_body(self) -> bytes:
        return self._padding

    def deserialize_response_body(self, data: bytes) -> None:
        offset = 58
        if len(data) >= offset:
            info_bytes = data[offset:]
            try:
                self.info = info_bytes.decode('gbk', errors='ignore').rstrip('\x00')
            except Exception:
                self.info = info_bytes.decode('utf-8', errors='ignore')
