// -*- coding: utf-8 -*-
// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// security_quote — 行情快照 (STD_SECURITY_QUOTES_OLD, 0x053e)

use crate::std::BinaryStream;

use super::super::super::command::*;
use super::super::super::helpers;
use super::super::super::protocol::{BaseMessage, RequestHeader, ResponseHeader};

#[derive(Debug, Clone)]
pub struct StockInfo {
    pub market: u8,
    pub code: String,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TradeState {
    Delisting,
    Normal,
    Suspend,
    Ipo,
}

#[derive(Debug, Clone)]
pub struct SecurityQuoteData {
    pub market: u8,
    pub code: String,
    pub active1: u16,
    pub price: f64,
    pub last_close: f64,
    pub open: f64,
    pub high: f64,
    pub low: f64,
    pub vol: i64,
    pub cur_vol: i64,
    pub amount: f64,
    pub s_vol: i64,
    pub b_vol: i64,
    pub bid: [f64; 5],
    pub ask: [f64; 5],
    pub bid_vol: [i64; 5],
    pub ask_vol: [i64; 5],
    pub rate: f64,
    pub active2: u16,
    pub state: TradeState,
}

impl SecurityQuoteData {
    pub fn new() -> Self {
        Self {
            market: 0,
            code: String::new(),
            active1: 0,
            price: 0.0,
            last_close: 0.0,
            open: 0.0,
            high: 0.0,
            low: 0.0,
            vol: 0,
            cur_vol: 0,
            amount: 0.0,
            s_vol: 0,
            b_vol: 0,
            bid: [0.0; 5],
            ask: [0.0; 5],
            bid_vol: [0; 5],
            ask_vol: [0; 5],
            rate: 0.0,
            active2: 0,
            state: TradeState::Normal,
        }
    }
}

#[derive(Debug, Clone)]
pub struct SecurityQuoteRequest {
    req_header: RequestHeader,
    resp_header: ResponseHeader,
    padding: Vec<u8>,
    pub stock_list: Vec<StockInfo>,
    pub count: u16,
    pub quotes: Vec<SecurityQuoteData>,
}

impl SecurityQuoteRequest {
    pub fn new(codes: &[String]) -> Self {
        let mut list: Vec<StockInfo> = Vec::new();
        for s in codes.iter() {
            let sc = s.trim();
            if sc.is_empty() {
                continue;
            }
            let (market, _flag, pure) = crate::exchange::detect_market(sc);
            let mut code = pure.clone();
            if code.len() > 6 {
                code.truncate(6);
            }
            list.push(StockInfo { market, code });
        }

        Self {
            req_header: RequestHeader::new(STD_SECURITY_QUOTES_OLD, FLAG_UNCOMPRESSED),
            resp_header: ResponseHeader::new(),
            padding: hex::decode("0500000000000000").unwrap_or_default(),
            stock_list: list,
            count: 0,
            quotes: Vec::new(),
        }
    }
}

fn format_time(stamp: i64) -> String {
    if stamp <= 0 {
        return "0".to_string();
    }
    let tm_h_width = 1_000_000i64;
    let tm_m_width = 10_000i64;
    let h = stamp / tm_h_width;
    let tmp1 = stamp % tm_h_width;
    let m1 = tmp1 / tm_m_width;
    let (m, st) = if m1 < 60 {
        let m = m1;
        let tmp3 = tmp1 % tm_m_width;
        (m, (tmp3 * 60) as f64 / (tm_m_width as f64))
    } else {
        let m = tmp1 / tm_h_width;
        let tmp3 = (tmp1 % tm_h_width) * 60;
        (m, (tmp3 as f64) / (tm_h_width as f64))
    };
    format!("{:02}:{:02}:{:06.3}", h, m, st)
}

impl BaseMessage for SecurityQuoteRequest {
    fn request_header(&self) -> &RequestHeader { &self.req_header }
    fn request_header_mut(&mut self) -> &mut RequestHeader { &mut self.req_header }
    fn response_header(&self) -> &ResponseHeader { &self.resp_header }
    fn response_header_mut(&mut self) -> &mut ResponseHeader { &mut self.resp_header }

    fn serialize_request_body(&mut self) -> Vec<u8> {
        let count = self.stock_list.len();
        let mut buf = BinaryStream::new();
        buf.push_byte_array(&self.padding);
        buf.push_u16(count as u16);
        for it in self.stock_list.iter() {
            buf.push_u8(it.market);
            let mut code_bytes = [0u8; 6];
            let b = it.code.as_bytes();
            for i in 0..b.len().min(6) {
                code_bytes[i] = b[i];
            }
            buf.push_byte_array(&code_bytes);
        }
        buf.data().clone()
    }

    fn deserialize_response_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        self.quotes.clear();
        self.count = 0;
        let mut bs = BinaryStream::from_vec(data.to_vec());
        bs.skip(2);
        self.count = bs.get_u16()?;
        for _ in 0..self.count {
            let mut ele = SecurityQuoteData::new();
            ele.market = bs.get_u8()?;
            ele.code = bs.get_string(6)?;
            let base_unit = helpers::default_base_unit(ele.market as i32, &ele.code);
            ele.active1 = bs.get_u16()?;

            let price_base = bs.varint_decode()?;
            ele.price = (price_base as f64) / base_unit;
            let tmp = bs.varint_decode()?;
            ele.last_close = ((price_base + tmp) as f64) / base_unit;
            ele.open = ((price_base + bs.varint_decode()?) as f64) / base_unit;
            ele.high = ((price_base + bs.varint_decode()?) as f64) / base_unit;
            ele.low = ((price_base + bs.varint_decode()?) as f64) / base_unit;

            let rb0 = bs.varint_decode()?;
            let _server_time = if rb0 > 0 { format_time(rb0) } else { "0".to_string() };
            let _rb1 = bs.varint_decode()?;

            ele.vol = bs.varint_decode()?;
            ele.vol *= 100;
            ele.cur_vol = bs.varint_decode()?;
            let raw_amount = bs.get_u32()?;
            ele.amount = helpers::int_to_float64(raw_amount);

            ele.s_vol = bs.varint_decode()?;
            ele.b_vol = bs.varint_decode()?;

            let _index_open_amount = bs.varint_decode()? * 100;
            let _stock_open_amount = bs.varint_decode()? * 100;

            for l in 0..5 {
                ele.bid[l] = ((bs.varint_decode()? + price_base) as f64) / base_unit;
                ele.ask[l] = ((bs.varint_decode()? + price_base) as f64) / base_unit;
                ele.bid_vol[l] = bs.varint_decode()?;
                ele.ask_vol[l] = bs.varint_decode()?;
            }

            let _reversed4 = bs.get_u16()?;
            let _reversed5 = bs.varint_decode()?;
            let _reversed6 = bs.varint_decode()?;
            let _reversed7 = bs.varint_decode()?;
            let _reversed8 = bs.varint_decode()?;

            let rev9 = bs.get_i16()?;
            ele.rate = (rev9 as f64) / 100.0;
            ele.active2 = bs.get_u16()?;

            // Determine trade state
            if ele.last_close == 0.0 && ele.open == 0.0 {
                ele.state = TradeState::Delisting;
            } else if ele.open != 0.0 {
                ele.state = TradeState::Normal;
            } else {
                ele.state = TradeState::Suspend;
            }

            self.quotes.push(ele);
        }
        Ok(())
    }
}
