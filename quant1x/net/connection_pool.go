package net

import (
	"context"
	"errors"
	"fmt"
	stdnet "net"
	"sync"
	"time"

	"gitee.com/quant1x/quant1x/quant1x/log"
)

// Connection is an RAII-style wrapper around an established TCP connection.
// Its lifecycle is managed by TcpConnectionPool; callers should not copy it.
type Connection struct {
	conn     *stdnet.TCPConn
	endpoint *stdnet.TCPAddr
}

func NewConnection(c *stdnet.TCPConn, ep *stdnet.TCPAddr) *Connection {
	return &Connection{conn: c, endpoint: ep}
}

func (c *Connection) Conn() *stdnet.TCPConn     { return c.conn }
func (c *Connection) Endpoint() *stdnet.TCPAddr { return c.endpoint }

func (c *Connection) Close() {
	if c == nil || c.conn == nil {
		return
	}
	_ = c.conn.Close()
}

func (c *Connection) IsOpen() bool {
	return c != nil && c.conn != nil
}

// TcpConnectionPool manages a pool of connections across multiple endpoints.
type TcpConnectionPool struct {
	// configuration
	minConnections int
	maxConnections int
	endpointWeight int

	// dependencies
	networkHandler  NetworkOperationHandler
	endpointManager *EndpointManager

	// lifecycle
	ctx     context.Context
	cancel  context.CancelFunc
	running bool

	// connection storage
	connectionsMutex sync.Mutex
	idleConnections  []*Connection
	idleCount        int
	activeCount      int

	// heartbeat
	heartbeatTicker *time.Ticker
}

// NewTcpConnectionPool creates a new pool. networkHandler must be non-nil.
func NewTcpConnectionPool(minConn, maxConn int, handler NetworkOperationHandler) (*TcpConnectionPool, error) {
	if minConn > maxConn {
		return nil, fmt.Errorf("min_connections cannot be greater than max_connections")
	}
	if maxConn == 0 {
		return nil, fmt.Errorf("max_connections cannot be zero")
	}
	if handler == nil {
		return nil, fmt.Errorf("network_handler cannot be nil")
	}

	ctx, cancel := context.WithCancel(context.Background())
	p := &TcpConnectionPool{
		minConnections:  minConn,
		maxConnections:  maxConn,
		endpointWeight:  1,
		networkHandler:  handler,
		endpointManager: NewEndpointManager(),
		ctx:             ctx,
		cancel:          cancel,
	}

	log.Debugf("[connection pool] min_connections=%d, max_connections=%d, endpoint_weight=%d", minConn, maxConn, p.endpointWeight)

	// start background heartbeat
	p.start()
	return p, nil
}

// AddEndpoint adds an endpoint by host/port
func (p *TcpConnectionPool) AddEndpoint(host string, port int, weight int) bool {
	if weight == 0 {
		weight = p.endpointWeight
	}
	return p.endpointManager.AddEndpoint(host, uint16(port), weight)
}

// AddEndpointAddr adds an endpoint by *net.TCPAddr (address object)
func (p *TcpConnectionPool) AddEndpointAddr(addr *stdnet.TCPAddr, weight int) bool {
	if weight == 0 {
		weight = p.endpointWeight
	}
	return p.endpointManager.AddEndpointAddr(addr, weight)
}

// Acquire attempts to reuse an idle connection or create a new one. It returns the
// connection and a release function which must be called when done with the connection.
func (p *TcpConnectionPool) Acquire() (*Connection, func(), error) {
	// 1. try reuse
	var c *Connection
	p.connectionsMutex.Lock()
	if len(p.idleConnections) > 0 {
		c = p.idleConnections[len(p.idleConnections)-1]
		p.idleConnections = p.idleConnections[:len(p.idleConnections)-1]
		p.idleCount--
		log.Debugf("Reused connection from pool (ptr: %p)", c)
	}
	p.connectionsMutex.Unlock()

	// 2. create new if needed
	if c == nil {
		log.Debugf("Creating new connection...")
		ep, ok := p.endpointManager.AcquireEndpoint()
		if !ok || ep == nil {
			log.Errorf("No available endpoints")
			return nil, nil, errors.New("no available endpoints")
		}

		// Dial with timeout
		dialer := &stdnet.Dialer{Timeout: p.networkHandler.Timeout()}
		addr := ep.String()
		rawConn, err := dialer.DialContext(p.ctx, "tcp", addr)
		if err != nil {
			p.endpointManager.ReleaseEndpoint(ep)
			log.Errorf("Error dialing %s: %v", addr, err)
			return nil, nil, err
		}
		tcpConn, ok2 := rawConn.(*stdnet.TCPConn)
		if !ok2 {
			rawConn.Close()
			p.endpointManager.ReleaseEndpoint(ep)
			return nil, nil, fmt.Errorf("unexpected connection type")
		}

		// handshake
		okHandshake, err := p.networkHandler.Handshake(tcpConn)
		if err != nil || !okHandshake {
			tcpConn.Close()
			p.endpointManager.ReleaseEndpoint(ep)
			log.Errorf("Handshake failed with %s: %v", addr, err)
			if err == nil {
				err = errors.New("handshake failed")
			}
			return nil, nil, err
		}

		c = NewConnection(tcpConn, ep)
		log.Debugf("Created new connection (ptr: %p)", c)
	}

	p.connectionsMutex.Lock()
	p.activeCount++
	p.connectionsMutex.Unlock()

	// release function
	released := false
	releaseFunc := func() {
		if released {
			return
		}
		released = true
		p.Release(c)
	}
	return c, releaseFunc, nil
}

