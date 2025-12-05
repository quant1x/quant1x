# -*- coding: UTF-8 -*-
from __future__ import annotations

import struct
from dataclasses import dataclass, field
from typing import List

from quant1x.level1.protocol import (
    FLAG_UNCOMPRESSED,
    COMMAND_BLOCK_META,
    sequence_id,
)

# Constants
BLOCK_ZHISHU = "block_zs.dat"
BLOCK_FENGGE = "block_fg.dat"
BLOCK_GAINIAN = "block_gn.dat"
BLOCK_DEFAULT = "block.dat"
BLOCK_CHUNKS_SIZE = 0x7530

@dataclass
class BlockMeta:
    size: int = 0
    c1: int = 0
    hash_value: bytes = b''
    c2: int = 0

class BlockMetaRequest:
    """
    板块元数据请求
    """
    def __init__(self, filename: str):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = sequence_id()
        self.packet_type = 0x01
        self.pkg_len1 = 0
        self.pkg_len2 = 0
        self.method = COMMAND_BLOCK_META
        self.block_filename = filename

    def serialize(self) -> bytes:
        # Body: BlockFilename(40)
        # PkgLen = Body + 2 = 42 (0x2A)
        self.pkg_len1 = 0x2A
        self.pkg_len2 = 0x2A
        
        header = struct.pack('<B I B H H H', self.zip_flag, self.seq_id, self.packet_type, self.pkg_len1, self.pkg_len2, self.method)
        
        # Ensure filename is 40 bytes
        filename_bytes = self.block_filename.encode('ascii')[:40].ljust(40, b'\x00')
        
        return header + filename_bytes

class BlockMetaResponse:
    """
    板块元数据响应
    """
    def __init__(self):
        self.meta = BlockMeta()

    def deserialize(self, data: bytes) -> None:
        # Size(4) + C1(1) + HashValue(32) + C2(1) = 38 bytes
        if len(data) < 38:
            return
            
        self.meta.size = struct.unpack('<I', data[:4])[0]
        self.meta.c1 = data[4]
        self.meta.hash_value = data[5:37]
        self.meta.c2 = data[37]
