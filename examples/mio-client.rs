use bytes::{Buf, BytesMut};
use mio::net::TcpStream;
use mio::{Events, Interest, Poll, Token};
use std::collections::HashMap;
use std::io::{self, Read, Write};
use std::time::{Duration, Instant};

const SERVER_ADDR: &str = "127.0.0.1:7878";
const MAX_CONNECTIONS: usize = 10;
const HEARTBEAT_INTERVAL: Duration = Duration::from_secs(5);
const RECONNECT_INTERVAL: Duration = Duration::from_secs(3);
const MAX_RETRIES: u32 = 5;
const BUFFER_SIZE: usize = 8 * 1024;

/// 网络连接状态
enum _ConnectState {
    /// 未连接
    StateUnconnected,
    /// 正在连接
    StateConnecting,
    /// 已连接
    StateConnected,
    /// 在读
    StateRead,
}

struct Connection {
    stream: Option<TcpStream>,
    write_buffer: BytesMut,
    read_buffer: BytesMut,
    last_activity: Instant,
    token: Token,
    retries: u32,
    connecting: bool,
    retry_timer: Option<Instant>,
    is_initial: bool,
    on_data: Option<Box<dyn FnMut(&mut BytesMut) -> io::Result<usize> + Send>>,
}

impl Connection {
    fn new(token: Token) -> Self {
        Connection {
            stream: None,
            write_buffer: BytesMut::with_capacity(BUFFER_SIZE),
            read_buffer: BytesMut::with_capacity(BUFFER_SIZE),
            last_activity: Instant::now(),
            token,
            retries: 0,
            connecting: false,
            retry_timer: None,
            is_initial: true,
            on_data: None,
        }
    }

    fn set_data_callback<F>(&mut self, callback: F)
    where
        F: FnMut(&mut BytesMut) -> io::Result<usize> + Send + 'static,
    {
        self.on_data = Some(Box::new(callback));
    }

    fn connect(&mut self) -> io::Result<()> {
        let stream = TcpStream::connect(SERVER_ADDR.parse().unwrap())?;
        self.stream = Some(stream);
        self.connecting = true;
        self.last_activity = Instant::now();
        self.retry_timer = None;
        Ok(())
    }

    fn send_heartbeat(&mut self) -> io::Result<()> {
        if let Some(_stream) = self.stream.as_mut() {
            if !self.connecting {
                // 直接发送原始心跳数据（不带长度前缀）
                self.write_buffer.extend_from_slice(b"heartbeat");
                self.last_activity = Instant::now();
            }
        }
        Ok(())
    }

    fn check_connect_status(&mut self, poll: &Poll) -> io::Result<bool> {
        if let Some(stream) = self.stream.as_mut() {
            match stream.take_error()? {
                Some(err) => {
                    self.stream = None;
                    self.connecting = false;
                    Err(io::Error::new(io::ErrorKind::Other, err))
                }
                None if self.connecting => {
                    poll.registry().reregister(
                        stream,
                        self.token,
                        Interest::READABLE | Interest::WRITABLE,
                    )?;
                    self.connecting = false;
                    Ok(true)
                }
                _ => Ok(false),
            }
        } else {
            Ok(false)
        }
    }
}

