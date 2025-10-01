use indicatif::{MultiProgress, ProgressBar, ProgressStyle};
use mio::net::{TcpListener, TcpStream};
use mio::{Events, Interest, Poll, Token};
use std::collections::HashMap;
use std::io::{self, Read, Write};

const SERVER_TOKEN: Token = Token(0);
const BUFFER_SIZE: usize = 1024;

struct Connection {
    stream: Box<TcpStream>,
    write_buffer: Vec<u8>,
    progress_bar: ProgressBar,
}

impl Connection {
    fn new(stream: TcpStream, progress_bar: ProgressBar) -> Self {
        Connection {
            stream: Box::new(stream),
            write_buffer: Vec::new(),
            progress_bar,
        }
    }
}

fn main() -> io::Result<()> {
    let mut poll = Poll::new()?;
    let mut events = Events::with_capacity(1024);

    let addr = "0.0.0.0:7878".parse().unwrap();
    let mut listener = TcpListener::bind(addr)?;
    poll.registry().register(&mut listener, SERVER_TOKEN, Interest::READABLE)?;

    let mut connections = HashMap::new();
    let mut next_token = Token(1);

    // 初始化多进度条管理器
    let mp = MultiProgress::new();
    mp.set_draw_target(indicatif::ProgressDrawTarget::stderr());

    loop {
        poll.poll(&mut events, None)?;

        for event in events.iter() {
            match event.token() {
                SERVER_TOKEN => loop {
                    match listener.accept() {
                        Ok((stream, _)) => {
                            let token = next_token;
                            next_token.0 += 1;

                            // 创建新的进度条
                            let pb = mp.add(ProgressBar::new(0));
                            pb.set_style(ProgressStyle::default_bar()
                                .template("{msg} {spinner:.green} [{elapsed_precise}] [{wide_bar:.cyan/blue}] {bytes} @ {bytes_per_sec}")
                                .unwrap()
                                .progress_chars("#>-"));
                            pb.set_message(format!("Client {}", token.0));

                            let mut connection = Connection::new(stream, pb);

                            poll.registry()
                                .register(connection.stream.as_mut(), token, Interest::READABLE)
                                .unwrap();

                            connections.insert(token, connection);
                        }
                        Err(e) if e.kind() == io::ErrorKind::WouldBlock => break,
                        Err(e) => return Err(e),
                    }
                },
                token => {
                    if let Some(connection) = connections.get_mut(&token) {
                        let mut is_closed = false;

                        if event.is_readable() {
                            loop {
                                let mut buf = [0; BUFFER_SIZE];
                                match connection.stream.read(&mut buf) {
                                    Ok(0) => {
                                        is_closed = true;
                                        break;
                                    }
                                    Ok(n) => {
                                        connection.write_buffer.extend(&buf[..n]);
                                        connection.progress_bar.inc(n as u64);
                                    }
                                    Err(e) if e.kind() == io::ErrorKind::WouldBlock => break,
                                    Err(_) => {
                                        is_closed = true;
                                        break;
                                    }
                                }
                            }

                            if !is_closed {
                                if let Err(e) = poll.registry().reregister(
                                    connection.stream.as_mut(),
                                    token,
                                    Interest::WRITABLE,
                                ) {
                                    eprintln!("Reregister error: {}", e);
                                    is_closed = true;
                                }
                            }
                        }

                        if event.is_writable() && !connection.write_buffer.is_empty() {
                            match connection.stream.write(&connection.write_buffer) {
                                Ok(n) if n == connection.write_buffer.len() => {
                                    connection.progress_bar.inc(n as u64);
                                    connection.write_buffer.clear();
                                    if let Err(e) = poll.registry().reregister(
                                        connection.stream.as_mut(),
                                        token,
                                        Interest::READABLE,
                                    ) {
                                        eprintln!("Reregister error: {}", e);
                                        is_closed = true;
                                    }
                                }
                                Ok(n) => {
                                    connection.progress_bar.inc(n as u64);
                                    connection.write_buffer.drain(..n);
                                    if let Err(e) = poll.registry().reregister(
                                        connection.stream.as_mut(),
                                        token,
                                        Interest::WRITABLE,
                                    ) {
                                        eprintln!("Reregister error: {}", e);
                                        is_closed = true;
                                    }
                                }
                                Err(e) if e.kind() == io::ErrorKind::WouldBlock => {}
                                Err(_) => is_closed = true,
                            }
                        }

                        if is_closed {
                            // 关键修改：使用 finish_and_clear 清除进度条
                            connection.progress_bar.finish_and_clear();
                            poll.registry()
                                .deregister(connection.stream.as_mut())?;
                            connections.remove(&token);
                        }
                    }
                }
            }
        }
    }
}