use crate::std::BinaryStream;
use mio::net::TcpStream as MioTcpStream;
use std::io::Read;

// 定义Request trait，所有请求类型都需要实现此trait
pub trait Request {
    fn encode(&mut self, payload: &[u8]) -> Vec<u8>;
}

// 定义Response trait，所有响应类型都需要实现此trait
pub trait Response {
    fn decode(&mut self, data: &[u8]);
}

// 泛型process函数，匹配C++ protocol.h中的process模板函数
pub fn process<R: Request, S: Response>(
    stream: &mut MioTcpStream,
    request: &mut R,
    payload: &[u8],
    response: &mut S,
) -> std::io::Result<()> {
    log::info!(
        "Protocol process called with payload size: {}",
        payload.len()
    );

    // 编码请求
    let req_bytes = request.encode(payload);
    log::info!("Encoded request to {} bytes", req_bytes.len());

    // 使用现有的process_request处理网络通信
    let body = crate::level1::process_request(stream, &req_bytes)?;
    log::info!("Received response body of {} bytes", body.len());

    // 解码响应
    response.decode(&body);
    log::info!("Decoded response successfully");

    Ok(())
}