fn main() -> io::Result<()> {
    let mut poll = Poll::new()?;
    let mut events = Events::with_capacity(1024);
    let mut connections = HashMap::new();
    let mut next_token = Token(0);
    let mut last_check = Instant::now();

    for _ in 0..MAX_CONNECTIONS {
        let token = Token(next_token.0);
        next_token.0 += 1;

        let mut conn = Connection::new(token);
        // 显式标注闭包参数类型
        conn.set_data_callback(Box::new(|buf: &mut BytesMut| {
            println!("[business] Received raw data: {:?}", buf);
            Ok(buf.len()) // 消费全部数据
        }));

        match conn.connect() {
            Ok(_) => {
                poll.registry().register(
                    conn.stream.as_mut().unwrap(),
                    token,
                    Interest::WRITABLE,
                )?;
                connections.insert(token, conn);
                println!("[{}] Initial connection established", token.0);
            }
            Err(e) => {
                eprintln!("[{}] Initial connect failed: {}", token.0, e);
                conn.retry_timer = Some(Instant::now() + RECONNECT_INTERVAL);
                connections.insert(token, conn);
            }
        }
    }

    loop {
        poll.poll(&mut events, Some(HEARTBEAT_INTERVAL))?;

        for event in events.iter() {
            let token = event.token();

            if let Some(mut conn) = connections.remove(&token) {
                let mut is_closed = false;
                let mut reconnect_status = None;

                // 处理连接状态检查
                if conn.connecting {
                    match conn.check_connect_status(&poll) {
                        Ok(true) => reconnect_status = Some(conn.is_initial),
                        Ok(false) => {}
                        Err(e) => {
                            eprintln!("[{}] Connection failed: {}", token.0, e);
                            is_closed = true;
                        }
                    }
                }

                // 读处理
                if let Some(stream) = conn.stream.as_mut() {
                    if !conn.connecting && !is_closed && event.is_readable() {
                        let mut buf = [0; BUFFER_SIZE];
                        match stream.read(&mut buf) {
                            Ok(0) => is_closed = true,
                            Ok(n) => {
                                conn.read_buffer.extend_from_slice(&buf[..n]);
                                conn.last_activity = Instant::now();

                                // 调用业务回调
                                if let Some(callback) = conn.on_data.as_mut() {
                                    match callback(&mut conn.read_buffer) {
                                        Ok(consumed) => conn.read_buffer.advance(consumed),
                                        Err(e) => {
                                            eprintln!("[{}] Data processing error: {}", token.0, e);
                                            is_closed = true;
                                        }
                                    }
                                }
                            }
                            Err(e) if e.kind() == io::ErrorKind::WouldBlock => {}
                            Err(_) => is_closed = true,
                        }
                    }
                }

                // 写处理
                if let Some(stream) = conn.stream.as_mut() {
                    if !conn.connecting && !is_closed && event.is_writable() {
                        if !conn.write_buffer.is_empty() {
                            match stream.write(conn.write_buffer.chunk()) {
                                Ok(n) => {
                                    conn.write_buffer.advance(n);
                                    if conn.write_buffer.is_empty() {
                                        conn.write_buffer.clear();
                                    }
                                }
                                Err(e) if e.kind() == io::ErrorKind::WouldBlock => {}
                                Err(_) => is_closed = true,
                            }
                        }
                    }
                }

                // 处理连接状态变化
                if let Some(is_initial) = reconnect_status {
                    if is_initial {
                        println!("[{}] Connected successfully", token.0);
                    } else {
                        println!("[{}] Reconnected successfully", token.0);
                    }
                }

                // 处理连接关闭
                if is_closed {
                    println!(
                        "[{}] Connection lost, retry count: {}",
                        token.0, conn.retries
                    );
                    conn.stream = None;
                    conn.connecting = false;
                    conn.retries = conn.retries.saturating_add(1);
                    conn.retry_timer = Some(Instant::now() + RECONNECT_INTERVAL);
                    conn.is_initial = false;
                    if conn.retries <= MAX_RETRIES {
                        connections.insert(token, conn);
                    } else {
                        println!("[{}] Max retries reached, giving up", token.0);
                    }
                } else {
                    connections.insert(token, conn);
                }
            }
        }

        // 处理重连
        let now = Instant::now();
        for token in connections.keys().cloned().collect::<Vec<_>>() {
            if let Some(mut conn) = connections.remove(&token) {
                if conn.stream.is_none()
                    && conn.retries <= MAX_RETRIES
                    && conn.retry_timer.map(|t| now >= t).unwrap_or(false)
                {
                    println!("[{}] Attempting reconnect...", token.0);
                    match conn.connect() {
                        Ok(_) => {
                            poll.registry().register(
                                conn.stream.as_mut().unwrap(),
                                token,
                                Interest::WRITABLE,
                            )?;
                            connections.insert(token, conn);
                        }
                        Err(e) => {
                            eprintln!("[{}] Reconnect failed: {}", token.0, e);
                            conn.retry_timer = Some(now + RECONNECT_INTERVAL);
                            connections.insert(token, conn);
                        }
                    }
                } else {
                    connections.insert(token, conn);
                }
            }
        }

        // 处理心跳
        if last_check.elapsed() >= HEARTBEAT_INTERVAL {
            for token in connections.keys().cloned().collect::<Vec<_>>() {
                if let Some(mut conn) = connections.remove(&token) {
                    if conn.stream.is_some() && !conn.connecting {
                        if now.duration_since(conn.last_activity) >= HEARTBEAT_INTERVAL {
                            println!("[{}] Sending heartbeat", token.0);
                            if let Err(e) = conn.send_heartbeat() {
                                eprintln!("[{}] Heartbeat failed: {}", token.0, e);
                            }
                        }
                        poll.registry().reregister(
                            conn.stream.as_mut().unwrap(),
                            token,
                            Interest::READABLE | Interest::WRITABLE,
                        )?;
                    }
                    connections.insert(token, conn);
                }
            }
            last_check = now;
        }
    }
}
