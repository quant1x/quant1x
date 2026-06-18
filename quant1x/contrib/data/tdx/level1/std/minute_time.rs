// -*- coding: utf-8 -*-
// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// minute_time — 历史分时数据 (STD_HISTORY_MINUTE_DATA, 0x0fb4)

use crate::std::BinaryStream;

use super::super::super::command::*;
use super::super::super::helpers;
use super::super::super::protocol::{BaseMessage, RequestHeader, ResponseHeader};

#[derive(Debug, Clone)]
pub struct MinuteTime {
    pub price: f32,
    pub vol: i64,
}

impl MinuteTime {
    pub fn new() -> Self {
        Self { price: 0.0, vol: 0 }
    }
}

#[derive(Debug, Clone)]
pub struct HistoryMinuteTimeRequest {
    req_header: RequestHeader,
    resp_header: ResponseHeader,
    date: u32,
    market: u8,
    code: [u8; 6],
    pub count: u16,
    pub list: Vec<MinuteTime>,
}

impl HistoryMinuteTimeRequest {
    pub fn new(security_code: &str, date: u32) -> Self {
        let inst = crate::data::market::detect_symbol(security_code);
        let market = helpers::exchange_to_market(inst.exchange.code()).unwrap_or(0) as u8;
        let pure = inst.marker_ticker().to_string();
        let mut code = [0u8; 6];
        let sym = pure.as_bytes();
        let copy_len = std::cmp::min(sym.len(), 6);
        code[..copy_len].copy_from_slice(&sym[..copy_len]);

        let mut req_header = RequestHeader::new(STD_HISTORY_MINUTE_DATA, FLAG_UNCOMPRESSED);
        req_header.packet_ctrl = 0x00;

        Self {
            req_header,
            resp_header: ResponseHeader::new(),
            date,
            market,
            code,
            count: 0,
            list: Vec::new(),
        }
    }
}

impl BaseMessage for HistoryMinuteTimeRequest {
    fn request_header(&self) -> &RequestHeader { &self.req_header }
    fn request_header_mut(&mut self) -> &mut RequestHeader { &mut self.req_header }
    fn response_header(&self) -> &ResponseHeader { &self.resp_header }
    fn response_header_mut(&mut self) -> &mut ResponseHeader { &mut self.resp_header }

    fn serialize_request_body(&mut self) -> Vec<u8> {
        let mut buf = BinaryStream::new();
        buf.push_u32(self.date);
        buf.push_u8(self.market);
        buf.push_byte_array(&self.code);
        buf.data().clone()
    }

    fn deserialize_response_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        self.list.clear();
        if data.len() < 2 {
            return Ok(());
        }
        let mut bs = BinaryStream::from_vec(data.to_vec());
        self.count = bs.get_u16()?;
        if self.count == 0 {
            return Ok(());
        }

        let base_unit = helpers::default_base_unit(self.market as i32, unsafe { std::str::from_utf8_unchecked(&self.code) }.trim_end_matches('\0'));
        let mut last_price: i64 = 0;
        bs.skip(4);
        for _ in 0..self.count {
            let raw_price = bs.varint_decode()?;
            let _ignored = bs.varint_decode()?;
            let vol = bs.varint_decode()?;
            let mut entry = MinuteTime::new();
            entry.vol = vol;
            last_price += raw_price;
            entry.price = (last_price as f32) / (base_unit as f32);
            self.list.push(entry);
        }
        Ok(())
    }
}

/// 获取指定证券指定日期的历史分时数据
pub fn fetch_history_minute_time(security_code: &str, date: u32) -> Option<HistoryMinuteTimeRequest> {
    match super::super::super::client::get_std_conn() {
        Ok(mut conn) => {
            let mut msg = HistoryMinuteTimeRequest::new(security_code, date);
            match super::super::super::protocol::process_level1_stream(conn.stream(), &mut msg) {
                Ok(()) => {
                    log::info!("level1::minute_time - code={} date={} count={}", security_code, date, msg.count);
                    Some(msg)
                }
                Err(e) => {
                    log::error!("level1 process error for minute_time code={} date={}: {}", security_code, date, e);
                    None
                }
            }
        }
        Err(e) => {
            log::error!("failed to acquire level1 client for minute_time code={} date={}: {}", security_code, date, e);
            None
        }
    }
}
