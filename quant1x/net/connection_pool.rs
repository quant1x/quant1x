use std::collections::VecDeque;
use std::net::SocketAddr;
use std::sync::{Arc, Mutex, Weak};
use std::thread;
use std::time::{Duration, Instant};

use mio::net::TcpStream;

/// Trait that the user of the connection pool should implement to perform
/// protocol-specific work: handshake and keepalive checks.
pub trait NetworkHandler: Send + Sync + 'static {
    fn handshake(&self, _stream: &mut TcpStream) -> std::io::Result<()> {
        Ok(())
    }
    fn keepalive(&self, _stream: &mut TcpStream) -> std::io::Result<bool> {
        Ok(true)
    }
    fn timeout(&self) -> Duration { Duration::from_secs(5) }
    fn check_interval(&self) -> Duration { Duration::from_secs(5) }
}

/// A pooled connection wrapper. The pool owns connections and returns a
/// guard that will return the connection to the pool when dropped.
pub struct Connection {
    stream: TcpStream,
    addr: SocketAddr,
    last_used: Instant,
}

impl Connection {
    pub fn new(stream: TcpStream, addr: SocketAddr) -> Self {
        Self { stream, addr, last_used: Instant::now() }
    }

    pub fn stream(&mut self) -> &mut TcpStream { &mut self.stream }

    pub fn addr(&self) -> SocketAddr { self.addr }
}

/// The Mio-based TCP connection pool. This is a simplified port of the C++
/// TcpConnectionPool semantics: acquire returns a connection which is
/// automatically returned on Drop.
pub struct TcpConnectionPool<H: NetworkHandler> {
    handler: Arc<H>,
    max: usize,
    endpoint_manager: Arc<crate::net::endpoint::EndpointManager>,
    idle: Mutex<VecDeque<Connection>>,
}

impl<H: NetworkHandler> TcpConnectionPool<H> {
    pub fn new(min: usize, max: usize, handler: Arc<H>, endpoint_manager: Arc<crate::net::endpoint::EndpointManager>) -> Arc<Self> {
        let pool = Arc::new(Self { handler: Arc::clone(&handler), max, endpoint_manager: Arc::clone(&endpoint_manager), idle: Mutex::new(VecDeque::new()) });

        // Pre-warm: attempt to create `min` connections and place into idle queue.
        // Failures are ignored (network may be unavailable at startup).
        if min > 0 {
            let mut idle = pool.idle.lock().unwrap();
            for _ in 0..min {
                if let Some(ep) = pool.endpoint_manager.acquire_endpoint() {
                    if let Ok(mut stream) = TcpStream::connect(ep) {
                        let _ = stream.set_nodelay(true);
                        // run handshake but ignore errors during pre-warm
                        let _ = pool.handler.handshake(&mut stream);
                        idle.push_back(Connection::new(stream, ep));
                    } else {
                        // if connect failed, release endpoint slot
                        pool.endpoint_manager.release_endpoint(ep);
                    }
                } else {
                    break;
                }
            }
        }

        // Spawn a background heartbeat thread which periodically checks idle
        // connections by calling the handler's keepalive. Use a Weak reference
        // so the thread will exit once the pool is dropped.
        let weak_pool: Weak<TcpConnectionPool<H>> = Arc::downgrade(&pool);
        thread::spawn(move || {
            loop {
                // Attempt to upgrade; if pool is gone, exit the thread.
                let pool_arc = match weak_pool.upgrade() {
                    Some(p) => p,
                    None => break,
                };

                // Sleep according to handler's preferred interval. If the pool
                // is dropped while sleeping, the next upgrade will fail and exit.
                let interval = pool_arc.handler.check_interval();
                thread::sleep(interval);

                // Drain idle connections quickly while holding the lock, then
                // perform keepalive operations without holding the mutex.
                let mut drained: Vec<Connection> = Vec::new();
                {
                    let mut idle = pool_arc.idle.lock().unwrap();
                    while let Some(c) = idle.pop_front() {
                        drained.push(c);
                    }
                }

                if drained.is_empty() {
                    continue;
                }

                // For each drained connection, run keepalive. If keepalive
                // succeeds and returns true, the connection is still healthy
                // and will be returned to the idle queue. Otherwise it will
                // be dropped.
                let mut survivors: Vec<Connection> = Vec::with_capacity(drained.len());
                for mut conn in drained {
                    match pool_arc.handler.keepalive(conn.stream()) {
                        Ok(true) => { conn.last_used = Instant::now(); survivors.push(conn); }
                        _ => { 
                            // dead or error => release endpoint slot and drop
                            pool_arc.endpoint_manager.release_endpoint(conn.addr);
                        }
                    }
                }

                // Push survivors back into the idle queue (respecting max)
                if !survivors.is_empty() {
                    let mut idle = pool_arc.idle.lock().unwrap();
                    for conn in survivors {
                        if idle.len() < pool_arc.max {
                            idle.push_back(conn);
                        } else {
                            // pool is full — drop extra
                            break;
                        }
                    }
                }
            }
        });

        pool
    }

