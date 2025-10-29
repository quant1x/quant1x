use mio::net::TcpStream as MioTcpStream;

use crate::std::BinaryStream;
use flate2::read::ZlibDecoder;
use std::io::{Read, Write};

const RESPONSE_HEADER_LEN: usize = 16;

pub mod commands {
    #![allow(non_upper_case_globals)]

    pub const HEARTBEAT: u16 = 0x0004;
    pub const LOGIN1: u16 = 0x000d;
    pub const LOGIN2: u16 = 0x0fdb;
    pub const XDXR_INFO: u16 = 0x000f;
    pub const FINANCE_INFO: u16 = 0x0010;
    pub const PING: u16 = 0x0015;
    pub const COMPANY_CATEGORY: u16 = 0x02cf;
    pub const COMPANY_CONTENT: u16 = 0x02d0;
    pub const SECURITY_COUNT: u16 = 0x044e;
    pub const SECURITY_LIST: u16 = 0x044d;
    pub const OLD_SECURITY_LIST: u16 = 0x0450;
    pub const INDEX_BARS: u16 = 0x052d;
    pub const SECURITY_BARS: u16 = 0x052d; // same numeric value as INDEX_BARS in C++
    pub const SECURITY_QUOTES_OLD: u16 = 0x053e;
    pub const SECURITY_QUOTES_NEW: u16 = 0x054c;
    pub const MINUTE_TIME_DATA: u16 = 0x051d;
    pub const BLOCK_META: u16 = 0x02c5;
    pub const BLOCK_DATA: u16 = 0x06b9;
    pub const TRANSACTION_DATA: u16 = 0x0fc5;
    pub const HISTORY_MINUTE_DATA: u16 = 0x0fb4;
    pub const HISTORY_TRANSACTION_DATA: u16 = 0x0fb5;
}

#[derive(Debug, Clone)]
pub struct RequestHeader {
    pub zip_flag: u8,
    pub seq_id: u32,
    pub packet_type: u8,
    pub pkg_len1: u16,
    pub pkg_len2: u16,
    pub method: u16,
}

impl RequestHeader {
    pub fn new() -> Self {
        Self {
            zip_flag: 0,
            seq_id: 0,
            packet_type: 0,
            pkg_len1: 0,
            pkg_len2: 0,
            method: 0,
        }
    }

    fn command(&self) -> &'static str {
        command_to_string(self.method)
    }

    fn serialize(&mut self, payload: &[u8]) -> Vec<u8> {
        self.pkg_len1 = 2u16 + payload.len() as u16;
        self.pkg_len2 = self.pkg_len1;

        let mut header_stream = BinaryStream::new();
        header_stream.push_u8(self.zip_flag);
        header_stream.push_u32(self.seq_id);
        header_stream.push_u8(self.packet_type);
        header_stream.push_u16(self.pkg_len1);
        header_stream.push_u16(self.pkg_len2);
        header_stream.push_u16(self.method);

        let mut buffer = header_stream.data().clone();
        buffer.extend_from_slice(payload);
        buffer
    }

    fn header_string(&self) -> String {
        format!(
            "RequestHeader{{ZipFlag:{}, SeqID:{}, PacketType:{}, PkgLen1:{}, PkgLen2:{}, Method:0x{:04x}}}",
            self.zip_flag, self.seq_id, self.packet_type, self.pkg_len1, self.pkg_len2, self.method
        )
    }
}

#[derive(Debug, Clone)]
pub struct ResponseHeader {
    pub i1: u32,
    pub zip_flag: u8,
    pub seq_id: u32,
    pub i2: u8,
    pub method: u16,
    pub zip_size: u16,
    pub unzip_size: u16,
}

impl ResponseHeader {
    pub fn new() -> Self {
        Self {
            i1: 0,
            zip_flag: 0,
            seq_id: 0,
            i2: 0,
            method: 0,
            zip_size: 0,
            unzip_size: 0,
        }
    }

