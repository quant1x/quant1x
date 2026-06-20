// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// datasource — 通达信数据源实现, 与 Python contrib/data/tdx/datasource.py 对齐
// 实现 DataHandler trait, 作为通达信行情数据的统一入口

use crate::data::datasource::{DataHandler, PlateCategory};
use crate::data::meta::exchange::Exchange;
use crate::data::meta::instrument::Instrument;
use crate::data::schema::{Bar, Sector, Transaction};

use super::instruments::get_instrument_info;
use super::bar::get_cross_section_forward_adjusted_klines;
use super::bar::resolve_as_of_date;
use super::sector::get_sector_list as get_tdx_sector_list;
use super::trans::checkout_transaction_data;
use super::trans::resolve_transaction_date;

/// A股核心指数列表
/// 与 Python datasource.py ALL_INDEX_LIST 对齐
const ALL_INDEX_LIST: &[&str] = &[
    // 综合指数
    "sh000001", // 上证综合指数
    "sz399001", // 深证成份指数
    "bj899050", // 北证50指数
    "sz399006", // 创业板指
    // 宽基指数
    "sh000016", // 上证50
    "sh000300", // 沪深300指数
    "sh000688", // 科创50指数
    "sh000905", // 中证500指数
    "sh000852", // 中证1000指数
    // 板块
    "sh880005", // 通达信板块-涨跌家数
    // ETF
    "sh510050", // 上证50ETF
    "sh510300", // 沪深300ETF
    "sh588000", // 科创50ETF
    "sh510500", // 中证500ETF
    "sh512100", // 中证1000ETF
    "sh510900", // H股ETF
    "sh518880", // 黄金ETF
    "sh512480", // 半导体ETF
    "sh562500", // 机器人ETF
];

/// 需要忽略的关键字(退市/摘牌)
/// 与 Python is_need_ignore() 对齐
const IGNORED_KEYWORDS: &[&str] = &["退", "摘牌"];

/// 检查证券代码是否需要忽略
/// 与 Python is_need_ignore(code) 对齐:
///   - 查不到 instrument → 忽略
///   - 名称包含"退"或"摘牌" → 忽略
fn is_need_ignore(code: &str) -> bool {
    let instrument = match get_instrument_info(code) {
        Some(inst) => inst,
        None => return true, // 没找到直接忽略
    };
    let upper_name = instrument.name.to_uppercase();
    IGNORED_KEYWORDS
        .iter()
        .any(|keyword| upper_name.contains(keyword))
}

/// 通达信数据源
///
/// 实现 DataHandler trait, 作为 A 股(SSE/SZSE/BSE)行情的统一入口. 
/// 数据来源: 本地 CSV 缓存文件. 
#[derive(Debug, Default)]
pub struct TdxDataSource;

impl TdxDataSource {
    /// 创建新的通达信数据源实例
    pub fn new() -> Self {
        TdxDataSource
    }
}

impl DataHandler for TdxDataSource {
    /// 返回通达信数据源支持的市场列表
    /// 与 Python get_market_list() 对齐: [SSE, SZSE, BSE]
    fn get_market_list(&self) -> Vec<Exchange> {
        vec![Exchange::SSE, Exchange::SZSE, Exchange::BSE]
    }

    /// 获取指定市场的指数列表
    /// 与 Python get_index_list(market="all") 对齐
    fn get_index_list(&self, _market: Option<&[String]>) -> Vec<Instrument> {
        let mut index_list: Vec<Instrument> = Vec::new();
        for &code in ALL_INDEX_LIST {
            if let Some(inst) = get_instrument_info(code) {
                // TODO: 过滤不符合条件的指数
                index_list.push(inst);
            }
        }
        index_list
    }

    /// 获取指定类别的板块列表
    /// 与 Python get_sector_list(category=PlateCategory.UNKNOWN) 对齐
    fn get_sector_list(&self, _category: PlateCategory) -> Vec<Sector> {
        get_tdx_sector_list()
    }

