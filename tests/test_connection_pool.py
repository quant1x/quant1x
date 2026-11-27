# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X Team
# SPDX-License-Identifier: MIT

import threading
import socket
import time


from quant1x.io.connection_pool import TcpConnectionPool
from quant1x.io.operation_handler import NetworkOperationHandler


class MockNetworkHandler(NetworkOperationHandler):
    def __init__(self):
        super().__init__()
        # shorter values for tests
        self.set_timeout(2.0)
        self.set_check_interval(0.2)

    def handshake(self, sock: socket.socket) -> bool:
        # no extra handshake, succeed
        return True

    def keepalive(self, sock: socket.socket) -> bool:
        # treat socket as alive if fileno is valid
        try:
            return sock.fileno() != -1
        except Exception:
            return False


def _start_dummy_server(listen_ready: threading.Event, port_holder: list):
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind(('127.0.0.1', 0))
    srv.listen(5)
    _addr, port = srv.getsockname()
    port_holder.append(port)
    listen_ready.set()

    try:
        while True:
            conn, _ = srv.accept()
            # 逐字节回声, 连接在客户端主动关闭前一直保持
            threading.Thread(target=_echo_until_closed, args=(conn,), daemon=True).start()
    except Exception:
        try:
            srv.close()
        except Exception:  # noqa: S110 - 尽力而为: 关闭连接失败无需处理
            pass


def _echo_until_closed(conn: socket.socket) -> None:
    try:
        while True:
            data = conn.recv(1)
            if not data:
                break
            conn.sendall(data)
    except Exception:  # noqa: S110 - 尽力而为: 对端断开即结束回声
        pass
    finally:
        conn.close()


def test_basic_acquire_release():
    ready = threading.Event()
    port_holder = []
    t = threading.Thread(target=_start_dummy_server, args=(ready, port_holder), daemon=True)
    t.start()
    ready.wait(timeout=2.0)
    port = port_holder[0]

    handler = MockNetworkHandler()
    pool = TcpConnectionPool(1, 2, handler)
    pool.add_endpoint('127.0.0.1', port, 2)

    # acquire 返回 ConnectionHandle, 用它做一次实际收发, 确认连到的是刚登记的端点
    with pool.acquire() as handle:
        assert handle is not None
        handle.settimeout(2.0)
        handle.sendall(b'q')
        assert handle.recv(1) == b'q'

    # 退出 with 后连接归还空闲池, 活跃计数归零
    time.sleep(0.1)
    assert pool.active_connection_count == 0
    assert pool.idle_connection_count == 1
    # cleanup
    pool.stop()