    fn command(&self) -> &'static str {
        command_to_string(self.method)
    }

    pub fn deserialize(&mut self, header: &[u8]) -> Result<(), crate::std::DeserializeError> {
        let mut bs = BinaryStream::from_vec(header.to_vec());
        self.i1 = bs.get_u32()?;
        self.zip_flag = bs.get_u8()?;
        self.seq_id = bs.get_u32()?;
        self.i2 = bs.get_u8()?;
        self.method = bs.get_u16()?;
        self.zip_size = bs.get_u16()?;
        self.unzip_size = bs.get_u16()?;
        Ok(())
    }

    fn header_string(&self) -> String {
        format!(
            "ResponseHeader{{I1:{}, ZipFlag:{}, SeqID:{}, I2:{}, Method:0x{:04x}, ZipSize:{}, UnZipSize:{}}}",
            self.i1,
            self.zip_flag,
            self.seq_id,
            self.i2,
            self.method,
            self.zip_size,
            self.unzip_size
        )
    }

    fn zip_size(&self) -> usize {
        self.zip_size as usize
    }
}

// Request trait，与 C++ 的 RequestHeader 接口保持一致
// 仅用于描述请求头/载荷的序列化行为
pub trait Request {
    fn header(&self) -> &RequestHeader;
    fn header_mut(&mut self) -> &mut RequestHeader;
    fn serialize_payload(&mut self) -> Vec<u8>;
    fn payload_string(&self) -> String;

    fn method(&self) -> u16 {
        self.header().method
    }

    fn command(&self) -> &'static str {
        self.header().command()
    }

    fn serialize(&mut self) -> Vec<u8> {
        let payload = self.serialize_payload();
        self.header_mut().serialize(&payload)
    }

    fn to_string(&self) -> String {
        format!("{}{}", self.header().header_string(), self.payload_string())
    }
}

// Response trait，与 C++ 的 ResponseHeader 接口保持一致
// 描述响应头反序列化与业务数据解析接口
pub trait Response {
    fn header(&self) -> &ResponseHeader;
    fn header_mut(&mut self) -> &mut ResponseHeader;
    fn deserialize_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError>;
    fn body_string(&self) -> String;

    fn header_deserialize(&mut self, header: &[u8]) -> Result<(), crate::std::DeserializeError> {
        self.header_mut().deserialize(header)
    }

    fn header_debug_string(&self) -> String {
        self.header().header_string()
    }

    fn zip_size(&self) -> usize {
        self.header().zip_size()
    }

    fn deserialize(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        self.deserialize_body(data)
    }

    fn to_string(&self) -> String {
        format!("{}{}", self.header().header_string(), self.body_string())
    }

    fn command(&self) -> &'static str {
        self.header().command()
    }
}

// 将 numeric command 转为可读字符串（与 C++ 常量对应）
fn command_to_string(cmd: u16) -> &'static str {
    match cmd {
        commands::HEARTBEAT => "L1:HEARTBEAT",
        commands::LOGIN1 => "L1:LOGIN1",
        commands::LOGIN2 => "L1:LOGIN2",
        commands::XDXR_INFO => "L1:XDXR_INFO",
        commands::FINANCE_INFO => "L1:FINANCE_INFO",
        commands::PING => "L1:PING",
        commands::COMPANY_CATEGORY => "L1:COMPANY_CATEGORY",
        commands::COMPANY_CONTENT => "L1:COMPANY_CONTENT",
        commands::SECURITY_COUNT => "L1:SECURITY_COUNT",
        commands::SECURITY_LIST => "L1:SECURITY_LIST",
        commands::SECURITY_BARS => "L1:SECURITY_BARS",
        commands::SECURITY_QUOTES_OLD => "L1:SECURITY_QUOTES_OLD",
        commands::SECURITY_QUOTES_NEW => "L1:SECURITY_QUOTES_NEW",
        commands::MINUTE_TIME_DATA => "L1:MINUTE_TIME_DATA",
        commands::BLOCK_META => "L1:BLOCK_META",
        commands::BLOCK_DATA => "L1:BLOCK_DATA",
        commands::TRANSACTION_DATA => "L1:TRANSACTION_DATA",
        commands::HISTORY_MINUTE_DATA => "L1:HISTORY_MINUTE_DATA",
        commands::HISTORY_TRANSACTION_DATA => "L1:HISTORY_TRANSACTION_DATA",
        _ => "L1:UNKNOWN_CMD",
    }
}

