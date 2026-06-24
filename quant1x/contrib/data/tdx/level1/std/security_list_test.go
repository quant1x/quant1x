package std

import (
	"net"
	"testing"
	"time"

	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/tdxproto"
	qio "github.com/quant1x/quant1x/quant1x/io"
	"github.com/quant1x/quant1x/quant1x/data"
)

// testServerAddr is a known TDX standard server for testing.
const testServerAddr = "119.97.185.59:7709"

// testHandshake performs the TDX level1 handshake (StdLogin + UpgradeTip)
// on a raw TCP connection by wrapping it in a qio.Connection temporarily.
func testHandshake(conn *net.TCPConn) error {
	tmpConn := qio.NewConnection(conn, nil)
	req1 := NewStdLoginContext()
	if err := tdxproto.TransactMessageSync(tmpConn, req1); err != nil {
		return err
	}
	if req1.Info == "" {
		return errHandshakeEmptyInfo
	}
	req2 := NewUpgradeTipContext()
	if err := tdxproto.TransactMessageSync(tmpConn, req2); err != nil {
		return err
	}
	if req2.Info == "" {
		return errHandshakeEmptyInfo
	}
	return nil
}

var errHandshakeEmptyInfo = stdError("handshake returned empty info")

type stdError string

func (e stdError) Error() string { return string(e) }

// dialTestServer dials a TDX server, performs handshake, and returns a connection.
func dialTestServer() (*qio.Connection, func(), error) {
	tcpAddr, err := net.ResolveTCPAddr("tcp", testServerAddr)
	if err != nil {
		return nil, nil, err
	}
	conn, err := net.DialTimeout("tcp", testServerAddr, 5*time.Second)
	if err != nil {
		return nil, nil, err
	}
	tcpConn, ok := conn.(*net.TCPConn)
	if !ok {
		conn.Close()
		return nil, nil, stdError("unexpected connection type")
	}
	if err := testHandshake(tcpConn); err != nil {
		tcpConn.Close()
		return nil, nil, err
	}
	c := qio.NewConnection(tcpConn, tcpAddr)
	released := false
	release := func() {
		if released {
			return
		}
		released = true
		c.Close()
	}
	return c, release, nil
}

func TestSecurityListReal(t *testing.T) {
	if testing.Short() {
		t.Skip("skipping real security list test in short mode")
	}

	// Set up the connection provider for this test
	SetConnectionProvider(dialTestServer)

	conn, release, err := GetStdConnection()
	if err != nil {
		t.Fatalf("GetStdConnection() returned error: %v", err)
	}
	defer release()

	markets := []struct {
		exchange data.Exchange
		name     string
	}{
		{data.ExchangeSSE, "sh"},
		{data.ExchangeSZSE, "sz"},
		{data.BSE, "bj"},
	}

	for _, m := range markets {
		ctx := NewSecurityListContext(m.exchange, 0, SecurityListPerRequestMax)
		if err := tdxproto.TransactMessageSync(conn, ctx); err != nil {
			t.Fatalf("TransactMessageSync(%s) failed: %v", m.name, err)
		}
		if ctx.RespCount == 0 {
			t.Fatalf("expected non-zero count for market %s", m.name)
		}
		if len(ctx.List) != int(ctx.RespCount) {
			t.Fatalf("list length mismatch for market %s: count=%d len=%d", m.name, ctx.RespCount, len(ctx.List))
		}
		first := ctx.List[0]
		if first.Code == "" {
			t.Fatalf("empty code for market %s", m.name)
		}
		if first.Name == "" {
			t.Fatalf("empty name for market %s", m.name)
		}
	}
}