// Release returns a connection to the idle pool
func (p *TcpConnectionPool) Release(c *Connection) {
	if c == nil {
		return
	}
	connPtr := fmt.Sprintf("%p", c)
	log.Debugf("Returning connection %s to pool", connPtr)

	p.connectionsMutex.Lock()
	defer p.connectionsMutex.Unlock()
	p.idleConnections = append(p.idleConnections, c)
	p.idleCount++
	if p.activeCount > 0 {
		p.activeCount--
	}
}

// closeConnection closes a single idle connection and releases its endpoint
func (p *TcpConnectionPool) closeConnection(c *Connection) {
	if c == nil {
		return
	}
	log.Debugf("Closing connection (ptr: %p)", c)
	// Release the endpoint back to manager, then close the socket.
	// This mirrors the C++ closeConnection behavior which returns the
	// endpoint allocation when a connection is being permanently closed.
	p.endpointManager.ReleaseEndpoint(c.Endpoint())
	c.Close()

	p.connectionsMutex.Lock()
	if p.idleCount > 0 {
		p.idleCount--
	}
	p.connectionsMutex.Unlock()
}

// start launches heartbeat timer and background routines
func (p *TcpConnectionPool) start() {
	if p.running {
		return
	}
	p.running = true
	interval := p.networkHandler.CheckInterval()
	if interval <= 0 {
		interval = 5 * time.Second
	}
	p.heartbeatTicker = time.NewTicker(interval)
	go func() {
		for {
			select {
			case <-p.ctx.Done():
				log.Infof("heartbeat exiting")
				return
			case <-p.heartbeatTicker.C:
				if !p.running {
					log.Infof("heartbeat stopping")
					return
				}
				p.checkConnections()
				p.tryCreateConnections()
			}
		}
	}()
}

// Stop stops the pool and closes connections
func (p *TcpConnectionPool) Stop() {
	if !p.running {
		return
	}
	p.running = false
	if p.heartbeatTicker != nil {
		p.heartbeatTicker.Stop()
	}
	p.cancel()
	p.closeAllConnections()
}

// checkConnections validates idle connections using handler.Keepalive
func (p *TcpConnectionPool) checkConnections() {
	p.connectionsMutex.Lock()
	defer p.connectionsMutex.Unlock()
	// Follow C++ semantics closely:
	// - If keepalive() returns true, keep the connection in idle pool.
	// - If keepalive() returns false, the connection is removed and the
	//   socket is closed (the endpoint allocation is NOT released here),
	//   mirroring the C++ remove_if + unique_ptr destruction behavior.
	// - If keepalive() throws/returns an error, we call closeConnection
	//   which closes the socket AND releases the endpoint allocation.

	var survivors []*Connection
	for _, conn := range p.idleConnections {
		if conn == nil {
			continue
		}
		ok, err := p.networkHandler.Keepalive(conn.Conn())
		if err != nil {
			// Exception path: permanently close and release endpoint
			log.Errorf("keepalive error: %v", err)
			p.connectionsMutex.Unlock() // unlock while performing closeConnection which will lock
			p.closeConnection(conn)
			p.connectionsMutex.Lock()
			continue
		}
		if ok {
			survivors = append(survivors, conn)
		} else {
			// keepalive=false: close socket but DO NOT release endpoint here
			log.Debugf("connection not healthy, closing socket: %p", conn)
			conn.Close()
			// do NOT call ReleaseEndpoint here (mirror C++ behavior)
		}
	}
	p.idleConnections = survivors
	p.idleCount = len(survivors)
}

// tryCreateConnections attempts to ensure minConnections are available
func (p *TcpConnectionPool) tryCreateConnections() {
	retries := 0
	for p.activeCount+p.idleCount < p.minConnections && retries < 10 {
		available := p.endpointManager.GetAvailableResources()
		if available == 0 {
			log.Infof("endpoint resources insufficient, retry %d/10", retries)
			time.Sleep(100 * time.Millisecond)
			retries++
			continue
		}
		conn, release, err := p.Acquire()
		if err != nil {
			log.Errorf("Error acquiring new connection: %v", err)
			break
		}
		// Immediately release the connection to mirror C++ unique_ptr
		// lifetime behavior in try_create_connections (destructor returns
		// connection to pool).
		if release != nil {
			release()
		}
		// increment retry counter on attempt
		retries++
		log.Debugf("supplemented 1 connection, endpoint=%v", conn.Endpoint())
	}
}

// closeAllConnections closes all idle connections
func (p *TcpConnectionPool) closeAllConnections() {
	p.connectionsMutex.Lock()
	defer p.connectionsMutex.Unlock()
	for _, c := range p.idleConnections {
		if c != nil && c.IsOpen() {
			c.Close()
		}
	}
	p.idleConnections = nil
	p.idleCount = 0
}

// GetEndpointStats returns stats for a given host/port
func (p *TcpConnectionPool) GetEndpointStats(host string, port int) (int, int, error) {
	ep, err := stdnet.ResolveIPAddr("ip", host)
	if err != nil {
		return 0, 0, err
	}
	tcpAddr := &stdnet.TCPAddr{IP: ep.IP, Port: port}
	max, active, err := p.endpointManager.GetEndpointStats(tcpAddr)
	if err != nil {
		return 0, 0, err
	}
	return int(max), int(active), nil
}
