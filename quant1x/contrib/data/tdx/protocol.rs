// -*- coding: utf-8 -*-
// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// TDX protocol — 协议头与消息基类
// 对应 Python quant1x/contrib/data/tdx/protocol.py

use crate::std::BinaryStream;
use flate2::read::ZlibDecoder;
use std::io::{Read, Write};

use super::command::{Command, FLAG_UNCOMPRESSED};
use super::helpers::msg_sequence_id;

// ============================================================
// 请求头 (12 字节)
// ============================================================

/// 请求头，对应 Python `RequestHeader`
///
/// 布局 (小端): zip_flag(u8) + sequence_id(u32) + packet_type(u8) + body_wire_len(u16) + body_raw_len(u16) + command(u16) = 12 字节
#[derive(Debug, Clone)]
pub struct RequestHeader {
    pub zip_flag: u8,
    pub sequence_id: u32,
    pub packet_type: u8,
    pub body_wire_len: u16,
    pub body_raw_len: u16,
    pub command: Command,
}

impl RequestHeader {
    /// 创建请求头，自动分配 sequence_id
    pub fn new(cmd: Command, flags: u8) -> Self {
        Self {
            zip_flag: flags,
            sequence_id: msg_sequence_id(),
            packet_type: 0x01,
            body_wire_len: 0,
            body_raw_len: 0,
            command: cmd,
        }
    }

    /// 固定 12 字节
    pub fn byte_size(&self) -> usize {
        12
    }

    /// 序列化为小端字节数组
    pub fn serialize(&self) -> Vec<u8> {
        let mut bs = BinaryStream::new();
        bs.push_u8(self.zip_flag);
        bs.push_u32(self.sequence_id);
        bs.push_u8(self.packet_type);
        bs.push_u16(self.body_wire_len);
        bs.push_u16(self.body_raw_len);
        bs.push_u16(self.command.value);
        bs.data().clone()
    }

    pub fn to_string(&self) -> String {
        format!(
            "RequestHeader(zip_flag: {}, sequence_id: {}, packet_type: {}, body_wire_len: {}, body_raw_len: {}, command: {:?})",
            self.zip_flag, self.sequence_id, self.packet_type, self.body_wire_len, self.body_raw_len, self.command
        )
    }
}

// ============================================================
// 响应头 (16 字节)
// ============================================================

/// 响应头，对应 Python `ResponseHeader`
///
/// 布局 (小端): magic_number(u32) + zip_flag(u8) + sequence_id(u32) + packet_type(u8) + command(u16) + body_wire_len(u16) + body_raw_len(u16) = 16 字节
#[derive(Debug, Clone)]
pub struct ResponseHeader {
    pub magic_number: u32,
    pub zip_flag: u8,
    pub sequence_id: u32,
    pub packet_type: u8,
    pub command: Command,
    pub body_wire_len: u16,
    pub body_raw_len: u16,
}

impl ResponseHeader {
    pub fn new() -> Self {
        Self {
            magic_number: 0,
            zip_flag: 0,
            sequence_id: 0,
            packet_type: 0,
            command: super::command::CMD_UNKNOWN,
            body_wire_len: 0,
            body_raw_len: 0,
        }
    }

    /// 固定 16 字节
    pub fn byte_size(&self) -> usize {
        16
    }

    /// 从 16 字节数据反序列化
    pub fn deserialize(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        let mut bs = BinaryStream::from_vec(data.to_vec());
        self.magic_number = bs.get_u32()?;
        self.zip_flag = bs.get_u8()?;
        self.sequence_id = bs.get_u32()?;
        self.packet_type = bs.get_u8()?;
        let cmd_value = bs.get_u16()?;
        self.body_wire_len = bs.get_u16()?;
        self.body_raw_len = bs.get_u16()?;

        // 尝试查找已知命令，未知则用 UNKNOWN
        self.command = Command::from_value(cmd_value)
            .copied()
            .unwrap_or_else(|| {
                log::warn!("未知的命令值 0x{:04x}", cmd_value);
                super::command::CMD_UNKNOWN
            });

        Ok(())
    }

    pub fn to_string(&self) -> String {
        format!(
            "ResponseHeader(magic_number: {}, zip_flag: {}, sequence_id: {}, packet_type: {}, command: {:?}, body_wire_len: {}, body_raw_len: {})",
            self.magic_number, self.zip_flag, self.sequence_id, self.packet_type, self.command, self.body_wire_len, self.body_raw_len
        )
    }
}

// ============================================================
// 消息基类 — BaseMessage
// ============================================================

