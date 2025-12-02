#![allow(dead_code)]

use super::sequence_id;
use crate::level1::commands::*;
use crate::level1::protocol::{Request, RequestHeader, Response, ResponseHeader};
use crate::std::BinaryStream;
use crate::Timestamp;
use hex;
use std::collections::{HashMap, VecDeque};
use std::panic;

#[derive(Debug, Clone)]
pub enum TradeState {
    Delisting,
    Normal,
    Suspend,
    Ipo,
}

/// 最小化的 StockInfo，映射 C++ 中的 `level1::StockInfo { int market; std::string code; }`
#[derive(Debug, Clone)]
pub struct StockInfo {
    pub market: u8,
    pub code: String,
}

#[derive(Debug, Clone)]
pub struct SecurityQuote {
    pub market: u8,
    pub code: String,
    pub active1: u16,
    pub price: f64,
    pub last_close: f64,
    pub open: f64,
    pub high: f64,
    pub low: f64,
    pub server_time: String,
    pub vol: i64,
    pub cur_vol: i64,
    pub amount: f64,
    pub s_vol: i64,
    pub b_vol: i64,
    pub index_open_amount: i64,
    pub stock_open_amount: i64,
    pub open_volume: i64,
    pub close_volume: i64,
    pub bid: [f64; 5],
    pub ask: [f64; 5],
    pub bid_vol: [i64; 5],
    pub ask_vol: [i64; 5],
    pub reversed4: u16,
    pub reversed5: i64,
    pub reversed6: i64,
    pub reversed7: i64,
    pub reversed8: i64,
    pub rate: f64,
    pub active2: u16,
    pub time_stamp: String,
    pub state: TradeState,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SpreadLevel {
    VeryLow,
    Low,
    Medium,
    High,
    VeryHigh,
}

// 默认阈值（百分比）
pub const SPREAD_PCT_VERY_LOW: f64 = 0.05; // < 0.05%
pub const SPREAD_PCT_LOW: f64 = 0.2; // 0.05% - 0.2%
pub const SPREAD_PCT_MEDIUM: f64 = 0.8; // 0.2% - 0.8%
pub const SPREAD_PCT_HIGH: f64 = 2.0; // 0.8% - 2.0%

impl SecurityQuote {
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
            server_time: String::new(),
            vol: 0,
            cur_vol: 0,
            amount: 0.0,
            s_vol: 0,
            b_vol: 0,
            index_open_amount: 0,
            stock_open_amount: 0,
            open_volume: 0,
            close_volume: 0,
            bid: [0.0; 5],
            ask: [0.0; 5],
            bid_vol: [0; 5],
            ask_vol: [0; 5],
            reversed4: 0,
            reversed5: 0,
            reversed6: 0,
            reversed7: 0,
            reversed8: 0,
            rate: 0.0,
            active2: 0,
            time_stamp: String::new(),
            state: TradeState::Normal,
        }
    }

    /// 计算隐形价差（价格单位）
    /// 使用常见定义: effective spread = 2 * |trade_price - midpoint(bid1, ask1)|
    /// 回退: 若 price 不可用或 <= 0，则使用在盘价差 (ask1 - bid1)；若也不可用则返回 0.0
    pub fn implicit_spread(&self) -> f64 {
        // 如果交易价格不可用，回退到在盘价差
        if self.price.is_nan() || self.price <= 0.0 {
            if self.ask[0] > 0.0 && self.bid[0] > 0.0 {
                return self.ask[0] - self.bid[0];
            }
            return 0.0;
        }

        if self.ask[0] > 0.0 && self.bid[0] > 0.0 {
            let mid = (self.ask[0] + self.bid[0]) / 2.0;
            2.0 * (self.price - mid).abs()
        } else if self.ask[0] > 0.0 && self.bid[0] > 0.0 {
            // 冗余分支以防万一
            self.ask[0] - self.bid[0]
        } else {
            0.0
        }
    }

    /// 计算隐形价差占比（%）
    /// 使用 midpoint 作为基准:  implicit_spread / midpoint * 100
    /// 若 midpoint 不可用则回退至 last_close；若仍不可用返回 0.
    pub fn implicit_spread_pct(&self) -> f64 {
        if self.ask[0] > 0.0 && self.bid[0] > 0.0 {
            let mid = (self.ask[0] + self.bid[0]) / 2.0;
            let s = self.implicit_spread();
            if mid > 0.0 {
                return s / mid * 100.0;
            }
        }
        if self.last_close > 0.0 {
            let s = self.implicit_spread();
            return s / self.last_close * 100.0;
        }
        0.0
    }

    pub fn implicit_spread_level(&self) -> SpreadLevel {
        let pct = self.implicit_spread_pct();
        if pct < SPREAD_PCT_VERY_LOW {
            SpreadLevel::VeryLow
        } else if pct < SPREAD_PCT_LOW {
            SpreadLevel::Low
        } else if pct < SPREAD_PCT_MEDIUM {
            SpreadLevel::Medium
        } else if pct < SPREAD_PCT_HIGH {
            SpreadLevel::High
        } else {
            SpreadLevel::VeryHigh
        }
    }
}

