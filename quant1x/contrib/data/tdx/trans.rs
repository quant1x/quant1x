// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// trans — 逐笔交易数据缓存读取, 与 Python contrib/data/tdx/trans.py 对齐
// 作为 datasource 的本地代理, 仅从本地缓存CSV文件读取数据, 不依赖 level1 协议

use crate::data::meta::instrument::{Instrument, InstrumentType};
use crate::data::meta::Timestamp;
use crate::data::schema::{Direction, Transaction};

/// 获取逐笔交易缓存文件路径
/// 与 Python get_historical_trade_filename(inst, date) 对齐:
///   date_str = date.replace('-', '').replace('/', '')
///   year = date_str[:4]
///   base_path = os.path.join(config.data_path, 'trans', inst.cache_dir())
///   code = inst.symbol()
///   return os.path.join(base_path, year, date_str, f"{code}.csv")
fn get_historical_trade_filename(inst: &Instrument, date_str: &str) -> String {
    let clean_date = date_str.replace('-', "").replace('/', "");
    let year = if clean_date.len() >= 4 {
        &clean_date[..4]
    } else {
        "0000"
    };
    let symbol = inst.symbol();
    format!(
        "{}/trans/{}/{}/{}/{}.csv",
        crate::config::default_cache_path(),
        inst.cache_dir(),
        year,
        clean_date,
        symbol
    )
}

/// 从CSV缓存文件加载逐笔交易数据
/// 与 Python load_transaction_data_from_cache 对齐(仅缓存读取部分)
fn load_transaction_data_from_cache(inst: &Instrument, date_str: &str) -> Vec<Transaction> {
    let filename = get_historical_trade_filename(inst, date_str);

    match std::fs::File::open(&filename) {
        Ok(f) => {
            let mut rdr = csv::ReaderBuilder::new().has_headers(true).from_reader(f);
            let mut transactions: Vec<Transaction> = Vec::new();
            for result in rdr.records() {
                if let Ok(rec) = result {
                    transactions.push(Transaction {
                        time: rec.get(0).unwrap_or("").to_string(),
                        price: rec.get(1).and_then(|s| s.parse().ok()).unwrap_or(0.0),
                        volume: rec.get(2).and_then(|s| s.parse().ok()).unwrap_or(0),
                        num: rec.get(3).and_then(|s| s.parse().ok()).unwrap_or(0),
                        amount: rec.get(4).and_then(|s| s.parse().ok()).unwrap_or(0.0),
                        direction: rec
                            .get(5)
                            .and_then(|s| s.parse().ok())
                            .unwrap_or(Direction::Neutral as i32),
                    });
                }
            }
            transactions
        }
        Err(_) => Vec::new(),
    }
}

/// 获取指定证券在特定日期的逐笔交易数据
/// 与 Python checkout_transaction_data(inst, feature_date, ignore_previous_data=False) 对齐:
///   - 从本地 trans/{cache_dir}/{year}/{date}/{symbol}.csv 缓存文件加载
pub fn checkout_transaction_data(
    inst: &Instrument,
    feature_date: Timestamp,
    _ignore_previous_data: bool,
) -> Vec<Transaction> {
    if !inst.is_valid() {
        return Vec::new();
    }

    let date_str = feature_date.only_date();
    log::debug!(
        "[trans] loading transaction data for {} on {}",
        inst.symbol(),
        date_str
    );

    load_transaction_data_from_cache(inst, &date_str)
}

/// 解析交易日期时间戳
/// 与 Python datasource.py transactions() 中的逻辑对齐:
///   if date is None:
///       timestamp = last_trading_day()
///   else:
///       timestamp = Timestamp.parse(date)
pub fn resolve_transaction_date(date: Option<&str>) -> Timestamp {
    match date {
        Some(d) => Timestamp::parse(d).unwrap_or_else(|_| Timestamp::now()),
        None => crate::data::meta::calendar::last_trading_day(Timestamp::now(), None),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::data::meta::exchange::Exchange;

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
    fn test_get_historical_trade_filename() {
        let inst = make_test_instrument();
        let filename = get_historical_trade_filename(&inst, "2026-02-06");
        assert!(filename.contains("trans/"));
        assert!(filename.ends_with(".csv"));
        println!("trans filename: {}", filename);
    }

    #[test]
    fn test_resolve_transaction_date() {
        let ts = resolve_transaction_date(Some("2026-02-06"));
        assert_eq!(ts.only_date(), "2026-02-06");

        let ts2 = resolve_transaction_date(None);
        assert!(!ts2.only_date().is_empty());
    }
}