// 解压 zlib 编码的响应体
fn unzip(body: Vec<u8>, unzipped_size: usize) -> std::io::Result<Vec<u8>> {
    let mut decoder = ZlibDecoder::new(&body[..]);
    let mut out = Vec::with_capacity(unzipped_size);
    decoder.read_to_end(&mut out)?;
    Ok(out)
}

pub(crate) fn process_request_raw(
    stream: &mut MioTcpStream,
    req_buf: &[u8],
) -> Result<(Vec<u8>, Vec<u8>), crate::std::DeserializeError> {
    stream
        .write_all(req_buf)
        .map_err(|e| crate::std::DeserializeError::Other(e.to_string()))?;

    let mut hdr = vec![0u8; RESPONSE_HEADER_LEN];
    stream
        .read_exact(&mut hdr)
        .map_err(|e| crate::std::DeserializeError::Other(e.to_string()))?;

    let mut hdr_reader = BinaryStream::from_vec(hdr.clone());
    let _i1 = hdr_reader.get_u32()?;
    let _zip_flag = hdr_reader.get_u8()?;
    let _seq_id = hdr_reader.get_u32()?;
    let _i2 = hdr_reader.get_u8()?;
    let _method = hdr_reader.get_u16()?;
    let zip_size = hdr_reader.get_u16()? as usize;
    let unzip_size = hdr_reader.get_u16()? as usize;

    if zip_size == 0 {
        return Ok((hdr, Vec::new()));
    }

    let mut body = vec![0u8; zip_size];
    stream
        .read_exact(&mut body)
        .map_err(|e| crate::std::DeserializeError::Other(e.to_string()))?;

    let final_body = if zip_size != unzip_size {
        unzip(body, unzip_size).map_err(|e| crate::std::DeserializeError::Other(e.to_string()))?
    } else {
        body
    };

    Ok((hdr, final_body))
}

// Blocking std::net::TcpStream variant used for handshake performed on blocking socket
pub(crate) fn process_request_raw_std(
    stream: &mut std::net::TcpStream,
    req_buf: &[u8],
) -> Result<(Vec<u8>, Vec<u8>), crate::std::DeserializeError> {
    stream
        .write_all(req_buf)
        .map_err(|e| crate::std::DeserializeError::Other(e.to_string()))?;

    let mut hdr = vec![0u8; RESPONSE_HEADER_LEN];
    stream
        .read_exact(&mut hdr)
        .map_err(|e| crate::std::DeserializeError::Other(e.to_string()))?;

    let mut hdr_reader = BinaryStream::from_vec(hdr.clone());
    let _i1 = hdr_reader.get_u32()?;
    let _zip_flag = hdr_reader.get_u8()?;
    let _seq_id = hdr_reader.get_u32()?;
    let _i2 = hdr_reader.get_u8()?;
    let _method = hdr_reader.get_u16()?;
    let zip_size = hdr_reader.get_u16()? as usize;
    let unzip_size = hdr_reader.get_u16()? as usize;

    if zip_size == 0 {
        return Ok((hdr, Vec::new()));
    }

    let mut body = vec![0u8; zip_size];
    stream
        .read_exact(&mut body)
        .map_err(|e| crate::std::DeserializeError::Other(e.to_string()))?;

    let final_body = if zip_size != unzip_size {
        unzip(body, unzip_size).map_err(|e| crate::std::DeserializeError::Other(e.to_string()))?
    } else {
        body
    };

    Ok((hdr, final_body))
}

