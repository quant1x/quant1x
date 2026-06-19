// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// instruments — 证券信息缓存读取

use crate::data::meta::exchange::Exchange;
use crate::data::meta::instrument::{Instrument, InstrumentType};
use crate::data::status;
use crate::helpers;
use std::collections::HashMap;
use std::io;
use std::fs::{self, File};
use std::io::Write;
use std::sync::Mutex;
use once_cell::sync::Lazy;
use std::path::Path;
use csv::WriterBuilder; // 引入 csv crate

static SECURITY_MAP: Lazy<Mutex<HashMap<String, Instrument>>> =
    Lazy::new(|| Mutex::new(HashMap::new()));

/// 获取证券列表文件名
fn get_security_filename() -> String {
    crate::config::get_security_filename()
}

/// 从 CSV 文件加载证券列表到全局安全映射中. 
///
/// 读取由 `get_security_filename()` 指定的 CSV 文件, 逐行解析为 `Instrument` 对象, 
/// 并以 `symbol` 为键插入全局 `SECURITY_MAP`. 加载前会清空已有数据. 
///
/// # Returns
///
/// - `true`: 成功加载且映射非空
/// - `false`: 文件打开失败或映射为空
///
/// # Panics
///
/// 当 `SECURITY_MAP` 的 Mutex 被 poison 时会 panic. 
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
                    let exchange_str = rec.get(0).unwrap_or("unknown");
                    let exchange = Exchange::parse(exchange_str).unwrap_or(Exchange::UNKNOWN);
                    let type_str = rec.get(1).unwrap_or("unknown");
                    let instrument_type = InstrumentType::from_string(type_str);
                    let ticker = rec.get(2).unwrap_or("").to_string();
                    let name = rec.get(3).unwrap_or("").to_string();
                    let lot_size: i32 = rec.get(4).and_then(|s| s.parse().ok()).unwrap_or(100);
                    let price_precision: i32 = rec.get(5).and_then(|s| s.parse().ok()).unwrap_or(2);
                    let ext_market: i32 = rec.get(6).and_then(|s| s.parse().ok()).unwrap_or(0);
                    let ext_category: i32 = rec.get(7).and_then(|s| s.parse().ok()).unwrap_or(0);
                    let alias_ticker = rec.get(8).unwrap_or("").to_string();

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
                    let symbol = inst.symbol();
                    map.insert(symbol, inst);
                }
            }
        }
        Err(_) => {
            return false;
        }
    }
    !map.is_empty()
}

/// 根据证券代码字符串, 自动识别市场并返回标准化的证券代码. 
///
/// 如果无法识别有效的市场或代码, 则返回空字符串. 
///
/// # Examples
///
/// ```ignore
/// let code = correct_security_code("sh600000");
/// assert_eq!(code, "sh600000");
///
/// let invalid = correct_security_code("invalid");
/// assert_eq!(invalid, "");
/// ```
fn correct_security_code(symbol: &str) -> String {
    let inst = crate::data::market::detect_symbol(symbol);
    if inst.can_construct_symbol() {
        inst.symbol()
    } else {
        String::new()
    }
}

