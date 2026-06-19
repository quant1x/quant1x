// -*- coding: utf-8 -*-
// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// hello — 标准行情握手1/2 (STD_SYNCHRONIZE1, STD_SYNCHRONIZE2)

use encoding_rs::GBK;

use super::super::super::command::*;
use super::super::super::protocol::{BaseFrame, RequestHeader, ResponseHeader};

// ============================================================
// StdLoginContext (STD_SYNCHRONIZE1, 0x000d) — 标准行情握手1
// ============================================================

#[derive(Debug, Clone)]
pub struct StdLoginContext {
    pub req_header: RequestHeader,
    pub resp_header: ResponseHeader,
    pub info: String,
}

impl BaseFrame for StdLoginContext {
    fn request_header(&self) -> &RequestHeader { &self.req_header }
    fn request_header_mut(&mut self) -> &mut RequestHeader { &mut self.req_header }
    fn response_header(&self) -> &ResponseHeader { &self.resp_header }
    fn response_header_mut(&mut self) -> &mut ResponseHeader { &mut self.resp_header }

    fn serialize_request_body(&mut self) -> Vec<u8> {
        let padding = hex::decode("01").unwrap_or_default();
        padding
    }

    fn deserialize_response_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        let offset = 68usize;
        if data.len() >= offset {
            let info_bytes = &data[offset..];
            let (cow, _, _) = GBK.decode(info_bytes);
            self.info = cow.into_owned();
        }
        Ok(())
    }
}

impl StdLoginContext {
    pub fn new() -> Self {
        Self {
            req_header: RequestHeader::new(STD_SYNCHRONIZE1, FLAG_UNCOMPRESSED),
            resp_header: ResponseHeader::new(),
            info: String::new(),
        }
    }
}

// ============================================================
// UpgradeTipContext (STD_SYNCHRONIZE2, 0x0fdb) — 标准行情握手2
// ============================================================

#[derive(Debug, Clone)]
pub struct UpgradeTipContext {
    pub req_header: RequestHeader,
    pub resp_header: ResponseHeader,
    pub info: String,
}

impl BaseFrame for UpgradeTipContext {
    fn request_header(&self) -> &RequestHeader { &self.req_header }
    fn request_header_mut(&mut self) -> &mut RequestHeader { &mut self.req_header }
    fn response_header(&self) -> &ResponseHeader { &self.resp_header }
    fn response_header_mut(&mut self) -> &mut ResponseHeader { &mut self.resp_header }

    fn serialize_request_body(&mut self) -> Vec<u8> {
        hex::decode("d5d0c9ccd6a4a8af0000008fc22540130000d500c9ccbdf0d7ea00000002")
            .unwrap_or_default()
    }

    fn deserialize_response_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        let offset = 58usize;
        if data.len() >= offset {
            let (cow, _, _) = GBK.decode(&data[offset..]);
            self.info = cow.into_owned();
        }
        Ok(())
    }
}

impl UpgradeTipContext {
    pub fn new() -> Self {
        Self {
            req_header: RequestHeader::new(STD_SYNCHRONIZE2, FLAG_UNCOMPRESSED),
            resp_header: ResponseHeader::new(),
            info: String::new(),
        }
    }
}
