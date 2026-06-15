// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// kline_raw — 未复权K线数据适配器，与 Python contrib/data/tdx/kline_raw.py 对齐
//
// 不依赖 crate::contrib::data::tdx::standard，所有类型均使用 tdx/ 本地模块定义。

use std::sync::Arc;
use serde::{Deserialize, Serialize};

use crate::data::adapter::DataAdapter;
use crate::data::meta::instrument::Instrument;
use crate::data::meta::Timestamp;
use crate::data::{BaseRawDailyKLine};
use super::command::{EXT_INSTRUMENT_BARS, FLAG_GENERIC};
use super::helpers::msg_sequence_id;
use super::level1::std::security_bars::{SecurityBarsRequest, SecurityBar as StdSecurityBar};
use super::protocol::{BaseMessage, RequestHeader, ResponseHeader};
use crate::std::BinaryStream;

/// 日线增量更新时丢弃的缓存天数，与 Python MaxCachedDaysToDropOnIncrementalUpdate 对齐
const MAX_CACHED_DAYS_TO_DROP: usize = 1;

/// 每页请求的最大K线数量，与 Python SECURITY_BARS_PRE_REQUEST_MAX 对齐
const SECURITY_BARS_PRE_REQUEST_MAX: usize = 800;

// ============================================================
// SecurityBar — 本地定义的K线Bar（不依赖 crate::level1）
// ============================================================

/// 单根K线数据，与 Python level1/__init__.py 的 SecurityBar 对齐
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
    pub up_count: i32,
    pub down_count: i32,
}

/// SecurityBars 响应包装（本地定义）
#[derive(Debug, Clone)]
pub struct SecurityBarsResponse {
    pub count: u16,
    pub list: Vec<SecurityBar>,
    pub is_index: bool,
    pub category: u16,
}

impl SecurityBarsResponse {
    pub fn new() -> Self {
        Self { count: 0, list: Vec::new(), is_index: false, category: 0 }
    }
}

// ============================================================
// KLineType — 本地定义，不依赖 crate::contrib::data::tdx::standard
// ============================================================

/// K线类型，与 Python level1/__init__.py 的 KLineType 对齐
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[allow(non_camel_case_types)]
pub enum KLineType {
    _1Min,
    _5Min,
    _15Min,
    _30Min,
    _1Hour,
    Daily,
    Weekly,
    Monthly,
    _3Month,
    Yearly,
    Exhq1Min,
    RiK,
}

/// 将 KLineType 转换为频率值（用于 InstrumentBars）
fn kline_type_to_value(kline_type: KLineType) -> u16 {
    match kline_type {
        KLineType::_1Min => 8,
        KLineType::_5Min => 0,
        KLineType::_15Min => 1,
        KLineType::_30Min => 2,
        KLineType::_1Hour => 3,
        KLineType::Daily | KLineType::RiK => 4,
        KLineType::Weekly => 5,
        KLineType::Monthly => 6,
        KLineType::_3Month => 10,
        KLineType::Yearly => 11,
        KLineType::Exhq1Min => 7,
    }
}

// ============================================================
// InstrumentBars — 扩展行情K线请求/响应
// 对应 Python level1/ext.py 的 InstrumentBars
// ============================================================

/// 扩展行情K线请求（同时承载响应数据，与 Python InstrumentBars 设计一致）
/// 命令字: EXT_INSTRUMENT_BARS (0x23ff)
#[derive(Debug, Clone)]
pub struct InstrumentBars {
    pub req_header: RequestHeader,
    pub resp_header: ResponseHeader,
    pub market: u8,
    pub ticker: String,
    pub category: u16,
    pub frequency: u16,
    pub start: u32,
    pub count: u16,
    /// 响应数据 — 对应 Python bars.reply
    pub reply: Vec<SecurityBar>,
}

impl BaseMessage for InstrumentBars {
    fn request_header(&self) -> &RequestHeader {
        &self.req_header
    }
    fn request_header_mut(&mut self) -> &mut RequestHeader {
        &mut self.req_header
    }
    fn response_header(&self) -> &ResponseHeader {
        &self.resp_header
    }
    fn response_header_mut(&mut self) -> &mut ResponseHeader {
        &mut self.resp_header
    }

