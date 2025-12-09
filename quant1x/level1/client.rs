use mio::net::TcpStream as MioTcpStream;
use std::net::SocketAddr;
use std::str::FromStr;
use std::sync::{Arc, Mutex, OnceLock};
use std::time::Duration;

use super::{
    HeartbeatRequest, HeartbeatResponse, Hello1Request, Hello1Response, Hello2Request,
    Hello2Response,
};

/// 精简的客户端协议辅助工具, 模拟 C++ �?StandardProtocolHandler 的握手与保活实现.
///
/// 说明:
///   该处 Rust 端口保留了相同的连接池语�? 它会�?meta 目录下的 `server.bin` 读取已缓存的服务器列�? 若缓存缺失或过期则回退到检测例�?
///   此处不使用环境变量覆盖服务器列表.
/// - C++ 实现�?`client()` 会构建一�?TcpConnectionPool，并通过检测例程填充服务器端点并缓存结果�?/// - Rust 端口保留相同语义：优先加载缓存（meta/server.bin），若缺失则运行检测逻辑并缓存检测结果�?pub struct StandardProtocolHandler {}

impl StandardProtocolHandler {
    /// 使用阻塞�?Mio TcpStream 执行两步握手（hello1 + hello2�?    pub fn handshake(stream: &mut MioTcpStream) -> std::io::Result<bool> {
        // Hello1
        let mut req1 = Hello1Request::new();
        let mut resp1 = Hello1Response::new();
        log::debug!("StandardProtocolHandler::handshake -> sending Hello1");
        
        match crate::level1::process(stream, &mut req1, &mut resp1)
            .map_err(|e| std::io::Error::new(std::io::ErrorKind::Other, e))
        {
            Ok(_) => {
                log::debug!(
                    "StandardProtocolHandler::handshake Hello1 parsed info: {}",
                    resp1.info
                );
                // validate Hello1 response: must contain non-empty info
                if resp1.info.trim().is_empty() {
                    log::error!(
                        "StandardProtocolHandler::handshake Hello1 validation failed: empty info"
                    );
                    return Err(std::io::Error::new(
                        std::io::ErrorKind::Other,
                        "Hello1 response invalid or empty",
                    ));
                }
            }
            Err(e) => {
                log::error!("StandardProtocolHandler::handshake Hello1 failed: {}", e);
                return Err(e);
            }
        }

        // Hello2
        let mut req2 = Hello2Request::new();
        let mut resp2 = Hello2Response::new();
        log::debug!("StandardProtocolHandler::handshake -> sending Hello2");

        match crate::level1::process(stream, &mut req2, &mut resp2)
            .map_err(|e| std::io::Error::new(std::io::ErrorKind::Other, e))
        {
            Ok(_) => {
                log::debug!(
                    "StandardProtocolHandler::handshake Hello2 parsed info: {}",
                    resp2.info
                );
                // validate Hello2 response as well
                if resp2.info.trim().is_empty() {
                    log::error!(
                        "StandardProtocolHandler::handshake Hello2 validation failed: empty info"
                    );
                    return Err(std::io::Error::new(
                        std::io::ErrorKind::Other,
                        "Hello2 response invalid or empty",
                    ));
                }
            }
            Err(e) => {
                log::error!("StandardProtocolHandler::handshake Hello2 failed: {}", e);
                return Err(e);
            }
        }

