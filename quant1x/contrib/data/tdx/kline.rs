// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// kline — 前复权K线数据缓存读取, 与 Python contrib/data/tdx/kline.py 对齐
// 作为 datasource 的本地代理, 仅从本地缓存CSV文件读取数据, 不依赖 level1 协议

use std::sync::Arc;

use crate::data::adapter::DataAdapter;
use crate::data::meta::calendar::next_trading_day;
use crate::data::meta::exchange::Exchange;
use crate::data::meta::instrument::{Instrument, InstrumentType};
use crate::data::meta::Timestamp;
use crate::data::schema::{Bar, CumulativeAdjustment, XdxrInfo};
use crate::data::{BaseKLine, DEFAULT_DATA_PROVIDER};

/// 中国股市首日上市日期
const MARKET_CN_FIRST_LIST_TIME: &str = "1990-12-19";

/// 每页请求的最大K线数量, 与 Python SECURITY_BARS_PRE_REQUEST_MAX 对齐
const SECURITY_BARS_PRE_REQUEST_MAX: usize = 700;

/// 日线增量更新时丢弃的缓存天数
const MAX_CACHED_DAYS_TO_DROP: usize = 1;

/// 获取前复权K线缓存文件路径
/// 与 Python get_kline_filename(inst, freq=FREQ_DAILY) 对齐:
///   module_name = freq.cache_key()  # "day"
///   symbol = inst.symbol()
///   sub = f"{module_name}/{inst.cache_dir()}"
///   return f'{config.data_path}/{sub}/{symbol}.csv'
fn get_kline_filename(inst: &Instrument) -> String {
    let symbol = inst.symbol();
    let sub = format!("day/{}", inst.cache_dir());
    format!(
        "{}/{}/{}.csv",
        crate::config::default_cache_path(),
        sub,
        symbol
    )
}

/// 从CSV缓存文件加载前复权K线数据
/// 与 Python read_kline_from_csv(filename) 对齐
fn read_kline_from_csv(filename: &str) -> Vec<Bar> {
    let mut klines: Vec<Bar> = Vec::new();
    match std::fs::File::open(filename) {
        Ok(f) => {
            let mut rdr = csv::ReaderBuilder::new().has_headers(true).from_reader(f);
            for result in rdr.records() {
                if let Ok(rec) = result {
                    klines.push(Bar {
                        date: rec.get(0).unwrap_or("").to_string(),
                        open: rec.get(1).and_then(|s| s.parse().ok()).unwrap_or(0.0),
                        close: rec.get(2).and_then(|s| s.parse().ok()).unwrap_or(0.0),
                        high: rec.get(3).and_then(|s| s.parse().ok()).unwrap_or(0.0),
                        low: rec.get(4).and_then(|s| s.parse().ok()).unwrap_or(0.0),
                        volume: rec.get(5).and_then(|s| s.parse().ok()).unwrap_or(0.0),
                        amount: rec.get(6).and_then(|s| s.parse().ok()).unwrap_or(0.0),
                        up: rec.get(7).and_then(|s| s.parse().ok()).unwrap_or(0),
                        down: rec.get(8).and_then(|s| s.parse().ok()).unwrap_or(0),
                        timestamp: rec.get(9).unwrap_or("").to_string(),
                        adjustment_count: rec.get(10).and_then(|s| s.parse().ok()).unwrap_or(0),
                    });
                }
            }
        }
        Err(_) => { /* 文件不存在, 返回空列表 */ }
    }
    klines
}

/// 从缓存文件加载前复权K线数据
/// 与 Python load_kline(inst, freq=FREQ_DAILY) 对齐
pub fn load_kline(inst: &Instrument) -> Vec<Bar> {
    let filename = get_kline_filename(inst);
    log::debug!("[kline] kline file: {}", filename);
    read_kline_from_csv(&filename)
}

