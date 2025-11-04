import threading
import socket
import time
import logging


from quant1x.net.connection_pool import TcpConnectionPool
from quant1x.net.operation_handler import OperationHandler


class MockNetworkHandler(OperationHandler):
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
    addr, port = srv.getsockname()
    port_holder.append(port)
    listen_ready.set()

    try:
        while True:
            conn, _ = srv.accept()
            # keep connection open until closed by client
            threading.Thread(target=lambda c: c.recv(1), args=(conn,), daemon=True).start()
    except Exception:
        try:
            srv.close()
        except Exception:
            pass


def test_basic_acquire_release():
    logging.basicConfig(level=logging.DEBUG)
    ready = threading.Event()
    port_holder = []
    t = threading.Thread(target=_start_dummy_server, args=(ready, port_holder), daemon=True)
    t.start()
    ready.wait(timeout=2.0)
    port = port_holder[0]

    handler = MockNetworkHandler()
    pool = TcpConnectionPool(1, 2, handler)
    pool.add_endpoint('127.0.0.1', port, 2)

    # Acquire a connection via context manager
    with pool.acquire() as conn:
        assert conn is not None
        assert conn.is_open()
        assert conn.endpoint[1] == port

    # after context exit, connection should be returned to pool
    time.sleep(0.1)
    # cleanup
    pool.stop()
