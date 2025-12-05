# -*- coding: UTF-8 -*-
from __future__ import annotations

import struct
from dataclasses import dataclass, field
from typing import List

from quant1x.level1.protocol import (
    FLAG_UNCOMPRESSED,
    COMMAND_BLOCK_DATA,
    sequence_id,
)

BLOCK_CHUNKS_SIZE = 0x7530

@dataclass
class BlockInfo:
    block_name: str = ""
    block_type: int = 0
    stock_count: int = 0
    code_list: List[str] = field(default_factory=list)

class BlockInfoRequest:
    """
    板块数据请求
    """
    def __init__(self, filename: str, offset: int):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = sequence_id()
        self.packet_type = 0x01
        self.pkg_len1 = 0
        self.pkg_len2 = 0
        self.method = COMMAND_BLOCK_DATA
        
        self.start = offset
        self.size = BLOCK_CHUNKS_SIZE
        self.block_filename = filename

    def serialize(self) -> bytes:
        # Body: Start(4) + Size(4) + BlockFilename(100) = 108 bytes
        # PkgLen = Body + 2 = 110 (0x6E)
        self.pkg_len1 = 0x6E
        self.pkg_len2 = 0x6E
        
        header = struct.pack('<B I B H H H', self.zip_flag, self.seq_id, self.packet_type, self.pkg_len1, self.pkg_len2, self.method)
        
        # Ensure filename is 100 bytes
        filename_bytes = self.block_filename.encode('ascii')[:100].ljust(100, b'\x00')
        
        body = struct.pack('<I I', self.start, self.size) + filename_bytes
        return header + body

class BlockInfoResponse:
    """
    板块数据响应
    """
    def __init__(self):
        self.size = 0
        self.data = bytearray()

    def deserialize(self, data: bytes) -> None:
        if len(data) < 4:
            return
            
        self.size = struct.unpack('<I', data[:4])[0]
        if self.size > 0:
            # The rest is data
            self.data = bytearray(data[4:])
