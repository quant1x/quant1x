# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import time
from typing import Any

import quant1x.level1.client as l1client
from quant1x.level1.client import init_std_pool, get_std_conn
from quant1x.level1 import config as l1config


def test_real_network_handshake_first_available():
    """尝试与真实候选服务器建立连接并完成握手。

    说明：
    - 本测试直接与 `StandardServerList` 中的真实服务器建立 TCP 连接。
    - 为避免长时间阻塞，只尝试前 N 个候选地址（默认 8）。
    - 成功则立刻通过；若全部失败则测试失败。
    """
    candidates = l1config.StandardServerList[:8]
    assert candidates, "StandardServerList is empty"

    success = False
    last_exc: Any = None

    for entry in candidates:
        host = entry.get("Host")
        port = int(entry.get("Port"))
        try:
            # initialize pool pointing to single real server
            init_std_pool(servers=[(host, port)], min_conn=1, max_conn=1)

            # speed up timeouts for test (internal API access)
            try:
                if getattr(l1client, '_std_pool', None) is not None:
                    l1client._std_pool.network_handler.set_timeout(3)
                    l1client._std_pool.network_handler.set_check_interval(1)
            except Exception:
                # ignore if internals differ
                pass

            # acquire triggers connection creation and handshake
            handle = get_std_conn()
            with handle as conn:
                # ConnectionHandle 不再直接暴露底层 socket，验证基本 I/O API 可用
                assert hasattr(conn, 'sendall') and hasattr(conn, 'recv')
            success = True
            break
        except Exception as e:
            last_exc = e
            # try next candidate
            time.sleep(0.1)
            continue

    if not success:
        raise AssertionError(f"No server responded to handshake; last error: {last_exc}")
