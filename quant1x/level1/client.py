"""Python `level1.client()` implementation that uses the connection pool and
the `OperationHandler` interface implemented earlier.

This module provides a small, explicit initializer for the singleton
`TcpConnectionPool`. Unlike an earlier prototype, it does NOT read
configuration from environment variables. Call `init_pool(...)` from your
application startup code and provide the server endpoints programmatically
to match the C++ design where endpoints are configured explicitly.

The `ProtocolHandler` here is a minimal implementation that uses
`level1.protocol` helpers for handshake and keepalive. Replace or extend it
if you need additional protocol checks.
"""
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


class ProtocolHandler(NetworkOperationHandler):
    """Protocol handler that performs Hello1/Hello2 handshake and Heartbeat.

    This implementation calls into `level1.protocol` to serialize requests
    and perform blocking read/write on the provided socket.
    """

    def handshake(self, sock) -> bool:
        # perform Hello1 then Hello2 using blocking request helper
        try:
            from quant1x.level1.protocol import Hello1Request, Hello1Response, Hello2Request, Hello2Response, process_request_std

            req1 = Hello1Request()
            body1 = process_request_std(sock, req1.serialize())
            resp1 = Hello1Response()
            resp1.deserialize(body1)
            # Accept any Hello1 response that deserializes without error.
            # C++ implementation does not require non-empty Info field.

            req2 = Hello2Request()
            body2 = process_request_std(sock, req2.serialize())
            resp2 = Hello2Response()
            resp2.deserialize(body2)
            # Accept any Hello2 response that deserializes without error.
            # Return True if both phases completed without exceptions.
            return True
        except Exception as e:
            log.exception('ProtocolHandler.handshake failed: %s', e)
            return False

    def keepalive(self, sock) -> bool:
        try:
            from quant1x.level1.protocol import HeartbeatRequest, HeartbeatResponse, process_request_std

            req = HeartbeatRequest()
            body = process_request_std(sock, req.serialize())
            resp = HeartbeatResponse()
            resp.deserialize(body)
            return True
        except Exception as e:
            log.exception('ProtocolHandler.keepalive failed: %s', e)
            return False


_pool_lock = threading.Lock()
_pool: Optional[TcpConnectionPool] = None

def _build_pool(*, min_conn: int, max_conn: int, servers: Optional[List[Tuple[str, int]]]) -> TcpConnectionPool:
    """Construct and return a TcpConnectionPool mirroring C++ tdx_connection_pool.

    - Read cache file and determine whether to run detection (pre-market staleness).
    - If detection runs, persist detected list to cache and limit concurrency.
    - Always read the cache and seed endpoints from it (or from `servers`).
    Exceptions from detect/cache IO are allowed to propagate so callers see
    initialization failures (fail-fast), consistent with C++ behaviour.
    """
    from quant1x.level1 import config as l1config

    handler = ProtocolHandler()

    # default concurrency bounded by max_conn (C++ uses 10 as default)
    default_concurrency = max_conn

    discovered: List[Tuple[str, int]] = []

    # decide whether to update server cache
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

    # read cached servers
    try:
        cached = l1config.read_cache()
        if cached:
            for s in cached:
                h = s.get("Host")
                p_obj: Any = s.get("Port")
                try:
                    p = int(str(p_obj)) if p_obj is not None else None
                except Exception:
                    p = None
                if isinstance(h, str) and p is not None:
                    discovered.append((h, p))
    except Exception:
        log.exception("level1._build_pool: failed to read server cache")

    pool = TcpConnectionPool(min_conn, default_concurrency, handler)

    # seed endpoints from provided servers or discovered cache
    if servers:
        for host, port in servers:
            pool.add_endpoint(host, port)
    else:
        for h, p in discovered:
            pool.add_endpoint(h, p)

    return pool


def client():
    """Return a pooled connection handle to a level1 server.

    Usage:
        with client() as conn:
            sock = conn.socket
            ...

    Raises RuntimeError if no endpoints have been configured for the pool.
    """
    if _pool is None:
        # Lazily initialize via the single public init function.
        init_pool()
    assert _pool is not None
    return _pool.acquire()


def init_pool(servers: Optional[List[Tuple[str, int]]] = None, *, min_conn: int = 1, max_conn: int = 10) -> None:
    """Initialize the module-level connection pool singleton.

    Parameters:
        servers: Optional list of (host, port) tuples to seed the pool with.
                 If omitted, the pool is created without endpoints and callers
                 must add endpoints via `_pool.add_endpoint(host, port)`.
        min_conn: minimum number of connections maintained by the pool.
        max_conn: maximum number of connections allowed by the pool.

    This must be called once during application startup before `client()` is
    used. Re-calling has no effect.
    """
    global _pool
    with _pool_lock:
        if _pool is not None:
            return
        # Build pool and assign; allow exceptions to propagate so caller
        # observes initialization failures (match C++ behaviour).
        _pool = _build_pool(min_conn=min_conn, max_conn=max_conn, servers=servers)
