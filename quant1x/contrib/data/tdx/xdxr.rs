// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// xdxr — 除权除息数据缓存读写，与 Python contrib/data/tdx/xdxr.py 对齐
// 作为 DataAdapter 的 Rust 实现，负责网络拉取、本地 CSV 缓存和加载。

use std::sync::Arc;

use crate::data::adapter::{DataAdapter, Schema};
use crate::data::meta::instrument::Instrument;
use crate::data::meta::Timestamp;
use crate::data::schema::XdxrInfo;
use crate::data::{BaseXdxr, DEFAULT_DATA_PROVIDER};

// ============================================================
// 路径辅助
// ============================================================

/// 与 Python `_get_xdxr_filename(inst)` / C++ `xdxr_cache_filename(inst)` 对齐:
///   {cache}/xdxr/{cache_dir}/{symbol}.csv
fn xdxr_cache_filename(inst: &Instrument) -> String {
    format!(
        "{}/xdxr/{}/{}.csv",
        crate::config::default_cache_path(),
        inst.cache_dir(),
        inst.symbol()
    )
}

// ============================================================
// CSV 常量（与 Python save_xdxr 对齐）
// ============================================================

const XDXR_CSV_HEADER: &[&str] = &[
    "date",
    "category",
    "name",
    "fen_hong",
    "dividend_currency",
    "pei_gu_jia",
    "rights_currency",
    "song_zhuan_gu",
    "pei_gu",
    "suo_gu",
    "qian_liu_tong",
    "hou_liu_tong",
    "qian_zong_gu_ben",
    "hou_zong_gu_ben",
    "fen_shu",
    "xing_quan_jia",
];

fn xdxr_info_to_record(info: &XdxrInfo) -> Vec<String> {
    vec![
        info.date.clone(),
        info.category.to_string(),
        info.name.clone(),
        info.fen_hong.to_string(),
        info.dividend_currency.clone(),
        info.pei_gu_jia.to_string(),
        info.rights_currency.clone(),
        info.song_zhuan_gu.to_string(),
        info.pei_gu.to_string(),
        info.suo_gu.to_string(),
        info.qian_liu_tong.to_string(),
        info.hou_liu_tong.to_string(),
        info.qian_zong_gu_ben.to_string(),
        info.hou_zong_gu_ben.to_string(),
        info.fen_shu.to_string(),
        info.xing_quan_jia.to_string(),
    ]
}

fn record_to_xdxr_info(rec: &csv::StringRecord) -> XdxrInfo {
    XdxrInfo {
        date: rec.get(0).unwrap_or("").to_string(),
        category: rec.get(1).and_then(|s| s.parse().ok()).unwrap_or(0),
        name: rec.get(2).unwrap_or("").to_string(),
        fen_hong: rec.get(3).and_then(|s| s.parse().ok()).unwrap_or(0.0),
        dividend_currency: rec.get(4).unwrap_or("").to_string(),
        pei_gu_jia: rec.get(5).and_then(|s| s.parse().ok()).unwrap_or(0.0),
        rights_currency: rec.get(6).unwrap_or("").to_string(),
        song_zhuan_gu: rec.get(7).and_then(|s| s.parse().ok()).unwrap_or(0.0),
        pei_gu: rec.get(8).and_then(|s| s.parse().ok()).unwrap_or(0.0),
        suo_gu: rec.get(9).and_then(|s| s.parse().ok()).unwrap_or(0.0),
        qian_liu_tong: rec.get(10).and_then(|s| s.parse().ok()).unwrap_or(0.0),
        hou_liu_tong: rec.get(11).and_then(|s| s.parse().ok()).unwrap_or(0.0),
        qian_zong_gu_ben: rec.get(12).and_then(|s| s.parse().ok()).unwrap_or(0.0),
        hou_zong_gu_ben: rec.get(13).and_then(|s| s.parse().ok()).unwrap_or(0.0),
        fen_shu: rec.get(14).and_then(|s| s.parse().ok()).unwrap_or(0.0),
        xing_quan_jia: rec.get(15).and_then(|s| s.parse().ok()).unwrap_or(0.0),
    }
}

