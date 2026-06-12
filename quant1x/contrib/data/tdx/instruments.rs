// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// instruments — 证券信息缓存读取，1:1 还原 Python contrib/data/tdx/instruments.py

use crate::data::meta::exchange::Exchange;
use crate::data::meta::instrument::{Instrument, InstrumentType};
use std::collections::HashMap;
use std::fs;
use std::io::Write;
use std::sync::Mutex;
use once_cell::sync::Lazy;

// 1:1 还原 Python: _SECURITY_MAP = {}
static SECURITY_MAP: Lazy<Mutex<HashMap<String, Instrument>>> =
    Lazy::new(|| Mutex::new(HashMap::new()));

// 1:1 还原 Python: _get_security_filename() -> os.path.join(config.meta_path, "securities.csv")
fn get_security_filename() -> String {
    crate::config::get_security_filename()
}

// 1:1 还原 Python: _load_securities() -> bool
fn load_securities() -> bool {
    let fname = get_security_filename();
    log::debug!("Loading securities from {}", fname);

    let mut map = SECURITY_MAP.lock().unwrap();
    map.clear();

    match std::fs::File::open(&fname) {
        Ok(f) => {
            let mut rdr = csv::ReaderBuilder::new().has_headers(true).from_reader(f);
            for result in rdr.records() {
                if let Ok(rec) = result {
                    // 1:1 还原 Python: exchange = Exchange.parse(row.get('exchange'))
                    let exchange_str = rec.get(0).unwrap_or("unknown");
                    let exchange = Exchange::parse(exchange_str).unwrap_or(Exchange::UNKNOWN);
                    // 1:1 还原 Python: type = InstrumentType.from_string(row.get('type'))
                    let type_str = rec.get(1).unwrap_or("unknown");
                    let instrument_type = InstrumentType::from_string(type_str);
                    // 1:1 还原 Python: code = row.get('code') or ''
                    let ticker = rec.get(2).unwrap_or("").to_string();
                    let name = rec.get(3).unwrap_or("").to_string();
                    // 1:1 还原 Python: lot_size = int(row.get('lot_size') or '100')
                    let lot_size: i32 = rec.get(4).and_then(|s| s.parse().ok()).unwrap_or(100);
                    // 1:1 还原 Python: price_precision = int(row.get('price_precision') or '2')
                    let price_precision: i32 = rec.get(5).and_then(|s| s.parse().ok()).unwrap_or(2);
                    // 1:1 还原 Python: ext_market = int(row.get('ext_market') or '')
                    let ext_market: i32 = rec.get(6).and_then(|s| s.parse().ok()).unwrap_or(0);
                    let ext_category: i32 = rec.get(7).and_then(|s| s.parse().ok()).unwrap_or(0);
                    let alias_ticker = rec.get(8).unwrap_or("").to_string();

                    // 1:1 还原 Python: code = code.lower()
                    let inst = Instrument {
                        exchange,
                        instrument_type,
                        ticker: ticker.to_lowercase(),
                        name,
                        lot_size,
                        price_precision,
                        ext_market,
                        ext_category,
                        alias_ticker,
                    };
                    // 1:1 还原 Python: symbol = inst.symbol(); _SECURITY_MAP[symbol] = inst
                    let symbol = inst.symbol();
                    map.insert(symbol, inst);
                }
            }
        }
        Err(_) => {
            // 1:1 还原 Python: except FileNotFoundError: return False
            return false;
        }
    }
    // 1:1 还原 Python: if len(_SECURITY_MAP) > 0: return True; return False
    !map.is_empty()
}

// 1:1 还原 Python: market.correct_security_code(symbol)
fn correct_security_code(symbol: &str) -> String {
    let inst = crate::data::market::detect_symbol(symbol);
    if inst.can_construct_symbol() {
        inst.symbol()
    } else {
        String::new()
    }
}

