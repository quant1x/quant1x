import socket
import threading
import struct
import time

from quant1x.level1.client import StandardProtocolHandler


def _recv_exact(conn: socket.socket, n: int) -> bytes:
    buf = bytearray()
    while len(buf) < n:
        chunk = conn.recv(n - len(buf))
        if not chunk:
            raise ConnectionError("socket closed")
        buf.extend(chunk)
    return bytes(buf)


def _dummy_protocol_server(listen_ready: threading.Event, port_holder: list):
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind(('127.0.0.1', 0))
    srv.listen(1)
    host, port = srv.getsockname()
    port_holder.append(port)
    listen_ready.set()

    conn, _ = srv.accept()
    try:
        # Handle Hello1 request
        hdr = _recv_exact(conn, 12)
        # parse pkg_len1 from header (offset 4+1+1? layout: B I B H H H)
        zip_flag, seq_id, packet_type, pkg_len1, pkg_len2, method = struct.unpack('<B I B H H H', hdr)
        payload_len = max(0, pkg_len1 - 2)
        if payload_len:
            _ = _recv_exact(conn, payload_len)

        # craft Hello1 response body: offset 68 then GBK-encoded info
        info = 'OK'.encode('gbk')
        body = b'\x00' * 68 + info
        zip_size = len(body)
        unzip_size = zip_size
        # response header: I B I B H H H
        resp_hdr = struct.pack('<I B I B H H H', 0, 0, 0, 0, 0x000d, zip_size, unzip_size)
        conn.sendall(resp_hdr + body)

        # Handle Hello2 request
        hdr2 = _recv_exact(conn, 12)
        zip_flag2, seq_id2, packet_type2, pkg_len12, pkg_len22, method2 = struct.unpack('<B I B H H H', hdr2)
        payload_len2 = max(0, pkg_len12 - 2)
        if payload_len2:
            _ = _recv_exact(conn, payload_len2)

        # craft Hello2 response body: offset 58 then GBK-encoded info
        info2 = 'OK2'.encode('gbk')
        body2 = b'\x00' * 58 + info2
        zip_size2 = len(body2)
        unzip_size2 = zip_size2
        resp_hdr2 = struct.pack('<I B I B H H H', 0, 0, 0, 0, 0x0fdb, zip_size2, unzip_size2)
        conn.sendall(resp_hdr2 + body2)

        # keep connection briefly
        time.sleep(0.1)
    finally:
        try:
            conn.close()
        except Exception:
            pass
        srv.close()


def test_handshake():
    ready = threading.Event()
    port_holder = []
    t = threading.Thread(target=_dummy_protocol_server, args=(ready, port_holder), daemon=True)
    t.start()
    assert ready.wait(timeout=2.0)
    port = port_holder[0]

    # connect and use StandardProtocolHandler.handshake
    sock = socket.create_connection(('127.0.0.1', port), timeout=2.0)
    try:
        handler = StandardProtocolHandler()
        ok = handler.handshake(sock)
        assert ok is True
    finally:
        try:
            sock.close()
        except Exception:
            pass

if __name__ == "__main__":
    test_handshake()
    print("test_handshake passed")
