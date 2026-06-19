# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import struct

from ...command import Command
from ... import protocol

BLOCK_CHUNKS_SIZE = 0x7530


class BlockFileContext(protocol.BaseFrame):
    """板块数据(合并Request和Response)"""
    def __init__(self, filename: str, offset: int):
        super().__init__(Command.STD_BLOCK_DATA)
        self._filename = filename
        self._offset = offset
        self._chunk_size = BLOCK_CHUNKS_SIZE

        self.size = 0
        self.data = bytearray()

    def serialize_request_body(self) -> bytes:
        filename_bytes = self._filename.encode('ascii')[:100].ljust(100, b'\x00')
        return struct.pack('<I I', self._offset, self._chunk_size) + filename_bytes

    def deserialize_response_body(self, data: bytes) -> None:
        self.data = bytearray()
        if len(data) < 4:
            return
        self.size = struct.unpack('<I', data[:4])[0]
        if self.size > 0:
            self.data = bytearray(data[4:])
