use std::collections::VecDeque;
use std::net::{Shutdown, SocketAddr};
use std::sync::{Arc, Mutex, Weak};
use std::thread;
use std::time::{Duration, Instant};

use mio::net::TcpStream;
use std::net::TcpStream as StdTcpStream;

/// Trait that the user of the connection pool should implement to perform
/// protocol-specific work: handshake and keepalive checks.
pub trait NetworkHandler: Send + Sync + 'static {
    fn handshake(&self, _stream: &mut TcpStream) -> std::io::Result<()> {
        Ok(())
    }
    fn keepalive(&self, _stream: &mut TcpStream) -> std::io::Result<bool> {
        Ok(true)
    }
    fn timeout(&self) -> Duration {
        Duration::from_secs(10)
    }
    fn check_interval(&self) -> Duration {
        Duration::from_secs(5)
    }
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
        Self {
            stream,
            addr,
            last_used: Instant::now(),
        }
    }

    pub fn stream(&mut self) -> &mut TcpStream {
        &mut self.stream
    }

    pub fn addr(&self) -> SocketAddr {
        self.addr
    }

    pub fn close(&mut self) {
        if let Err(e) = self.stream.shutdown(Shutdown::Both) {
            if e.kind() != std::io::ErrorKind::NotConnected {
                log::debug!("connection_pool: shutdown error for {}: {}", self.addr, e);
            }
        }
    }
}

/// The Mio-based TCP connection pool. This is a simplified port of the C++
/// TcpConnectionPool semantics: acquire returns a connection which is
/// automatically returned on Drop.
pub struct TcpConnectionPool<H: NetworkHandler> {
    handler: Arc<H>,
    max: usize,
    endpoint_manager: Arc<crate::net::endpoint::EndpointManager>,
    idle: Mutex<VecDeque<Connection>>,
    // Number of currently active (checked-out) connections
    active: Mutex<usize>,
}