/// 保存前复权K线数据到CSV文件
/// 与 Python save_kline(filename, values) 对齐
fn save_kline(filename: &str, values: &[Bar]) {
    if values.is_empty() {
        return;
    }
    if let Some(parent) = std::path::Path::new(filename).parent() {
        if let Err(e) = std::fs::create_dir_all(parent) {
            log::error!("[kline] create_dir_all failed for {:?}: {}", parent, e);
            return;
        }
    }
    let tmp = format!("{}.tmp", filename);
    match std::fs::File::create(&tmp) {
        Ok(f) => {
            let mut w = csv::Writer::from_writer(f);
            if let Err(e) = w.write_record(Bar::headers()) {
                log::error!("[kline] write header failed: {}", e);
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
                    row.timestamp.clone(),
                    row.adjustment_count.to_string(),
                ];
                if let Err(e) = w.write_record(rec) {
                    log::error!("[kline] write row failed: {}", e);
                }
            }
            let _ = w.flush();
            if let Err(e) = std::fs::rename(&tmp, filename) {
                log::error!("[kline] rename failed {} -> {}: {}", tmp, filename, e);
            }
        }
        Err(e) => log::error!("[kline] create tmp {} failed: {}", tmp, e),
    }
}

// ============================================================
// 前复权逻辑 — 与 Python kline.py 对齐
// ============================================================

/// 对K线数据进行前复权处理(事件驱动模式)
/// 与 Python apply_forward_adjustment_for_event(klines, current_start_date, dividends) 对齐
fn apply_forward_adjustment_for_event(
    klines: &mut [Bar],
    current_start_date: &Timestamp,
    dividends: &[XdxrInfo],
) {
    if klines.is_empty() {
        return;
    }

    // 最后一根K线的日期
    let last_day = &klines[klines.len() - 1].date;
    let ts_last_day = match Timestamp::parse(last_day) {
        Ok(ts) => match ts.pre_market_time_from_current() {
            Some(t) => t,
            None => return,
        },
        Err(_) => return,
    };
    // 最后一个交易日的下一个交易日
    let last_trading_day_ts = next_trading_day(ts_last_day);
    let last_day_next = last_trading_day_ts.only_date();
    let start_date_str = current_start_date.only_date();

    // 过滤 category == 1 的除权除息记录, 且日期 <= last_day_next
    let xdxr_infos: Vec<&XdxrInfo> = dividends
        .iter()
        .filter(|x| x.date <= last_day_next && x.category == 1)
        .collect();

    for info in xdxr_infos.iter() {
        if info.date <= start_date_str {
            // IPO 之前的除权记录跳过
            continue;
        }

        let (m, a) = info.adjust_factor();
        let share_ratio = info.compute_share_adjustment_ratio();

        for kline in klines.iter_mut() {
            if kline.date >= info.date {
                break;
            }

            kline.open = kline.open * m + a;
            kline.close = kline.close * m + a;
            kline.high = kline.high * m + a;
            kline.low = kline.low * m + a;

            if kline.volume != 0.0 {
                let ap = kline.amount / kline.volume;
                let ap_adjusted = ap * m + a;
                kline.volume *= 1.0 + share_ratio;
                kline.amount = kline.volume * ap_adjusted;
            }

            kline.adjustment_count += 1;
        }
    }
}

/// 从原始K线响应转换为 Bar 列表
/// 对应 Python 中 fetch_kline_raw 返回 List[Bar] 后的转换逻辑
fn fetch_kline_raw_as_bars(inst: &Instrument, start: u32, count: u16) -> Vec<Bar> {
    let resp = match super::kline_raw::fetch_kline_raw(inst, start, count) {
        Some(r) => r,
        None => return Vec::new(),
    };

    resp.list
        .iter()
        .map(|b| Bar {
            date: format!("{:04}-{:02}-{:02}", b.year, b.month, b.day),
            open: b.open,
            close: b.close,
            high: b.high,
            low: b.low,
            volume: b.vol * 100.0,
            amount: b.amount,
            up: b.up_count,
            down: b.down_count,
            timestamp: b.datetime.clone(),
            adjustment_count: 0,
        })
        .collect()
}

/// 获取除权除息数据列表
/// 与 Python from .xdxr import get_xdxr_list 对齐
fn get_xdxr_list(inst: &Instrument) -> Vec<XdxrInfo> {
    use super::level1::std::xdxr_info;
    let exchange = inst.exchange;
    let ticker = inst.market_ticker();
    match xdxr_info::fetch_xdxr(exchange, ticker) {
        Some(msg) => msg.list,
        None => Vec::new(),
    }
}

// ============================================================
// DataKLine — 前复权K线数据适配器
// 与 Python class DataKLine(adapter.DataAdapter) 对齐
// ============================================================

/// 前复权K线数据适配器
#[derive(Debug)]
pub struct DataKLine;

