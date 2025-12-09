use super::*;
use std::net::{TcpListener, TcpStream as StdTcpStream};
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;

/// Mock handler for testing
#[derive(Debug)]
struct MockHandler {
    handshake_success: bool,
    keepalive_success: bool,
    timeout: Duration,
    check_interval: Duration,
}

impl MockHandler {
    fn new(handshake_success: bool, keepalive_success: bool) -> Self {
        Self {
            handshake_success,
            keepalive_success,
            timeout: Duration::from_secs(1),
            check_interval: Duration::from_millis(50),
        }
    }
}

impl NetworkOperationHandler for MockHandler {
    fn handshake(&self, _stream: &mut TcpStream) -> std::io::Result<()> {
        if self.handshake_success {
            Ok(())
        } else {
            Err(std::io::Error::new(std::io::ErrorKind::ConnectionRefused, "Mock handshake failed"))
        }
    }

    fn handshake_std(&self, _stream: &mut StdTcpStream) -> std::io::Result<()> {
        if self.handshake_success {
            Ok(())
        } else {
            Err(std::io::Error::new(std::io::ErrorKind::ConnectionRefused, "Mock handshake failed"))
        }
    }

    fn keepalive(&self, _stream: &mut TcpStream) -> std::io::Result<bool> {
        Ok(self.keepalive_success)
    }

    fn timeout(&self) -> Duration {
        self.timeout
    }

    fn check_interval(&self) -> Duration {
        self.check_interval
    }
}

/// Test helper to create a local TCP server
struct TestServer {
    listener: TcpListener,
    accepted: Arc<Mutex<Vec<StdTcpStream>>>,
}

impl TestServer {
    fn new() -> Self {
        let listener = TcpListener::bind(("127.0.0.1", 0)).expect("Failed to bind test server");
        listener.set_nonblocking(true).expect("Failed to set nonblocking");
        Self {
            listener,
            accepted: Arc::new(Mutex::new(Vec::new())),
        }
    }

    fn addr(&self) -> SocketAddr {
        self.listener.local_addr().unwrap()
    }