    /// Acquire a connection using the endpoint manager (round-robin / available).
    pub fn acquire(self: &Arc<Self>) -> std::io::Result<PooledConnection<H>> {
        // Try to pop an idle connection first
        if let Some(mut conn) = self.idle.lock().unwrap().pop_front() {
            conn.last_used = Instant::now();
            return Ok(PooledConnection { pool: Arc::clone(self), conn: Some(conn) });
        }

        // If no idle connection, request an endpoint from the manager
        let endpoint = match self.endpoint_manager.acquire_endpoint() {
            Some(ep) => ep,
            None => return Err(std::io::Error::new(std::io::ErrorKind::Other, "No available endpoints")),
        };

        // Create a new non-blocking TcpStream and connect
        let mut stream = TcpStream::connect(endpoint)?;
        stream.set_nodelay(true)?;

        // perform handshake via handler
        self.handler.handshake(&mut stream)?;

        let conn = Connection::new(stream, endpoint);
        Ok(PooledConnection { pool: Arc::clone(self), conn: Some(conn) })
    }

    fn release(&self, mut conn: Connection) {
        conn.last_used = Instant::now();
        // When a connection is returned to the pool (idle), we keep its
        // endpoint slot occupied so the total active count reflects pooled
        // + in-use connections. Only when we actually drop a connection
        // (because the pool is full) do we release the endpoint slot.
        let mut idle = self.idle.lock().unwrap();
        if idle.len() < self.max {
            idle.push_back(conn);
        } else {
            // Pool full: drop connection and release endpoint slot
            self.endpoint_manager.release_endpoint(conn.addr);
        }
    }

    /// Add an endpoint to the manager
    pub fn add_endpoint(&self, addr: SocketAddr, max_connections: usize) -> bool {
        self.endpoint_manager.add_endpoint(addr, max_connections)
    }

    pub fn get_endpoint_stats(&self, addr: SocketAddr) -> Option<(usize, usize)> {
        self.endpoint_manager.get_endpoint_stats(addr)
    }
}

/// RAII guard that returns the connection to the pool when dropped.
pub struct PooledConnection<H: NetworkHandler> {
    pool: Arc<TcpConnectionPool<H>>,
    conn: Option<Connection>,
}

impl<H: NetworkHandler> PooledConnection<H> {
    pub fn stream(&mut self) -> &mut TcpStream { &mut self.conn.as_mut().unwrap().stream }
    pub fn addr(&self) -> SocketAddr { self.conn.as_ref().unwrap().addr }
}

impl<H: NetworkHandler> Drop for PooledConnection<H> {
    fn drop(&mut self) {
        if let Some(conn) = self.conn.take() {
            self.pool.release(conn);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::net::{TcpListener, TcpStream as StdTcpStream};
    use std::sync::{Arc, Mutex};
    use std::thread;
    use std::time::Duration;

    struct TestHandler;
    impl NetworkHandler for TestHandler {
        fn handshake(&self, _stream: &mut TcpStream) -> std::io::Result<()> { Ok(()) }
        fn keepalive(&self, _stream: &mut TcpStream) -> std::io::Result<bool> { Ok(true) }
        fn timeout(&self) -> Duration { Duration::from_secs(1) }
        fn check_interval(&self) -> Duration { Duration::from_millis(50) }
    }

    #[test]
    fn test_endpoint_manager_basic() {
        let mgr = crate::net::endpoint::EndpointManager::new();
        // add_endpoint expects a concrete port; cannot add 0 here, so we test add/remove semantics
        // by creating a listener to get an assigned port.
        let listener = TcpListener::bind(("127.0.0.1", 0)).expect("bind");
        let addr = listener.local_addr().unwrap();
        assert!(mgr.add_endpoint(addr, 2));
        assert!(mgr.get_all_endpoints().contains(&addr));
        // acquire twice should succeed
        let a1 = mgr.acquire_endpoint();
        let a2 = mgr.acquire_endpoint();
        assert!(a1.is_some());
        assert!(a2.is_some());
        // third should fail as max_connections == 2
        let a3 = mgr.acquire_endpoint();
        assert!(a3.is_none());
        // release one and acquire again
        mgr.release_endpoint(a1.unwrap());
        let a4 = mgr.acquire_endpoint();
        assert!(a4.is_some());
        mgr.remove_endpoint(addr);
        assert!(!mgr.get_all_endpoints().contains(&addr));
        drop(listener);
    }

    #[test]
    fn test_connection_pool_with_local_server() {
        // Start a local TCP listener to accept connections
        let listener = TcpListener::bind(("127.0.0.1", 0)).expect("bind");
        let addr = listener.local_addr().unwrap();

        // Keep accepted streams alive so server side doesn't close immediately
        let accepted: Arc<Mutex<Vec<StdTcpStream>>> = Arc::new(Mutex::new(Vec::new()));
        let accepted_clone = Arc::clone(&accepted);
        thread::spawn(move || {
            for _ in 0..10 {
                if let Ok((stream, _)) = listener.accept() {
                    // hold the stream
                    accepted_clone.lock().unwrap().push(stream);
                }
            }
        });

        let mgr = crate::net::endpoint::EndpointManager::new();
        assert!(mgr.add_endpoint(addr, 2));

        let handler = Arc::new(TestHandler {});
        let pool = TcpConnectionPool::new(1, 2, handler, Arc::new(mgr));

        // Acquire one connection
        let conn = pool.acquire().expect("acquire 1");
        assert_eq!(conn.addr(), addr);
        // Drop to return to pool
        drop(conn);

        // Acquire up to max
        let c1 = pool.acquire().expect("acquire c1");
        let c2 = pool.acquire().expect("acquire c2");

        // third should fail (max_connections == 2)
        let res = pool.acquire();
        assert!(res.is_err());

        drop(c1);
        drop(c2);

        // allow some time for heartbeat to run (check_interval is small)
        thread::sleep(Duration::from_millis(200));

        // Clean up accepted streams so accept thread can finish
        accepted.lock().unwrap().clear();
    }
}