impl crate::data::Schema for DataKLine {
    fn kind(&self) -> crate::data::Kind {
        BaseKLine
    }
    fn owner(&self) -> String {
        DEFAULT_DATA_PROVIDER.to_string()
    }
    fn key(&self) -> String {
        "day".to_string()
    }
    fn name(&self) -> String {
        "前复权K线".to_string()
    }
    fn usage(&self) -> String {
        "前复权K线数据".to_string()
    }
}

impl DataAdapter for DataKLine {
    fn print(&self, _inst: &Instrument, _dates: &[Timestamp]) {}

    fn update(&self, inst: &Instrument, _date: Timestamp) {
        // 1. 从本地缓存确定起始日期
        let mut current_start_date = Timestamp::parse(MARKET_CN_FIRST_LIST_TIME)
            .unwrap_or_else(|_| Timestamp::zero())
            .pre_market_time_from_current()
            .unwrap_or_else(Timestamp::zero);
        let cache_filename = get_kline_filename(inst);
        let cache_klines = read_kline_from_csv(&cache_filename);

        let klines_length = cache_klines.len();
        let mut klines_offset_days = MAX_CACHED_DAYS_TO_DROP;
        let mut adjust_times = 0i32;

        if klines_length > 0 {
            if klines_offset_days > klines_length {
                klines_offset_days = klines_length;
            }
            let kline = &cache_klines[klines_length - klines_offset_days];
            if let Ok(ts) = Timestamp::parse(&kline.date) {
                if let Some(pt) = ts.pre_market_time_from_current() {
                    current_start_date = pt;
                }
            }
            adjust_times = kline.adjustment_count;
        }

        // 2. 确定结束日期
        let current_end_date = Timestamp::now()
            .pre_market_time_from_current()
            .unwrap_or_else(Timestamp::now);

        log::debug!(
            "[DataKLine] [{}]: from {} to {}",
            inst.symbol(),
            current_start_date.only_date(),
            current_end_date.only_date()
        );

        // 3. 分页拉取数据
        let step = SECURITY_BARS_PRE_REQUEST_MAX;
        let mut start: u32 = 0;
        let mut hs: Vec<Vec<Bar>> = Vec::new();
        let mut fetch_failed = false;

        loop {
            let count = std::cmp::min(step, u16::MAX as usize) as u16;
            let reply = fetch_kline_raw_as_bars(inst, start, count);
            if reply.is_empty() {
                if start == 0 {
                    fetch_failed = true;
                    log::warn!(
                        "[DataKLine] [{}] fetch_kline_raw returned empty (start={}, count={}) — server may be unreachable",
                        inst.symbol(),
                        start,
                        count
                    );
                }
                break;
            }

            let reply_len = reply.len();
            let last_bar_before_start = reply.last().map_or(false, |last_bar| {
                Timestamp::parse(&last_bar.date)
                    .map(|ts| {
                        ts.pre_market_time_from_current()
                            .map_or(false, |pt| pt < current_start_date)
                    })
                    .unwrap_or(false)
            });

            hs.push(reply);
            if last_bar_before_start {
                break;
            }
            if reply_len < count as usize {
                break;
            }
            start += count as u32;
        }

        // 4. 反转页面(时间升序)
        hs.reverse();

        // 5. 构建增量K线列表
        let mut incremental_klines: Vec<Bar> = Vec::new();
        for page in hs.iter() {
            for row in page.iter() {
                let date_time = match Timestamp::parse(&row.date) {
                    Ok(ts) => match ts.pre_market_time_from_current() {
                        Some(pt) => pt,
                        None => continue,
                    },
                    Err(_) => continue,
                };
                if date_time < current_start_date || date_time > current_end_date {
                    continue;
                }
                let kx = Bar {
                    date: date_time.only_date(),
                    open: row.open,
                    close: row.close,
                    high: row.high,
                    low: row.low,
                    volume: row.volume,
                    amount: row.amount,
                    up: row.up,
                    down: row.down,
                    timestamp: row.timestamp.clone(),
                    adjustment_count: 0,
                };
                incremental_klines.push(kx);
            }
        }

        // 6. 前复权处理
        let is_fresh_fetch_require_adjustment = adjust_times == 1;
        let dividends = get_xdxr_list(inst);

        if is_fresh_fetch_require_adjustment {
            apply_forward_adjustment_for_event(
                &mut incremental_klines,
                &current_start_date,
                &dividends,
            );
        }

        // 7. 合并缓存和增量数据
        let mut klines: Vec<Bar> = Vec::new();
        if klines_length > klines_offset_days {
            klines.extend_from_slice(&cache_klines[..(klines_length - klines_offset_days)]);
        }
        // 如果拉取失败且没有缓存数据, 不要写入空文件
        if fetch_failed && klines.is_empty() && incremental_klines.is_empty() {
            log::warn!(
                "[DataKLine] [{}] no data fetched and no cache — skipping save",
                inst.symbol()
            );
            return;
        }
        klines.extend(incremental_klines);

        // 8. 对新合并的数据再复权(非首次拉取的情况)
        if !is_fresh_fetch_require_adjustment {
            apply_forward_adjustment_for_event(&mut klines, &current_start_date, &dividends);
        }

        // 9. 保存
        save_kline(&cache_filename, &klines);
    }
}

