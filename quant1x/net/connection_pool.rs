use std::collections::VecDeque;
use std::net::{Shutdown, SocketAddr};
use std::sync::{Arc, Mutex, Weak};
use std::thread;
use std::time::{Duration, Instant};

use mio::net::TcpStream;
use std::net::TcpStream as StdTcpStream;

/// 用户应该实现的连接池特征，用于执行协议特定的工作: 握手和保活检查.
pub trait NetworkHandler: Send + Sync + 'static {
    fn handshake(&self, _stream: &mut TcpStream) -> std::io::Result<()> {
        Ok(())
    }
    /// 可选的阻塞握手，用于在阻塞的 `std::net::TcpStream` 上执行握手。
    /// 默认实现会将其转换为 `mio::TcpStream` 并调用 `handshake`.
    fn handshake_std(&self, stream: &mut std::net::TcpStream) -> std::io::Result<()> {
        // 将流转换为 mio 并调用非阻塞的握手实现 (默认行为).
        let mut mio_stream = TcpStream::from_std(stream.try_clone().map_err(|e| e)?);
        self.handshake(&mut mio_stream)
    }
    fn keepalive(&self, _stream: &mut TcpStream) -> std::io::Result<bool> {
        Ok(true)
    }
    fn timeout(&self) -> Duration {
        // 默认超时 (秒)
        Duration::from_secs(10)
    }
    fn check_interval(&self) -> Duration {
        // 保活检查间隔 (秒)
        Duration::from_secs(5)
    }
}

/// 池化连接包装器。池拥有连接，并在丢弃时返回连接到池的守卫。
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

/// 基于 Mio 的 TCP 连接池. 这是 C++ TcpConnectionPool 语义的简化端口: acquire 返回一个连接, 该连接在 Drop 时自动返回.
pub struct TcpConnectionPool<H: NetworkHandler> {
    handler: Arc<H>,
    max: usize,
    endpoint_manager: Arc<crate::net::endpoint::EndpointManager>,
    idle: Mutex<VecDeque<Connection>>,
    // 当前活跃（已检出）连接的数量
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

