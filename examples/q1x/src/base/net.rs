//use std::fmt::Debug;
use std::net::TcpStream;
use std::time::Duration;

const TCP_TIMEOUT: Duration = Duration::new(10, 0);

/// 同步方式连接服务器
pub fn connect(addr: String) -> bool {
    let sa: std::net::SocketAddr = addr.parse().unwrap();
    let client = TcpStream::connect_timeout(&sa, TCP_TIMEOUT);
    if let Ok(stream) =  client{
        _ = stream;
        println!("Connected to the server!");
    } else {
        println!("Couldn't connect to server...");
    }
    true
}

//
// #[cfg(test)]
// mod api_tests {
//     use super::*;
//
//     #[test]
//     fn test_tdx_server_connect() {
//         let addr = "127.0.0.1:8080";
//         let result = connect(addr.to_string());
//         assert_eq!(result,false)
//     }
// }