/// 初始化并注册 DataKLine 插件
/// 与 Python _data_kline_plugin = adapter.register(DataKLine) 对齐
pub fn init() {
    let plugin = Arc::new(DataKLine) as Arc<dyn DataAdapter>;
    crate::data::register(plugin);
}

/// 获取指定证券代码截至指定日期的前复权K线数据
/// 与 Python get_cross_section_forward_adjusted_klines(inst, as_of_date) 对齐:
///   - Python 先调用 checkout_kline_raw(inst) 获取原始数据并复权
///   - Rust 版本通过 DataKLine adapter 确保缓存存在后再加载
pub fn get_cross_section_forward_adjusted_klines(
    inst: &Instrument,
    as_of_date: &str,
) -> Vec<Bar> {
    let filename = get_kline_filename(inst);
    log::debug!(
        "[kline] loading forward adjusted klines for {} from {}",
        inst.symbol(),
        filename
    );

    // 如果缓存文件不存在, 先通过 DataKLine adapter 从服务器拉取数据并生成缓存
    if !std::path::Path::new(&filename).exists() {
        log::info!(
            "[kline] cache not found for {}, triggering DataKLine update",
            inst.symbol()
        );
        let adapter = DataKLine;
        adapter.update(inst, Timestamp::now());
    }

    let all_klines = read_kline_from_csv(&filename);
    if all_klines.is_empty() {
        return Vec::new();
    }

    // 过滤出 as_of_date 及之前的K线
    all_klines
        .into_iter()
        .filter(|k| k.date.as_str() <= as_of_date)
        .collect()
}

/// 确定 klines 查询的截止时间戳
/// 与 Python datasource.py klines() 中的逻辑对齐:
///   if end_date is None:
///       as_of_ts = last_trading_day() if A股 else Timestamp.now().offset(hour=-24)
///   else:
///       as_of_ts = Timestamp.parse(end_date)
pub fn resolve_as_of_date(inst: &Instrument, end_date: Option<&str>) -> String {
    match end_date {
        Some(d) => {
            let ts = Timestamp::parse(d).unwrap_or_else(|_| Timestamp::now());
            ts.only_date()
        }
        None => {
            if inst.exchange == Exchange::SSE
                || inst.exchange == Exchange::SZSE
                || inst.exchange == Exchange::BSE
            {
                crate::data::meta::calendar::last_trading_day(Timestamp::now(), None).only_date()
            } else {
                // 非A股: 当天日期
                Timestamp::now().only_date()
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn make_test_instrument() -> Instrument {
        Instrument {
            exchange: Exchange::SSE,
            instrument_type: InstrumentType::STOCK,
            ticker: "600000".to_string(),
            name: "浦发银行".to_string(),
            lot_size: 100,
            price_precision: 2,
            ext_market: 0,
            ext_category: 0,
            alias_ticker: String::new(),
        }
    }

    #[test]
    #[ignore = "requires config file"]
    fn test_get_kline_filename() {
        let inst = make_test_instrument();
        let filename = get_kline_filename(&inst);
        assert!(filename.contains("day/"));
        assert!(filename.ends_with(".csv"));
        println!("kline filename: {}", filename);
    }

    #[test]
    fn test_resolve_as_of_date() {
        let inst_a = make_test_instrument();
        let date = resolve_as_of_date(&inst_a, None);
        assert!(!date.is_empty());

        let date2 = resolve_as_of_date(&inst_a, Some("2020-01-15"));
        assert_eq!(date2, "2020-01-15");
    }
}
