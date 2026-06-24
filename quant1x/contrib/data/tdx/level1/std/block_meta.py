# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import struct
from dataclasses import dataclass

from ...command import Command
from ... import protocol

# Constants
BLOCK_ZHISHU = "block_zs.dat"
BLOCK_FENGGE = "block_fg.dat"
BLOCK_GAINIAN = "block_gn.dat"
BLOCK_DEFAULT = "block.dat"


@dataclass
class BlockMeta:
    """板块元数据"""
    size: int = 0
    c1: int = 0
    hash_value: bytes = b''
    c2: int = 0


class BlockFileMetaContext(protocol.BaseFrame):
    """板块元数据请求"""
    def __init__(self, filename: str):
        super().__init__(Command.STD_BLOCK_META)
        self._filename = filename

        self.meta = BlockMeta()

    def serialize_request_body(self) -> bytes:
        # Body: filename(40 bytes)
        filename_bytes = self._filename.encode('ascii')[:40].ljust(40, b'\x00')
        return filename_bytes

    def deserialize_response_body(self, data: bytes) -> None:
        # Size(4) + C1(1) + HashValue(32) + C2(1) = 38 bytes
        if len(data) < 38:
            return
        self.meta.size = struct.unpack('<I', data[:4])[0]
        self.meta.c1 = data[4]
        self.meta.hash_value = data[5:37]
        self.meta.c2 = data[37]
