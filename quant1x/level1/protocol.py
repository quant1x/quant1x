"""Minimal Python port of level1 protocol helpers used by ProtocolHandler.

This implements request header serialization for Hello1/Hello2/Heartbeat and
blocking `process_request_std` that mirrors the Rust/C++ behavior: write the
request to the blocking socket, read a 16-byte response header and the body,
handle optional zlib decompression, and return the response body bytes.
"""
from __future__ import annotations

import socket
import struct
import threading
import zlib
from typing import Tuple

COMMAND_HEARTBEAT = 0x0004
COMMAND_LOGIN1 = 0x000d
COMMAND_LOGIN2 = 0x0fdb

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


def process_request_raw_std(sock: socket.socket, req_buf: bytes) -> Tuple[bytes, bytes]:
    # send
    sock.sendall(req_buf)

    # read 16-byte response header
    hdr = _recv_exact(sock, 16)

    # parse header: <I B I B H H H> => u32, u8, u32, u8, u16, u16, u16
    i1, zip_flag, seq_id, i2, method, zip_size, unzip_size = struct.unpack('<IBIBHHH', hdr)

    if zip_size == 0:
        return hdr, b''

    body = _recv_exact(sock, zip_size)
    if zip_size != unzip_size:
        # zlib-compressed
        body = zlib.decompress(body)
    return hdr, body


def process_request_std(sock: socket.socket, req_buf: bytes) -> bytes:
    """Send request and return response body bytes (decompressed if needed)."""
    _, body = process_request_raw_std(sock, req_buf)
    return body


class Hello1Request:
    def __init__(self):
        self.zip_flag = 0x0C
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
        self.zip_flag = 0x0C
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
        self.zip_flag = 0x0C
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