    /// 加载全部指数, 板块和个股的代码
    /// 与 Python list_instruments(market="all") 对齐:
    ///   1. 指数(含重要板块及 ETF)
    ///   2. 板块(排除已在指数列表中的)
    ///   3. 个股(仅上市公司股票)
    fn list_instruments(&self, _market: Option<&[String]>) -> Vec<Instrument> {
        let mut code_list: Vec<Instrument> = Vec::new();

        // 1. 指数
        let index_list = self.get_index_list(None);
        log::debug!(
            "[tdx/datasource] list_instruments: index_list len={}",
            index_list.len()
        );
        code_list.extend(index_list);

        // 2. 板块(排除已在 ALL_INDEX_LIST 中的)
        let sectors = self.get_sector_list(PlateCategory::Unknown);
        for s in &sectors {
            if ALL_INDEX_LIST.contains(&s.code.as_str()) {
                continue;
            }
            if let Some(inst) = get_instrument_info(&s.code) {
                code_list.push(inst);
            }
        }

        // 3. 个股
        let stock_list = get_stock_list();
        log::debug!(
            "[tdx/datasource] list_instruments: stock_list len={}",
            stock_list.len()
        );
        code_list.extend(stock_list);

        code_list
    }

    /// 获取指定证券代码对应的证券信息
    /// 与 Python get_instrument(symbol) 对齐:
    ///   - 查不到时返回 None(Python 抛 ValueError)
    fn get_instrument(&self, symbol: &str) -> Option<Instrument> {
        get_instrument_info(symbol)
    }

    /// 获取指定证券代码的K线数据
    /// 与 Python klines(symbol, start_date, end_date, freq) 对齐:
    ///   - end_date=None → 取最近交易日
    ///   - 返回前复权日K线
    fn klines(
        &self,
        symbol: &str,
        _start_date: Option<&str>,
        end_date: Option<&str>,
        _freq: Option<&str>,
    ) -> Option<Vec<Bar>> {
        let inst = self.get_instrument(symbol)?;

        let as_of_date = resolve_as_of_date(&inst, end_date);
        log::debug!(
            "[tdx/datasource] Getting klines for {} as of {}",
            symbol,
            as_of_date
        );

        let bars = get_cross_section_forward_adjusted_klines(&inst, &as_of_date);

        log::debug!(
            "[tdx/datasource] Klines for {}: {} bars",
            symbol,
            bars.len()
        );
        Some(bars)
    }

    /// 获取指定证券代码的交易数据
    /// 与 Python transactions(symbol, date) 对齐:
    ///   - date=None → 取最近交易日
    fn transactions(&self, symbol: &str, date: Option<&str>) -> Option<Vec<Transaction>> {
        let inst = self.get_instrument(symbol)?;
        let timestamp = resolve_transaction_date(date);
        let trans = checkout_transaction_data(&inst, timestamp, false);
        Some(trans)
    }
}

