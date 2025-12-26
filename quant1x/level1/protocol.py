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
COMMAND_COMPANY_CATEGORY = 0x02cf         # 公司信息分类
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

FLAG_ZIP = 0x10                  # 压缩标志
FLAG_UNCOMPRESSED = 0x0C        # 未压缩标志
FLAG_ZIPPED = FLAG_ZIP | FLAG_UNCOMPRESSED # 压缩标志

_seq_lock = threading.Lock()
_seq_id = 0


def sequence_id() -> int:
    global _seq_id
    with _seq_lock:
        _seq_id = (_seq_id + 1) & 0xFFFFFFFF
        return _seq_id


def _recv_exact(conn_like, n: int) -> bytes:
    """从支持 `recv(n)` 的对象读取恰好 `n` 字节。

    `conn_like` 预期为 `ConnectionHandle`（或任何实现 `recv` 的对象），
    用于屏蔽对原始 socket 的直接访问。
    """
    buf = bytearray()
    while len(buf) < n:
        chunk = conn_like.recv(n - len(buf))
        if not chunk:
            raise ConnectionError("socket closed while reading")
        buf.extend(chunk)
    return bytes(buf)


def process(conn_handle, request, response) -> None:
    """使用 `ConnectionHandle` 发送请求并填充响应对象。

    `conn_handle` 必须支持 `sendall(bytes)` 和 `recv(n)` 方法（由 `ConnectionHandle` 提供），
    以避免直接暴露原始 `socket.socket` 给调用方。
    """
    req_buf = request.serialize()
    conn_handle.sendall(req_buf)

    # 读取 16 字节响应头
    hdr = _recv_exact(conn_handle, 16)

    # 解析头部: <I B I B H H H> => u32, u8, u32, u8, u16, u16, u16
    i1, zip_flag, seq_id, i2, method, zip_size, unzip_size = struct.unpack('<IBIBHHH', hdr)

    if zip_size == 0:
        return

    body = _recv_exact(conn_handle, zip_size)
    if zip_size != unzip_size:
        # 如果压缩长度与解压长度不一致，则为 zlib 压缩数据，需要解压
        body = zlib.decompress(body)

    response.deserialize(body)
