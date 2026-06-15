use mio::net::TcpStream;
use std::net::TcpStream as StdTcpStream;
use std::time::Duration;

/// 用户应该实现的连接池特征，用于执行协议特定的工作: 握手和保活检查.
pub trait NetworkOperationHandler: Send + Sync + 'static {
    /// 在阻塞的 std::net::TcpStream 上执行协议握手。
    fn handshake(&self, _stream: &mut StdTcpStream) -> std::io::Result<()> {
        Ok(())
    }
    /// 在 mio::net::TcpStream 上执行保活检测。
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
