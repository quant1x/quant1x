// -*- coding: utf-8 -*-
// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// security_count — 证券数量 (STD_SECURITY_COUNT, 0x044e)

use crate::base::BinaryStream;

use super::super::super::command::*;
use super::super::super::protocol::{BaseFrame, RequestHeader, ResponseHeader};

#[derive(Debug, Clone)]
pub struct SecurityCountContext {
    req_header: RequestHeader,
    resp_header: ResponseHeader,
    pub market: u16,
    pub count: usize,
}

impl SecurityCountContext {
    pub fn new(market: u16) -> Self {
        Self {
            req_header: RequestHeader::new(STD_SECURITY_COUNT, FLAG_UNCOMPRESSED),
            resp_header: ResponseHeader::new(),
            market,
            count: 0,
        }
    }
}

impl BaseFrame for SecurityCountContext {
    fn request_header(&self) -> &RequestHeader { &self.req_header }
    fn request_header_mut(&mut self) -> &mut RequestHeader { &mut self.req_header }
    fn response_header(&self) -> &ResponseHeader { &self.resp_header }
    fn response_header_mut(&mut self) -> &mut ResponseHeader { &mut self.resp_header }

    fn serialize_request_body(&mut self) -> Vec<u8> {
        let mut buf = BinaryStream::new();
        buf.push_u16(self.market);
        buf.push_byte_array(&[0x75, 0xc7, 0x33, 0x01]);
        buf.data().clone()
    }

    fn deserialize_response_body(&mut self, data: &[u8]) -> Result<(), crate::base::DeserializeError> {
        if data.len() < 2 {
            return Ok(());
        }
        let mut bs = BinaryStream::from_vec(data.to_vec());
        let c = bs.get_u16()?;
        self.count = c as usize;
        Ok(())
    }
}

/// 获取指定市场的证券数量
pub fn fetch_security_count(market: u16) -> Option<SecurityCountContext> {
    match super::super::super::client::get_std_conn() {
        Ok(mut conn) => {
            let mut msg = SecurityCountContext::new(market);
            match super::super::super::protocol::transact_message_sync(conn.stream(), &mut msg) {
                Ok(()) => {
                    log::info!("level1::security_count - market={} count={}", market, msg.count);
                    Some(msg)
                }
                Err(e) => {
                    log::error!("level1 process error for security_count market={}: {}", market, e);
                    None
                }
            }
        }
        Err(e) => {
            log::error!("failed to acquire level1 client for security_count market={}: {}", market, e);
            None
        }
    }
}
