#![allow(dead_code)]
use super::sequence_id;
use crate::level1::protocol::{self, commands, Request, RequestHeader, Response, ResponseHeader};
use crate::std::BinaryStream;

// Request builder for HISTORY_MINUTE_DATA
#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct HistoryMinuteTimeRequest {
    header: RequestHeader,
    date: u32,
    market: u8,
    code: [u8; 6],
}

impl HistoryMinuteTimeRequest {
    pub fn new(security_code: &str, date: u32) -> Self {
        let (market, _flag, pure) = crate::exchange::detect_market(security_code);
        let mut code = [0u8; 6];
        let sym = pure.as_bytes();
        let copy_len = std::cmp::min(sym.len(), 6);
        code[..copy_len].copy_from_slice(&sym[..copy_len]);

        let mut header = RequestHeader::new();
        header.zip_flag = crate::level1::protocol::zlib_flag::UNCOMPRESSED;
        header.seq_id = sequence_id();
        header.packet_type = 0x00;
        header.method = commands::HISTORY_MINUTE_DATA;

        HistoryMinuteTimeRequest {
            header,
            date,
            market,
            code,
        }
    }

    pub fn date(&self) -> u32 {
        self.date
    }

    pub fn market(&self) -> u8 {
        self.market
    }

    pub fn code(&self) -> &[u8; 6] {
        &self.code
    }

    pub fn code_string(&self) -> String {
        let nul_pos = self
            .code
            .iter()
            .position(|&b| b == 0)
            .unwrap_or(self.code.len());
        String::from_utf8_lossy(&self.code[..nul_pos]).into_owned()
    }
}

impl Request for HistoryMinuteTimeRequest {
    fn header(&self) -> &RequestHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut RequestHeader {
        &mut self.header
    }

    fn serialize_payload(&mut self) -> Vec<u8> {
        let mut payload = BinaryStream::new();
        payload.push_u32(self.date);
        payload.push_u8(self.market);
        payload.push_byte_array(&self.code);
        payload.data().clone()
    }

    fn payload_string(&self) -> String {
        format!(
            "{{Date:{}, Market:{}, Code:{}}}",
            self.date,
            self.market,
            self.code_string()
        )
    }
}

pub fn fetch_history_minute_time(security_code: &str, date: u32) -> Option<MinuteTimeResponse> {
    match crate::level1::client::get_std_conn() {
        Ok(mut pooled) => {
            let mut request = HistoryMinuteTimeRequest::new(security_code, date);
            let mut response = MinuteTimeResponse::new_from_request(&request);
            match protocol::process(pooled.stream(), &mut request, &mut response) {
                Ok(_) => Some(response),
                Err(e) => {
                    log::error!(
                        "level1 protocol::process error for history_minute_time {} date {}: {}",
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
    header: ResponseHeader,
    pub count: u16,
    pub list: Vec<MinuteTime>,
    pub market_: i32,
    pub code_: String,
}

#[allow(dead_code)]
impl MinuteTimeResponse {
    pub fn new(market: i32, code: &str) -> Self {
        Self {
            header: ResponseHeader::new(),
            count: 0,
            list: Vec::new(),
            market_: market,
            code_: code.to_string(),
        }
    }

    pub fn new_from_request(req: &HistoryMinuteTimeRequest) -> Self {
        Self::new(req.market() as i32, &req.code_string())
    }

    fn clear(&mut self) {
        self.list.clear();
        self.count = 0;
    }
}

impl Response for MinuteTimeResponse {
    fn header(&self) -> &ResponseHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut ResponseHeader {
        &mut self.header
    }

    fn deserialize_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        self.clear();

        if data.len() < 2 {
            return Ok(());
        }

        let mut bs = BinaryStream::from_vec(data.to_vec());
        self.count = bs.get_u16()?;
        if self.count == 0 {
            return Ok(());
        }

        let min_required = 6 + (self.count as usize) * 3;
        if data.len() < min_required {
            log::warn!(
                "insufficient data for {} minute times: data len {}, min required {}",
                self.count,
                data.len(),
                min_required
            );
            self.count = 0;
            return Ok(());
        }

        self.list.reserve(self.count as usize);
        let base_unit = super::default_base_unit(self.market_, &self.code_);
        let _is_index =
            crate::exchange::assert_index_by_market_and_code(self.market_ as u8, &self.code_);
        let mut last_price: i64 = 0;
        bs.skip(4);
        for _ in 0..self.count {
            let mut entry = MinuteTime::new();
            let raw_price = bs.varint_decode()?;
            let _ignored = bs.varint_decode()?;
            let vol = bs.varint_decode()?;
            entry.vol = vol;
            last_price += raw_price;
            entry.price = (last_price as f32) / (base_unit as f32);
            self.list.push(entry);
        }
        Ok(())
    }

    fn body_string(&self) -> String {
        format!(
            "{{Count:{}, Market:{}, Code:{}}}",
            self.count, self.market_, self.code_
        )
    }
}
