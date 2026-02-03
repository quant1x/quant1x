# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations
from typing import Callable
import socket
from .endpoint import Endpoint
from quant1x.log import logger

class Connection:
    """Wrapper for a connected socket and its endpoint.

    Lifecycle is managed by TcpConnectionPool; do not close externally unless
    explicitly desired.
    """

    def __init__(self, sock: socket.socket, endpoint: Endpoint):
        if sock is None or sock.fileno() == -1:
            raise ValueError("Socket must be connected")
        self._sock = sock
        self._endpoint = endpoint

    @property
    def socket(self) -> socket.socket:
        return self._sock

    @property
    def endpoint(self) -> Endpoint:
        return self._endpoint

    def close(self) -> None:
        try:
            try:
                self._sock.shutdown(socket.SHUT_RDWR)
            except Exception:
                pass
            self._sock.close()
        finally:
            return

    def is_open(self) -> bool:
        try:
            return self._sock.fileno() != -1
        except Exception:
            return False


class ConnectionHandle:
    """A RAII-style handle returned by TcpConnectionPool.acquire().

    The handle can be used as a context manager. When closed/released it will
    return the Connection object to the originating pool.
    """

    def __init__(self, conn: Connection, releaser: Callable[[Connection], None]):
        self._conn = conn
        self._releaser = releaser
        self._released = False

    def __enter__(self) -> "ConnectionHandle":
        # 返回句柄本身，使调用方在使用 `with` 时得到 `ConnectionHandle`（而非原始 `Connection`）。
        # 这样可以屏蔽底层 socket，鼓励通过句柄提供的 API 进行操作。
        return self

    # 在句柄上暴露最小的类 socket API（sendall/recv/settimeout），
    # 以便调用方（或协议层）在不直接访问原始 socket 的情况下进行 I/O 操作。
    def sendall(self, data: bytes) -> None:
        return self._conn.socket.sendall(data)

    def recv(self, n: int) -> bytes:
        return self._conn.socket.recv(n)

    def settimeout(self, t: float) -> None:
        try:
            return self._conn.socket.settimeout(t)
        except Exception:
            return None

    def __exit__(self, exc_type, exc, tb):
        """
        上下文管理器退出时调用的方法，负责释放资源。
        
        Args:
            exc_type: 异常类型，如果没有异常则为 None
            exc: 异常实例，如果没有异常则为 None
            tb: 回溯对象，如果没有异常则为 None
        """
        self.release()

    def release(self) -> None:
        """
        释放连接资源
        
        如果连接尚未被释放，则调用释放函数并标记为已释放状态。
        
        Args:
            无
        
        Returns:
            None: 无返回值
        
        Raises:
            任何由_releaser函数可能抛出的异常
        """
        if not self._released:
            try:
                self._releaser(self._conn)
            finally:
                self._released = True

    def __del__(self):
        # 尽力自动释放（用于模拟 C++ unique_ptr 的析构行为）
        try:
            self.release()
        except Exception:
            logger.exception("Error during auto-release of connection")