impl<H: NetworkHandler> TcpConnectionPool<H> {
    pub fn new(
        min: usize,
        max: usize,
        handler: Arc<H>,
        endpoint_manager: Arc<crate::net::endpoint::EndpointManager>,
    ) -> Arc<Self> {
        let pool = Arc::new(Self {
            handler: Arc::clone(&handler),
            max,
            endpoint_manager: Arc::clone(&endpoint_manager),
            idle: Mutex::new(VecDeque::new()),
            active: Mutex::new(0),
        });

        // Pre-warm: attempt to create `min` connections and place into idle queue.
        // Failures are ignored (network may be unavailable at startup).
        if min > 0 {
            for _ in 0..min {
                if let Some(ep) = pool.endpoint_manager.acquire_endpoint() {
                    // Use connect_timeout with a short pre-warm timeout so startup
                    // doesn't block for long when endpoints are unreachable.
                    let timeout = std::time::Duration::from_millis(500);
                    log::debug!(
                        "connection_pool: pre-warm trying to connect to {} (timeout {:?})",
                        ep,
                        timeout
                    );
                    match StdTcpStream::connect_timeout(&ep, timeout) {
                        Ok(std_stream) => {
                            let _ = std_stream.set_nodelay(true);
                            // set read/write timeouts to avoid blocking indefinitely
                            let _ = std_stream.set_read_timeout(Some(timeout));
                            let _ = std_stream.set_write_timeout(Some(timeout));
                            // Convert to mio TcpStream
                            let mut stream = TcpStream::from_std(std_stream);
                            // run handshake but ignore errors during pre-warm
                            match pool.handler.handshake(&mut stream) {
                                Ok(()) => {
                                    log::debug!(
                                        "connection_pool: pre-warm handshake ok for {}",
                                        ep
                                    );
                                    // lock only when pushing back into idle to avoid
                                    // holding the idle mutex while performing network ops
                                    let mut idle = pool.idle.lock().unwrap();
                                    idle.push_back(Connection::new(stream, ep));
                                }
                                Err(e) => {
                                    if let Err(shutdown_err) = stream.shutdown(Shutdown::Both) {
                                        if shutdown_err.kind() != std::io::ErrorKind::NotConnected {
                                            log::debug!(
                                                "connection_pool: pre-warm shutdown error for {}: {}",
                                                ep,
                                                shutdown_err
                                            );
                                        }
                                    }
                                    log::warn!(
                                        "connection_pool: pre-warm handshake failed for {}: {}",
                                        ep,
                                        e
                                    );
                                    pool.endpoint_manager.release_endpoint(ep);
                                }
                            }
                        }
                        Err(e) => {
                            log::warn!("connection_pool: pre-warm connect to {} failed: {}", ep, e);
                            // if connect failed, release endpoint slot
                            pool.endpoint_manager.release_endpoint(ep);
                        }
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
                        Ok(true) => {
                            conn.last_used = Instant::now();
                            survivors.push(conn);
                        }
                        _ => {
                            // dead or error => release endpoint slot and drop
                            conn.close();
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
        const WAIT_INTERVAL_MS: u64 = 50;
        const MAX_CONNECT_ATTEMPTS: usize = 5;
        let wait_interval = Duration::from_millis(WAIT_INTERVAL_MS);
        let cooldown = Duration::from_secs(30);

        let mut connect_attempts = 0usize;

        loop {
            if let Some(conn) = self.try_take_idle() {
                return Ok(conn);
            }

            {
                let active = self.active.lock().unwrap();
                if *active >= self.max {
                    log::trace!(
                        "connection_pool: waiting for free slot (active={} max={})",
                        *active,
                        self.max
                    );
                    drop(active);
                    thread::sleep(wait_interval);
                    continue;
                }
            }

            match self.endpoint_manager.acquire_endpoint() {
                Some(endpoint) => {
                    connect_attempts += 1;
                    log::debug!(
                        "connection_pool: acquire connecting to {} (attempt {}/{})",
                        endpoint,
                        connect_attempts,
                        MAX_CONNECT_ATTEMPTS
                    );

                    match self.establish_connection(endpoint, cooldown) {
                        Ok(conn) => return Ok(conn),
                        Err(e) => {
                            if connect_attempts >= MAX_CONNECT_ATTEMPTS {
                                return Err(e);
                            }
                            thread::sleep(wait_interval);
                        }
                    }
                }
                None => {
                    log::trace!("connection_pool: no endpoints currently available, waiting");
                    thread::sleep(wait_interval);
                }
            }
        }
    }

    fn try_take_idle(self: &Arc<Self>) -> Option<PooledConnection<H>> {
        let maybe_conn = {
            let mut idle = self.idle.lock().unwrap();
            idle.pop_front()
        };

        if let Some(mut conn) = maybe_conn {
            conn.last_used = Instant::now();
            {
                let mut active = self.active.lock().unwrap();
                *active += 1;
            }
            Some(PooledConnection {
                pool: Arc::clone(self),
                conn: Some(conn),
            })
        } else {
            None
        }
    }

    fn establish_connection(
        self: &Arc<Self>,
        endpoint: SocketAddr,
        cooldown: Duration,
    ) -> std::io::Result<PooledConnection<H>> {
        let timeout = self.handler.timeout();
        match StdTcpStream::connect_timeout(&endpoint, timeout) {
            Ok(std_stream) => {
                let _ = std_stream.set_nodelay(true);
                let _ = std_stream.set_read_timeout(Some(timeout));
                let _ = std_stream.set_write_timeout(Some(timeout));
                let mut stream = TcpStream::from_std(std_stream);
                log::debug!("connection_pool: running handshake for {}", endpoint);
                match self.handler.handshake(&mut stream) {
                    Ok(()) => {
                        log::debug!("connection_pool: handshake succeeded for {}", endpoint);
                        {
                            let mut active = self.active.lock().unwrap();
                            *active += 1;
                        }
                        let conn = Connection::new(stream, endpoint);
                        Ok(PooledConnection {
                            pool: Arc::clone(self),
                            conn: Some(conn),
                        })
                    }
                    Err(e) => {
                        if let Err(shutdown_err) = stream.shutdown(Shutdown::Both) {
                            if shutdown_err.kind() != std::io::ErrorKind::NotConnected {
                                log::debug!(
                                    "connection_pool: handshake shutdown error for {}: {}",
                                    endpoint,
                                    shutdown_err
                                );
                            }
                        }
                        log::error!("connection_pool: handshake failed for {}: {}", endpoint, e);
                        self.endpoint_manager.mark_failed(endpoint, cooldown);
                        self.endpoint_manager.release_endpoint(endpoint);
                        Err(e)
                    }
                }
            }
            Err(e) => {
                log::error!("connection_pool: connect failed for {}: {}", endpoint, e);
                self.endpoint_manager.mark_failed(endpoint, cooldown);
                self.endpoint_manager.release_endpoint(endpoint);
                Err(e)
            }
        }
    }

    fn release(&self, mut conn: Connection) {
        conn.last_used = Instant::now();
        // Match the C++ release behavior: always return the connection to the
        // idle queue while keeping the endpoint allocation reserved. The
        // endpoint is only released when the connection is explicitly closed
        // or deemed unhealthy.
        {
            let mut idle = self.idle.lock().unwrap();
            log::debug!(
                "connection_pool: returning connection to idle for {}",
                conn.addr
            );
            idle.push_back(conn);
        }

        let mut active = self.active.lock().unwrap();
        if *active > 0 {
            *active -= 1;
        }
    }

    /// Add an endpoint to the manager
    pub fn add_endpoint(&self, addr: SocketAddr, max_connections: usize) -> bool {
        self.endpoint_manager.add_endpoint(addr, max_connections)
    }

    pub fn get_endpoint_stats(&self, addr: SocketAddr) -> Option<(usize, usize)> {
        self.endpoint_manager.get_endpoint_stats(addr)
    }

    /// Return the configured maximum number of connections for this pool.
    pub fn max_connections(&self) -> usize {
        let endpoint_count = self.endpoint_manager.get_all_endpoints().len();
        if endpoint_count == 0 {
            return self.max;
        }
        std::cmp::min(self.max, endpoint_count)
    }
}

/// RAII guard that returns the connection to the pool when dropped.
pub struct PooledConnection<H: NetworkHandler> {
    pool: Arc<TcpConnectionPool<H>>,
    conn: Option<Connection>,
}

impl<H: NetworkHandler> PooledConnection<H> {
    pub fn stream(&mut self) -> &mut TcpStream {
        &mut self.conn.as_mut().unwrap().stream
    }
    pub fn addr(&self) -> SocketAddr {
        self.conn.as_ref().unwrap().addr
    }
}

impl<H: NetworkHandler> Drop for PooledConnection<H> {
    fn drop(&mut self) {
        if let Some(conn) = self.conn.take() {
            self.pool.release(conn);
        } else {
            // If no conn, it means connection creation failed, decrement active
            let mut active = self.pool.active.lock().unwrap();
            *active = active.saturating_sub(1);
            // No Condvar present to notify; removed waiting semantics to match C++ flow.
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
        fn handshake(&self, _stream: &mut TcpStream) -> std::io::Result<()> {
            Ok(())
        }
        fn keepalive(&self, _stream: &mut TcpStream) -> std::io::Result<bool> {
            Ok(true)
        }
        fn timeout(&self) -> Duration {
            Duration::from_secs(1)
        }
        fn check_interval(&self) -> Duration {
            Duration::from_millis(50)
        }
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
