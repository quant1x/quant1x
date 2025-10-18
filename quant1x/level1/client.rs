use mio::net::TcpStream as MioTcpStream;
use std::net::SocketAddr;
use std::str::FromStr;
use std::sync::{Arc, Mutex, OnceLock};
use std::time::Duration;

use super::{
    HeartbeatRequest, HeartbeatResponse, Hello1Request, Hello1Response, Hello2Request,
    Hello2Response,
};

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
        log::debug!(
            "ProtocolHandler::handshake -> sending Hello1 ({} bytes): {}",
            req_buf1.len(),
            hex::encode(&req_buf1)
        );
        match crate::level1::process_request(stream, &req_buf1) {
            Ok(body1) => {
                log::debug!(
                    "ProtocolHandler::handshake <- received Hello1 body ({} bytes): {}",
                    body1.len(),
                    if body1.len() > 128 {
                        hex::encode(&body1[..128]) + "..."
                    } else {
                        hex::encode(&body1)
                    }
                );
                let mut resp1 = Hello1Response::new();
                resp1.deserialize(&body1);
                log::debug!(
                    "ProtocolHandler::handshake Hello1 parsed info: {}",
                    resp1.info
                );
                // validate Hello1 response: must contain non-empty info
                if resp1.info.trim().is_empty() {
                    log::error!("ProtocolHandler::handshake Hello1 validation failed: empty info");
                    return Err(std::io::Error::new(
                        std::io::ErrorKind::Other,
                        "Hello1 response invalid or empty",
                    ));
                }
            }
            Err(e) => {
                log::error!("ProtocolHandler::handshake Hello1 failed: {}", e);
                return Err(e);
            }
        }

        // Hello2
        let mut req2 = Hello2Request::new();
        let req_buf2 = req2.serialize();
        log::debug!(
            "ProtocolHandler::handshake -> sending Hello2 ({} bytes): {}",
            req_buf2.len(),
            hex::encode(&req_buf2)
        );
        match crate::level1::process_request(stream, &req_buf2) {
            Ok(body2) => {
                log::debug!(
                    "ProtocolHandler::handshake <- received Hello2 body ({} bytes): {}",
                    body2.len(),
                    if body2.len() > 128 {
                        hex::encode(&body2[..128]) + "..."
                    } else {
                        hex::encode(&body2)
                    }
                );
                let mut resp2 = Hello2Response::new();
                resp2.deserialize(&body2);
                log::debug!(
                    "ProtocolHandler::handshake Hello2 parsed info: {}",
                    resp2.info
                );
                // validate Hello2 response as well
                if resp2.info.trim().is_empty() {
                    log::error!("ProtocolHandler::handshake Hello2 validation failed: empty info");
                    return Err(std::io::Error::new(
                        std::io::ErrorKind::Other,
                        "Hello2 response invalid or empty",
                    ));
                }
            }
            Err(e) => {
                log::error!("ProtocolHandler::handshake Hello2 failed: {}", e);
                return Err(e);
            }
        }

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
static CONNECTION_POOL: OnceLock<Arc<crate::net::TcpConnectionPool<ProtocolHandler>>> =
    OnceLock::new();

/// Acquire a pooled connection to a level1 server.
///
/// This returns a `PooledConnection<ProtocolHandler>` which will return the
/// connection to the pool when dropped. Endpoints may be seeded from the
/// `QUANT1X_LEVEL1_SERVERS` environment variable, e.g.:
///
///   QUANT1X_LEVEL1_SERVERS=110.41.147.114:7709,124.70.176.52:7709
pub fn client() -> std::io::Result<crate::net::PooledConnection<ProtocolHandler>> {
    let pool = CONNECTION_POOL
        .get_or_init(|| {
            let endpoint_manager = Arc::new(crate::net::endpoint::EndpointManager::new());

            // gather server list, prefer cached and fall back to detection
            let mut servers: Vec<crate::level1::config::ServerInfo> = Vec::new();
            if let Some(cached) = crate::level1::config::load_cached_servers() {
                if !cached.is_empty() {
                    log::debug!("level1: loaded {} cached servers", cached.len());
                    servers = cached;
                }
            }

            if servers.is_empty() {
                log::debug!("level1: no cached servers, running detect()");
                let detected = crate::level1::config::detect(
                    crate::level1::config::MAX_ELAPSED_TIME_MS,
                    crate::level1::config::MAX_CONNECTIONS,
                    crate::level1::config::DEFAULT_CONNECT_TIMEOUT_MS,
                );
                log::debug!("level1: detect() returned {} servers", detected.len());
                if !detected.is_empty() {
                    crate::level1::config::save_cached_servers(&detected);
                }
                servers = detected;
            } else {
                log::debug!("level1: using cached servers for pool seeding");
            }

            if servers.is_empty() {
                log::warn!("level1: detection produced no servers, falling back to standard list");
                servers = crate::level1::config::standard_server_list();
            }

            // seed endpoint manager before constructing pool so pre-warm can reuse them
            for s in servers.iter() {
                match SocketAddr::from_str(&s.addr()) {
                    Ok(addr) => {
                        let _ = endpoint_manager.add_endpoint(addr, 1);
                    }
                    Err(e) => {
                        log::warn!("level1: invalid server addr {}: {}", s.addr(), e);
                    }
                }
            }

            let server_count = servers.len();
            if server_count == 0 {
                // standard list should not be empty, but guard anyway
                log::error!("level1: no servers available for connection pool initialization");
                panic!("level1: server list empty");
            }

            let pool_max =
                std::cmp::min(crate::level1::config::MAX_CONNECTIONS, server_count.max(1));
            log::debug!(
                "level1: initializing pool with max_connections={} (servers={})",
                pool_max,
                server_count
            );

            let handler = Arc::new(ProtocolHandler {});
            crate::net::TcpConnectionPool::new(1, pool_max, handler, endpoint_manager)
        })
        .clone();

    pool.acquire()
}

/// If the global pool has been initialized, return its configured max connections.
pub fn pool_max_connections() -> Option<usize> {
    CONNECTION_POOL.get().map(|p| p.max_connections())
}

// Implement the Net handler trait so the connection pool can use our handshake/keepalive
impl crate::net::NetworkHandler for ProtocolHandler {
    fn handshake(&self, stream: &mut mio::net::TcpStream) -> std::io::Result<()> {
        match ProtocolHandler::handshake(stream) {
            Ok(true) => Ok(()),
            Ok(false) => Err(std::io::Error::new(
                std::io::ErrorKind::Other,
                "handshake failed",
            )),
            Err(e) => Err(e),
        }
    }

    fn keepalive(&self, stream: &mut mio::net::TcpStream) -> std::io::Result<bool> {
        ProtocolHandler::keepalive(stream)
    }

    fn timeout(&self) -> Duration {
        Duration::from_secs(10)
    }
    fn check_interval(&self) -> Duration {
        Duration::from_secs(5)
    }
}
