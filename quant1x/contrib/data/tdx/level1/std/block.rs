// -*- coding: utf-8 -*-
// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// block — 板块文件数据请求 (STD_BLOCK_DATA, 0x06b9)
// 对应 Python quant1x/contrib/data/tdx/level1/std/block.py

use super::super::super::command::*;
use super::super::super::protocol::{BaseFrame, RequestHeader, ResponseHeader};

/// Python `BLOCK_CHUNKS_SIZE` = 0x7530
pub const BLOCK_CHUNKS_SIZE: usize = 0x7530;

/// 板块文件数据请求/响应
/// 对应 Python `BlockFileContext`
pub struct BlockFileContext {
    req_header: RequestHeader,
    resp_header: ResponseHeader,
    filename: String,
    offset: u32,
    chunk_size: u32,
    pub size: u32,
    pub data: Vec<u8>,
}

impl BlockFileContext {
    pub fn new(filename: &str, offset: u32) -> Self {
        Self {
            req_header: RequestHeader::new(STD_BLOCK_DATA, FLAG_UNCOMPRESSED),
            resp_header: ResponseHeader::new(),
            filename: filename.to_string(),
            offset,
            chunk_size: BLOCK_CHUNKS_SIZE as u32,
            size: 0,
            data: Vec::new(),
        }
    }
}

impl BaseFrame for BlockFileContext {
    fn request_header(&self) -> &RequestHeader {
        &self.req_header
    }
    fn request_header_mut(&mut self) -> &mut RequestHeader {
        &mut self.req_header
    }
    fn response_header(&self) -> &ResponseHeader {
        &self.resp_header
    }
    fn response_header_mut(&mut self) -> &mut ResponseHeader {
        &mut self.resp_header
    }

    fn serialize_request_body(&mut self) -> Vec<u8> {
        // struct.pack('<I I', offset, chunk_size) + filename_bytes[:100].ljust(100, b'\x00')
        let mut buf = Vec::with_capacity(8 + 100);
        buf.extend_from_slice(&self.offset.to_le_bytes());
        buf.extend_from_slice(&self.chunk_size.to_le_bytes());
        let filename_bytes = self.filename.as_bytes();
        let copy_len = filename_bytes.len().min(100);
        buf.extend_from_slice(&filename_bytes[..copy_len]);
        // padding
        if copy_len < 100 {
            buf.resize(buf.len() + (100 - copy_len), 0u8);
        }
        buf
    }

    fn deserialize_response_body(&mut self, data: &[u8]) -> Result<(), crate::base::DeserializeError> {
        self.data.clear();
        if data.len() < 4 {
            self.size = 0;
            return Ok(());
        }
        self.size = u32::from_le_bytes([data[0], data[1], data[2], data[3]]);
        if self.size > 0 && data.len() > 4 {
            self.data.extend_from_slice(&data[4..]);
        }
        Ok(())
    }
}