/// 从指定交易所获取证券列表. 
///
/// 根据交易所类型映射为对应的市场ID, 调用通达信协议获取证券列表, 
/// 并将返回的原始数据转换为 `Instrument` 集合. 
///
/// # Arguments
///
/// - `exchange` - 交易所枚举, 支持 SSE(沪), SZSE(深), BSE(京)
/// - `start` - 请求起始位置
/// - `count` - 请求数量
///
/// # Returns
///
/// 返回该交易所指定范围内的证券列表. 若交易所不支持或协议请求失败, 返回空向量. 
fn fetch_security_list(exchange: Exchange, start: u32, count: u32) -> Vec<Instrument> {
    let market_id = helpers::exchange_to_market(exchange.code()).unwrap_or(0) as u16;
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

/// 初始化证券列表, 从通达信服务器拉取并缓存到本地 CSV 文件. 
///
/// 首先检查是否需要更新(通过 `should_initialize_file` 判断), 若无需更新则尝试从已有 CSV 加载；
/// 若 CSV 加载失败或标记为需要更新, 则从服务器分页拉取证券列表: 
/// - 标准行情: 拉取沪市(SSE), 深市(SZSE), 北交所(BSE)的 A 股列表
/// - 扩展行情: 拉取港交所(HKEX)的证券列表
///
/// 拉取完成后写入 CSV 缓存文件, 并将证券列表加载到内存中的 `SECURITY_MAP`. 
pub fn init_securities() {
    let fname = get_security_filename();
    let mut create_or_update = status::should_initialize_file(fname.as_str(), Exchange::SSE);
    if !create_or_update {
        let map = SECURITY_MAP.lock().unwrap();
        if map.is_empty() {
            drop(map); // drop 后会触发 poison 
            create_or_update = !load_securities(); // CSV 加载失败 → need fetch
        }
    }
    log::debug!("init_securities create_or_update={}", create_or_update);

    if create_or_update {
        let mut instruments: Vec<Instrument> = Vec::new();

        // 1. 标准行情: A股
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
            rows.sort_by(|a, b| a.ticker.cmp(&b.ticker));
            instruments.extend(rows);
        }

        let ext_markets = [Exchange::HKEX];
        let offset = super::level1::ext::EXT_PRE_REQUEST_MAX;
        for _m in &ext_markets {
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
                let mut ii = super::level1::ext::InstrumentInfoRequest::new(start, offset);
                let fetch_count = match super::protocol::transact_message_sync(pooled.stream(), &mut ii) {
                    Ok(()) => ii.list.len(),
                    Err(e) => {
                        log::error!("[tdx/instruments] ext InstrumentInfo request failed: {}", e);
                        break;
                    }
                };
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
            // 排序
            rows.sort_by(|a, b| {
                a.ext_market.cmp(&b.ext_market)
                    .then_with(|| a.ext_category.cmp(&b.ext_category))
                    .then_with(|| a.ticker.cmp(&b.ticker))
            });
            log::debug!("init_securities rows[ext]={:?}", rows);
            instruments.extend(rows);
        }

        // 写缓存文件
        if !instruments.is_empty() {
            let result = write_securities_csv(&fname, &instruments);
            if result.is_err() {
                panic!("init_securities write_securities_csv failed: {}", result.unwrap_err());
            }
        } else {
            panic!("init_securities instruments is empty");
        }
        // 证券列表加载到内存
        let _ = load_securities();
    }
}

/// 将证券列表写入 CSV 文件. 
///
/// 自动创建目标文件所在的目录结构, 写入表头及所有证券记录. 
///
/// # Errors
///
/// 若文件创建或写入失败, 错误会被静默忽略(使用 `let _` 丢弃). 
fn write_securities_csv_v1(fname: &str, instruments: &[Instrument]) {
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

fn write_securities_csv(fname: &str, instruments: &[Instrument]) -> io::Result<()> {
    if let Some(parent) = Path::new(fname).parent() {
        if !parent.as_os_str().is_empty() {
            fs::create_dir_all(parent)?;
        }
    }

    let file = File::create(fname)?;
    
    // 使用 csv::Writer, 它自带缓冲, 且自动处理 CSV 转义
    let mut wtr = WriterBuilder::new()
        .has_headers(true) // 自动处理表头
        .from_writer(file);

    for inst in instruments {
        // serialize 会自动将结构体字段按顺序写入, 并安全处理特殊字符
        // 注意: 这要求 Instrument 或其引用的字段实现了 serde::Serialize 
        // 如果不想用 serde, 也可以使用 wtr.write_record(&[ ... ]) 手动传入字符串切片
        wtr.write_record(&[
            inst.exchange.identifier().to_string(),
            inst.instrument_type.0.to_string(),
            inst.ticker.clone(),
            inst.name.clone(),
            inst.lot_size.to_string(),
            inst.price_precision.to_string(),
            inst.ext_market.to_string(),
            inst.ext_category.to_string(),
            inst.alias_ticker.clone(),
        ])?;
    }

    // flush 确保所有数据写入磁盘
    wtr.flush()?;

    log::info!("[tdx/instruments] successfully wrote {} instruments to {}", instruments.len(), fname);
    Ok(())
}

/// 根据证券代码获取证券信息
///
/// 自动校正输入的证券代码格式, 并从全局证券映射表中查找对应的 [`Instrument`]. 
/// 若映射表尚未初始化, 会先触发初始化再进行查找. 
///
/// # Arguments
///
/// * `symbol` - 证券代码, 支持带或不带市场前缀的格式
///
/// # Returns
///
/// - `Some(Instrument)` - 找到对应的证券信息时返回
/// - `None` - 未找到对应证券时返回
///
/// # Panics
///
/// 当全局证券映射表的互斥锁被毒化(持有锁的线程 panic)时会 panic. 
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

/// 确保证券缓存已初始化(供外部调用)
pub fn ensure_securities_initialized() {
    init_securities();
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_ensure_securities_initialized() {
        ensure_securities_initialized();
    }

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
        // Python 920000 纯数字推断为 sh920000(6位数字先匹配上交所规则)
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
        // Python ixic.us → ixic.os(Exchange.USA.identifier = "os")
        assert_eq!(correct_security_code("ixic.us"), "ixic.os");
        assert_eq!(correct_security_code("aapl.us"), "aapl.us");
    }
}