/// 获取所有A股股票列表(遍历代码范围 + is_need_ignore 过滤)
/// 与 Python get_stock_list(market="all") 对齐
fn get_stock_list() -> Vec<Instrument> {
    let mut stock_list: Vec<Instrument> = Vec::new();

    // 上海证券交易所 (sh600000-sh609999)
    for i in 600000..610000 {
        let fc = format!("sh{:06}", i);
        if !is_need_ignore(&fc) {
            if let Some(inst) = get_instrument_info(&fc) {
                stock_list.push(inst);
            }
        }
    }

    // 科创板 (sh688000-sh689999)
    for i in 688000..690000 {
        let fc = format!("sh{:06}", i);
        if !is_need_ignore(&fc) {
            if let Some(inst) = get_instrument_info(&fc) {
                stock_list.push(inst);
            }
        }
    }

    // 深圳主板 (sz000000-sz000999)
    for i in 0..1000 {
        let fc = format!("sz{:06}", i);
        if !is_need_ignore(&fc) {
            if let Some(inst) = get_instrument_info(&fc) {
                stock_list.push(inst);
            }
        }
    }

    // 中小板 (sz001000-sz009999)
    for i in 1000..10000 {
        let fc = format!("sz{:06}", i);
        if !is_need_ignore(&fc) {
            if let Some(inst) = get_instrument_info(&fc) {
                stock_list.push(inst);
            }
        }
    }

    // 创业板 (sz300000-sz300999)
    for i in 300000..310000 {
        let fc = format!("sz{:06}", i);
        if !is_need_ignore(&fc) {
            if let Some(inst) = get_instrument_info(&fc) {
                stock_list.push(inst);
            }
        }
    }

    // 北交所 (bj920000-bj920999)
    for i in 920000..921000 {
        let fc = format!("bj{:06}", i);
        if !is_need_ignore(&fc) {
            if let Some(inst) = get_instrument_info(&fc) {
                stock_list.push(inst);
            }
        }
    }

    stock_list
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_tdx_data_source_new() {
        let ds = TdxDataSource::new();
        let markets = ds.get_market_list();
        assert_eq!(markets.len(), 3);
        assert!(markets.contains(&Exchange::SSE));
        assert!(markets.contains(&Exchange::SZSE));
        assert!(markets.contains(&Exchange::BSE));
    }

    #[test]
    fn test_all_index_list() {
        assert!(ALL_INDEX_LIST.contains(&"sh000001"));
        assert!(ALL_INDEX_LIST.contains(&"sz399001"));
    }

    #[test]
    fn test_ignored_keywords() {
        assert!(IGNORED_KEYWORDS.contains(&"退"));
        assert!(IGNORED_KEYWORDS.contains(&"摘牌"));
    }

    /// 集成测试: 严格对齐 Python datasource.py __main__ 的测试逻辑
    ///
    /// Python __main__ 实际执行的代码(未注释部分):
    ///   1. code = '00077.hk'
    ///   2. inst = D.get_instrument(code); print(inst)
    ///   3. df = D.klines(code); print(df)
    ///
    /// 已注释掉的部分(get_sector_list / get_index_list / get_stock_list / list_instruments / transactions)
    /// Rust 测试同样注释掉, 只保留 Python 实际执行的部分. 
    ///
    /// 注意: 此测试依赖本地缓存数据, 在没有缓存的环境下可能部分断言失败. 
    /// 标记为 #[ignore] 避免 CI 环境因缺少缓存文件而失败, 开发时手动运行:
    ///   cargo test --package quant1x -- test_tdx_datasource_main --ignored --nocapture
    #[test]
    #[ignore]
    fn test_tdx_datasource_main() {
        // 启用调试日志(对齐 Python config.debug = True)
        let _ = env_logger::try_init();

        let ds = TdxDataSource::new();

        // ---- 对齐 Python __main__: get_instrument + klines ----
        // Python: code = 'sh562500' → code = 'hsi.hk' → code = 'ixic.us' → code = '00077.hk'
        let code = "00077.hk";
        let inst = ds.get_instrument(code);
        log::info!("[tdx/datasource test] instrument {}: {:?}", code, inst);
        assert!(inst.is_some(), "instrument {} should exist", code);
        let inst = inst.unwrap();
        log::info!(
            "[tdx/datasource test] {} name={}, exchange={:?}, type={:?}",
            code,
            inst.name,
            inst.exchange,
            inst.instrument_type
        );

        // Python: df = D.klines(code); print(df)
        let bars = ds.klines(code, None, None, None);
        log::info!(
            "[tdx/datasource test] klines for {}: {} bars",
            code,
            bars.as_ref().map_or(0, |v| v.len())
        );
        assert!(bars.is_some(), "klines should return data for {}", code);
        let bars = bars.unwrap();
        assert!(!bars.is_empty(), "klines should have at least one bar");

        // 验证 Bar 字段
        let first = &bars[0];
        log::info!(
            "[tdx/datasource test] first bar: date={}, open={}, close={}, high={}, low={}, volume={}",
            first.date,
            first.open,
            first.close,
            first.high,
            first.low,
            first.volume
        );
        assert!(first.open > 0.0, "open should be positive");
        assert!(first.close > 0.0, "close should be positive");
        assert!(first.high > 0.0, "high should be positive");
        assert!(first.low > 0.0, "low should be positive");
        assert!(first.high >= first.low, "high >= low");

        // (Python 注释) date = '2026-02-06'; trans = D.transactions(code, date)

        log::info!("[tdx/datasource test] all checks passed!");
    }
}