    fn serialize_request_body(&mut self) -> Vec<u8> {
        let ticker_bytes = self.ticker.as_bytes();
        let mut ticker_padded = [0u8; 9];
        let len = ticker_bytes.len().min(9);
        ticker_padded[..len].copy_from_slice(&ticker_bytes[..len]);

        let mut bs = BinaryStream::new();
        bs.push_u8(self.market);
        bs.push_byte_array(&ticker_padded);
        bs.push_u16(self.category);
        bs.push_u16(self.frequency);
        bs.push_u32(self.start);
        bs.push_u16(self.count);
        bs.data().clone()
    }

    fn deserialize_response_body(&mut self, data: &[u8]) -> Result<(), crate::std::DeserializeError> {
        // 与 Python InstrumentBars.deserialize_response_body 对齐:
        //   body = data[14:]  # 跳过前14字节
        //   start, count = struct.unpack("<IH", body[:6])
        //   for each bar: 28 bytes = <ffffIIf
        if data.len() < 20 {
            return Ok(());
        }
        let mut pos: usize = 14;
        if pos + 6 > data.len() {
            return Ok(());
        }
        let resp_start = u32::from_le_bytes([data[pos], data[pos+1], data[pos+2], data[pos+3]]);
        let resp_count = u16::from_le_bytes([data[pos+4], data[pos+5]]);
        pos += 6;

        self.start = resp_start;
        self.count = resp_count;

        let count_usize = resp_count as usize;
        let record_size: usize = 28;
        if pos + count_usize * record_size > data.len() {
            log::warn!(
                "[InstrumentBars] body too short for {} records (available={}, needed={})",
                resp_count,
                data.len() - pos,
                count_usize * record_size
            );
            return Ok(());
        }

        for _ in 0..count_usize {
            if pos + record_size > data.len() {
                break;
            }
            // 日期: u32 YYYYMMDD (日线/周线/月线)
            let zipday = u32::from_le_bytes([data[pos], data[pos+1], data[pos+2], data[pos+3]]);
            let year = (zipday / 10000) as i32;
            let month = ((zipday % 10000) / 100) as i32;
            let day = (zipday % 100) as i32;
            pos += 4;

            // OHLC: 4 × f32 = 16 bytes
            let open = f32::from_le_bytes([data[pos], data[pos+1], data[pos+2], data[pos+3]]) as f64;
            let high = f32::from_le_bytes([data[pos+4], data[pos+5], data[pos+6], data[pos+7]]) as f64;
            let low = f32::from_le_bytes([data[pos+8], data[pos+9], data[pos+10], data[pos+11]]) as f64;
            let close = f32::from_le_bytes([data[pos+12], data[pos+13], data[pos+14], data[pos+15]]) as f64;
            pos += 16;

            // position(u32) + volume(u32) + price(f32) = 12 bytes
            let _position = u32::from_le_bytes([data[pos], data[pos+1], data[pos+2], data[pos+3]]);
            let volume = u32::from_le_bytes([data[pos+4], data[pos+5], data[pos+6], data[pos+7]]);
            let _price = f32::from_le_bytes([data[pos+8], data[pos+9], data[pos+10], data[pos+11]]);
            pos += 12;

            let datetime = format!("{:04}-{:02}-{:02} 15:00:00", year, month, day);

            self.reply.push(SecurityBar {
                open, close, high, low,
                vol: volume as f64,
                amount: 0.0,
                year, month, day,
                hour: 15,
                minute: 0,
                datetime,
                up_count: 0,
                down_count: 0,
            });
        }
        Ok(())
    }
}

impl InstrumentBars {
    pub fn new(market: u8, ticker: &str, category: u16, start: u32, count: u16) -> Self {
        Self {
            req_header: RequestHeader::new(EXT_INSTRUMENT_BARS, FLAG_GENERIC),
            resp_header: ResponseHeader::new(),
            market,
            ticker: ticker.to_string(),
            category,
            frequency: 1,
            start,
            count,
            reply: Vec::new(),
        }
    }
}

// ============================================================
// 文件路径 & 缓存
// ============================================================

#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct KLineRaw {
    #[serde(rename = "date")]
    pub date: String,
    #[serde(rename = "open")]
    pub open: f64,
    #[serde(rename = "close")]
    pub close: f64,
    #[serde(rename = "high")]
    pub high: f64,
    #[serde(rename = "low")]
    pub low: f64,
    #[serde(rename = "volume")]
    pub volume: f64,
    #[serde(rename = "amount")]
    pub amount: f64,
    #[serde(rename = "up")]
    pub up: i32,
    #[serde(rename = "down")]
    pub down: i32,
    #[serde(rename = "datetime")]
    pub datetime: String,
}

