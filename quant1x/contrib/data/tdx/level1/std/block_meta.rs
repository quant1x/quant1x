// -*- coding: utf-8 -*-
// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// block_meta — 板块元数据 (STD_BLOCK_META, 0x02c5)

use crate::std::BinaryStream;

use super::super::super::command::*;
use super::super::super::protocol::{BaseFrame, RequestHeader, ResponseHeader};

// Constants
pub const BLOCK_ZHISHU: &str = "block_zs.dat";
pub const BLOCK_FENGGE: &str = "block_fg.dat";
pub const BLOCK_GAINIAN: &str = "block_gn.dat";
pub const BLOCK_DEFAULT: &str = "block.dat";

#[derive(Debug, Clone)]
pub struct BlockMeta {
    pub size: u32,
    pub c1: u8,
    pub hash_value: [u8; 32],
    pub c2: u8,
}

impl BlockMeta {
    pub fn new() -> Self {
        Self {
            size: 0,
            c1: 0,
            hash_value: [0u8; 32],
            c2: 0,
        }
    }
}

#[derive(Debug, Clone)]
pub struct BlockFileMetaContext {
    req_header: RequestHeader,
    resp_header: ResponseHeader,
    filename: String,
    pub meta: BlockMeta,
}

impl BlockFileMetaContext {
    pub fn new(filename: &str) -> Self {
        Self {
            req_header: RequestHeader::new(STD_BLOCK_META, FLAG_UNCOMPRESSED),
            resp_header: ResponseHeader::new(),
            filename: filename.to_string(),
            meta: BlockMeta::new(),
        }
    }
}

impl BaseFrame for BlockFileMetaContext {
    fn request_header(&self) -> &RequestHeader { &self.req_header }
    fn request_header_mut(&mut self) -> &mut RequestHeader { &mut self.req_header }
    fn response_header(&self) -> &ResponseHeader { &self.resp_header }
    fn response_header_mut(&mut self) -> &mut ResponseHeader { &mut self.resp_header }

    fn serialize_request_body(&mut self) -> Vec<u8> {
        let filename_bytes = self.filename.as_bytes();
        let mut buf = BinaryStream::new();
        let mut padded = [0u8; 40];
        let copy_len = filename_bytes.len().min(40);
        padded[..copy_len].copy_from_slice(&filename_bytes[..copy_len]);
        buf.push_byte_array(&padded);
        buf.data().clone()
    }

    fn deserialize_response_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        // Size(4) + C1(1) + HashValue(32) + C2(1) = 38 bytes
        if data.len() < 38 {
            return Ok(());
        }
        let mut bs = BinaryStream::from_vec(data.to_vec());
        self.meta.size = bs.get_u32()?;
        self.meta.c1 = bs.get_u8()?;
        bs.get_byte_array(&mut self.meta.hash_value)?;
        self.meta.c2 = bs.get_u8()?;
        Ok(())
    }
}
