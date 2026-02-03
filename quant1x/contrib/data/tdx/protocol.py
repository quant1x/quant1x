# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

import struct
import zlib
from quant1x.net.conn import ConnectionHandle
from quant1x.log import logger

def _recv_exact(conn_like: ConnectionHandle, n: int) -> bytes:
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


def process(conn_handle: ConnectionHandle, request, response) -> None:
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

from quant1x.net.handler import NetworkOperationHandler

class StandardProtocolHandler(NetworkOperationHandler):
    """标准协议处理器，执行Hello1/Hello2握手和心跳。

    此实现调用`level1.protocol`来序列化请求，并在提供的套接字上执行阻塞读/写。
    """

    def handshake(self, conn) -> bool:
        # 使用阻塞请求助手执行Hello1然后Hello2
        try:
            from .level1.hello1 import Hello1Request, Hello1Response
            from .level1.hello2 import Hello2Request, Hello2Response
            

            req1 = Hello1Request()
            resp1 = Hello1Response()
            process(conn, req1, resp1)
            # 接受任何没有反序列化错误的Hello1响应。
            # C++实现不需要非空的Info字段。

            req2 = Hello2Request()
            resp2 = Hello2Response()
            process(conn, req2, resp2)
            # 接受任何没有反序列化错误的Hello2响应。
            # 如果两个阶段都没有异常完成，则返回True。
            return True
        except Exception as e:
            # 使用调试日志以避免在服务器检测期间产生噪音
            logger.exception('StandardProtocolHandler.handshake failed: {}', e)
            return False

    def keepalive(self, conn) -> bool:
        try:
            from .level1.heartbeat import HeartbeatRequest, HeartbeatResponse

            req = HeartbeatRequest()
            resp = HeartbeatResponse()
            process(conn, req, resp)
            return True
        except Exception as e:
            logger.exception('StandardProtocolHandler.keepalive failed: {}', e)
            return False