impl KLineRaw {
    pub fn headers() -> Vec<String> {
        vec![
            "date".into(),
            "open".into(),
            "close".into(),
            "high".into(),
            "low".into(),
            "volume".into(),
            "amount".into(),
            "up".into(),
            "down".into(),
            "datetime".into(),
        ]
    }
}

/// 生成原始K线缓存文件路径，与 Python get_kline_raw_filename 对齐
pub fn get_kline_raw_filename(inst: &Instrument) -> String {
    let symbol = inst.symbol();
    let sub = format!("day_raw/{}", inst.cache_dir());
    format!("{}/{}/{}.raw", crate::config::default_cache_path(), sub, symbol)
}

/// 从缓存文件加载原始K线数据
pub fn load_kline_raw(inst: &Instrument) -> Vec<KLineRaw> {
    let cache_filename = get_kline_raw_filename(inst);
    let mut klines: Vec<KLineRaw> = Vec::new();
    match std::fs::File::open(&cache_filename) {
        Ok(f) => {
            let mut rdr = csv::ReaderBuilder::new().has_headers(true).from_reader(f);
            match rdr.deserialize::<KLineRaw>().collect::<Result<Vec<KLineRaw>, csv::Error>>() {
                Ok(v) => klines = v,
                Err(e) => log::error!("[kline_raw] failed to deserialize {}: {}", cache_filename, e),
            }
        }
        Err(_) => {}
    }
    klines
}

/// 确保原始K线数据已更新
pub fn ensure_kline_raw_updated(inst: &Instrument) {
    let adapter = DataKLineRaw;
    adapter.update(inst, Timestamp::now());
}

/// 获取未复权K线数据，如果缓存不存在或过期则先更新
pub fn checkout_kline_raw(inst: &Instrument) -> Vec<KLineRaw> {
    ensure_kline_raw_updated(inst);
    load_kline_raw(inst)
}

/// 保存原始K线数据到CSV文件
fn save_kline_raw(filename: &str, values: &[KLineRaw]) {
    if values.is_empty() {
        return;
    }
    if let Some(parent) = std::path::Path::new(filename).parent() {
        if let Err(e) = std::fs::create_dir_all(parent) {
            log::error!("[kline_raw] create_dir_all failed for {:?}: {}", parent, e);
            return;
        }
    }
    let tmp = format!("{}.tmp", filename);
    match std::fs::File::create(&tmp) {
        Ok(f) => {
            let mut w = csv::Writer::from_writer(f);
            if let Err(e) = w.write_record(KLineRaw::headers()) {
                log::error!("[kline_raw] write header failed: {}", e);
            }
            for row in values.iter() {
                let rec: Vec<String> = vec![
                    row.date.clone(),
                    row.open.to_string(),
                    row.close.to_string(),
                    row.high.to_string(),
                    row.low.to_string(),
                    row.volume.to_string(),
                    row.amount.to_string(),
                    row.up.to_string(),
                    row.down.to_string(),
                    row.datetime.clone(),
                ];
                if let Err(e) = w.write_record(rec) { log::error!("[kline_raw] write row failed: {}", e); }
            }
            let _ = w.flush();
            if let Err(e) = std::fs::rename(&tmp, filename) {
                log::error!("[kline_raw] rename failed {} -> {}: {}", tmp, filename, e);
            }
        }
        Err(e) => log::error!("[kline_raw] create tmp {} failed: {}", tmp, e),
    }
}

// ============================================================
// fetch_kline_raw — 根据交易所类型分发
// ============================================================

/// 根据交易所类型分发到标准行情或扩展行情获取原始K线
/// 对应 Python fetch_kline_raw(inst, start, count, freq)
pub fn fetch_kline_raw(
    inst: &Instrument,
    start: u32,
    count: u16,
) -> Option<SecurityBarsResponse> {
    if inst.exchange.is_std_quote() {
        fetch_kline_raw_from_std(inst, start, count)
    } else if inst.exchange.is_ext_quote() {
        fetch_kline_raw_from_ext(inst, start, count)
    } else {
        None
    }
}


