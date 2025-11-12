package level1

import (
	stdnet "net"
	"sync/atomic"
	"testing"
	"time"

	qnet "gitee.com/quant1x/quant1x/quant1x/net"
)

type instrumentedHandler struct {
	*StandardProtocolHandler
	keepaliveHits int32
}

func newInstrumentedHandler(timeout, interval time.Duration) *instrumentedHandler {
	base := NewStandardProtocolHandler(timeout, interval).(*StandardProtocolHandler)
	return &instrumentedHandler{StandardProtocolHandler: base}
}

func (h *instrumentedHandler) Keepalive(conn *stdnet.TCPConn) (bool, error) {
	atomic.AddInt32(&h.keepaliveHits, 1)
	return h.StandardProtocolHandler.Keepalive(conn)
}

func (h *instrumentedHandler) keepaliveCount() int32 {
	return atomic.LoadInt32(&h.keepaliveHits)
}

func TestConnectionPoolHeartbeatReal(t *testing.T) {
	if testing.Short() {
		t.Skip("skipping real heartbeat test in short mode")
	}

	handler := newInstrumentedHandler(5*time.Second, 2*time.Second)

	servers := detectServers(handler.StandardProtocolHandler, 1500*time.Millisecond, 3, 5*time.Second)
	if len(servers) == 0 {
		servers = standardServerList()
	}
	if len(servers) == 0 {
		t.Fatalf("no servers available for heartbeat test")
	}

	pool, err := qnet.NewTcpConnectionPool(0, 1, handler)
	if err != nil {
		t.Fatalf("NewTcpConnectionPool failed: %v", err)
	}
	defer pool.Stop()

	added := 0
	for _, srv := range servers {
		if pool.AddEndpoint(srv.Host, int(srv.Port), 1) {
			added++
		}
		if added >= 1 {
			break
		}
	}
	if added == 0 {
		t.Fatalf("failed to add any endpoint to pool")
	}

	conn, release, err := pool.Acquire()
	if err != nil {
		t.Fatalf("Acquire failed: %v", err)
	}
	if conn == nil || release == nil {
		t.Fatalf("Acquire returned invalid connection or release func")
	}

	release()

	deadline := time.Now().Add(10 * time.Second)
	for handler.keepaliveCount() == 0 && time.Now().Before(deadline) {
		time.Sleep(100 * time.Millisecond)
	}

	if handler.keepaliveCount() == 0 {
		t.Fatalf("expected keepalive to trigger at least once")
	}
}
