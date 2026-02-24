# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

from __future__ import annotations

import threading
from typing import List, Tuple, Optional, Any

from quant1x.data import status
from quant1x.data.meta.exchange import Exchange
from quant1x.net.conn import ConnectionHandle
from quant1x.net.tcp_client_pool import TcpConnectionPool
from . import protocol, config
from quant1x.log import logger

_std_pool_lock = threading.Lock()
_std_pool: Optional[TcpConnectionPool] = None

def _build_std_pool(*, min_conn: int, max_conn: int, servers: Optional[List[Tuple[str, int]]]) -> TcpConnectionPool:
    """构造并返回一个镜像C++ tdx_connection_pool的TcpConnectionPool

    - 读取缓存文件并确定是否运行检测(盘前陈旧性)
    - 如果运行检测, 将检测到的列表持久化到缓存并限制并发
    - 始终读取缓存并从中(或从`servers`)播种端点
    允许来自检测/缓存IO的异常传播, 以便调用者看到初始化失败(快速失败), 与C++行为一致
    """
    key = 'standard'
    handler = protocol.StandardProtocolHandler()

    # 默认并发受max_conn限制(C++默认使用10)
    default_concurrency = max_conn

    discovered: List[Tuple[str, int]] = []

    # 决定是否更新服务器缓存
    cache_fn = config._cache_filename()
    # 如果缓存文件不存在, 则创建或更新缓存文件
    create_or_update = status.should_initialize_file(cache_fn)
    if not create_or_update:
        cached = config.read_cache(key)
        create_or_update = len(cached) == 0
        
    if create_or_update:
        detected = config.detect(conn_limit=10)
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
        cached = config.read_cache(key)
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
    """初始化模块级连接池单例

    参数:
        servers: 可选的(host, port)元组列表, 用于播种池
                 如果省略, 则创建没有端点的池, 调用者必须通过`_pool.add_endpoint(host, port)`添加端点
        min_conn: 池维护的最小连接数
        max_conn: 池允许的最大连接数

    必须在应用程序启动期间在调用`client()`之前调用一次, 重复调用无效
    """
    global _std_pool
    with _std_pool_lock:
        if _std_pool is not None:
            return
        # 构建池并分配；允许异常传播, 以便调用者观察初始化失败(匹配C++行为)
        _std_pool = _build_std_pool(min_conn=min_conn, max_conn=max_conn, servers=servers)

def get_std_conn() -> ConnectionHandle:
    """返回一个到level1服务器的池化连接句柄

    用法:
        with get_std_conn() as conn:
            # 使用 ConnectionHandle 提供的 I/O 方法(例如用于自定义协议)
            conn.sendall(b'...')
            ...

    如果池没有配置端点, 则引发RuntimeError.
    """
    if _std_pool is None:
        # 通过单个公共初始化函数延迟初始化.
        init_std_pool()
    assert _std_pool is not None
    return _std_pool.acquire()


_ext_pool_lock = threading.Lock()
_ext_pool: Optional[TcpConnectionPool] = None

def _build_ext_pool(*, min_conn: int, max_conn: int, servers: Optional[List[Tuple[str, int]]]) -> TcpConnectionPool:
    """构造并返回一个镜像C++ tdx_connection_pool的TcpConnectionPool

    - 读取缓存文件并确定是否运行检测(盘前陈旧性)
    - 如果运行检测, 将检测到的列表持久化到缓存并限制并发
    - 始终读取缓存并从中(或从`servers`)播种端点
    允许来自检测/缓存IO的异常传播, 以便调用者看到初始化失败(快速失败), 与C++行为一致
    """
    key = 'extension'
    handler = protocol.ExtensionProtocolHandler()

    # 默认并发受max_conn限制(C++默认使用10)
    default_concurrency = max_conn

    discovered: List[Tuple[str, int]] = []

    # 决定是否更新服务器缓存
    cache_fn = config._cache_filename()
    # 如果缓存文件不存在, 则创建或更新缓存文件
    create_or_update = status.should_initialize_file(cache_fn)
    if not create_or_update:
        cached = config.read_cache(key)
        create_or_update = len(cached) == 0
        
    if create_or_update:
        detected = config.detect(conn_limit=10)
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
        cached = config.read_cache(key)
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
            default_concurrency = min(default_concurrency, max(1, len(discovered)))
    except Exception:
        logger.exception("level1._build_pool: failed to read server cache")
    logger.warning(f"discovered: {discovered}, default_concurrency: {default_concurrency}, min_conn: {min_conn}, max_conn: {max_conn}")
    pool = TcpConnectionPool(min_conn, default_concurrency, handler)

    # 从提供的服务器或发现的缓存中播种端点
    if servers:
        logger.debug(f"Using provided servers for extension pool: {servers}")
        for host, port in servers:
            pool.add_endpoint(host, port)
    else:
        logger.debug(f"Using discovered servers for extension pool: {discovered}")
        for h, p in discovered:
            pool.add_endpoint(h, p)
    logger.warning(f"Extension pool initialized with endpoints: min_connections={pool.min_connections}, max_connections={pool.max_connections}")
    return pool


