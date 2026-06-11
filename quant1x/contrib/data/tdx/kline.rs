// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// kline — 前复权K线数据缓存读取，与 Python contrib/data/tdx/kline.py 对齐
// 作为 datasource 的本地代理，仅从本地缓存CSV文件读取数据，不依赖 level1 协议

use crate::data::adapter::DataAdapter;
use crate::data::meta::exchange::Exchange;
use crate::data::meta::instrument::{Instrument, InstrumentType};
use crate::data::meta::Timestamp;
use crate::data::schema::Bar;

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
        Err(_) => { /* 文件不存在，返回空列表 */ }
    }
    klines
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

    // 如果缓存文件不存在，先通过 DataKLine adapter 从服务器拉取数据并生成缓存
    if !std::path::Path::new(&filename).exists() {
        log::info!(
            "[kline] cache not found for {}, triggering DataKLine update",
            inst.symbol()
        );
        let adapter = crate::data::kline::DataKLine;
        adapter.update(inst, crate::data::meta::Timestamp::now());
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
                crate::data::meta::calendar::last_trading_day(Timestamp::now()).only_date()
            } else {
                // 非A股：当天日期
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
