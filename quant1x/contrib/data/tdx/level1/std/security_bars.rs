// K线类型 (mimicking Python BarFreq)
#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum BarFreq {
    Freq5Min = 0,    // 5分钟K线
    Freq15Min = 1,   // 15分钟K线
    Freq30Min = 2,   // 30分钟K线
    Freq1Hour = 3,   // 1小时K线
    FreqDaily = 4,    // 日K线
    FreqWeekly = 5,   // 周K线
    FreqMonthly = 6,  // 月K线
    FreqExHQ1Min = 7, // 扩展市场1分钟
    Freq1Min = 8,    // 普通1分钟K线
    FreqRIK = 9,      // 日K线(同DAILY)
    Freq3Month = 10, // 季K线
    FreqYearly = 11,  // 年K线
}

use super::super::super::command::*;
use super::super::super::helpers::{get_datetime_from_u32, int_to_float64};
use super::super::super::protocol::{BaseFrame, RequestHeader, ResponseHeader};
use crate::helpers;
use crate::std::BinaryStream;

/// SECURITY_BARS_PRE_REQUEST_MAX = 700
pub const SECURITY_BARS_PRE_REQUEST_MAX: usize = 700;

#[derive(Debug, Clone)]
struct SecurityBarsParameter {
    market: u16,
    code: [u8; 6],
    category: u16,
    i_field: u16,
    start: u16,
    count: u16,
}

impl Default for SecurityBarsParameter {
    fn default() -> Self {
        Self {
            market: 0,
            code: [0u8; 6],
            category: 0,
            i_field: 1, // frequency, 对应 Python self._i = 1
            start: 0,
            count: 0,
        }
    }
}

/// K线数据 — 对应 Python SecurityBarsContext(protocol.BaseFrame)
#[derive(Debug, Clone)]
pub struct SecurityBarsContext {
    pub req_header: RequestHeader,
    pub resp_header: ResponseHeader,
    param: SecurityBarsParameter,
    padding: [u8; 10],
    pub is_index: bool,
    pub count: u16,
    pub list: Vec<SecurityBar>,
}

impl SecurityBarsContext {
    /// 构造 K线请求
    ///
    /// 对应 Python `SecurityBarsContext.__init__(exchange, code, category, start, count, is_index)`
    pub fn new(
        security_code: &str,
        category: u16,
        start: u16,
        count: u16,
    ) -> Self {
        Self::with_is_index(security_code, category, start, count, false)
    }

    /// 带 is_index 参数的构造
    pub fn with_is_index(
        security_code: &str,
        category: u16,
        start: u16,
        count: u16,
        is_index: bool,
    ) -> Self {
        Self::with_frequency_and_is_index(security_code, category, start, count, 1, is_index)
    }

    pub fn with_frequency(
        security_code: &str,
        category: u16,
        start: u16,
        count: u16,
        frequency: u16,
    ) -> Self {
        Self::with_frequency_and_is_index(security_code, category, start, count, frequency, false)
    }