/// 消息基类，对应 Python `BaseMessage`
///
/// 包含请求头和响应头，子类需实现 `serialize_request_body` 和 `deserialize_response_body`。
pub trait BaseMessage {
    fn request_header(&self) -> &RequestHeader;
    fn request_header_mut(&mut self) -> &mut RequestHeader;
    fn response_header(&self) -> &ResponseHeader;
    fn response_header_mut(&mut self) -> &mut ResponseHeader;

    /// 序列化请求体
    fn serialize_request_body(&mut self) -> Vec<u8>;

    /// 反序列化响应体
    fn deserialize_response_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError>;

    /// 完整序列化请求（头 + 体）
    fn serialize_request(&mut self) -> Vec<u8> {
        let body = self.serialize_request_body();
        let header = self.request_header_mut();
        header.body_wire_len = 2 + body.len() as u16;
        header.body_raw_len = header.body_wire_len;

        let mut buf = header.serialize();
        buf.extend_from_slice(&body);
        buf
    }

    /// 反序列化响应头
    fn deserialize_response_header(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        self.response_header_mut().deserialize(data)
    }

    /// 获取响应压缩体大小
    fn response_zip_size(&self) -> usize {
        self.response_header().body_wire_len as usize
    }

    /// 获取响应解压后大小
    fn response_unzip_size(&self) -> usize {
        self.response_header().body_raw_len as usize
    }
}

// ============================================================
// 底层 I/O — recv_exact
// ============================================================

/// 从实现了 `Read` 的对象读取恰好 `n` 字节
///
/// 对应 Python `_recv_exact`
///
/// 兼容阻塞和非阻塞 socket：遇到 WouldBlock 时短暂等待后重试。
pub fn recv_exact<R: Read>(reader: &mut R, n: usize) -> std::io::Result<Vec<u8>> {
    let mut buf = vec![0u8; n];
    let mut offset = 0;
    while offset < n {
        match reader.read(&mut buf[offset..]) {
            Ok(0) => {
                return Err(std::io::Error::new(
                    std::io::ErrorKind::UnexpectedEof,
                    "connection closed",
                ));
            }
            Ok(nread) => {
                offset += nread;
            }
            Err(ref e) if e.kind() == std::io::ErrorKind::WouldBlock => {
                std::thread::sleep(std::time::Duration::from_millis(10));
                continue;
            }
            Err(e) => return Err(e),
        }
    }
    Ok(buf)
}

// ============================================================
// zlib 解压
// ============================================================

/// 解压 zlib 编码的数据
fn unzip(body: Vec<u8>, unzipped_size: usize) -> std::io::Result<Vec<u8>> {
    let mut decoder = ZlibDecoder::new(&body[..]);
    let mut out = Vec::with_capacity(unzipped_size);
    decoder.read_to_end(&mut out)?;
    Ok(out)
}

// ============================================================
// process_level1 — 泛型协议处理
// ============================================================

/// 泛型 process 函数，对应 Python `process_level1_new`
///
/// 发送请求、读取响应头、按需解压响应体、解析响应体。
pub fn process_level1<M: BaseMessage, R: Read>(
    reader: &mut R,
    writer: &mut dyn std::io::Write,
    msg: &mut M,
) -> Result<(), crate::std::DeserializeError> {
    process_level1_impl(reader, writer, msg)
}

/// 对单一 Read+Write stream 执行 process_level1
/// 使用阻塞 std::net::TcpStream，已设置读写超时
pub fn process_level1_stream<M: BaseMessage, T: Read + Write>(
    stream: &mut T,
    msg: &mut M,
) -> Result<(), crate::std::DeserializeError> {
    let cmd_value = msg.request_header().command.value;
    let cmd_desc = msg.request_header().command.desc;

    // 1. 发送请求
    let req_buf = msg.serialize_request();
    log::debug!("process_level1_stream: request={}", msg.request_header().to_string());
    log::debug!("process_level1_stream: req_buf hex={}", hex::encode(&req_buf));
    stream
        .write_all(&req_buf)
        .map_err(|e| crate::std::DeserializeError::Other(e.to_string()))?;

    // 2. 读取 16 字节响应头
    let resp_header_len = msg.response_header().byte_size();
    let resp_header_bytes = recv_exact(stream, resp_header_len)
        .map_err(|e| crate::std::DeserializeError::Other(e.to_string()))?;

    // 3. 反序列化响应头
    msg.deserialize_response_header(&resp_header_bytes)?;

    // 4. 如果没有 body，直接返回
    if msg.response_zip_size() == 0 {
        return Ok(());
    }

    log::debug!("process_level1_stream: response_header={}", msg.response_header().to_string());

    // 5. 读取响应体
    let resp_body_bytes = recv_exact(stream, msg.response_zip_size())
        .map_err(|e| crate::std::DeserializeError::Other(e.to_string()))?;

    // 6. 如果压缩长度 != 解压长度，zlib 解压
    let final_body = if msg.response_zip_size() != msg.response_unzip_size() {
        unzip(resp_body_bytes, msg.response_unzip_size())
            .map_err(|e| crate::std::DeserializeError::Other(e.to_string()))?
    } else {
        resp_body_bytes
    };

    // 7. 反序列化响应体
    msg.deserialize_response_body(&final_body).map_err(|e| {
        crate::std::DeserializeError::Other(format!(
            "response body deserialize error for {} (0x{:04x}): {}",
            cmd_desc, cmd_value, e
        ))
    })?;

    Ok(())
}

