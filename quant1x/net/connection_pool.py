"""Python implementation of TcpConnectionPool mirroring the C++ design.

This module implements:
- Connection: RAII-like wrapper around a socket (life-cycle owned by the pool)
- TcpConnectionPool: thread-safe pool matching the C++ API and semantics

Notes:
- Endpoints are (host, port) tuples and managed by EndpointManager from
  quant1x.net.endpoint.
- Network handler is a user-provided object exposing the methods:
  - timeout() -> float (seconds)
  - handshake(socket.socket) -> bool
  - keepalive(socket.socket) -> bool
  - check_interval() -> float (seconds)

The pool returns a ConnectionHandle which is a context-manager and will
automatically return the connection to the pool when exiting the context or
when the handle is garbage collected.
"""
from __future__ import annotations

import socket
import threading
import time
import logging
from collections import deque
from typing import Optional, Callable, Deque, Tuple

from .endpoint import EndpointManager, Endpoint

log = logging.getLogger(__name__)


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

    def __enter__(self) -> Connection:
        return self._conn

    def __exit__(self, exc_type, exc, tb):
        self.release()

    def release(self) -> None:
        if not self._released:
            try:
                self._releaser(self._conn)
            finally:
                self._released = True

    def __del__(self):
        # best-effort auto-release to mimic C++ unique_ptr deleter
        try:
            self.release()
        except Exception:
            log.exception("Error during auto-release of connection")


class TcpConnectionPool:
    def __init__(self, min_connections: int, max_connections: int, network_handler):
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

        # Start background thread executing heartbeat and creation attempts
        self._worker_threads = []
        # create 2 worker threads to mimic C++ io threads (they only run timers here)
        for i in range(2):
            t = threading.Thread(target=self._worker_loop, name=f"connpool-io-{i}", daemon=True)
            self._worker_threads.append(t)
            t.start()

        # start pool
        self.start()

    # ---------- endpoint helpers (C++-compatible names) ----------
    def add_endpoint(self, ip: str, port: int, weight: int = 0) -> bool:
        return self.endpoint_manager.add_endpoint(ip, port, weight or self.endpoint_weight)

    def addEndpoint(self, ip: str, port: int, weight: int = 0) -> bool:
        return self.add_endpoint(ip, port, weight)

    def add_endpoint_obj(self, endpoint: Endpoint, weight: int = 0) -> bool:
        return self.endpoint_manager.add_endpoint_obj(endpoint, weight or self.endpoint_weight)

    # ---------- core pool API ----------
    def acquire(self) -> ConnectionHandle:
        # 1. try reuse
        raw_conn: Optional[Connection] = None
        with self._connections_mutex:
            if self._idle_connections:
                raw_conn = self._idle_connections.popleft()
                self.idle_connection_count -= 1
                log.debug("Reused connection from pool: %s", raw_conn)

        # 2. if none, create new
        endpoint = None
        if raw_conn is None:
            log.debug("Creating new connection...")
            ep = self.endpoint_manager.acquire_endpoint()
            if ep is None:
                log.error("No available endpoints")
                raise RuntimeError("No available endpoints")
            endpoint = ep
            sock = None
            try:
                # create and connect with timeout from handler
                timeout = float(self.network_handler.timeout())
                sock = socket.create_connection(endpoint, timeout=timeout)
                # also set socket timeout for subsequent recv/send operations so
                # blocking reads used by protocol helpers (process_request_std)
                # will raise socket.timeout instead of blocking forever.
                try:
                    sock.settimeout(timeout)
                except Exception:
                    pass
                # set TCP_NODELAY
                try:
                    sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
                except Exception:
                    pass

                # handshake
                ok = self.network_handler.handshake(sock)
                if not ok:
                    sock.close()
                    self.endpoint_manager.release_endpoint(endpoint)
                    log.error("Handshake failed with %s:%s", endpoint[0], endpoint[1])
                    raise RuntimeError("Handshake failed")

                raw_conn = Connection(sock, endpoint)
                log.debug("Created new connection %s", raw_conn)
            except Exception:
                if sock is not None:
                    try:
                        sock.close()
                    except Exception:
                        pass
                # release endpoint reservation
                if endpoint is not None:
                    self.endpoint_manager.release_endpoint(endpoint)
                raise

        # create deleter/releaser
        def deleter(conn: Connection) -> None:
            try:
                self.release(conn)
            except Exception:
                log.exception("Error during auto-release")

        self.active_connection_count += 1
        return ConnectionHandle(raw_conn, deleter)

    def release(self, conn: Connection) -> None:
        if conn is None:
            return
        conn_id = id(conn)
        log.debug("Returning connection %s", conn_id)

        # Do NOT release endpoint here (same semantics as C++)
        with self._connections_mutex:
            self._idle_connections.append(conn)
            self.idle_connection_count += 1
            self.active_connection_count -= 1
            log.debug("Connection %s returned to pool", conn_id)

    def close_connection(self, conn: Optional[Connection]) -> None:
        if conn is None:
            return
        conn_id = id(conn)
        log.debug("Closing connection %s", conn_id)
        # 1. release endpoint
        try:
            self.endpoint_manager.release_endpoint(conn.endpoint)
        except Exception:
            log.exception("Error releasing endpoint")
        # 2. close socket
        try:
            conn.close()
        except Exception:
            log.exception("Error closing connection socket")
        # 3. update counters
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

    # ---------- internal helpers ----------
    def _start_heartbeat_timer(self) -> None:
        interval = float(self.network_handler.check_interval())

        def tick():
            if not self.running:
                return
            try:
                self._check_connections()
                self._try_create_connections()
            except Exception:
                log.exception("Exception in heartbeat tick")
            # reschedule
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
                    # treat exception as dead
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
                log.warning("endpoint resources exhausted, retry %s/%s", retry_count, max_retries)
                time.sleep(0.1)
                retry_count += 1
                continue
            else:
                try:
                    handle = self.acquire()
                    # release immediately to put into idle pool
                    handle.release()
                    retry_count += 1
                    log.debug("Supplemented 1 connection")
                except Exception:
                    log.exception("Error acquiring new connection")
                    break

    def _close_all_connections(self) -> None:
        with self._connections_mutex:
            for conn in list(self._idle_connections):
                try:
                    if conn and conn.is_open():
                        conn.close()
                except Exception:
                    log.exception("Error closing idle connection")
            self._idle_connections.clear()
            self.idle_connection_count = 0

    def _worker_loop(self) -> None:
        # simple loop to keep threads alive; the real work is done by timers
        while self.running:
            time.sleep(0.5)
