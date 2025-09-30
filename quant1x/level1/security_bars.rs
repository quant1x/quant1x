use super::sequence_id;
use crate::std::BinaryStream;
use crate::level1::commands::*;
use std::panic;

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct SecurityBarsRequest {
    pub zip_flag: u8,
    pub seq_id: u32,
    pub packet_type: u8,
    pub pkg_len1: u16,
    pub pkg_len2: u16,
    pub method: u16,
    pub padding: [u8; 10],
}
#[allow(dead_code)]
impl SecurityBarsRequest {
    pub fn new() -> Self {
        SecurityBarsRequest {
            zip_flag: 0x0C,
            seq_id: sequence_id(),
            packet_type: 0x00,
            pkg_len1: 0,
            pkg_len2: 0,
            method: SECURITY_BARS,
            padding: [0u8; 10],
        }
    }

    // Construct a SecurityBarsRequest together with its binary payload and index flag,
    // following the C++ SecurityBarsRequest constructor behavior.
    // Returns (request, payload_bytes, is_index)
    pub fn with_params(
        security_code: &str,
        category: u16,
        start: u16,
        count: u16,
        i_field: u16,
    ) -> (Self, Vec<u8>, bool) {
        let req = SecurityBarsRequest::new();
        // prepare payload using BinaryStream
        let (_mid, _flag, pure) = crate::exchange::detect_market(security_code);
        let mut payload_bs = crate::std::BinaryStream::new();
        // Market (u16 little-endian)
        payload_bs.push_u16(_mid as u16);
        // code 6 bytes (fixed width, NUL-padded)
        let mut code_arr = [0u8; 6];
        let code_bytes = pure.as_bytes();
        for i in 0..code_bytes.len().min(6) {
            code_arr[i] = code_bytes[i];
        }
        payload_bs.push_byte_array(&code_arr);
        // category (u16)
        payload_bs.push_u16(category);
        // I field (provided)
        payload_bs.push_u16(i_field);
        // start (u16)
        payload_bs.push_u16(start);
        // count (u16)
        payload_bs.push_u16(count);
        // padding 10 bytes (use request padding field to match C++ constructor behavior)
        payload_bs.push_byte_array(&req.padding);
        let payload = payload_bs.data().clone();

        // determine if this code is an index by market and code
        let is_index =
            crate::level1::helpers::assert_index_by_market_and_code(_mid as i32, pure.as_str());
        (req, payload, is_index)
    }

    // Convenience constructor using default I = 1 to match the C++ constructor behavior.
    pub fn with_params_default(
        security_code: &str,
        category: u16,
        start: u16,
        count: u16,
    ) -> (Self, Vec<u8>, bool) {
        SecurityBarsRequest::with_params(security_code, category, start, count, 1u16)
    }
    // Serialize the full request header and append the provided payload bytes.
    // This method updates PkgLen fields based on payload length to ensure header parity with C++.
    pub fn serialize(&mut self, payload: &[u8]) -> Vec<u8> {
        // PkgLen = 2 + payload length (matches C++ PkgLen1 = 2 + sizeof(SecurityBarsParameter) + padding.size())
        let payload_len = payload.len() as u16;
        self.pkg_len1 = 2u16 + payload_len;
        self.pkg_len2 = self.pkg_len1;

        // build header
        let mut buf = BinaryStream::new();
        buf.push_u8(self.zip_flag);
        buf.push_u32(self.seq_id);
        buf.push_u8(self.packet_type);
        buf.push_u16(self.pkg_len1);
        buf.push_u16(self.pkg_len2);
        buf.push_u16(self.method);

        // append payload bytes
        buf.push_byte_array(payload);
        buf.data().clone()
    }
}

#[derive(Debug, Clone)]
pub struct SecurityBar {
    pub open: f64,
    pub close: f64,
    pub high: f64,
    pub low: f64,
    pub vol: f64,
    pub amount: f64,
    pub year: i32,
    pub month: i32,
    pub day: i32,
    pub hour: i32,
    pub minute: i32,
    pub datetime: String,
    pub up_count: u16,
    pub down_count: u16,
}

impl SecurityBar {
    pub fn new() -> Self {
        Self {
            open: 0.0,
            close: 0.0,
            high: 0.0,
            low: 0.0,
            vol: 0.0,
            amount: 0.0,
            year: 0,
            month: 0,
            day: 0,
            hour: 0,
            minute: 0,
            datetime: String::new(),
            up_count: 0,
            down_count: 0,
        }
    }
}

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct SecurityBarsResponse {
    pub count: u16,
    pub list: Vec<SecurityBar>,
    // additional metadata typically provided by the request/response constructor
    pub is_index: bool,
    pub category: u16,
}