/// Request builder for SECURITY_QUOTES (old)
#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct SecurityQuoteRequest {
    header: RequestHeader,
    pub padding: Vec<u8>,
    pub list: Vec<StockInfo>,
}

impl Request for SecurityQuoteRequest {
    fn header(&self) -> &RequestHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut RequestHeader {
        &mut self.header
    }

    fn serialize_payload(&mut self) -> Vec<u8> {
        let count = self.list.len();
        // PkgLen1 = 2 (count) + count*7 (1 market + 6 code) + 10 (padding/header extras per C++)
        self.header.pkg_len1 = 2u16 + (count as u16).saturating_mul(7u16) + 10u16;
        self.header.pkg_len2 = self.header.pkg_len1;

        let mut buf = BinaryStream::new();
        // padding
        buf.push_byte_array(&self.padding);

        // count
        buf.push_u16(count as u16);

        // entries: market (u8) + code (6 bytes)
        for it in self.list.iter() {
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

    fn payload_string(&self) -> String {
        format!("SecurityQuoteRequest{{count:{}}}", self.list.len())
    }
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
            // ensure at most 6 chars
            if code.len() > 6 {
                code.truncate(6);
            }
            list.push(StockInfo { market, code });
        }

        let mut header = RequestHeader::new();
        header.zip_flag = crate::level1::protocol::zlib_flag::UNCOMPRESSED;
        header.seq_id = sequence_id();
        header.packet_type = 0x01;
        header.method = SECURITY_QUOTES_OLD;

        SecurityQuoteRequest {
            header,
            padding: hex::decode("0500000000000000").unwrap_or_default(),
            list,
        }
    }
}

/// 为一组证券代码获取即时行情。请求/响应的编解码在 level1 内部处理。
/// 成功时返回 Some(SecurityQuoteResponse)，发生 IO 错误时返回 None。
pub fn fetch_security_quote(codes: &[String]) -> Option<SecurityQuoteResponse> {
    match crate::level1::client::get_std_conn() {
        Ok(mut pooled) => {
            let mut req = SecurityQuoteRequest::new(codes);

            // build a code map for verify_delisted_securities
            let mut code_map: HashMap<String, StockInfo> = HashMap::new();
            for it in req.list.iter() {
                let prefix = match it.market {
                    1 => "sh",
                    0 => "sz",
                    2 => "bj",
                    _ => "sz",
                };
                let key = format!("{}{}", prefix, it.code);
                code_map.insert(
                    key,
                    StockInfo {
                        market: it.market,
                        code: it.code.clone(),
                    },
                );
            }

            let mut resp = SecurityQuoteResponse::new();
            match crate::level1::process(pooled.stream(), &mut req, &mut resp) {
                Ok(_) => {
                    resp.verify_delisted_securities(&mut code_map);
                    log::info!(
                        "level1::security_quote - requested={} received_count={}",
                        codes.len(),
                        resp.count
                    );
                    Some(resp)
                }
                Err(e) => {
                    log::error!("level1 process error for security_quote: {}", e);
                    None
                }
            }
        }
        Err(e) => {
            log::error!("failed to acquire level1 client for security_quote: {}", e);
            None
        }
    }
}

fn format_time(stamp: i64) -> String {
    // Port of the C++ helpers::format_time simplified to produce HH:MM:SS.sss
    // The original encodes hours/minutes/milliseconds into an integer.
    if stamp <= 0 {
        return "0".to_string();
    }
    let tm_h_width = 1_000_000i64;
    let tm_m_width = 10_000i64;
    let h = stamp / tm_h_width;
    let tmp1 = stamp % tm_h_width;
    let m1 = tmp1 / tm_m_width;
    if h > 100 {}
    let (m, st) = if m1 < 60 {
        let m = m1;
        let tmp3 = tmp1 % tm_m_width;
        (m, (tmp3 * 60) as f64 / (tm_m_width as f64))
    } else {
        let m = (tmp1 / tm_h_width) as i64;
        let tmp3 = (tmp1 % tm_h_width) * 60;
        (m, (tmp3 as f64) / (tm_h_width as f64))
    };
    format!("{:02}:{:02}:{:06.3}", h, m, st)
}

#[derive(Debug, Clone)]
pub struct SecurityQuoteResponse {
    header: ResponseHeader,
    pub count: u16,
    pub list: Vec<SecurityQuote>,
}

impl Response for SecurityQuoteResponse {
    fn header(&self) -> &ResponseHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut ResponseHeader {
        &mut self.header
    }

