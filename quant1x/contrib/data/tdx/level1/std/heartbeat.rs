// -*- coding: utf-8 -*-
// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// heartbeat — 心跳维持 (STD_HEARTBEAT, 0x0004)

use crate::std::BinaryStream;

use super::super::super::command::*;
use super::super::super::helpers::get_sequence_id;
use super::super::super::protocol::{BaseFrame, RequestHeader, ResponseHeader};

#[derive(Debug, Clone)]
pub struct HeartbeatContext {
    pub req_header: RequestHeader,
    pub resp_header: ResponseHeader,
    pub info: String,
}

impl BaseFrame for HeartbeatContext {
    fn request_header(&self) -> &RequestHeader { &self.req_header }
    fn request_header_mut(&mut self) -> &mut RequestHeader { &mut self.req_header }
    fn response_header(&self) -> &ResponseHeader { &self.resp_header }
    fn response_header_mut(&mut self) -> &mut ResponseHeader { &mut self.resp_header }

    fn serialize_request_body(&mut self) -> Vec<u8> {
        Vec::new()
    }

    fn deserialize_response_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        let mut bs = BinaryStream::from_vec(data.to_vec());
        self.info = bs.get_string(10)?;
        Ok(())
    }
}

impl HeartbeatContext {
    pub fn new() -> Self {
        let mut header = RequestHeader::new(STD_HEARTBEAT, FLAG_UNCOMPRESSED);
        header.sequence_id = get_sequence_id();
        header.packet_ctrl = 0x02;
        Self {
            req_header: header,
            resp_header: ResponseHeader::new(),
            info: String::new(),
        }
    }
}