// ============================================================
// load / save — 使用 csv crate 读写，与 Python 对齐
// ============================================================

/// 从 CSV 缓存文件加载除权除息数据
/// 与 Python `load_xdxr(inst)` 对齐
pub fn load_xdxr(inst: &Instrument) -> Vec<XdxrInfo> {
    let filename = xdxr_cache_filename(inst);
    log::debug!("Loading Xdxr data from {}", filename);

    let mut result = Vec::new();
    match std::fs::File::open(&filename) {
        Ok(f) => {
            let mut rdr = csv::ReaderBuilder::new().has_headers(true).from_reader(f);
            for row_result in rdr.records() {
                if let Ok(rec) = row_result {
                    result.push(record_to_xdxr_info(&rec));
                }
            }
        }
        Err(e) => {
            log::debug!("[xdxr] cannot open {}: {}", filename, e);
        }
    }
    result
}

/// 保存除权除息数据到 CSV 文件
/// 与 Python `save_xdxr(inst, values)` 对齐
pub fn save_xdxr(inst: &Instrument, values: &[XdxrInfo]) {
    let filename = xdxr_cache_filename(inst);
    if let Some(parent) = std::path::Path::new(&filename).parent() {
        if let Err(e) = std::fs::create_dir_all(parent) {
            log::error!("[xdxr] create_dir_all failed for {:?}: {}", parent, e);
            return;
        }
    }
    match std::fs::File::create(&filename) {
        Ok(f) => {
            let mut w = csv::Writer::from_writer(f);
            if let Err(e) = w.write_record(XDXR_CSV_HEADER) {
                log::error!("[xdxr] write header failed: {}", e);
                return;
            }
            for info in values.iter() {
                if let Err(e) = w.write_record(xdxr_info_to_record(info)) {
                    log::error!("[xdxr] write row failed: {}", e);
                }
            }
            let _ = w.flush();
        }
        Err(e) => log::error!("[xdxr] create file {} failed: {}", filename, e),
    }
}

// ============================================================
// DataXdxr — 除权除息数据适配器
// 与 Python class DataXdxr(DataAdapter) 对齐
// ============================================================

/// 除权除息数据适配器
#[derive(Debug)]
pub struct DataXdxr;

impl Schema for DataXdxr {
    fn kind(&self) -> crate::data::Kind {
        BaseXdxr
    }
    fn owner(&self) -> String {
        DEFAULT_DATA_PROVIDER.to_string()
    }
    fn key(&self) -> String {
        "xdxr".to_string()
    }
    fn name(&self) -> String {
        "除权除息".to_string()
    }
    fn usage(&self) -> String {
        "".to_string()
    }
}

impl DataAdapter for DataXdxr {
    fn print(&self, _inst: &Instrument, _dates: &[Timestamp]) {}

    fn update(&self, inst: &Instrument, _date: Timestamp) {
        let symbol = inst.symbol();
        log::debug!("[DataXdxr] update xdxr data for {}", symbol);

        // 与 C++ DataXdxr::Update / Python update_xdxr 对齐:
        //   使用批量协议拉取单只证券的除权除息数据
        let ticker = inst.market_ticker();
        let exchange = inst.exchange;

        match super::level1::std::xdxr_info::fetch_xdxr_batch(vec![(exchange, ticker.to_string())]) {
            Some(batch) => {
                let mut all_infos: Vec<XdxrInfo> = Vec::new();
                for entry in batch.list.iter() {
                    all_infos.extend(entry.list.clone());
                }
                if !all_infos.is_empty() {
                    save_xdxr(inst, &all_infos);
                } else {
                    log::debug!("[DataXdxr] no xdxr records for {}", symbol);
                }
            }
            None => {
                log::warn!("[DataXdxr] fetch_xdxr_batch returned None for {}", symbol);
            }
        }
    }
}

// ============================================================
// 注册插件 — 与 Python _data_xdxr_plugin = adapter.register(DataXdxr) 对齐
// ============================================================

pub fn init() {
    let plugin = Arc::new(DataXdxr) as Arc<dyn DataAdapter>;
    crate::data::register(plugin);
}