        // 预热：尝试创建 `min` 个连接并放入空闲队列。
        // 失败被忽略（启动时网络可能不可用）。
        if min > 0 {
            for _ in 0..min {
                if let Some(ep) = pool.endpoint_manager.acquire_endpoint() {
                    // 使用 handler 指定的超时（与 C++ 行为保持一致），确保 connect/read/write 超时一致
                    let timeout = pool.handler.timeout();
                    log::debug!(
                        "connection_pool: pre-warm trying to connect to {} (timeout {:?})",
                        ep,
                        timeout
                    );
                    match StdTcpStream::connect_timeout(&ep, timeout) {
                        Ok(std_stream) => {
                            let _ = std_stream.set_nodelay(true);
                            // 设置读/写超时以避免无限阻塞
                            let _ = std_stream.set_read_timeout(Some(timeout));
                            let _ = std_stream.set_write_timeout(Some(timeout));
                            // 转换为 mio::TcpStream
                            let mut stream = TcpStream::from_std(std_stream);
                            // 运行握手，但在预热期间忽略错误
                            match pool.handler.handshake(&mut stream) {
                                Ok(()) => {
                                    log::debug!(
                                        "connection_pool: pre-warm handshake ok for {}",
                                        ep
                                    );
                                    // 仅在推回空闲队列时加锁，以避免在执行网络操作时持有空闲互斥锁
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
                            // 如果连接失败，释放端点槽位
                            pool.endpoint_manager.release_endpoint(ep);
                        }
                    }
                } else {
                    break;
                }
            }
        }

        // 生成一个后台心跳线程，该线程定期通过调用处理器的 keepalive 来检查空闲连接。使用 Weak 引用，这样一旦池被丢弃，线程会退出。
        let weak_pool: Weak<TcpConnectionPool<H>> = Arc::downgrade(&pool);
        thread::spawn(move || {
            loop {
                // 尝试升级；如果池不存在，则退出线程。
                let pool_arc = match weak_pool.upgrade() {
                    Some(p) => p,
                    None => break,
                };

                // 根据处理器的首选间隔睡眠。如果池在睡眠期间被丢弃，则下次升级将失败并退出。
                let interval = pool_arc.handler.check_interval();
                thread::sleep(interval);

                // 在持有锁的同时快速排出空闲连接，然后在不持有互斥锁的情况下执行保活操作。
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

                // 对于每个排出的连接，运行保活。如果保活成功并返回 true，则连接仍然健康并返回到空闲队列；否则将被丢弃。
                let mut survivors: Vec<Connection> = Vec::with_capacity(drained.len());
                for mut conn in drained {
                    match pool_arc.handler.keepalive(conn.stream()) {
                        Ok(true) => {
                            conn.last_used = Instant::now();
                            survivors.push(conn);
                        }
                        _ => {
                            // 死连接或错误 => 释放端点槽位并丢弃
                            conn.close();
                            pool_arc.endpoint_manager.release_endpoint(conn.addr);
                        }
                    }
                }

                // 将幸存者推回空闲队列（遵守最大值限制）
                if !survivors.is_empty() {
                    let mut idle = pool_arc.idle.lock().unwrap();
                    for conn in survivors {
                        if idle.len() < pool_arc.max {
                            idle.push_back(conn);
                        } else {
                            // 池已满 — 丢弃多余的
                            break;
                        }
                    }
                }
            }
        });

        pool
    }

    /// 使用端点管理器获取连接（轮询 / 可用）。
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
        let connect_start = Instant::now();
        match StdTcpStream::connect_timeout(&endpoint, timeout) {
            Ok(mut std_stream) => {
                let connect_elapsed = connect_start.elapsed();
                log::debug!(
                    "connection_pool: connect to {} succeeded (elapsed {:?}), setting timeouts {:?}",
                    endpoint,
                    connect_elapsed,
                    timeout
                );

                let _ = std_stream.set_nodelay(true);
                let _ = std_stream.set_read_timeout(Some(timeout));
                let _ = std_stream.set_write_timeout(Some(timeout));

                // Perform blocking handshake on the std stream so that std read/write timeouts are honored.
                log::debug!(
                    "connection_pool: running blocking handshake for {}",
                    endpoint
                );
                let hs_start = Instant::now();
                match self.handler.handshake_std(&mut std_stream) {
                    Ok(()) => {
                        let hs_elapsed = hs_start.elapsed();
                        // convert to mio stream after successful blocking handshake
                        let stream = TcpStream::from_std(std_stream);
                        log::debug!(
                            "connection_pool: handshake succeeded for {} (handshake {:?})",
                            endpoint,
                            hs_elapsed
                        );
                        {
                            let mut active = self.active.lock().unwrap();
                            *active += 1;
                        }
                        let conn = Connection::new(stream, endpoint);
                        return Ok(PooledConnection {
                            pool: Arc::clone(self),
                            conn: Some(conn),
                        });
                    }
                    Err(e) => {
                        let hs_elapsed = hs_start.elapsed();
                        log::error!(
                            "connection_pool: handshake failed for {} (handshake {:?}): {} (kind={:?} raw_os={:?})",
                            endpoint,
                            hs_elapsed,
                            e,
                            e.kind(),
                            e.raw_os_error()
                        );
                        self.endpoint_manager.mark_failed(endpoint, cooldown);
                        self.endpoint_manager.release_endpoint(endpoint);
                        return Err(e);
                    }
                }
            }
            Err(e) => {
                let connect_elapsed = connect_start.elapsed();
                log::error!(
                    "connection_pool: connect failed for {} (elapsed {:?}): {} (kind={:?} raw_os={:?})",
                    endpoint,
                    connect_elapsed,
                    e,
                    e.kind(),
                    e.raw_os_error()
                );
                self.endpoint_manager.mark_failed(endpoint, cooldown);
                self.endpoint_manager.release_endpoint(endpoint);
                Err(e)
            }
        }
    }