// 1:1 还原 Python: fetch_security_list(exchange, start, count) -> List[Instrument]
fn fetch_security_list(exchange: Exchange, start: u32, count: u32) -> Vec<Instrument> {
    let market_id = match exchange {
        Exchange::SSE => 1u16,
        Exchange::SZSE => 0u16,
        Exchange::BSE => 2u16,
        _ => return Vec::new(),
    };

    let resp = match crate::contrib::data::tdx::level1::std::security_list::fetch_security_list(market_id, start, count) {
        Some(r) => r,
        None => return Vec::new(),
    };

    let mut instruments: Vec<Instrument> = Vec::new();
    for sec in &resp.list {
        let inst = Instrument {
            exchange,
            instrument_type: InstrumentType::from_string("stock"),
            ticker: sec.code.to_lowercase(),
            name: sec.name.clone(),
            lot_size: sec.vol_unit as i32,
            price_precision: sec.decimal_point as i32,
            ext_market: 0,
            ext_category: 0,
            alias_ticker: String::new(),
        };
        instruments.push(inst);
    }
    instruments
}

// 1:1 还原 Python: init_securities()
pub fn init_securities() {
    let fname = get_security_filename();

    // 1:1 还原 Python:
    //   ensure_updated = status.should_initialize_file(fname)
    //   if not ensure_updated:
    //       ensure_updated = _load_securities() is False
    let ensure_updated = {
        let map = SECURITY_MAP.lock().unwrap();
        if map.is_empty() {
            drop(map);
            !load_securities() // CSV 加载失败 → need fetch
        } else {
            false
        }
    };

    log::debug!("init_securities ensure_updated={}", ensure_updated);

    if ensure_updated {
        let mut instruments: Vec<Instrument> = Vec::new();

        // 1:1 还原 Python: # 1. 标准行情: A股
        //   markets = [Exchange.SSE, Exchange.SZSE, Exchange.BSE]
        let markets = [Exchange::SSE, Exchange::SZSE, Exchange::BSE];
        for m in &markets {
            let mut start: u32 = 0;
            let mut rows: Vec<Instrument> = Vec::new();
            loop {
                // 1:1 还原 Python: page = fetch_security_list(m, start, SECURITY_LIST_PRE_REQUEST_MAX)
                // 1:1 还原 Python: SECURITY_LIST_PRE_REQUEST_MAX = 1600
                let pre_request_max: u32 = 1600;
                let page = fetch_security_list(*m, start, pre_request_max);
                if page.is_empty() {
                    break;
                }
                let page_len = page.len() as u32;
                rows.extend(page);
                if page_len < pre_request_max {
                    break;
                }
                start += pre_request_max;
            }
            // 1:1 还原 Python: rows.sort(key=lambda x: x.ticker)
            rows.sort_by(|a, b| a.ticker.cmp(&b.ticker));
            // 1:1 还原 Python: instruments.extend(rows)
            instruments.extend(rows);
        }

        // 1:1 还原 Python: # 2. 扩展行情: 港股等
        //   from .level1.ext import InstrumentInfo
        //   markets = [Exchange.HKEX]
        //   offset = InstrumentInfo.PRE_REQUEST_MAX
        //   for m in markets:
        //       start = 0
        //       rows = []
        //       conn = client.get_ext_conn()
        //       while True:
        //           ii = InstrumentInfo(start, offset)
        //           protocol.process_level1_new(conn, ii)
        //           fetch_count = ii.reply.get('count', 0)
        //           ...
        let ext_markets = [Exchange::HKEX];
        let offset = super::level1::ext::EXT_PRE_REQUEST_MAX;
        for _m in &ext_markets {
            // 1:1 还原 Python: conn = client.get_ext_conn()
            let mut pooled = match super::client::get_ext_conn() {
                Ok(p) => p,
                Err(e) => {
                    log::error!("[tdx/instruments] get_ext_conn failed: {}", e);
                    continue;
                }
            };

            let mut start: u32 = 0;
            let mut rows: Vec<Instrument> = Vec::new();
            loop {
                // 1:1 还原 Python: ii = InstrumentInfo(start, offset)
                let mut ii = super::level1::ext::InstrumentInfoRequest::new(start, offset);
                // 1:1 还原 Python: protocol.process_level1_new(conn, ii)
                let fetch_count = match super::protocol::process_level1_stream(pooled.stream(), &mut ii) {
                    Ok(()) => ii.list.len(),
                    Err(e) => {
                        log::error!("[tdx/instruments] ext InstrumentInfo request failed: {}", e);
                        break;
                    }
                };
                // 1:1 还原 Python: fetch_count = ii.reply.get('count', 0)
                if fetch_count > 0 {
                    rows.extend(ii.list);
                } else {
                    break;
                }
                if fetch_count < offset as usize {
                    break;
                }
                start += offset as u32;
            }
            // 1:1 还原 Python: rows.sort(key=lambda x: (ext_market, ext_category, ticker))
            rows.sort_by(|a, b| {
                a.ext_market.cmp(&b.ext_market)
                    .then_with(|| a.ext_category.cmp(&b.ext_category))
                    .then_with(|| a.ticker.cmp(&b.ticker))
            });
            log::debug!("init_securities rows[ext]={:?}", rows);
            // 1:1 还原 Python: instruments.extend(rows)
            instruments.extend(rows);
        }

        // 1:1 还原 Python: write CSV if we have instruments
        if !instruments.is_empty() {
            write_securities_csv(&fname, &instruments);
        }
        // 1:1 还原 Python: _ = _load_securities()
        let _ = load_securities();
    }
}

