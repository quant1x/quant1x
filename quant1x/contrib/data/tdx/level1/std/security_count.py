# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import struct

from ...command import Command
from ... import protocol


class SecurityCountContext(protocol.BaseFrame):
    """证券数量请求"""
    def __init__(self, market: int = 0):
        super().__init__(Command.STD_SECURITY_COUNT)
        self._market = market

        self.count: int = 0

    def serialize_request_body(self) -> bytes:
        from quant1x.data.meta import Timestamp
        yyyymmdd = Timestamp.now().yyyymmdd()
        padding = struct.pack('<I', yyyymmdd)
        return struct.pack('<H', self._market) + padding

    def deserialize_response_body(self, data: bytes) -> None:
        if len(data) < 2:
            return
        self.count = struct.unpack('<H', data[:2])[0]