    fn release(&self, mut conn: Connection) {
        conn.last_used = Instant::now();
        // 匹配 C++ release 行为：始终将连接返回到空闲队列，同时保留端点分配。端点仅在连接被显式关闭或被视为不健康时才被释放。
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

    /// 向管理器添加端点
    pub fn add_endpoint(&self, addr: SocketAddr, max_connections: usize) -> bool {
        self.endpoint_manager.add_endpoint(addr, max_connections)
    }

    pub fn get_endpoint_stats(&self, addr: SocketAddr) -> Option<(usize, usize)> {
        self.endpoint_manager.get_endpoint_stats(addr)
    }

    /// 返回此池的配置最大连接数。
    pub fn max_connections(&self) -> usize {
        let endpoint_count = self.endpoint_manager.get_all_endpoints().len();
        if endpoint_count == 0 {
            return self.max;
        }
        std::cmp::min(self.max, endpoint_count)
    }
}

/// RAII 守卫，在丢弃时将连接返回到池。
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
            // 如果没有连接，意味着连接创建失败，减少活跃数
            let mut active = self.pool.active.lock().unwrap();
            *active = active.saturating_sub(1);
            // 没有 Condvar 来通知；移除了等待语义以匹配 C++ 流程。
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
        // add_endpoint 需要一个具体的端口；这里不能添加 0，所以我们通过创建一个监听器来获取分配的端口来测试添加/删除语义。
        let listener = TcpListener::bind(("127.0.0.1", 0)).expect("bind");
        let addr = listener.local_addr().unwrap();
        assert!(mgr.add_endpoint(addr, 2));
        assert!(mgr.get_all_endpoints().contains(&addr));
        // 获取两次应该成功
        let a1 = mgr.acquire_endpoint();
        let a2 = mgr.acquire_endpoint();
        assert!(a1.is_some());
        assert!(a2.is_some());
        // 第三次应该失败，因为 max_connections == 2
        let a3 = mgr.acquire_endpoint();
        assert!(a3.is_none());
        // 释放一个并再次获取
        mgr.release_endpoint(a1.unwrap());
        let a4 = mgr.acquire_endpoint();
        assert!(a4.is_some());
        mgr.remove_endpoint(addr);
        assert!(!mgr.get_all_endpoints().contains(&addr));
        drop(listener);
    }

    #[test]
    #[ignore = "requires local client/server environment"]
    fn test_connection_pool_with_local_server() {
        // 启动一个本地 TCP 监听器来接受连接
        let listener = TcpListener::bind(("127.0.0.1", 0)).expect("bind");
        let addr = listener.local_addr().unwrap();

        // 保持接受的流存活，这样服务器端不会立即关闭
        let accepted: Arc<Mutex<Vec<StdTcpStream>>> = Arc::new(Mutex::new(Vec::new()));
        let accepted_clone = Arc::clone(&accepted);
        let (tx, rx) = std::sync::mpsc::channel::<()>();

        // 将 listener 设置为非阻塞并在后台循环短暂轮询以避免阻塞测试线程
        listener.set_nonblocking(true).expect("set_nonblocking");

        let server_thread = thread::spawn(move || {
            let start = Instant::now();
            // 在最多 1 秒内尝试接受若干连接，然后退出
            while start.elapsed() < Duration::from_secs(1) {
                match listener.accept() {
                    Ok((stream, _)) => {
                        accepted_clone.lock().unwrap().push(stream);
                        // notify main thread that we accepted one
                        let _ = tx.send(());
                    }
                    Err(ref e) if e.kind() == std::io::ErrorKind::WouldBlock => {
                        // 没有可用连接，稍作等待
                        thread::sleep(Duration::from_millis(10));
                    }
                    Err(_) => break,
                }
            }
        });

        let mgr = crate::net::endpoint::EndpointManager::new();
        assert!(mgr.add_endpoint(addr, 2));

        let handler = Arc::new(TestHandler {});
        let pool = TcpConnectionPool::new(1, 2, handler, Arc::new(mgr));

        // 获取一个连接
        let conn = pool.acquire().expect("acquire 1");
        assert_eq!(conn.addr(), addr);
        // 丢弃以返回到池
        drop(conn);

        // 获取到最大值
        let c1 = pool.acquire().expect("acquire c1");
        let c2 = pool.acquire().expect("acquire c2");

        // 第三次应该失败（max_connections == 2）
        let res = pool.acquire();
        assert!(res.is_err());

        drop(c1);
        drop(c2);

        // 等待服务器至少接受两次连接（有超时以避免 hang）
        for _ in 0..2 {
            assert!(
                rx.recv_timeout(Duration::from_secs(1)).is_ok(),
                "server did not accept connection in time"
            );
        }

        // 允许一些时间让心跳运行（check_interval 很小）
        thread::sleep(Duration::from_millis(200));

        // 清理接受的流，以便接受线程可以完成
        accepted.lock().unwrap().clear();

        // 等待服务器线程退出
        let _ = server_thread.join();
    }
}