/// 从标准行情获取原始K线
/// 对应 Python kline_raw.py fetch_kline_raw_from_std:
///   msg = SecurityBars(inst.exchange, inst.ticker, kline_type, start, count, inst.type.is_index())
/// 使用 STD_SECURITY_BARS (0x052d) 命令，通过标准行情连接获取
fn fetch_kline_raw_from_std(
    inst: &Instrument,
    start: u32,
    count: u16,
) -> Option<SecurityBarsResponse> {
    let code = inst.code();
    let ticker = code.to_uppercase();
    let category = kline_type_to_value(KLineType::Daily);

    match super::client::get_std_conn() {
        Ok(mut conn) => {
            // 使用 SecurityBarsRequest (STD_SECURITY_BARS, 0x052d)，不是 InstrumentBars (EXT_INSTRUMENT_BARS, 0x23ff)
            let symbol = format!("{}{}", inst.exchange.identifier(), ticker);
            let mut bars = SecurityBarsRequest::with_is_index(
                &symbol,
                category,
                start as u16,
                count,
                inst.instrument_type.is_index(),
            );
            match super::protocol::process_level1_stream(conn.stream(), &mut bars) {
                Ok(()) => {
                    log::debug!("[kline_raw] fetch_kline_raw_from_std: {} bars for {}", bars.list.len(), inst.symbol());
                    let list: Vec<SecurityBar> = bars.list.into_iter().map(|b| SecurityBar {
                        open: b.open,
                        close: b.close,
                        high: b.high,
                        low: b.low,
                        vol: b.volume,
                        amount: b.amount,
                        year: b.year,
                        month: b.month,
                        day: b.day,
                        hour: b.hour,
                        minute: b.minute,
                        datetime: b.timestamp,
                        up_count: b.up as i32,
                        down_count: b.down as i32,
                    }).collect();
                    Some(SecurityBarsResponse {
                        count: list.len() as u16,
                        list,
                        is_index: inst.instrument_type.is_index(),
                        category,
                    })
                }
                Err(e) => {
                    log::error!("[kline_raw] fetch_kline_raw_from_std failed for {}: {}", inst.symbol(), e);
                    None
                }
            }
        }
        Err(e) => {
            log::error!("[kline_raw] get_std_conn failed: {}", e);
            None
        }
    }
}

/// 从扩展行情获取原始K线（港股/美股等）
/// 复用 client::get_ext_conn() 连接池
fn fetch_kline_raw_from_ext(
    inst: &Instrument,
    start: u32,
    count: u16,
) -> Option<SecurityBarsResponse> {
    let code = inst.code();
    let ticker = code.to_uppercase();
    let category = kline_type_to_value(KLineType::Daily);

    match super::client::get_ext_conn() {
        Ok(mut conn) => {
            let mut bars = InstrumentBars::new(inst.ext_market as u8, &ticker, category, start, count);
            match super::protocol::process_level1_stream(conn.stream(), &mut bars) {
                Ok(()) => {
                    log::debug!("[kline_raw] fetch_kline_raw_from_ext: {} bars for {}", bars.reply.len(), inst.symbol());
                    Some(SecurityBarsResponse {
                        count: bars.reply.len() as u16,
                        list: bars.reply,
                        is_index: inst.instrument_type.is_index(),
                        category,
                    })
                }
                Err(e) => {
                    log::error!("[kline_raw] fetch_kline_raw_from_ext failed for {}: {}", inst.symbol(), e);
                    None
                }
            }
        }
        Err(e) => {
            log::error!("[kline_raw] get_ext_conn failed: {}", e);
            None
        }
    }
}

// ============================================================
// DataKLineRaw — 未复权日K线数据适配器
// ============================================================

#[derive(Debug)]
pub struct DataKLineRaw;

impl crate::data::Schema for DataKLineRaw {
    fn kind(&self) -> crate::data::Kind { BaseRawDailyKLine }
    fn owner(&self) -> String { crate::data::DEFAULT_DATA_PROVIDER.to_string() }
    fn key(&self) -> String { "day_raw".to_string() }
    fn name(&self) -> String { "日K线RAW".to_string() }
    fn usage(&self) -> String { "日K线RAW数据适配器".to_string() }
}

impl DataAdapter for DataKLineRaw {
    fn print(&self, _inst: &Instrument, _dates: &[Timestamp]) {}