def init_ext_pool(servers: Optional[List[Tuple[str, int]]] = None, *, min_conn: int = 1, max_conn: int = 10):
    """初始化模块级连接池单例

    参数:
        servers: 可选的(host, port)元组列表, 用于播种池
                 如果省略, 则创建没有端点的池, 调用者必须通过`_pool.add_endpoint(host, port)`添加端点
        min_conn: 池维护的最小连接数
        max_conn: 池允许的最大连接数

    必须在应用程序启动期间在调用`client()`之前调用一次, 重复调用无效
    """
    global _ext_pool
    with _ext_pool_lock:
        if _ext_pool is not None:
            return
        # 构建池并分配；允许异常传播, 以便调用者观察初始化失败(匹配C++行为)
        _ext_pool = _build_ext_pool(min_conn=min_conn, max_conn=max_conn, servers=servers)

def get_ext_conn() -> ConnectionHandle:
    """返回一个到level1服务器的池化连接句柄

    用法:
        with get_ext_conn() as conn:
            # 使用 ConnectionHandle 提供的 I/O 方法(例如用于自定义协议)
            conn.sendall(b'...')
            ...

    如果池没有配置端点, 则引发RuntimeError.
    """
    if _ext_pool is None:
        # 通过单个公共初始化函数延迟初始化.
        init_ext_pool()
    assert _ext_pool is not None
    return _ext_pool.acquire()

def get_conn(exchange: Exchange = Exchange.SSE) -> ConnectionHandle:
    """根据exchange参数返回对应的连接池连接句柄"""
    if exchange in (Exchange.SSE, Exchange.SZSE, Exchange.BSE):
        return get_std_conn()
    else:
        return get_ext_conn()

if __name__ == '__main__':
    import pandas as pd
    # 市场代码列表
    from .level1.ext import MarketList
    conn = get_ext_conn()
    market_list = MarketList()
    protocol.process_level1_new(conn, market_list)
    if market_list.reply:
        df = pd.DataFrame(market_list.reply)
        df.to_csv('ext-markets.csv', index=False)
        print(df)
    
    # 证券代码总数
    from .level1.ext import InstrumentCount
    ic = InstrumentCount()
    protocol.process_level1_new(conn, ic)
    print(f'instrument count: {ic.reply}')
    
    # 证券代码列表
    from .level1.ext import InstrumentInfo
    list =[]
    start = 0
    offset = InstrumentInfo.PRE_REQUEST_MAX
    while True:
        ii = InstrumentInfo(start, offset)
        protocol.process_level1_new(conn, ii)
        if ii.reply['count']>0:
            list.extend(ii.reply['list'])
        else:
            break
        start += offset
    df = pd.DataFrame(list)
    df.to_csv('ext-instruments.csv', index=False)
    print(df)
    # K线数据
    from .level1.ext import InstrumentBars
    #bars = InstrumentBars(9, 0x17, ticker='HSIL8', start=0, count=700)
    bars = InstrumentBars(8, 31, ticker='00700', start=0, count=700)
    #bars = InstrumentBars(9, 12, ticker='A_IXIC', start=0, count=700)
    protocol.process_level1_new(conn, bars)
    if bars.reply:
        df = pd.DataFrame(bars.reply)
        print(df)