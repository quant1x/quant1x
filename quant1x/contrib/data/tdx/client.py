# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

import threading
from typing import List, Tuple, Optional, Any

from quant1x.data import status
from quant1x.net.conn import ConnectionHandle
from quant1x.net.tcp_client_pool import TcpConnectionPool
from . import protocol, config
from quant1x.log import logger


_std_pool_lock = threading.Lock()
_std_pool: Optional[TcpConnectionPool] = None

def _build_std_pool(*, min_conn: int, max_conn: int, servers: Optional[List[Tuple[str, int]]]) -> TcpConnectionPool:
    """构造并返回一个镜像C++ tdx_connection_pool的TcpConnectionPool。

    - 读取缓存文件并确定是否运行检测（盘前陈旧性）。
    - 如果运行检测，将检测到的列表持久化到缓存并限制并发。
    - 始终读取缓存并从中（或从`servers`）播种端点。
    允许来自检测/缓存IO的异常传播，以便调用者看到初始化失败（快速失败），与C++行为一致。
    """
    handler = protocol.StandardProtocolHandler()

    # 默认并发受max_conn限制（C++默认使用10）
    default_concurrency = max_conn

    discovered: List[Tuple[str, int]] = []

    # 决定是否更新服务器缓存
    cache_fn = config._cache_filename()
    # 如果缓存文件不存在，则创建或更新缓存文件
    create_or_update = status.should_initialize_file(cache_fn)
    if create_or_update:
        total_candidates = len(getattr(config, 'StandardServerList', []))
        detected = []
        if total_candidates > 0:
            detected = config.detect(conn_limit=total_candidates)
        if detected:
            try:
                config.write_cache(detected)
            except Exception:
                logger.error("level1._build_pool: failed to write server cache")
        try:
            if detected:
                default_concurrency = min(default_concurrency, max(1, len(detected)))
        except Exception:
            pass

    # 读取缓存的服务器
    try:
        cached = config.read_cache()
        if cached:
            for s in cached:
                h = s.get("host") or s.get("Host")
                p_obj: Any = s.get("port") or s.get("Port")
                try:
                    p = int(str(p_obj)) if p_obj is not None else None
                except Exception:
                    p = None
                if isinstance(h, str) and p is not None:
                    discovered.append((h, p))
    except Exception:
        logger.exception("level1._build_pool: failed to read server cache")

    pool = TcpConnectionPool(min_conn, default_concurrency, handler)

    # 从提供的服务器或发现的缓存中播种端点
    if servers:
        for host, port in servers:
            pool.add_endpoint(host, port)
    else:
        for h, p in discovered:
            pool.add_endpoint(h, p)

    return pool


def init_std_pool(servers: Optional[List[Tuple[str, int]]] = None, *, min_conn: int = 1, max_conn: int = 10):
    """初始化模块级连接池单例。

    参数:
        servers: 可选的(host, port)元组列表，用于播种池。
                 如果省略，则创建没有端点的池，调用者必须通过`_pool.add_endpoint(host, port)`添加端点。
        min_conn: 池维护的最小连接数。
        max_conn: 池允许的最大连接数。

    必须在应用程序启动期间在调用`client()`之前调用一次。重复调用无效。
    """
    global _std_pool
    with _std_pool_lock:
        if _std_pool is not None:
            return
        # 构建池并分配；允许异常传播，以便调用者观察初始化失败（匹配C++行为）。
        _std_pool = _build_std_pool(min_conn=min_conn, max_conn=max_conn, servers=servers)

def get_std_conn() -> ConnectionHandle:
    """返回一个到level1服务器的池化连接句柄。

    用法:
        with get_std_conn() as conn:
            # 使用 ConnectionHandle 提供的 I/O 方法（例如用于自定义协议）
            conn.sendall(b'...')
            ...

    如果池没有配置端点，则引发RuntimeError。
    """
    if _std_pool is None:
        # 通过单个公共初始化函数延迟初始化。
        init_std_pool()
    assert _std_pool is not None
    return _std_pool.acquire()
