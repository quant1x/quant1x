// -*- coding: utf-8 -*-
// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// security_list — 证券列表 (STD_SECURITY_LIST, 0x044d)
// 对应 Python level1/std/security_list.py

use encoding::{DecoderTrap, Encoding};
use encoding::all::GBK;

use crate::std::BinaryStream;

use super::super::super::command::*;
use super::super::super::helpers::int_to_float64;
use super::super::super::protocol::{BaseMessage, RequestHeader, ResponseHeader};

/// 单次请求的最大记录数，对应 Python SECURITY_LIST_PRE_REQUEST_MAX
pub const PRE_REQUEST_MAX: u32 = 1600;

/// 证券条目
#[derive(Debug, Clone)]
pub struct Security {
    pub code: String,
    pub vol_unit: u16,
    pub name: String,
    pub decimal_point: u8,
    pub pre_close: f64,
}

/// 证券列表请求/响应
#[derive(Debug, Clone)]
pub struct SecurityListRequest {
    req_header: RequestHeader,
    resp_header: ResponseHeader,
    pub market: u16,
    pub start: u32,
    pub count: u32,
    pub list: Vec<Security>,
}

impl SecurityListRequest {
    pub fn new(market: u16, start: u32, count: u32) -> Self {
        Self {
            req_header: RequestHeader::new(STD_SECURITY_LIST, FLAG_UNCOMPRESSED),
            resp_header: ResponseHeader::new(),
            market,
            start,
            count: if count == 0 || count > PRE_REQUEST_MAX { PRE_REQUEST_MAX } else { count },
            list: Vec::new(),
        }
    }
}

impl BaseMessage for SecurityListRequest {
    fn request_header(&self) -> &RequestHeader { &self.req_header }
    fn request_header_mut(&mut self) -> &mut RequestHeader { &mut self.req_header }
    fn response_header(&self) -> &ResponseHeader { &self.resp_header }
    fn response_header_mut(&mut self) -> &mut ResponseHeader { &mut self.resp_header }

    fn serialize_request_body(&mut self) -> Vec<u8> {
        let mut bs = BinaryStream::new();
        bs.push_u16(self.market);
        bs.push_u32(self.start);
        bs.push_u32(self.count);
        bs.push_u32(0);
        bs.data().clone()
    }

    fn deserialize_response_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        self.list.clear();
        if data.is_empty() {
            return Ok(());
        }

        let mut bs = BinaryStream::from_vec(data.to_vec());
        let data_len = data.len();

        // Check minimum: 2 bytes for count
        if data_len < 2 {
            return Ok(());
        }

        let cnt = bs.get_u16()? as usize;

        for _ in 0..cnt {
            let remaining = data_len.saturating_sub(bs.position());
            if remaining < 25 {
                log::warn!("Insufficient data when parsing SECURITY_LIST payload");
                break;
            }

            // code: 6 bytes (ASCII)
            let mut code_bytes = [0u8; 6];
            bs.get_byte_array(&mut code_bytes)?;
            let code = String::from_utf8_lossy(&code_bytes)
                .trim_end_matches('\0')
                .trim()
                .to_string();

            // vol_unit: u16
            let vol_unit = bs.get_u16()?;

            // name: 16 bytes (GBK encoded)
            let mut name_bytes = [0u8; 16];
            bs.get_byte_array(&mut name_bytes)?;
            let name = {
                let nul_pos = name_bytes.iter().position(|&b| b == 0).unwrap_or(name_bytes.len());
                GBK.decode(&name_bytes[..nul_pos], DecoderTrap::Replace)
                    .unwrap_or_else(|_| String::from_utf8_lossy(&name_bytes[..nul_pos]).to_string())
                    .trim()
                    .to_string()
            };

            // reversed2: 4 bytes
            bs.skip(4);

            // decimal_point: u8
            let decimal_point = bs.get_u8()?;

            // pre_close: u32 (as float64)
            let tmp_u32 = bs.get_u32()?;
            let pre_close = int_to_float64(tmp_u32);

            // reversed3: 4 bytes
            bs.skip(4);

            self.list.push(Security {
                code,
                vol_unit,
                name,
                decimal_point,
                pre_close,
            });
        }

        log::debug!("security_list fetched market={} start={} count={} parsed={}",
                    self.market, self.start, cnt, self.list.len());

        Ok(())
    }
}

/// 获取证券列表
pub fn fetch_security_list(market: u16, start: u32, count: u32) -> Option<SecurityListRequest> {
    match super::super::super::client::get_std_conn() {
        Ok(mut conn) => {
            let mut msg = SecurityListRequest::new(market, start, count);
            match super::super::super::protocol::process_level1_stream(conn.stream(), &mut msg) {
                Ok(()) => Some(msg),
                Err(e) => {
                    log::error!("level1 security_list process error for market={} start={}: {}", market, start, e);
                    None
                }
            }
        }
        Err(e) => {
            log::error!("failed to acquire level1 client for security_list market={} start={}: {}", market, start, e);
            None
        }
    }
}
