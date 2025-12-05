# -*- coding: UTF-8 -*-
from __future__ import annotations

import threading
import logging
from typing import List, Tuple, Optional

from quant1x.net.connection_pool import TcpConnectionPool
from quant1x.net.operation_handler import NetworkOperationHandler
from typing import Any

import os
import time

log = logging.getLogger(__name__)


class StandardProtocolHandler(NetworkOperationHandler):
    """标准协议处理器，执行Hello1/Hello2握手和心跳。

    此实现调用`level1.protocol`来序列化请求，并在提供的套接字上执行阻塞读/写。
    """

    def handshake(self, sock) -> bool:
        # 使用阻塞请求助手执行Hello1然后Hello2
        try:
            from quant1x.level1.protocol import Hello1Request, Hello1Response, Hello2Request, Hello2Response, process

            req1 = Hello1Request()
            resp1 = Hello1Response()
            process(sock, req1, resp1)
            # 接受任何没有反序列化错误的Hello1响应。
            # C++实现不需要非空的Info字段。

            req2 = Hello2Request()
            resp2 = Hello2Response()
            process(sock, req2, resp2)
            # 接受任何没有反序列化错误的Hello2响应。
            # 如果两个阶段都没有异常完成，则返回True。
            return True
        except Exception as e:
            # 使用调试日志以避免在服务器检测期间产生噪音
            log.debug('StandardProtocolHandler.handshake failed: %s', e)
            return False

    def keepalive(self, sock) -> bool:
        try:
            from quant1x.level1.protocol import HeartbeatRequest, HeartbeatResponse, process

            req = HeartbeatRequest()
            resp = HeartbeatResponse()
            process(sock, req, resp)
            return True
        except Exception as e:
            log.exception('StandardProtocolHandler.keepalive failed: %s', e)
            return False


_std_pool_lock = threading.Lock()
_std_pool: Optional[TcpConnectionPool] = None

def _build_std_pool(*, min_conn: int, max_conn: int, servers: Optional[List[Tuple[str, int]]]) -> TcpConnectionPool:
    """构造并返回一个镜像C++ tdx_connection_pool的TcpConnectionPool。

    - 读取缓存文件并确定是否运行检测（盘前陈旧性）。
    - 如果运行检测，将检测到的列表持久化到缓存并限制并发。
    - 始终读取缓存并从中（或从`servers`）播种端点。
    允许来自检测/缓存IO的异常传播，以便调用者看到初始化失败（快速失败），与C++行为一致。
    """
    from quant1x.level1 import config as l1config

    handler = StandardProtocolHandler()

    # 默认并发受max_conn限制（C++默认使用10）
    default_concurrency = max_conn

    discovered: List[Tuple[str, int]] = []

    # 决定是否更新服务器缓存
    cache_fn = None
    try:
        cache_fn = l1config._cache_filename()
    except Exception:
        cache_fn = None

    need_update = False
    try:
        if not cache_fn or not os.path.isfile(cache_fn) or os.path.getsize(cache_fn) == 0:
            need_update = True
        else:
            mtime = os.path.getmtime(cache_fn)
            now = time.time()
            t = time.localtime()
            try:
                pre_ts = time.mktime((t.tm_year, t.tm_mon, t.tm_mday, 9, 0, 0, t.tm_wday, t.tm_yday, t.tm_isdst))
            except Exception:
                pre_ts = 0.0
            if pre_ts and now >= pre_ts and mtime < pre_ts:
                need_update = True
    except Exception:
        need_update = True

    if need_update:
        total_candidates = len(getattr(l1config, 'StandardServerList', []))
        detected = []
        if total_candidates > 0:
            detected = l1config.detect(conn_limit=total_candidates)
        if detected:
            try:
                l1config.write_cache(detected)
            except Exception:
                log.exception("level1._build_pool: failed to write server cache")
        try:
            if detected:
                default_concurrency = min(default_concurrency, max(1, len(detected)))
        except Exception:
            pass

    # 读取缓存的服务器
    try:
        cached = l1config.read_cache()
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
        log.exception("level1._build_pool: failed to read server cache")

    pool = TcpConnectionPool(min_conn, default_concurrency, handler)

    # 从提供的服务器或发现的缓存中播种端点
    if servers:
        for host, port in servers:
            pool.add_endpoint(host, port)
    else:
        for h, p in discovered:
            pool.add_endpoint(h, p)

    return pool


def get_std_conn():
    """返回一个到level1服务器的池化连接句柄。

    用法:
        with get_std_conn() as conn:
            sock = conn.socket
            ...

    如果池没有配置端点，则引发RuntimeError。
    """
    if _std_pool is None:
        # 通过单个公共初始化函数延迟初始化。
        init_std_pool()
    assert _std_pool is not None
    return _std_pool.acquire()


def init_std_pool(servers: Optional[List[Tuple[str, int]]] = None, *, min_conn: int = 1, max_conn: int = 10) -> None:
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

# 兼容旧代码
client = get_std_conn
init_pool = init_std_pool