// 1:1 还原 Python: writer.writerow(r.to_iterable()) → CSV write
fn write_securities_csv(fname: &str, instruments: &[Instrument]) {
    if let Some(parent) = std::path::Path::new(fname).parent() {
        let _ = fs::create_dir_all(parent);
    }
    if let Ok(mut f) = fs::File::create(fname) {
        let _ = writeln!(
            f,
            "exchange,type,code,name,lot_size,price_precision,ext_market,ext_category,alias_ticker"
        );
        for inst in instruments {
            let _ = writeln!(
                f,
                "{},{},{},{},{},{},{},{},{}",
                inst.exchange.identifier(),
                inst.instrument_type.0,
                inst.ticker,
                inst.name,
                inst.lot_size,
                inst.price_precision,
                inst.ext_market,
                inst.ext_category,
                inst.alias_ticker,
            );
        }
        log::info!("[tdx/instruments] wrote {} instruments to {}", instruments.len(), fname);
    }
}

// 1:1 还原 Python: get_instrument_info(symbol) -> Optional[Instrument]
//   _SECURITY_ONCE.do(init_securities)
//   security_code = market.correct_security_code(symbol)
//   return _SECURITY_MAP.get(security_code)
pub fn get_instrument_info(symbol: &str) -> Option<Instrument> {
    let security_code = correct_security_code(symbol);
    log::debug!("get_instrument_info: symbol={}, security_code={}", symbol, security_code);

    let map = SECURITY_MAP.lock().unwrap();
    if map.is_empty() {
        drop(map);
        init_securities();
        let map = SECURITY_MAP.lock().unwrap();
        map.get(&security_code).cloned()
    } else {
        map.get(&security_code).cloned()
    }
}

/// 确保证券缓存已初始化（供外部调用）
pub fn ensure_securities_initialized() {
    init_securities();
}

// ============================================================
// 1:1 还原 Python instruments.py __main__ 中的 correct_security_code 行为
// 测试用例直接对 Python 实际输出
// ============================================================
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_correct_security_code_sh() {
        assert_eq!(correct_security_code("600000"), "sh600000");
        assert_eq!(correct_security_code("600000.SH"), "sh600000");
        assert_eq!(correct_security_code("sh600000"), "sh600000");
    }

    #[test]
    fn test_correct_security_code_sz() {
        assert_eq!(correct_security_code("000001"), "sz000001");
        assert_eq!(correct_security_code("000001.SZ"), "sz000001");
        assert_eq!(correct_security_code("sz000001"), "sz000001");
        assert_eq!(correct_security_code("300001"), "sz300001");
    }

    #[test]
    fn test_correct_security_code_bj() {
        // Python 920000 纯数字推断为 sh920000（6位数字先匹配上交所规则）
        assert_eq!(correct_security_code("920000"), "sh920000");
        assert_eq!(correct_security_code("920000.BJ"), "bj920000");
    }

    #[test]
    fn test_correct_security_code_hk() {
        assert_eq!(correct_security_code("00700.hk"), "00700.hk");
        assert_eq!(correct_security_code("hsi.hk"), "hsi.hk");
    }

    #[test]
    fn test_correct_security_code_us() {
        // Python ixic.us → ixic.os（Exchange.USA.identifier = "os"）
        assert_eq!(correct_security_code("ixic.us"), "ixic.os");
        assert_eq!(correct_security_code("aapl.us"), "aapl.us");
    }
}
