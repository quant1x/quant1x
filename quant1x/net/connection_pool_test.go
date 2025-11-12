package net

import (
	"net"
	"sync/atomic"
	"testing"
	"time"
)

type mockHandler struct {
	timeout       time.Duration
	interval      time.Duration
	keepaliveHits int32
	handshakeHits int32
}

func (m *mockHandler) Timeout() time.Duration {
	return m.timeout
}

func (m *mockHandler) Handshake(conn *net.TCPConn) (bool, error) {
	atomic.AddInt32(&m.handshakeHits, 1)
	return true, nil
}

func (m *mockHandler) Keepalive(conn *net.TCPConn) (bool, error) {
	atomic.AddInt32(&m.keepaliveHits, 1)
	return true, nil
}

func (m *mockHandler) CheckInterval() time.Duration {
	return m.interval
}

func TestTcpConnectionPoolHeartbeat(t *testing.T) {
	listener, err := net.ListenTCP("tcp", &net.TCPAddr{IP: net.ParseIP("127.0.0.1"), Port: 0})
	if err != nil {
		t.Fatalf("ListenTCP failed: %v", err)
	}
	defer listener.Close()

	handler := &mockHandler{timeout: 200 * time.Millisecond, interval: 50 * time.Millisecond}

	pool, err := NewTcpConnectionPool(0, 1, handler)
	if err != nil {
		t.Fatalf("NewTcpConnectionPool failed: %v", err)
	}
	defer pool.Stop()

	addr := listener.Addr().(*net.TCPAddr)
	if ok := pool.AddEndpoint(addr.IP.String(), addr.Port, 1); !ok {
		t.Fatalf("AddEndpoint returned false")
	}

	serverConnCh := make(chan *net.TCPConn, 1)
	stopServer := make(chan struct{})
	go func() {
		conn, err := listener.AcceptTCP()
		if err != nil {
			return
		}
		select {
		case serverConnCh <- conn:
		default:
			conn.Close()
			return
		}
		<-stopServer
		conn.Close()
	}()

	clientConn, release, err := pool.Acquire()
	if err != nil {
		t.Fatalf("Acquire failed: %v", err)
	}
	if clientConn == nil {
		t.Fatalf("Acquire returned nil connection")
	}
	if release == nil {
		t.Fatalf("Acquire returned nil release func")
	}

	serverConn := <-serverConnCh

	release()

	deadline := time.Now().Add(500 * time.Millisecond)
	for atomic.LoadInt32(&handler.keepaliveHits) == 0 && time.Now().Before(deadline) {
		time.Sleep(10 * time.Millisecond)
	}

	if atomic.LoadInt32(&handler.keepaliveHits) == 0 {
		t.Fatalf("expected keepalive to be invoked at least once")
	}

	close(stopServer)
	serverConn.Close()
	clientConn.Close()
}
