use mio::net::TcpStream as MioTcpStream;
use std::net::SocketAddr;
use std::str::FromStr;
use std::sync::{Arc, Mutex, OnceLock};
use std::time::Duration;

use super::{
    HeartbeatRequest, HeartbeatResponse, Hello1Request, Hello1Response, Hello2Request,
    Hello2Response,
};

pub struct StandardProtocolHandler {}

impl StandardProtocolHandler {
    /// 执行标准协议握手流程
    ///
    /// 该函数执行两个阶段的握手：
    /// 1. Hello1 - 验证服务器基本信息
    /// 2. Hello2 - 确认连接参数
    ///
    /// # 参数
    /// * `stream` - TCP 流连接
    ///
    /// # 返回值
    /// * `Ok(true)` - 握手成功
    /// * `Err(e)` - 握手失败，包含错误信息
    pub fn handshake(stream: &mut MioTcpStream) -> std::io::Result<bool> {
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

    /// 发送并处理心跳包以保持TCP连接活跃
    ///
    /// 该函数执行以下操作：
    /// 1. 创建心跳请求和响应对象
    /// 2. 通过底层协议处理器发送请求并等待响应
    ///
    /// # 参数
    /// * `stream` - 可变的TCP流连接引用
    ///
    /// # 返回值
    /// * `Ok(true)` - 心跳处理成功
    /// * `Err(e)` - 处理失败，包含IO错误信息
    ///
    /// # 错误
    /// 当底层协议处理失败时，会返回包含错误信息的`std::io::Error`
    pub fn keepalive(stream: &mut MioTcpStream) -> std::io::Result<bool> {
        let mut req = HeartbeatRequest::new();
        let mut resp = HeartbeatResponse::new();
        crate::level1::process(stream, &mut req, &mut resp)
            .map_err(|e| std::io::Error::new(std::io::ErrorKind::Other, e))?;
        Ok(true)
    }

    /// 执行标准协议握手流程（使用标准库TcpStream）
    ///
    /// 该函数执行两个阶段的握手协议：
    /// 1. Hello1 - 验证服务器基本信息
    /// 2. Hello2 - 确认连接参数
    ///
    /// # 参数
    /// * `stream` - 标准库的TCP流连接
    ///
    /// # 返回值
    /// * `Ok(true)` - 握手成功
    /// * `Err(e)` - 握手失败，包含错误信息
    ///
    /// # 错误
    /// * 当Hello1或Hello2阶段响应无效或为空时返回错误
    /// * 当网络通信或协议处理失败时返回错误
    ///
    /// # 注意
    /// * 每个阶段都会验证服务器返回的信息是否为空
    /// * 使用标准库的TcpStream而不是MioTcpStream
    pub fn handshake_std(stream: &mut std::net::TcpStream) -> std::io::Result<bool> {
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

static STD_CONNECTION_POOL: OnceLock<Arc<crate::io::TcpConnectionPool<StandardProtocolHandler>>> =
    OnceLock::new();

/// 获取标准协议连接池中的连接
///
/// 该函数会初始化或复用标准连接池，并按以下顺序获取服务器列表：
/// 1. 优先尝试加载缓存的服务器列表
/// 2. 若缓存为空，则执行自动检测获取可用服务器
/// 3. 若检测失败，则回退到标准服务器列表
///
/// # 返回值
/// * `Ok(PooledConnection)` - 成功获取的连接
/// * `Err(io::Error)` - 获取连接时发生的错误
///
/// # Panics
/// * 当服务器列表为空时，会触发panic（标准服务器列表不应为空）
///
/// # 注意
/// * 连接池会在首次调用时初始化，后续调用会复用已初始化的连接池
/// * 连接池大小受MAX_CONNECTIONS配置和实际服务器数量限制
pub fn get_std_conn() -> std::io::Result<crate::io::PooledConnection<StandardProtocolHandler>> {
    let pool = STD_CONNECTION_POOL
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

/// 获取连接池的最大连接数
///
/// 该函数返回标准连接池(STD_CONNECTION_POOL)中配置的最大连接数。
///
/// # 返回值
/// * `Some(usize)` - 如果连接池已初始化，返回最大连接数
/// * `None` - 如果连接池未初始化
pub fn pool_max_connections() -> Option<usize> {
    STD_CONNECTION_POOL.get().map(|p| p.max_connections())
}

impl crate::io::NetworkOperationHandler for StandardProtocolHandler {
    /// 执行标准协议握手流程
    ///
    /// 该函数通过调用 `StandardProtocolHandler::handshake` 执行握手协议，
    /// 并将结果转换为更简单的 `Result<(), io::Error>` 类型
    ///
    /// # 参数
    /// * `stream` - 要执行握手的 TCP 流
    ///
    /// # 返回值
    /// * `Ok(())` - 握手成功
    /// * `Err(e)` - 握手失败，包含错误信息
    ///
    /// # 错误
    /// 当握手失败时返回 `io::Error`，可能包含以下错误类型：
    /// * `ErrorKind::Other` - 握手协议失败
    /// * 其他底层 IO 错误
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

    /// 发送并处理保活消息
    ///
    /// 该函数通过底层协议处理器发送保活消息并等待响应，
    /// 用于维持TCP连接的活动状态
    ///
    /// # 参数
    /// * `stream` - 要发送保活消息的TCP流
    ///
    /// # 返回值
    /// * `Ok(true)` - 保活成功
    /// * `Err(e)` - 保活失败，包含错误信息
    fn keepalive(&self, stream: &mut mio::net::TcpStream) -> std::io::Result<bool> {
        StandardProtocolHandler::keepalive(stream)
    }

    /// 获取超时时间
    ///
    /// 返回一个固定的超时持续时间，当前设置为10秒
    ///
    /// # 返回值
    /// * `Duration` - 表示10秒的时间间隔
    fn timeout(&self) -> Duration {
        Duration::from_secs(10)
    }

    /// 获取检查间隔时间
    ///
    /// 返回一个固定的5秒时间间隔
    ///
    /// # 返回值
    /// * `Duration` - 表示5秒时间间隔的Duration对象
    fn check_interval(&self) -> Duration {
        Duration::from_secs(5)
    }
}
