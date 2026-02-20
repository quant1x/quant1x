# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

"""TcpConnectionPool 的 Python 实现（参考 C++ 设计）。

本模块实现：
- `Connection`：包裹底层套接字的对象（生命周期由连接池管理）
- `TcpConnectionPool`：线程安全的连接池，尽量保持与 C++ API/语义一致

说明：
- 端点以 `(host, port)` 元组表示，由 `quant1x.io.endpoint.EndpointManager` 管理。
- 网络处理器需要实现以下方法：
    - `timeout() -> float`：超时时间（秒）
    - `handshake(socket.socket) -> bool`：握手（成功返回 True）
    - `keepalive(socket.socket) -> bool`：心跳/保活检查
    - `check_interval() -> float`：心跳/维护定时器间隔（秒）

连接池返回一个 `ConnectionHandle`（上下文管理器），在 `with` 退出或
句柄被回收时会自动把连接归还到池中。
"""
from __future__ import annotations

import socket
import threading
import time
from collections import deque
from typing import Optional, Deque

from .endpoint import EndpointManager, Endpoint
from .conn import Connection, ConnectionHandle
from .handler import NetworkOperationHandler

from quant1x.log import logger


class TcpConnectionPool:
    def __init__(self, min_connections: int, max_connections: int, network_handler: NetworkOperationHandler):
        if min_connections > max_connections:
            raise ValueError("min_connections cannot be greater than max_connections")
        if max_connections == 0:
            raise ValueError("max_connections cannot be zero")
        if network_handler is None:
            raise ValueError("network_handler cannot be None")

        self.min_connections = int(min_connections)
        self.max_connections = int(max_connections)
        self.endpoint_weight = 1
        self.network_handler = network_handler
        self.endpoint_manager = EndpointManager()

        self._connections_mutex = threading.Lock()
        self._idle_connections: Deque[Connection] = deque()
        self.idle_connection_count = 0
        self.active_connection_count = 0

        self.running = False
        self._heartbeat_timer: Optional[threading.Timer] = None

        # 启动后台线程用于运行心跳和连接创建尝试
        self._worker_threads = []
        # 创建 2 个工作线程以模拟 C++ 的 IO 线程（这里线程仅用于运行定时器）
        for i in range(2):
            t = threading.Thread(target=self._worker_loop, name=f"connpool-io-{i}", daemon=True)
            self._worker_threads.append(t)
            t.start()

        # 启动连接池
        self.start()

    # ---------- 端点帮助函数（兼容 C++ 命名） ----------
    def add_endpoint(self, ip: str, port: int, weight: int = 0) -> bool:
        return self.endpoint_manager.add_endpoint(ip, port, weight or self.endpoint_weight)

    def addEndpoint(self, ip: str, port: int, weight: int = 0) -> bool:
        return self.add_endpoint(ip, port, weight)

    def add_endpoint_obj(self, endpoint: Endpoint, weight: int = 0) -> bool:
        return self.endpoint_manager.add_endpoint_obj(endpoint, weight or self.endpoint_weight)

    # ---------- 连接池核心 API ----------
    def acquire(self) -> ConnectionHandle:
        # 1. 尝试重用空闲连接
        raw_conn: Optional[Connection] = None
        with self._connections_mutex:
            if self._idle_connections:
                raw_conn = self._idle_connections.popleft()
                self.idle_connection_count -= 1
                logger.debug("Reused connection from pool: {}", raw_conn)

        # 2. 若无空闲连接则创建新连接
        endpoint = None
        if raw_conn is None:
            logger.debug("Creating new connection...")
            ep = self.endpoint_manager.acquire_endpoint()
            if ep is None:
                logger.error("No available endpoints")
                raise RuntimeError("No available endpoints")
            endpoint = ep
            sock = None
            try:
                # 使用 network_handler 提供的超时创建并连接套接字
                timeout = float(self.network_handler.timeout())
                sock = socket.create_connection(endpoint, timeout=timeout)
                # 同时为后续的 recv/send 设置套接字超时，
                # 这样协议层的阻塞读（例如 process_request_std）在超时后会抛出 socket.timeout，避免永久阻塞。
                try:
                    sock.settimeout(timeout)
                except Exception:
                    pass
                # 设置 TCP_NODELAY 以禁用 Nagle 算法
                try:
                    sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
                except Exception:
                    pass

                # 进行握手
                ok = self.network_handler.handshake(sock)
                if not ok:
                    sock.close()
                    self.endpoint_manager.release_endpoint(endpoint)
                    logger.error("Handshake failed with {}:{}", endpoint[0], endpoint[1])
                    raise RuntimeError("Handshake failed")

                raw_conn = Connection(sock, endpoint)
                logger.debug("Created new connection {}", raw_conn)
            except Exception:
                if sock is not None:
                    try:
                        sock.close()
                    except Exception:
                        pass
                # 释放端点预定（当连接创建失败或握手失败时）
                if endpoint is not None:
                    self.endpoint_manager.release_endpoint(endpoint)
                raise

        # 创建用于归还连接的回调（deleter/releaser）
        def deleter(conn: Connection) -> None:
            try:
                self.release(conn)
            except Exception:
                logger.exception("Error during auto-release")

        self.active_connection_count += 1
        return ConnectionHandle(raw_conn, deleter)

    def release(self, conn: Connection) -> None:
        if conn is None:
            return
        conn_id = id(conn)
        logger.debug("Returning connection {}", conn_id)

        # 注意：此处不要释放端点（与 C++ 的语义一致）
        with self._connections_mutex:
            self._idle_connections.append(conn)
            self.idle_connection_count += 1
            self.active_connection_count -= 1
            logger.debug("Connection {} returned to pool", conn_id)

    def close_connection(self, conn: Optional[Connection]) -> None:
        if conn is None:
            return
        conn_id = id(conn)
        logger.debug("Closing connection {}", conn_id)
        # 1. 释放端点
        try:
            self.endpoint_manager.release_endpoint(conn.endpoint)
        except Exception:
            logger.exception("Error releasing endpoint")
        # 2. 关闭套接字
        try:
            conn.close()
        except Exception:
            logger.exception("Error closing connection socket")
        # 3. 更新计数器
        with self._connections_mutex:
            self.idle_connection_count = max(0, self.idle_connection_count - 1)

    def start(self) -> None:
        if self.running:
            return
        self.running = True
        self._start_heartbeat_timer()

    def stop(self) -> None:
        self.running = False
        if self._heartbeat_timer:
            self._heartbeat_timer.cancel()
        self._close_all_connections()

    def get_endpoint_stats(self, host: str, port: int):
        ep = (host, int(port))
        try:
            return self.endpoint_manager.get_endpoint_stats(ep)
        except KeyError as e:
            raise RuntimeError("Invalid endpoint: %s" % e)

    # ---------- 内部辅助函数 ----------
    def _start_heartbeat_timer(self) -> None:
        interval = float(self.network_handler.check_interval())

        def tick():
            if not self.running:
                return
            try:
                self._check_connections()
                self._try_create_connections()
            except Exception:
                logger.exception("Exception in heartbeat tick")
            # 重新调度定时器
            if self.running:
                self._heartbeat_timer = threading.Timer(interval, tick)
                self._heartbeat_timer.daemon = True
                self._heartbeat_timer.start()

        self._heartbeat_timer = threading.Timer(interval, tick)
        self._heartbeat_timer.daemon = True
        self._heartbeat_timer.start()

    def _check_connections(self) -> None:
        with self._connections_mutex:
            new_idle = deque()
            while self._idle_connections:
                conn = self._idle_connections.popleft()
                if conn is None:
                    continue
                try:
                    alive = self.network_handler.keepalive(conn.socket)
                except Exception:
                    # 将异常视为连接已死（需要关闭）
                    self.close_connection(conn)
                    continue
                if not alive:
                    self.close_connection(conn)
                else:
                    new_idle.append(conn)
            self._idle_connections = new_idle

    def _try_create_connections(self) -> None:
        max_retries = 10
        retry_count = 0
        while self.active_connection_count + self.idle_connection_count < self.min_connections and retry_count < max_retries:
            available = self.endpoint_manager.get_available_resources()
            if available == 0:
                logger.warning("endpoint resources exhausted, retry {}/{}", retry_count, max_retries)
                time.sleep(0.1)
                retry_count += 1
                continue
            else:
                try:
                    handle = self.acquire()
                    # 立即释放以将连接放回空闲池
                    handle.release()
                    retry_count += 1
                    logger.debug("Supplemented 1 connection")
                except Exception:
                    logger.exception("Error acquiring new connection")
                    break

    def _close_all_connections(self) -> None:
        with self._connections_mutex:
            for conn in list(self._idle_connections):
                try:
                    if conn and conn.is_open():
                        conn.close()
                except Exception:
                    logger.exception("Error closing idle connection")
            self._idle_connections.clear()
            self.idle_connection_count = 0

    def _worker_loop(self) -> None:
        # 简单循环以保持线程存活；实际工作由定时器执行
        while self.running:
            time.sleep(0.5)