    fn update(&self, inst: &Instrument, _date: Timestamp) {
        let symbol = inst.symbol();

        // 1. 从本地缓存确定起始日期
        let cache_filename = get_kline_raw_filename(inst);
        let cache_klines = load_kline_raw(inst);

        let klines_length = cache_klines.len();
        let mut klines_offset_days = MAX_CACHED_DAYS_TO_DROP;
        let mut current_start_date =
            Timestamp::pre_market_time(1990, 12, 19).unwrap_or(Timestamp::zero());

        if klines_length > 0 {
            if klines_offset_days > klines_length {
                klines_offset_days = klines_length;
            }
            let kline = &cache_klines[klines_length - klines_offset_days];
            if let Ok(ts) = Timestamp::parse(&kline.date) {
                current_start_date = ts;
            }
        }

        // 2. 确定结束日期
        let current_end_date =
            Timestamp::pre_market_time_from_current(&Timestamp::now()).unwrap_or(Timestamp::now());

        log::debug!(
            "[DataKLineRaw] [{}]: from {} to {}",
            symbol, current_start_date.only_date(), current_end_date.only_date()
        );

        // 3. 分页拉取数据
        let step = SECURITY_BARS_PRE_REQUEST_MAX;
        let mut start: u32 = 0;
        let mut hs: Vec<Vec<SecurityBar>> = Vec::new();

        loop {
            let count = std::cmp::min(step, u16::MAX as usize) as u16;

            let reply = if inst.exchange.is_std_quote() {
                fetch_kline_raw_from_std(inst, start, count)
            } else if inst.exchange.is_ext_quote() {
                fetch_kline_raw_from_ext(inst, start, count)
            } else {
                None
            };

            match reply {
                Some(resp) => {
                    let resp_len = resp.list.len();
                    if resp_len == 0 { break; }
                    let last_bar_is_before_start = resp.list.last().map_or(false, |last_bar| {
                        Timestamp::pre_market_time(last_bar.year, last_bar.month as u32, last_bar.day as u32)
                            .map_or(false, |last_ts| last_ts < current_start_date)
                    });
                    hs.push(resp.list);
                    if last_bar_is_before_start { break; }
                    if resp_len < count as usize { break; }
                    start += count as u32;
                }
                None => break,
            }
        }

        // 4. 反转页面（时间升序）
        hs.reverse();

        // 5. 构建增量K线列表
        let mut incremental_klines: Vec<KLineRaw> = Vec::new();
        for page in hs.iter() {
            for row in page.iter() {
                if let Some(date_time) = Timestamp::pre_market_time(row.year, row.month as u32, row.day as u32) {
                    if date_time < current_start_date || date_time > current_end_date {
                        continue;
                    }
                    let kx = KLineRaw {
                        date: date_time.only_date(),
                        open: row.open,
                        close: row.close,
                        high: row.high,
                        low: row.low,
                        volume: row.vol * 100.0,
                        amount: row.amount,
                        up: row.up_count,
                        down: row.down_count,
                        datetime: row.datetime.clone(),
                    };
                    incremental_klines.push(kx);
                }
            }
        }

        // 6. 合并缓存和增量数据
        let mut klines: Vec<KLineRaw> = Vec::new();
        if klines_length > klines_offset_days {
            klines.extend_from_slice(&cache_klines[..(klines_length - klines_offset_days)]);
        }
        klines.extend(incremental_klines);

        // 7. 保存
        save_kline_raw(&cache_filename, &klines);
    }
}

/// 初始化并注册 DataKLineRaw 插件
pub fn init() {
    let plugin = Arc::new(DataKLineRaw) as Arc<dyn crate::data::DataAdapter>;
    crate::data::register(plugin);
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::data::market::detect_symbol;

    #[test]
    #[ignore = "requires config file and network"]
    fn test_kline_raw_update() {
        let adapter = DataKLineRaw;
        let code = "sh600000";
        let inst = detect_symbol(code);
        assert!(inst.can_construct_symbol());
        adapter.update(&inst, Timestamp::now());
    }

    #[test]
    #[ignore = "requires config file and network"]
    fn test_kline_raw_update_hk() {
        let adapter = DataKLineRaw;
        let code = "00700.hk";
        let inst = detect_symbol(code);
        assert!(inst.can_construct_symbol());
        adapter.update(&inst, Timestamp::now());
    }
}
