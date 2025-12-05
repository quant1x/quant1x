# -*- coding: UTF-8 -*-

from __future__ import annotations

import socket
import struct
import threading
import zlib
from typing import Tuple

COMMAND_HEARTBEAT = 0x0004                # 心跳维持
COMMAND_LOGIN1 = 0x000d                   # 第一次登录
COMMAND_LOGIN2 = 0x0fdb                   # 第二次登录
COMMAND_XDXR_INFO = 0x000f                # 除权除息信息
COMMAND_FINANCE_INFO = 0x0010             # 财务信息
COMMAND_PING = 0x0015                     # 测试连接
COMMAND_COMPANY_CATEGORY = 0x02cf         # 公司信息文件信息
COMMAND_COMPANY_CONTENT = 0x02d0          # 公司信息描述
COMMAND_SECURITY_COUNT = 0x044e           # 证券数量
COMMAND_SECURITY_LIST = 0x044d            # 证券列表
COMMAND_OLD_SECURITY_LIST = 0x0450        # 证券列表, 已废弃, 缺少北交所证券代码列表
COMMAND_INDEX_BARS = 0x052d               # 指数K线
COMMAND_SECURITY_BARS = 0x052d            # 股票K线
COMMAND_SECURITY_QUOTES_OLD = 0x053e      # 旧版行情信息
COMMAND_SECURITY_QUOTES_NEW = 0x054c      # 新版行情信息
COMMAND_MINUTE_TIME_DATA = 0x051d         # 分时数据
COMMAND_BLOCK_META = 0x02c5               # 板块文件信息
COMMAND_BLOCK_DATA = 0x06b9               # 板块文件数据
COMMAND_TRANSACTION_DATA = 0x0fc5         # 分笔成交信息
COMMAND_HISTORY_MINUTE_DATA = 0x0fb4      # 历史分时信息
COMMAND_HISTORY_TRANSACTION_DATA = 0x0fb5 # 历史分笔成交信息

FLAG_ZIP = 0x10
FLAG_UNCOMPRESSED = 0x0C
FLAG_ZIPPED = FLAG_ZIP | FLAG_UNCOMPRESSED

_seq_lock = threading.Lock()
_seq_id = 0


def sequence_id() -> int:
    global _seq_id
    with _seq_lock:
        _seq_id = (_seq_id + 1) & 0xFFFFFFFF
        return _seq_id


def _recv_exact(sock: socket.socket, n: int) -> bytes:
    buf = bytearray()
    while len(buf) < n:
        chunk = sock.recv(n - len(buf))
        if not chunk:
            raise ConnectionError("socket closed while reading")
        buf.extend(chunk)
    return bytes(buf)


def process(sock: socket.socket, request, response) -> None:
    """Send request and populate response."""
    req_buf = request.serialize()
    sock.sendall(req_buf)

    # read 16-byte response header
    hdr = _recv_exact(sock, 16)

    # parse header: <I B I B H H H> => u32, u8, u32, u8, u16, u16, u16
    i1, zip_flag, seq_id, i2, method, zip_size, unzip_size = struct.unpack('<IBIBHHH', hdr)

    if zip_size == 0:
        return

    body = _recv_exact(sock, zip_size)
    if zip_size != unzip_size:
        # zlib-compressed
        body = zlib.decompress(body)
    
    response.deserialize(body)


class Hello1Request:
    def __init__(self):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = sequence_id()
        self.packet_type = 0x01
        self.pkg_len1 = 0
        self.pkg_len2 = 0
        self.method = COMMAND_LOGIN1
        # padding bytes used by original implementation
        self.padding = bytes.fromhex('01')

    def serialize(self) -> bytes:
        self.pkg_len1 = 2 + len(self.padding)
        self.pkg_len2 = self.pkg_len1
        header = struct.pack('<B I B H H H', self.zip_flag, self.seq_id, self.packet_type, self.pkg_len1, self.pkg_len2, self.method)
        return header + self.padding


class Hello1Response:
    def __init__(self):
        self.info = ''

    def deserialize(self, data: bytes) -> None:
        # Follow Rust offset: 68
        offset = 68
        if len(data) >= offset:
            info_bytes = data[offset:]
            try:
                self.info = info_bytes.decode('gbk', errors='ignore').rstrip('\x00')
            except Exception:
                self.info = info_bytes.decode('utf-8', errors='ignore')


class Hello2Request:
    def __init__(self):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = sequence_id()
        self.packet_type = 0x01
        self.pkg_len1 = 0
        self.pkg_len2 = 0
        self.method = COMMAND_LOGIN2
        # padding taken from Rust example
        self.padding = bytes.fromhex('d5d0c9ccd6a4a8af0000008fc22540130000d500c9ccbdf0d7ea00000002')

    def serialize(self) -> bytes:
        self.pkg_len1 = 2 + len(self.padding)
        self.pkg_len2 = self.pkg_len1
        header = struct.pack('<B I B H H H', self.zip_flag, self.seq_id, self.packet_type, self.pkg_len1, self.pkg_len2, self.method)
        return header + self.padding


class Hello2Response:
    def __init__(self):
        self.info = ''

    def deserialize(self, data: bytes) -> None:
        # Rust uses offset 58
        offset = 58
        if len(data) >= offset:
            info_bytes = data[offset:]
            try:
                self.info = info_bytes.decode('gbk', errors='ignore').rstrip('\x00')
            except Exception:
                self.info = info_bytes.decode('utf-8', errors='ignore')


class HeartbeatRequest:
    def __init__(self):
        self.zip_flag = FLAG_UNCOMPRESSED
        self.seq_id = sequence_id()
        self.packet_type = 0x02
        self.pkg_len1 = 2
        self.pkg_len2 = 2
        self.method = COMMAND_HEARTBEAT

    def serialize(self) -> bytes:
        header = struct.pack('<B I B H H H', self.zip_flag, self.seq_id, self.packet_type, self.pkg_len1, self.pkg_len2, self.method)
        return header


class HeartbeatResponse:
    def __init__(self):
        self.info = ''

    def deserialize(self, data: bytes) -> None:
        # get_string(10) in Rust: read up to 10 bytes and strip at first NUL
        if len(data) >= 10:
            s = data[:10]
        else:
            s = data
        try:
            self.info = s.decode('gbk', errors='ignore').split('\x00', 1)[0]
        except Exception:
            self.info = s.decode('utf-8', errors='ignore').split('\x00', 1)[0]