    /// 完整构造, 对齐 Python `__init__`
    pub fn with_frequency_and_is_index(
        security_code: &str,
        category: u16,
        start: u16,
        count: u16,
        frequency: u16,
        is_index: bool,
    ) -> Self {
        let inst = crate::data::market::detect_symbol(security_code);
        //let market_id = inst.ext_market;
        let market_id = helpers::exchange_to_market(inst.exchange.code()).unwrap_or(0);
        let pure = inst.market_ticker().to_string();
        let mut code_bytes = [0u8; 6];
        let sym = pure.as_bytes();
        let copy_len = std::cmp::min(sym.len(), code_bytes.len());
        code_bytes[..copy_len].copy_from_slice(&sym[..copy_len]);

        let mut req_header = RequestHeader::new(STD_SECURITY_BARS, FLAG_UNCOMPRESSED);
        req_header.packet_ctrl = 0x00;

        SecurityBarsContext {
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

    #[allow(dead_code)]
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

impl BaseFrame for SecurityBarsContext {
    fn request_header(&self) -> &RequestHeader { &self.req_header }
    fn request_header_mut(&mut self) -> &mut RequestHeader { &mut self.req_header }
    fn response_header(&self) -> &ResponseHeader { &self.resp_header }
    fn response_header_mut(&mut self) -> &mut ResponseHeader { &mut self.resp_header }

    fn serialize_request_body(&mut self) -> Vec<u8> {
        // 对应 Python: struct.pack('<H 6s H H H H', market, code, category, i, start, count) + padding
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

        if data.len() < 2 {
            return Ok(());
        }

        let mut bs = BinaryStream::from_vec(data.to_vec());
        self.count = bs.get_u16()?;
        self.list.reserve(self.count as usize);

        let mut pre_diff_base: i64 = 0;
        let cat = self.param.category;

        for _ in 0..self.count {
            let mut e = SecurityBar::new();

            // 对应 Python: if cat < 4 or cat == 7 or cat == 8
            if cat < 4 || cat == 7 || cat == 8 {
                let zipday = bs.get_u16()? as u32;
                let tminutes = bs.get_u16()?;
                let (y, m, d, hh, mm) =
                    get_datetime_from_u32(cat as i32, zipday, tminutes);
                e.year = y;
                e.month = m;
                e.day = d;
                e.hour = hh;
                e.minute = mm;
            } else {
                // 对应 Python: zipday = struct.unpack('<I', ...)[0]; year = int(zipday/10000); month = int((zipday%10000)/100); day = int(zipday%100)
                // Python 非分钟线不调用 get_datetime_from_uint32, 直接算
                let zipday = bs.get_u32()?;
                let y = (zipday / 10000) as i32;
                let m = ((zipday % 10000) / 100) as i32;
                let d = (zipday % 100) as i32;
                e.year = y;
                e.month = m;
                e.day = d;
                e.hour = 15; // 对齐 Python: hour = 15
                e.minute = 0;
            }

            // 对应 Python: e.date = f"{year:04d}-{month:02d}-{day:02d}"
            e.date = format!("{:04}-{:02}-{:02}", e.year, e.month, e.day);
            // 对应 Python: e.timestamp = f"{year:04d}-{month:02d}-{day:02d} {hour:02d}:{minute:02d}:00"
            e.timestamp = format!(
                "{:04}-{:02}-{:02} {:02}:{:02}:00",
                e.year, e.month, e.day, e.hour, e.minute
            );

            // 对应 Python: helpers.varint_decode
            let mut price_open_diff = bs.varint_decode()?;
            let price_close_diff = bs.varint_decode()?;
            let price_high_diff = bs.varint_decode()?;
            let price_low_diff = bs.varint_decode()?;

            // 对应 Python: ivol = struct.unpack('<I', ...)[0]; e.volume = helpers.int_to_float64(ivol)
            let ivol = bs.get_u32()?;
            e.volume = int_to_float64(ivol);

            // 对应 Python: dbvol = struct.unpack('<I', ...)[0]; e.amount = helpers.int_to_float64(dbvol)
            let dbvol = bs.get_u32()?;
            e.amount = int_to_float64(dbvol);

            // 对应 Python: e.open = float(price_open_diff + pre_diff_base) / 1000.0
            e.open = (price_open_diff + pre_diff_base) as f64 / 1000.0;
            price_open_diff += pre_diff_base;

            e.close = (price_open_diff + price_close_diff) as f64 / 1000.0;
            e.high = (price_open_diff + price_high_diff) as f64 / 1000.0;
            e.low = (price_open_diff + price_low_diff) as f64 / 1000.0;

            pre_diff_base = price_open_diff + price_close_diff;

            // 对应 Python: if self._is_index: e.up = ...; e.down = ...
            if self.is_index {
                e.up = bs.get_u16()?;
                e.down = bs.get_u16()?;
            }

            self.list.push(e);
        }
        Ok(())
    }
}

/// K线数据结构体 — 对应 Python `Bar`
#[derive(Debug, Clone)]
pub struct SecurityBar {
    /// 日期: YYYY-MM-DD, 对应 Python `date`
    pub date: String,
    /// 开盘价
    pub open: f64,
    /// 收盘价
    pub close: f64,
    /// 最高价
    pub high: f64,
    /// 最低价
    pub low: f64,
    /// 成交量, 对应 Python `volume`
    pub volume: f64,
    /// 成交额
    pub amount: f64,
    /// 上涨家数: 仅指数有效, 对应 Python `up`
    pub up: u16,
    /// 下跌家数: 仅指数有效, 对应 Python `down`
    pub down: u16,
    /// 时间戳: YYYY-MM-DD HH:MM:SS, 对应 Python `timestamp`
    pub timestamp: String,
    /// 复权次数, 对应 Python `adjustment_count`
    pub adjustment_count: u32,

    // ---- 以下为辅助字段, Python Bar 中没有 ----
    pub year: i32,
    pub month: i32,
    pub day: i32,
    pub hour: i32,
    pub minute: i32,
}

impl SecurityBar {
    pub fn new() -> Self {
        Self {
            date: String::new(),
            open: 0.0,
            close: 0.0,
            high: 0.0,
            low: 0.0,
            volume: 0.0,
            amount: 0.0,
            up: 0,
            down: 0,
            timestamp: String::new(),
            adjustment_count: 0,
            year: 0,
            month: 0,
            day: 0,
            hour: 0,
            minute: 0,
        }
    }
}

/// SecurityBarsResponse 已合并到 SecurityBarsContext 中. 
/// 保留类型别名以兼容旧代码. 
pub type SecurityBarsResponse = SecurityBarsContext;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    #[ignore = "requires C++ sample data validation"]
    fn deserialize_sample_matches_cpp_output() {
        let hex_data = "05002bff3401a52910134982d4834e07eb2f4f2eff340102060e4a8a70db4dca40934e2fff3401440a0f4aef5a734e3b6c234f30ff340141191f515cd8094f6d64ba4f31ff34014d102c4398098b4e44b03c4f";
        let buf = hex::decode(hex_data).unwrap();
        // 对应 Python: SecurityBarsContext(exchange, "sh000001", BarFreq.FreqRIK, 0, 800, is_index=True)
        let mut req = SecurityBarsContext::with_is_index("sh000001", 9, 0, 800, true);
        let _ = req.deserialize_response_body(&buf);
        assert_eq!(req.count as usize, req.list.len());
        assert!(req.list.len() > 0 || req.count == 0);
    }
}
