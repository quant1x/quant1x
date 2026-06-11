// -*- coding: utf-8 -*-
// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// company — 公司信息分类/内容 (STD_COMPANY_CATEGORY, STD_COMPANY_CONTENT)

use crate::std::BinaryStream;

use super::super::super::command::*;
use super::super::super::protocol::{BaseMessage, RequestHeader, ResponseHeader};

// ============================================================
// CompanyCategory (STD_COMPANY_CATEGORY, 0x02cf)
// ============================================================

#[derive(Debug, Clone)]
pub struct CompanyCategoryRequest {
    req_header: RequestHeader,
    resp_header: ResponseHeader,
    pub content: String,
}

impl CompanyCategoryRequest {
    pub fn new() -> Self {
        Self {
            req_header: RequestHeader::new(STD_COMPANY_CATEGORY, FLAG_UNCOMPRESSED),
            resp_header: ResponseHeader::new(),
            content: String::new(),
        }
    }
}

impl BaseMessage for CompanyCategoryRequest {
    fn request_header(&self) -> &RequestHeader { &self.req_header }
    fn request_header_mut(&mut self) -> &mut RequestHeader { &mut self.req_header }
    fn response_header(&self) -> &ResponseHeader { &self.resp_header }
    fn response_header_mut(&mut self) -> &mut ResponseHeader { &mut self.resp_header }

    fn serialize_request_body(&mut self) -> Vec<u8> {
        Vec::new()
    }

    fn deserialize_response_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        self.content = String::from_utf8_lossy(data).into_owned();
        Ok(())
    }
}

// ============================================================
// CompanyContent (STD_COMPANY_CONTENT, 0x02d0)
// ============================================================

#[derive(Debug, Clone)]
pub struct CompanyContentRequest {
    req_header: RequestHeader,
    resp_header: ResponseHeader,
    pub content: String,
}

impl CompanyContentRequest {
    pub fn new() -> Self {
        Self {
            req_header: RequestHeader::new(STD_COMPANY_CONTENT, FLAG_UNCOMPRESSED),
            resp_header: ResponseHeader::new(),
            content: String::new(),
        }
    }
}

impl BaseMessage for CompanyContentRequest {
    fn request_header(&self) -> &RequestHeader { &self.req_header }
    fn request_header_mut(&mut self) -> &mut RequestHeader { &mut self.req_header }
    fn response_header(&self) -> &ResponseHeader { &self.resp_header }
    fn response_header_mut(&mut self) -> &mut ResponseHeader { &mut self.resp_header }

    fn serialize_request_body(&mut self) -> Vec<u8> {
        Vec::new()
    }

    fn deserialize_response_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        self.content = String::from_utf8_lossy(data).into_owned();
        Ok(())
    }
}