pub fn process_request_std(
    stream: &mut std::net::TcpStream,
    req_buf: &[u8],
) -> Result<Vec<u8>, crate::std::DeserializeError> {
    log::debug!(
        "level1::process_request_std - sending request ({} bytes)",
        req_buf.len()
    );

    let (header_bytes, body_bytes) = process_request_raw_std(stream, req_buf)?;

    let mut bs = BinaryStream::from_vec(header_bytes.clone());
    let i1 = bs.get_u32()?;
    let zip_flag = bs.get_u8()?;
    let seq_id = bs.get_u32()?;
    let i2 = bs.get_u8()?;
    let method = bs.get_u16()?;
    let zip_size = bs.get_u16()? as usize;
    let unzip_size = bs.get_u16()? as usize;

    log::debug!(
        "level1::process_request_std - parsed header: I1={}, ZipFlag={}, SeqID={}, I2={}, Method=0x{:04x}, ZipSize={}, UnZipSize={}",
        i1, zip_flag, seq_id, i2, method, zip_size, unzip_size
    );

    log::debug!(
        "level1::process_request_std - response body size: {}",
        body_bytes.len()
    );

    Ok(body_bytes)
}

pub fn process_request(
    stream: &mut MioTcpStream,
    req_buf: &[u8],
) -> Result<Vec<u8>, crate::std::DeserializeError> {
    log::debug!(
        "level1::process_request - sending request ({} bytes)",
        req_buf.len()
    );

    let (header_bytes, body_bytes) = process_request_raw(stream, req_buf)?;

    let mut bs = BinaryStream::from_vec(header_bytes.clone());
    let i1 = bs.get_u32()?;
    let zip_flag = bs.get_u8()?;
    let seq_id = bs.get_u32()?;
    let i2 = bs.get_u8()?;
    let method = bs.get_u16()?;
    let zip_size = bs.get_u16()? as usize;
    let unzip_size = bs.get_u16()? as usize;

    log::debug!(
        "level1::process_request - parsed header: I1={}, ZipFlag={}, SeqID={}, I2={}, Method=0x{:04x}, ZipSize={}, UnZipSize={}",
        i1, zip_flag, seq_id, i2, method, zip_size, unzip_size
    );

    log::debug!(
        "level1::process_request - response body size: {}",
        body_bytes.len()
    );

    Ok(body_bytes)
}

// 泛型process函数，匹配C++ protocol.h中的process模板函数
pub fn process<R: Request, S: Response>(
    stream: &mut MioTcpStream,
    request: &mut R,
    response: &mut S,
) -> Result<(), crate::std::DeserializeError> {
    let cmd = request.command();

    let req_bytes = request.serialize();
    log::debug!("[{}]Send request ({} bytes)", cmd, req_bytes.len());
    log::debug!("[{}]Send request header: {}", cmd, request.to_string());

    let (header_bytes, body_bytes) = process_request_raw(stream, &req_bytes)?;

    response.header_deserialize(&header_bytes).map_err(|e| {
        crate::std::DeserializeError::Other(format!(
            "response header deserialize error for {}: {}",
            cmd, e
        ))
    })?;
    log::debug!(
        "[{}]Recv response head: {}",
        cmd,
        response.header_debug_string()
    );

    if response.zip_size() == 0 {
        return Ok(());
    }

    log::debug!("[{}]Recv response body size: {}", cmd, body_bytes.len());
    response.deserialize(&body_bytes).map_err(|e| {
        crate::std::DeserializeError::Other(format!(
            "response body deserialize error for {}: {}",
            cmd, e
        ))
    })?;
    log::debug!("[{}]Recv response body: {}", cmd, response.to_string());

    Ok(())
}