#[allow(dead_code)]
impl SecurityBarsResponse {
    pub fn new() -> Self {
        Self {
            count: 0,
            list: Vec::new(),
            is_index: false,
            category: 0,
        }
    }
    pub fn new_with(is_index: bool, category: u16) -> Self {
        Self {
            count: 0,
            list: Vec::new(),
            is_index,
            category,
        }
    }

    pub fn deserialize(&mut self, data: &[u8]) {
        self.count = 0;
        self.list.clear();
        let mut bs = BinaryStream::from_vec(data.to_vec());
        let result = panic::catch_unwind(panic::AssertUnwindSafe(|| {
            self.count = bs.get_u16();
            self.list.reserve(self.count as usize);

            let mut pre_diff_base: i64 = 0;
            for _ in 0..self.count {
                let mut e = SecurityBar::new();

                // decode date/time depending on category
                if (self.category as i32) < 4 || self.category == 7 || self.category == 8 {
                    let zipday = bs.get_u16() as u32;
                    let tminutes = bs.get_u16();
                    let (y, m, d, hh, mm) =
                        super::get_datetime_from_u32(self.category as i32, zipday, tminutes);
                    e.year = y;
                    e.month = m;
                    e.day = d;
                    e.hour = hh;
                    e.minute = mm;
                } else {
                    let zipday = bs.get_u32();
                    let (y, m, d, hh, mm) =
                        super::get_datetime_from_u32(self.category as i32, zipday, 0);
                    e.year = y;
                    e.month = m;
                    e.day = d;
                    e.hour = hh;
                    e.minute = mm;
                }
                e.datetime = format!(
                    "{:04}-{:02}-{:02} {:02}:{:02}:00",
                    e.year, e.month, e.day, e.hour, e.minute
                );

                // price diffs (varint encoded)
                let mut price_open_diff = bs.varint_decode();
                let price_close_diff = bs.varint_decode();
                let price_high_diff = bs.varint_decode();
                let price_low_diff = bs.varint_decode();

                let ivol = bs.get_u32();
                e.vol = super::int_to_float64(ivol);

                let dbvol = bs.get_u32();
                e.amount = super::int_to_float64(dbvol);

                // compute prices: values are divided by 1000.0 per C++ implementation
                e.open = (price_open_diff + pre_diff_base) as f64 / 1000.0;
                price_open_diff += pre_diff_base;

                e.close = (price_open_diff + price_close_diff) as f64 / 1000.0;
                e.high = (price_open_diff + price_high_diff) as f64 / 1000.0;
                e.low = (price_open_diff + price_low_diff) as f64 / 1000.0;

                pre_diff_base = price_open_diff + price_close_diff;

                if self.is_index {
                    e.up_count = bs.get_u16();
                    e.down_count = bs.get_u16();
                }

                self.list.push(e);
            }
        }));
        if let Err(_) = result {
            log::warn!("insufficient data for {} bars, parsed {} successfully", self.count, self.list.len());
            self.count = self.list.len() as u16;
        }
    }

    // High-level helper: fetch security bars for a code with given category/start/count using the
    // level1 client. This wraps the SecurityBarsRequest/Response lifecycle and returns the parsed
    // SecurityBarsResponse on success.
    pub fn fetch_security_bars(
        code: &str,
        category: u16,
        i: u16,
        start: u32,
        count: u16,
    ) -> Option<SecurityBarsResponse> {
        // Build request header and payload using with_params (C++-like constructor behavior)
        let (mut req, payload_bytes, is_index) =
            SecurityBarsRequest::with_params(code, category, start as u16, count, i);
        let req_bytes = req.serialize(&payload_bytes);

        // perform network exchange
        match crate::level1::client::client() {
            Ok(mut pooled) => {
                match crate::level1::process_request(pooled.stream(), req_bytes.as_slice()) {
                    Ok(body) => {
                        let mut resp = SecurityBarsResponse::new_with(is_index, category);
                        resp.deserialize(&body);
                        Some(resp)
                    }
                    Err(e) => {
                        log::error!(
                            "level1 fetch_security_bars process_request error for {}: {}",
                            code,
                            e
                        );
                        None
                    }
                }
            }
            Err(e) => {
                log::error!(
                    "failed to acquire level1 client for fetch_security_bars {}: {}",
                    code,
                    e
                );
                None
            }
        }
    }
}

// Module-level wrapper so callers can invoke `crate::level1::fetch_security_bars(...)` after
// `pub use security_bars::*` from `level1::mod.rs`.
pub fn fetch_security_bars(
    code: &str,
    category: u16,
    i: u16,
    start: u32,
    count: u16,
) -> Option<SecurityBarsResponse> {
    SecurityBarsResponse::fetch_security_bars(code, category, i, start, count)
}
