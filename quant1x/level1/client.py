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
from quant1x.net.operation_handler import OperationHandler
from typing import Any

log = logging.getLogger(__name__)


class ProtocolHandler(OperationHandler):
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


 


def _init_pool() -> None:
    global _pool
    with _pool_lock:
        if _pool is not None:
            return

        # handler provides handshake/keepalive/timeouts
        handler = ProtocolHandler()

        # default pool sizes: min=1, max=10
        min_conn = 1
        max_conn = 10

        pool = TcpConnectionPool(min_conn, max_conn, handler)

        # No environment variable based configuration here. Callers must
        # configure endpoints explicitly (see `init_pool`).

        _pool = pool


def client():
    """Return a pooled connection handle to a level1 server.

    Usage:
        with client() as conn:
            sock = conn.socket
            ...

    Raises RuntimeError if no endpoints have been configured for the pool.
    """
    if _pool is None:
        _init_pool()
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

    # Determine server endpoints first (read cache, else detect)
    # Note: per C++ behaviour, detect() probes only `StandardServerList`.
    # `ExtensionServerList` is intentionally not probed by default because
    # extension servers may use different protocols/ports. If callers need
    # extension-server probing, they must add endpoints explicitly via the
    # `servers` parameter or implement a separate probing routine.
    discovered: List[Tuple[str, int]] = []
    try:
        from quant1x.level1 import config as l1config

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
        else:
            # request detection for the standard candidates only (C++ behaviour)
            try:
                total_candidates = len(l1config.StandardServerList)
            except Exception:
                total_candidates = 0
            detected = []
            try:
                if total_candidates > 0:
                    detected = l1config.detect(conn_limit=total_candidates)
            except Exception:
                log.exception("level1.init_pool: detect() raised an exception")

            if detected:
                for s in detected:
                    h = s.get("Host")
                    p_obj: Any = s.get("Port")
                    try:
                        p = int(str(p_obj)) if p_obj is not None else None
                    except Exception:
                        p = None
                    if isinstance(h, str) and p is not None:
                        discovered.append((h, p))
                # persist detected list back to cache
                try:
                    l1config.write_cache(detected)
                except Exception:
                    pass
    except Exception:
        log.exception("level1.init_pool: server detect/cache step failed")

    # Adjust concurrency similar to C++: concurrency = min(max_conn, len(discovered))
    concurrency = max_conn
    if discovered:
        try:
            concurrency = min(max_conn, max(1, len(discovered)))
        except Exception:
            concurrency = max_conn

    # handler provides handshake/keepalive/timeouts
    handler = ProtocolHandler()
    pool = TcpConnectionPool(min_conn, concurrency, handler)

    # seed endpoints if discovered or provided by caller
    if servers:
        for host, port in servers:
            pool.add_endpoint(host, port)
    else:
        for h, p in discovered:
            pool.add_endpoint(h, p)

    _pool = pool
