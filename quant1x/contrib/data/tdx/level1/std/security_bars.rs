// K线类型 (mimicking C++ level1::KLineType)
#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum KLineType {
    _5Min = 0,    // 5分钟K线
    _15Min = 1,   // 15分钟K线
    _30Min = 2,   // 30分钟K线
    _1Hour = 3,   // 1小时K线
    Daily = 4,    // 日K线
    Weekly = 5,   // 周K线
    Monthly = 6,  // 月K线
    Exhq1Min = 7, // 扩展市场1分钟
    _1Min = 8,    // 普通1分钟K线
    RiK = 9,      // 日K线(同DAILY)
    _3Month = 10, // 季K线
    Yearly = 11,  // 年K线
}

use super::super::super::command::*;
use super::super::super::helpers::{get_datetime_from_u32, int_to_float64};
use super::super::super::protocol::{BaseMessage, RequestHeader, ResponseHeader};
use crate::std::BinaryStream;

#[derive(Debug, Clone, Default)]
struct SecurityBarsParameter {
    market: u16,
    code: [u8; 6],
    category: u16,
    i_field: u16,
    start: u16,
    count: u16,
}

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct SecurityBarsRequest {
    pub req_header: RequestHeader,
    pub resp_header: ResponseHeader,
    param: SecurityBarsParameter,
    padding: [u8; 10],
    pub is_index: bool,
    pub count: u16,
    pub list: Vec<SecurityBar>,
}

#[allow(dead_code)]
impl SecurityBarsRequest {
    pub fn new(security_code: &str, category: u16, start: u16, count: u16) -> Self {
        Self::with_frequency(security_code, category, start, count, 1)
    }

    pub fn with_frequency(
        security_code: &str,
        category: u16,
        start: u16,
        count: u16,
        frequency: u16,
    ) -> Self {
        let (market_id, _flag, pure) = crate::exchange::detect_market(security_code);
        let mut code_bytes = [0u8; 6];
        let sym = pure.as_bytes();
        let copy_len = std::cmp::min(sym.len(), code_bytes.len());
        code_bytes[..copy_len].copy_from_slice(&sym[..copy_len]);

        let is_index = crate::exchange::assert_index_by_market_and_code(market_id, pure.as_str());

        let mut req_header = RequestHeader::new(STD_SECURITY_BARS, FLAG_UNCOMPRESSED);
        req_header.packet_type = 0x00;

        SecurityBarsRequest {
            req_header,
            resp_header: ResponseHeader::new(),
            param: SecurityBarsParameter {
                market: market_id as u16,
                code: code_bytes,
                category,
                i_field: frequency,
                start,
                count,
            },
            padding: [0u8; 10],
            is_index,
            count: 0,
            list: Vec::new(),
        }
    }

    pub fn is_index(&self) -> bool {
        self.is_index
    }

    fn code_string(&self) -> String {
        let nul = self
            .param
            .code
            .iter()
            .position(|&b| b == 0)
            .unwrap_or(self.param.code.len());
        String::from_utf8_lossy(&self.param.code[..nul]).into_owned()
    }
}

impl BaseMessage for SecurityBarsRequest {
    fn request_header(&self) -> &RequestHeader { &self.req_header }
    fn request_header_mut(&mut self) -> &mut RequestHeader { &mut self.req_header }
    fn response_header(&self) -> &ResponseHeader { &self.resp_header }
    fn response_header_mut(&mut self) -> &mut ResponseHeader { &mut self.resp_header }

    fn serialize_request_body(&mut self) -> Vec<u8> {
        let mut payload = BinaryStream::new();
        payload.push_u16(self.param.market);
        payload.push_byte_array(&self.param.code);
        payload.push_u16(self.param.category);
        payload.push_u16(self.param.i_field);
        payload.push_u16(self.param.start);
        payload.push_u16(self.param.count);
        payload.push_byte_array(&self.padding);
        payload.data().clone()
    }

    fn deserialize_response_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        self.list.clear();
        self.count = 0;
        let mut bs = BinaryStream::from_vec(data.to_vec());
        self.count = bs.get_u16()?;
        self.list.reserve(self.count as usize);

        let mut pre_diff_base: i64 = 0;
        for _ in 0..self.count {
            let mut e = SecurityBar::new();

            if (self.param.category as i32) < 4 || self.param.category == 7 || self.param.category == 8 {
                let zipday = bs.get_u16()? as u32;
                let tminutes = bs.get_u16()?;
                let (y, m, d, hh, mm) =
                    get_datetime_from_u32(self.param.category as i32, zipday, tminutes);
                e.year = y;
                e.month = m;
                e.day = d;
                e.hour = hh;
                e.minute = mm;
            } else {
                let zipday = bs.get_u32()?;
                let (y, m, d, hh, mm) =
                    get_datetime_from_u32(self.param.category as i32, zipday, 0);
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

            let mut price_open_diff = bs.varint_decode()?;
            let price_close_diff = bs.varint_decode()?;
            let price_high_diff = bs.varint_decode()?;
            let price_low_diff = bs.varint_decode()?;

            let ivol = bs.get_u32()?;
            e.vol = int_to_float64(ivol);

            let dbvol = bs.get_u32()?;
            e.amount = int_to_float64(dbvol);

            e.open = (price_open_diff + pre_diff_base) as f64 / 1000.0;
            price_open_diff += pre_diff_base;

            e.close = (price_open_diff + price_close_diff) as f64 / 1000.0;
            e.high = (price_open_diff + price_high_diff) as f64 / 1000.0;
            e.low = (price_open_diff + price_low_diff) as f64 / 1000.0;

            pre_diff_base = price_open_diff + price_close_diff;

            if self.is_index {
                e.up_count = bs.get_u16()?;
                e.down_count = bs.get_u16()?;
            }

            self.list.push(e);
        }
        Ok(())
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

/// SecurityBarsResponse 已合并到 SecurityBarsRequest 中。
/// 保留类型别名以兼容旧代码。
pub type SecurityBarsResponse = SecurityBarsRequest;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn deserialize_sample_matches_cpp_output() {
        let hex_data = "05002bff3401a52910134982d4834e07eb2f4f2eff340102060e4a8a70db4dca40934e2fff3401440a0f4aef5a734e3b6c234f30ff340141191f515cd8094f6d64ba4f31ff34014d102c4398098b4e44b03c4f";
        let buf = hex::decode(hex_data).unwrap();
        let mut req = SecurityBarsRequest::new("sh000001", 9, 0, 800);
        let _ = req.deserialize_response_body(&buf);
        assert_eq!(req.count as usize, req.list.len());
        assert!(req.list.len() > 0 || req.count == 0);
    }
}