        Ok(true)
    }

    pub fn keepalive(stream: &mut MioTcpStream) -> std::io::Result<bool> {
        let mut req = HeartbeatRequest::new();
        let mut resp = HeartbeatResponse::new();
        crate::level1::process(stream, &mut req, &mut resp)
            .map_err(|e| std::io::Error::new(std::io::ErrorKind::Other, e))?;
        Ok(true)
    }

    /// 使用 `std::net::TcpStream` 的阻塞握手。该方法会遵�?std 的读/写超时设置，
    /// 用于在将流转换为 `mio::TcpStream` 之前完成握手�?    pub fn handshake_std(stream: &mut std::net::TcpStream) -> std::io::Result<bool> {
        // Hello1
        let mut req1 = Hello1Request::new();
        let mut resp1 = Hello1Response::new();
        log::debug!("StandardProtocolHandler::handshake_std -> sending Hello1");

        match crate::level1::process(stream, &mut req1, &mut resp1)
            .map_err(|e| std::io::Error::new(std::io::ErrorKind::Other, e))
        {
            Ok(_) => {
                log::debug!(
                    "StandardProtocolHandler::handshake_std Hello1 parsed info: {}",
                    resp1.info
                );
                if resp1.info.trim().is_empty() {
                    log::error!(
                        "StandardProtocolHandler::handshake_std Hello1 validation failed: empty info"
                    );
                    return Err(std::io::Error::new(
                        std::io::ErrorKind::Other,
                        "Hello1 response invalid or empty",
                    ));
                }
            }
            Err(e) => {
                log::error!(
                    "StandardProtocolHandler::handshake_std Hello1 failed: {}",
                    e
                );
                return Err(e);
            }
        }

        // Hello2
        let mut req2 = Hello2Request::new();
        let mut resp2 = Hello2Response::new();
        log::debug!("StandardProtocolHandler::handshake_std -> sending Hello2");

        match crate::level1::process(stream, &mut req2, &mut resp2)
            .map_err(|e| std::io::Error::new(std::io::ErrorKind::Other, e))
        {
            Ok(_) => {
                log::debug!(
                    "StandardProtocolHandler::handshake_std Hello2 parsed info: {}",
                    resp2.info
                );
                if resp2.info.trim().is_empty() {
                    log::error!(
                        "StandardProtocolHandler::handshake_std Hello2 validation failed: empty info"
                    );
                    return Err(std::io::Error::new(
                        std::io::ErrorKind::Other,
                        "Hello2 response invalid or empty",
                    ));
                }
            }
            Err(e) => {
                log::error!(
                    "StandardProtocolHandler::handshake_std Hello2 failed: {}",
                    e
                );
                return Err(e);
            }
        }

        Ok(true)
    }
}

// 全局连接池单例（在首次调�?`client()` 时初始化）�?static CONNECTION_POOL: OnceLock<Arc<crate::io::TcpConnectionPool<StandardProtocolHandler>>> =
    OnceLock::new();

/// 获取一个到 level1 服务器的池化连接�?///
/// 返回值是 `PooledConnection<StandardProtocolHandler>`，该连接�?Drop 时会自动返回到池中�?/// 端点可以通过环境变量 `QUANT1X_LEVEL1_SERVERS` 进行预置，例如：
///
///   QUANT1X_LEVEL1_SERVERS=110.41.147.114:7709,124.70.176.52:7709
pub fn get_std_conn() -> std::io::Result<crate::io::PooledConnection<StandardProtocolHandler>> {
    let pool = CONNECTION_POOL
        .get_or_init(|| {
            let endpoint_manager = Arc::new(crate::io::endpoint::EndpointManager::new());

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

            let handler = Arc::new(StandardProtocolHandler {});
            crate::io::TcpConnectionPool::new(1, pool_max, handler, endpoint_manager)
        })
        .clone();

    pool.acquire()
}

/// 如果全局连接池已初始化，则返回其配置的最大连接数�?pub fn pool_max_connections() -> Option<usize> {
    CONNECTION_POOL.get().map(|p| p.max_connections())
}

// 实现 Net operation handler 特征，使连接池能够使用我们的握手/保活实现
impl crate::io::NetworkOperationHandler for StandardProtocolHandler {
    fn handshake(&self, stream: &mut mio::net::TcpStream) -> std::io::Result<()> {
        match StandardProtocolHandler::handshake(stream) {
            Ok(true) => Ok(()),
            Ok(false) => Err(std::io::Error::new(
                std::io::ErrorKind::Other,
                "handshake failed",
            )),
            Err(e) => Err(e),
        }
    }

    fn keepalive(&self, stream: &mut mio::net::TcpStream) -> std::io::Result<bool> {
        StandardProtocolHandler::keepalive(stream)
    }

    fn timeout(&self) -> Duration {
        // 默认超时 10 �?        Duration::from_secs(10)
    }
    fn check_interval(&self) -> Duration {
        // 保活检查间�?5 �?        Duration::from_secs(5)
    }
}
