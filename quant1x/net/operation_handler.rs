use mio::net::TcpStream;
use std::net::TcpStream as StdTcpStream;
use std::time::Duration;

/// 用户应该实现的连接池特征，用于执行协议特定的工作: 握手和保活检查.
pub trait NetworkHandler: Send + Sync + 'static {
    fn handshake(&self, _stream: &mut TcpStream) -> std::io::Result<()> {
        Ok(())
    }
    /// 可选的阻塞握手，用于在阻塞的 `std::net::TcpStream` 上执行握手。
    /// 默认实现会将其转换为 `mio::TcpStream` 并调用 `handshake`.
    fn handshake_std(&self, stream: &mut StdTcpStream) -> std::io::Result<()> {
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
