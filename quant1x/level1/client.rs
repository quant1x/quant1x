use mio::net::TcpStream as MioTcpStream;
use std::sync::{Arc, OnceLock};
use std::time::Duration;
use std::net::SocketAddr;
use std::str::FromStr;

use super::{Hello1Request, Hello1Response, Hello2Request, Hello2Response, HeartbeatRequest, HeartbeatResponse};

/// Minimal client-side protocol helper mirroring C++ ProtocolHandler handshake and keepalive.
///
/// Notes:
/// - The original C++ `client()` builds a TcpConnectionPool and seeds endpoints by running
///   a server detection routine and caching results. The Rust port here keeps the pool
///   semantics and intentionally mirrors the C++ behavior exactly: it reads cached
///   servers from the meta `server.bin` and falls back to running the detection routine
///   when the cache is missing or stale. No environment-variable override is used.
pub struct ProtocolHandler {}

impl ProtocolHandler {
    /// Perform a two-step handshake (hello1 + hello2) using blocking Mio TcpStream
    pub fn handshake(stream: &mut MioTcpStream) -> std::io::Result<bool> {
        // Hello1
        let mut req1 = Hello1Request::new();
        let req_buf1 = req1.serialize();
        let body1 = crate::level1::process_request(stream, &req_buf1)?;
        let mut resp1 = Hello1Response::new();
        resp1.deserialize(&body1);

        // Hello2
        let mut req2 = Hello2Request::new();
        let req_buf2 = req2.serialize();
        let body2 = crate::level1::process_request(stream, &req_buf2)?;
        let mut resp2 = Hello2Response::new();
        resp2.deserialize(&body2);

        Ok(true)
    }

    /// Send a heartbeat request and parse the response
    pub fn keepalive(stream: &mut MioTcpStream) -> std::io::Result<bool> {
        let mut req = HeartbeatRequest::new();
        let req_buf = req.serialize();
        let body = crate::level1::process_request(stream, &req_buf)?;
        let mut resp = HeartbeatResponse::new();
        resp.deserialize(&body);
        Ok(true)
    }
}

// Global connection pool singleton (initialized on first call to `client()`).
static CONNECTION_POOL: OnceLock<Arc<crate::net::TcpConnectionPool<ProtocolHandler>>> = OnceLock::new();

/// Acquire a pooled connection to a level1 server.
///
/// This returns a `PooledConnection<ProtocolHandler>` which will return the
/// connection to the pool when dropped. Endpoints may be seeded from the
/// `QUANT1X_LEVEL1_SERVERS` environment variable, e.g.:
///
///   QUANT1X_LEVEL1_SERVERS=110.41.147.114:7709,124.70.176.52:7709
pub fn client() -> std::io::Result<crate::net::PooledConnection<ProtocolHandler>> {
    let pool = CONNECTION_POOL.get_or_init(|| {
        // Build an endpoint manager and optionally seed from env
        let mgr = Arc::new(crate::net::endpoint::EndpointManager::new());

        // 1) Try loading cached servers from the crate meta `server.bin` (C++ parity)
        let mut seeded = false;
        if let Some(servers) = crate::level1::config::load_cached_servers() {
            for s in servers.iter() {
                if let Ok(addr) = SocketAddr::from_str(&s.addr()) {
                    let _ = mgr.add_endpoint(addr, 2);
                    seeded = true;
                }
            }
        }

        // 2) Fallback: run detection and seed endpoints, then save cache
        if !seeded {
            let detected = crate::level1::config::detect(100, 8, 500);
            if !detected.is_empty() {
                for s in detected.iter() {
                    if let Ok(addr) = SocketAddr::from_str(&s.addr()) {
                        let _ = mgr.add_endpoint(addr, 2);
                    }
                }
                // best-effort save
                crate::level1::config::save_cached_servers(&detected);
            }
        }

        let handler = Arc::new(ProtocolHandler {});
        crate::net::TcpConnectionPool::new(1, 10, handler, mgr)
    }).clone();

    pool.acquire()
}

// Implement the Net handler trait so the connection pool can use our handshake/keepalive
impl crate::net::NetworkHandler for ProtocolHandler {
    fn handshake(&self, stream: &mut mio::net::TcpStream) -> std::io::Result<()> {
        match ProtocolHandler::handshake(stream) {
            Ok(true) => Ok(()),
            Ok(false) => Err(std::io::Error::new(std::io::ErrorKind::Other, "handshake failed")),
            Err(e) => Err(e),
        }
    }

    fn keepalive(&self, stream: &mut mio::net::TcpStream) -> std::io::Result<bool> {
        ProtocolHandler::keepalive(stream)
    }

    fn timeout(&self) -> Duration { Duration::from_secs(5) }
    fn check_interval(&self) -> Duration { Duration::from_secs(5) }
}