fn process_level1_impl<M: BaseMessage, R: Read>(
    reader: &mut R,
    writer: &mut dyn std::io::Write,
    msg: &mut M,
) -> Result<(), crate::std::DeserializeError> {
    let cmd_value = msg.request_header().command.value;
    let cmd_desc = msg.request_header().command.desc;

    // 1. 发送请求
    let req_buf = msg.serialize_request();
    log::debug!("process_level1: request={}", msg.request_header().to_string());
    log::debug!("process_level1: req_buf hex={}", hex::encode(&req_buf));
    writer
        .write_all(&req_buf)
        .map_err(|e| crate::std::DeserializeError::Other(e.to_string()))?;

    // 2. 读取 16 字节响应头
    let resp_header_len = msg.response_header().byte_size();
    log::debug!("process_level1: response_header.byte_size={}", resp_header_len);
    let resp_header_bytes = recv_exact(reader, resp_header_len)
        .map_err(|e| crate::std::DeserializeError::Other(e.to_string()))?;

    // 3. 反序列化响应头
    msg.deserialize_response_header(&resp_header_bytes)?;

    // 4. 如果没有 body，直接返回
    if msg.response_zip_size() == 0 {
        return Ok(());
    }

    log::debug!("process_level1: response_header={}", msg.response_header().to_string());

    // 5. 读取响应体
    let resp_body_bytes = recv_exact(reader, msg.response_zip_size())
        .map_err(|e| crate::std::DeserializeError::Other(e.to_string()))?;

    // 6. 如果压缩长度 != 解压长度，zlib 解压
    let final_body = if msg.response_zip_size() != msg.response_unzip_size() {
        unzip(resp_body_bytes, msg.response_unzip_size())
            .map_err(|e| crate::std::DeserializeError::Other(e.to_string()))?
    } else {
        resp_body_bytes
    };

    // 7. 反序列化响应体
    msg.deserialize_response_body(&final_body).map_err(|e| {
        crate::std::DeserializeError::Other(format!(
            "response body deserialize error for {} (0x{:04x}): {}",
            cmd_desc, cmd_value, e
        ))
    })?;

    Ok(())
}

// ============================================================
// 测试
// ============================================================

// ============================================================
// StandardProtocolHandler — 标准行情连接处理器
// 对应 Python protocol.py StandardProtocolHandler
// ============================================================

use std::net::TcpStream as StdTcpStream;
use std::time::Duration;

use crate::io::operation_handler::NetworkOperationHandler;

use super::level1::std::hello::{Hello1Request, Hello2Request};
use super::level1::std::heartbeat::HeartbeatRequest;

pub struct StandardProtocolHandler;

impl NetworkOperationHandler for StandardProtocolHandler {
    fn handshake(&self, stream: &mut StdTcpStream) -> std::io::Result<()> {
        // Hello1
        let mut req1 = Hello1Request::new();
        match process_level1_stream(stream, &mut req1) {
            Ok(()) => {
                if req1.info.trim().is_empty() {
                    log::error!("StandardProtocolHandler::handshake Hello1 validation failed: empty info");
                    return Err(std::io::Error::new(std::io::ErrorKind::Other, "Hello1 response invalid or empty"));
                }
            }
            Err(e) => {
                log::error!("StandardProtocolHandler::handshake Hello1 failed: {}", e);
                return Err(std::io::Error::new(std::io::ErrorKind::Other, e.to_string()));
            }
        }

        // Hello2
        let mut req2 = Hello2Request::new();
        match process_level1_stream(stream, &mut req2) {
            Ok(()) => {
                if req2.info.trim().is_empty() {
                    log::error!("StandardProtocolHandler::handshake Hello2 validation failed: empty info");
                    return Err(std::io::Error::new(std::io::ErrorKind::Other, "Hello2 response invalid or empty"));
                }
            }
            Err(e) => {
                log::error!("StandardProtocolHandler::handshake Hello2 failed: {}", e);
                return Err(std::io::Error::new(std::io::ErrorKind::Other, e.to_string()));
            }
        }

        Ok(())
    }