    fn start_accepting(&self) {
        let listener = self.listener.try_clone().unwrap();
        let accepted = Arc::clone(&self.accepted);
        
        thread::spawn(move || {
            let start = Instant::now();
            while start.elapsed() < Duration::from_secs(2) {
                match listener.accept() {
                    Ok((stream, _)) => {
                        accepted.lock().unwrap().push(stream);
                    }
                    Err(ref e) if e.kind() == std::io::ErrorKind::WouldBlock => {
                        thread::sleep(Duration::from_millis(10));
                    }
                    Err(_) => break,
                }
            }
        });
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_connection_pool_creation() {
        /// Test basic pool creation and configuration
        let handler = Arc::new(MockHandler::new(true, true));
        let endpoint_manager = Arc::new(crate::io::endpoint::EndpointManager::new());
        
        let pool = TcpConnectionPool::new(2, 5, handler, endpoint_manager);
        
        assert_eq!(pool.max_connections(), 5);
    }

    #[test]
    fn test_connection_pool_with_endpoints() {
        /// Test pool creation with endpoint management
        let handler = Arc::new(MockHandler::new(true, true));
        let endpoint_manager = Arc::new(crate::io::endpoint::EndpointManager::new());
        
        let server = TestServer::new();
        server.start_accepting();
        
        // Add endpoint to manager
        assert!(endpoint_manager.add_endpoint(server.addr(), 3));
        
        let pool = TcpConnectionPool::new(1, 3, handler, endpoint_manager);
        
        // Pool max should be limited by endpoint count
        assert_eq!(pool.max_connections(), 3);
    }

    #[test]
    fn test_connection_acquire_and_release() {
        /// Test basic connection lifecycle
        let handler = Arc::new(MockHandler::new(true, true));
        let endpoint_manager = Arc::new(crate::io::endpoint::EndpointManager::new());
        
        let server = TestServer::new();
        server.start_accepting();
        
        assert!(endpoint_manager.add_endpoint(server.addr(), 2));
        
        let pool = TcpConnectionPool::new(0, 2, handler, endpoint_manager);
        
        // Acquire first connection
        let conn1 = pool.acquire().expect("Failed to acquire first connection");
        assert_eq!(conn1.addr(), server.addr());
        
        // Acquire second connection
        let conn2 = pool.acquire().expect("Failed to acquire second connection");
        assert_eq!(conn2.addr(), server.addr());
        
        // Release connections (happens automatically when dropped)
        drop(conn1);
        drop(conn2);
        
        // Verify connections are returned to pool
        let conn3 = pool.acquire().expect("Failed to acquire connection after release");
        assert_eq!(conn3.addr(), server.addr());
    }

    #[test]
    fn test_connection_pool_max_limit() {
        /// Test connection pool maximum limit enforcement
        let handler = Arc::new(MockHandler::new(true, true));
        let endpoint_manager = Arc::new(crate::io::endpoint::EndpointManager::new());
        
        let server = TestServer::new();
        server.start_accepting();
        
        assert!(endpoint_manager.add_endpoint(server.addr(), 2));
        
        let pool = TcpConnectionPool::new(0, 2, handler, endpoint_manager);
        
        // Acquire all available connections
        let conn1 = pool.acquire().expect("Failed to acquire first connection");
        let conn2 = pool.acquire().expect("Failed to acquire second connection");
        
        // Third acquisition should timeout or fail due to pool being full
        let result = std::panic::catch_unwind(|| {
            pool.acquire().expect_err("Should fail to acquire third connection");
        });
        
        assert!(result.is_ok(), "Third connection acquisition should have failed");
        
        drop(conn1);
        drop(conn2);
    }

    #[test]
    fn test_connection_pool_pre_warm() {
        /// Test connection pool pre-warming functionality
        let handler = Arc::new(MockHandler::new(true, true));
        let endpoint_manager = Arc::new(crate::io::endpoint::EndpointManager::new());
        
        let server = TestServer::new();
        server.start_accepting();
        
        assert!(endpoint_manager.add_endpoint(server.addr(), 3));
        
        // Create pool with pre-warm of 2 connections
        let pool = TcpConnectionPool::new(2, 3, handler, endpoint_manager);
        
        // Should be able to acquire connections immediately due to pre-warming
        let conn = pool.acquire().expect("Failed to acquire pre-warmed connection");
        assert_eq!(conn.addr(), server.addr());
        
        drop(conn);
    }

    #[test]
    fn test_connection_handshake_failure() {
        /// Test connection acquisition with handshake failure
        let handler = Arc::new(MockHandler::new(false, true)); // Handshake fails
        let endpoint_manager = Arc::new(crate::io::endpoint::EndpointManager::new());
        
        let server = TestServer::new();
        server.start_accepting();
        
        assert!(endpoint_manager.add_endpoint(server.addr(), 2));
        
        let pool = TcpConnectionPool::new(0, 2, handler, endpoint_manager);
        
        // Connection acquisition should fail due to handshake failure
        let result = pool.acquire();
        assert!(result.is_err(), "Connection should fail due to handshake error");
    }

    #[test]
    fn test_connection_keepalive_failure() {
        /// Test connection keepalive failure handling
        let handler = Arc::new(MockHandler::new(true, false)); // Keepalive fails
        let endpoint_manager = Arc::new(crate::io::endpoint::EndpointManager::new());
        
        let server = TestServer::new();
        server.start_accepting();
        
        assert!(endpoint_manager.add_endpoint(server.addr(), 2));
        
        let pool = TcpConnectionPool::new(1, 2, handler, endpoint_manager);
        
        // Wait for keepalive thread to run
        thread::sleep(Duration::from_millis(100));
        
        // The connection should be removed from pool due to keepalive failure
        // This is harder to test directly, but we can verify pool behavior
        let conn = pool.acquire().expect("Should be able to acquire new connection");
        assert_eq!(conn.addr(), server.addr());
        
        drop(conn);
    }

    #[test]
    fn test_pooled_connection_drop_behavior() {
        /// Test that connections are properly returned to pool when dropped
        let handler = Arc::new(MockHandler::new(true, true));
        let endpoint_manager = Arc::new(crate::io::endpoint::EndpointManager::new());
        
        let server = TestServer::new();
        server.start_accepting();
        
        assert!(endpoint_manager.add_endpoint(server.addr(), 2));
        
        let pool = TcpConnectionPool::new(0, 2, handler, endpoint_manager);
        
        // Track active connections
        let active_guard = pool.active.lock().unwrap();
        let initial_active = *active_guard;
        drop(active_guard);
        
        {
            let conn = pool.acquire().expect("Failed to acquire connection");
            let active_guard = pool.active.lock().unwrap();
            assert_eq!(*active_guard, initial_active + 1);
            drop(active_guard);
            
            // Connection is dropped at end of scope
        }
        
        // Verify connection was returned to pool
        let active_guard = pool.active.lock().unwrap();
        assert_eq!(*active_guard, initial_active);
    }

    #[test]
    fn test_endpoint_management() {
        /// Test endpoint addition, removal, and statistics
        let endpoint_manager = Arc::new(crate::io::endpoint::EndpointManager::new());
        
        let server1 = TestServer::new();
        let server2 = TestServer::new();
        
        // Add endpoints
        assert!(endpoint_manager.add_endpoint(server1.addr(), 2));
        assert!(endpoint_manager.add_endpoint(server2.addr(), 3));
        
        // Verify endpoints are tracked
        let endpoints = endpoint_manager.get_all_endpoints();
        assert!(endpoints.contains(&server1.addr()));
        assert!(endpoints.contains(&server2.addr()));
        
        // Test endpoint statistics
        let stats1 = endpoint_manager.get_endpoint_stats(server1.addr());
        assert!(stats1.is_some());
        let (current1, max1) = stats1.unwrap();
        assert_eq!(current1, 0); // No connections acquired yet
        assert_eq!(max1, 2);
        
        // Remove endpoint
        endpoint_manager.remove_endpoint(server1.addr());
        let endpoints_after_removal = endpoint_manager.get_all_endpoints();
        assert!(!endpoints_after_removal.contains(&server1.addr()));
        assert!(endpoints_after_removal.contains(&server2.addr()));
    }

    #[test]
    fn test_concurrent_connection_acquisition() {
        /// Test thread-safe connection acquisition
        let handler = Arc::new(MockHandler::new(true, true));
        let endpoint_manager = Arc::new(crate::io::endpoint::EndpointManager::new());
        
        let server = TestServer::new();
        server.start_accepting();
        
        assert!(endpoint_manager.add_endpoint(server.addr(), 5));
        
        let pool = TcpConnectionPool::new(0, 5, handler, endpoint_manager);
        
        let mut handles = vec![];
        let success_count = Arc::new(Mutex::new(0));
        
        // Spawn multiple threads to acquire connections concurrently
        for _ in 0..5 {
            let pool = Arc::clone(&pool);
            let success_count = Arc::clone(&success_count);
            
            let handle = thread::spawn(move || {
                match pool.acquire() {
                    Ok(conn) => {
                        *success_count.lock().unwrap() += 1;
                        // Hold connection for a short time
                        thread::sleep(Duration::from_millis(10));
                        drop(conn);
                    }
                    Err(_) => {
                        // Connection acquisition failed
                    }
                }
            });
            
            handles.push(handle);
        }
        
        // Wait for all threads to complete
        for handle in handles {
            handle.join().unwrap();
        }
        
        // All threads should have successfully acquired connections
        assert_eq!(*success_count.lock().unwrap(), 5);
    }

    #[test]
    fn test_connection_pool_cleanup() {
        /// Test that connections are properly cleaned up
        let handler = Arc::new(MockHandler::new(true, true));
        let endpoint_manager = Arc::new(crate::io::endpoint::EndpointManager::new());
        
        let server = TestServer::new();
        server.start_accepting();
        
        assert!(endpoint_manager.add_endpoint(server.addr(), 2));
        
        let pool = TcpConnectionPool::new(0, 2, handler, endpoint_manager);
        
        // Acquire and immediately release connections
        for _ in 0..3 {
            let conn = pool.acquire().expect("Failed to acquire connection");
            drop(conn);
        }
        
        // Pool should still be functional after multiple acquire/release cycles
        let conn = pool.acquire().expect("Pool should still work after cleanup");
        assert_eq!(conn.addr(), server.addr());
        
        drop(conn);
    }
}