    fn deserialize_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        self.count = 0;
        self.list.clear();
        let mut bs = BinaryStream::from_vec(data.to_vec());
        bs.skip(2);
        self.count = bs.get_u16()?;
        self.list.reserve(self.count as usize);
        for _ in 0..self.count {
            let mut ele = SecurityQuote::new();
            ele.market = bs.get_u8()?;
            ele.code = bs.get_string(6)?;
            let base_unit = super::default_base_unit(ele.market as i32, &ele.code);
            ele.active1 = bs.get_u16()?;

            let price_base = bs.varint_decode()?;
            ele.price = (price_base as f64) / base_unit;
            let tmp = bs.varint_decode()?;
            ele.last_close = ((price_base + tmp) as f64) / base_unit;
            ele.open = ((price_base + bs.varint_decode()?) as f64) / base_unit;
            ele.high = ((price_base + bs.varint_decode()?) as f64) / base_unit;
            ele.low = ((price_base + bs.varint_decode()?) as f64) / base_unit;

            ele.server_time = {
                let rb0 = bs.varint_decode()?;
                if rb0 > 0 {
                    format_time(rb0)
                } else {
                    "0".to_string()
                }
            };
            ele.reversed5 = bs.varint_decode()?;

            ele.vol = bs.varint_decode()?;
            ele.vol *= 100;
            ele.cur_vol = bs.varint_decode()?;
            let raw_amount = bs.get_u32()?;
            ele.amount = super::int_to_float64(raw_amount);

            ele.s_vol = bs.varint_decode()?;
            ele.b_vol = bs.varint_decode()?;

            ele.index_open_amount = bs.varint_decode()? * 100;
            ele.stock_open_amount = bs.varint_decode()? * 100;

            let is_index_or_block =
                crate::exchange::assert_index_by_market_and_code(ele.market, &ele.code);
            let tmp_open_volume = if is_index_or_block {
                if ele.open != 0.0 {
                    ((ele.index_open_amount as f64) / ele.open).round()
                } else {
                    0.0
                }
            } else {
                if ele.open != 0.0 {
                    ((ele.stock_open_amount as f64) / ele.open).round()
                } else {
                    0.0
                }
            };
            if tmp_open_volume.is_nan() {
                ele.open_volume = 0;
            } else {
                ele.open_volume = tmp_open_volume as i64;
            }

            for l in 0..5 {
                let bid_price = ((bs.varint_decode()? + price_base) as f64) / base_unit;
                let ask_price = ((bs.varint_decode()? + price_base) as f64) / base_unit;
                let bid_vol = bs.varint_decode()?;
                let ask_vol = bs.varint_decode()?;
                ele.bid[l] = bid_price;
                ele.ask[l] = ask_price;
                ele.bid_vol[l] = bid_vol;
                ele.ask_vol[l] = ask_vol;
            }

            ele.reversed4 = bs.get_u16()?;
            ele.reversed5 = bs.varint_decode()?;
            ele.reversed6 = bs.varint_decode()?;
            ele.reversed7 = bs.varint_decode()?;
            ele.reversed8 = bs.varint_decode()?;

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

            if is_index_or_block {
                ele.index_open_amount = ele.bid_vol[0]; // indexUp
                ele.index_open_amount = ele.bid_vol[1]; // indexUpLimit (approx)
            }

            // determine current session status using exchange session logic
            let now_ts = Timestamp::now();
            let (_update_rt, status) = crate::exchange::can_update_in_realtime(Some(now_ts));
            // closing call auction phase => MASK_CALL_AUCTION | MASK_CLOSING
            let in_closing = (status & crate::exchange::MASK_CALL_AUCTION) != 0
                && (status & crate::exchange::MASK_CLOSING) != 0;
            if in_closing {
                if is_index_or_block {
                    if ele.price != 0.0 {
                        ele.close_volume = ((ele.cur_vol * 100) as f64 / ele.price) as i64;
                    } else {
                        ele.close_volume = 0;
                    }
                } else {
                    ele.close_volume = ele.cur_vol * 100;
                }
            }

            ele.time_stamp = now_ts.to_string_with_layout("%Y%m%d%H%M%S%.3f");
            self.list.push(ele);
        }
        Ok(())
    }

    fn body_string(&self) -> String {
        format!("SecurityQuoteResponse{{count:{}}}", self.count)
    }
}

#[allow(dead_code)]
impl SecurityQuoteResponse {
    pub fn new() -> Self {
        Self {
            header: ResponseHeader::new(),
            count: 0,
            list: Vec::new(),
        }
    }

    pub fn verify_delisted_securities(&mut self, code_maps: &mut HashMap<String, StockInfo>) {
        // 1. Mark received
        for q in self.list.iter() {
            let prefix = match q.market {
                1 => "sh",
                0 => "sz",
                2 => "bj",
                _ => "sz",
            };
            let key = format!("{}{}", prefix, q.code);
            code_maps.remove(&key);
        }

        // 2. Remaining in code_maps are delisted or invalid
        for (_, v) in code_maps.iter() {
            let mut q = SecurityQuote::new();
            q.market = v.market;
            q.code = v.code.clone();
            q.state = TradeState::Delisting;
            self.list.push(q);
        }
        self.count = self.list.len() as u16;
    }
}