    fn keepalive(&self, stream: &mut mio::net::TcpStream) -> std::io::Result<bool> {
        let mut req = HeartbeatRequest::new();
        match process_level1_stream(stream, &mut req) {
            Ok(()) => Ok(true),
            Err(_) => Ok(false),
        }
    }

    fn timeout(&self) -> Duration {
        Duration::from_secs(10)
    }

    fn check_interval(&self) -> Duration {
        Duration::from_secs(5)
    }
}

// ============================================================
// ExtensionProtocolHandler — 扩展行情连接处理器
// 对应 Python protocol.py ExtensionProtocolHandler
// ============================================================

use super::level1::ext::{ExtSynchronizeRequest, InstrumentCountRequest};

pub struct ExtensionProtocolHandler;

impl NetworkOperationHandler for ExtensionProtocolHandler {
    fn handshake(&self, stream: &mut StdTcpStream) -> std::io::Result<()> {
        let mut req = ExtSynchronizeRequest::new();
        match process_level1_stream(stream, &mut req) {
            Ok(()) if req.success => Ok(()),
            Ok(()) => Err(std::io::Error::new(
                std::io::ErrorKind::Other,
                "ExtensionProtocolHandler: synchronize failed (success=false)",
            )),
            Err(e) => Err(std::io::Error::new(std::io::ErrorKind::Other, e.to_string())),
        }
    }

    fn keepalive(&self, stream: &mut mio::net::TcpStream) -> std::io::Result<bool> {
        let mut req = InstrumentCountRequest::new();
        match process_level1_stream(stream, &mut req) {
            Ok(()) => Ok(req.count > 0),
            Err(_) => Ok(false),
        }
    }

    fn timeout(&self) -> Duration {
        Duration::from_secs(10)
    }

    fn check_interval(&self) -> Duration {
        Duration::from_secs(5)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use super::super::command::{STD_HEARTBEAT, FLAG_UNCOMPRESSED};

    #[test]
    fn test_request_header_byte_size() {
        let hdr = RequestHeader::new(STD_HEARTBEAT, FLAG_UNCOMPRESSED);
        assert_eq!(hdr.byte_size(), 12);
    }

    #[test]
    fn test_request_header_serialize() {
        let hdr = RequestHeader::new(STD_HEARTBEAT, FLAG_UNCOMPRESSED);
        let data = hdr.serialize();
        assert_eq!(data.len(), 12);
        // zip_flag
        assert_eq!(data[0], FLAG_UNCOMPRESSED);
        // packet_type
        assert_eq!(data[5], 0x01);
        // command value (little-endian)
        let cmd_bytes = &data[10..12];
        assert_eq!(u16::from_le_bytes([cmd_bytes[0], cmd_bytes[1]]), STD_HEARTBEAT.value);
    }

    #[test]
    fn test_response_header_deserialize() {
        // 构造一个简单的响应头: magic=0x11223344, zip_flag=0x0C, seq_id=42, pkt_type=0, cmd=0x0004, wire=100, raw=200
        let mut buf = Vec::new();
        buf.extend_from_slice(&0x11223344u32.to_le_bytes());  // magic_number
        buf.push(0x0C);                                        // zip_flag
        buf.extend_from_slice(&42u32.to_le_bytes());           // sequence_id
        buf.push(0x00);                                        // packet_type
        buf.extend_from_slice(&0x0004u16.to_le_bytes());       // command = STD_HEARTBEAT
        buf.extend_from_slice(&100u16.to_le_bytes());          // body_wire_len
        buf.extend_from_slice(&200u16.to_le_bytes());          // body_raw_len

        let mut hdr = ResponseHeader::new();
        hdr.deserialize(&buf).unwrap();

        assert_eq!(hdr.magic_number, 0x11223344);
        assert_eq!(hdr.zip_flag, 0x0C);
        assert_eq!(hdr.sequence_id, 42);
        assert_eq!(hdr.command.value, 0x0004);
        assert_eq!(hdr.body_wire_len, 100);
        assert_eq!(hdr.body_raw_len, 200);
    }

    #[test]
    fn test_response_header_unknown_command() {
        let mut buf = Vec::new();
        buf.extend_from_slice(&0u32.to_le_bytes());
        buf.push(0);
        buf.extend_from_slice(&0u32.to_le_bytes());
        buf.push(0);
        buf.extend_from_slice(&0xFFFFu16.to_le_bytes());  // unknown command
        buf.extend_from_slice(&0u16.to_le_bytes());
        buf.extend_from_slice(&0u16.to_le_bytes());

        let mut hdr = ResponseHeader::new();
        hdr.deserialize(&buf).unwrap();
        assert_eq!(hdr.command.value, super::super::command::CMD_UNKNOWN.value);
    }
}
