#![allow(dead_code)]
use super::sequence_id;
use crate::std::BinaryStream;
use crate::level1::commands::*;

// Request builder for HISTORY_MINUTE_DATA
#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct HistoryMinuteTimeRequest {
    pub zip_flag: u8,
    pub seq_id: u32,
    pub packet_type: u8,
    pub pkg_len1: u16,
    pub pkg_len2: u16,
    pub method: u16,
    pub date: u32,
    pub market: u8,
    pub code: [u8; 6],
}

impl HistoryMinuteTimeRequest {
    pub fn new(security_code: &str, date: u32) -> Self {
        let (market, _flag, pure) = crate::exchange::detect_market(security_code);
        let mut code = [0u8; 6];
        let sym = pure.as_bytes();
        let copy_len = std::cmp::min(sym.len(), 6);
        code[..copy_len].copy_from_slice(&sym[..copy_len]);

        HistoryMinuteTimeRequest {
            zip_flag: 0x0C,
            seq_id: sequence_id(),
            packet_type: 0x00,
            pkg_len1: 0,
            pkg_len2: 0,
            method: HISTORY_MINUTE_DATA,
            date,
            market,
            code,
        }
    }

    pub fn serialize(&mut self) -> Vec<u8> {
        // payload Date(u32) + Market(u8) + Code[6] -> 4+1+6 = 11
        self.pkg_len1 = 2u16 + 4u16 + 1u16 + 6u16;
        self.pkg_len2 = self.pkg_len1;

        let mut header = BinaryStream::new();
        header.push_u8(self.zip_flag);
        header.push_u32(self.seq_id);
        header.push_u8(self.packet_type);
        header.push_u16(self.pkg_len1);
        header.push_u16(self.pkg_len2);
        header.push_u16(self.method);

        let mut stream = BinaryStream::new();
        stream.push_u32(self.date);
        stream.push_u8(self.market);
        stream.push_byte_array(&self.code);

        let mut buf = header.data().clone();
        let data = stream.data();
        buf.extend_from_slice(data);
        buf
    }
}

pub fn fetch_history_minute_time(security_code: &str, date: u32) -> Option<MinuteTimeResponse> {
    match crate::level1::client::client() {
        Ok(mut pooled) => {
            let mut req = HistoryMinuteTimeRequest::new(security_code, date);
            let req_buf = req.serialize();
            match crate::level1::process_request(pooled.stream(), req_buf.as_slice()) {
                Ok(body) => {
                    let mut resp = MinuteTimeResponse::new(
                        req.market as i32,
                        &String::from_utf8_lossy(&req.code),
                    );
                    resp.deserialize(&body);
                    Some(resp)
                }
                Err(e) => {
                    log::error!(
                        "level1 process_request error for history_minute_time {} date {}: {}",
                        security_code,
                        date,
                        e
                    );
                    None
                }
            }
        }
        Err(e) => {
            log::error!(
                "failed to acquire level1 client for history_minute_time {} date {}: {}",
                security_code,
                date,
                e
            );
            None
        }
    }
}

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
pub struct MinuteTimeResponse {
    pub count: u16,
    pub list: Vec<MinuteTime>,
    pub market_: i32,
    pub code_: String,
}

#[allow(dead_code)]
impl MinuteTimeResponse {
    pub fn new(market: i32, code: &str) -> Self {
        Self {
            count: 0,
            list: Vec::new(),
            market_: market,
            code_: code.to_string(),
        }
    }

    pub fn deserialize(&mut self, data: &[u8]) {
        if data.len() < 2 {
            return;
        }
        let mut bs = BinaryStream::from_vec(data.to_vec());
        self.count = bs.get_u16();
        self.list.reserve(self.count as usize);
        let base_unit = super::default_base_unit(self.market_, &self.code_);
        let _is_index = super::assert_index_by_market_and_code(self.market_, &self.code_);
        let mut last_price: i64 = 0;
        // skip 4 bytes as C++ does for history minute header
        bs.skip(4);
        for _ in 0..self.count {
            let mut e = MinuteTime::new();
            let raw_price = bs.varint_decode();
            let _reversed1 = bs.varint_decode(); // ignored
            let vol = bs.varint_decode();
            e.vol = vol;
            last_price += raw_price;
            e.price = (last_price as f32) / (base_unit as f32);
            self.list.push(e);
        }
    }
}
