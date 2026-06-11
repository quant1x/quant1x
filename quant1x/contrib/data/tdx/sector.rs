// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// sector — 板块列表加载，与 Python contrib/data/tdx/sector.py 对齐
// 作为 datasource 的本地代理，从缓存CSV加载板块数据

use crate::data::schema::Sector;
use std::io::Read;
use std::sync::Mutex;
use once_cell::sync::Lazy;

/// 内存缓存: 板块列表
static SECTOR_CACHE: Lazy<Mutex<Option<Vec<Sector>>>> = Lazy::new(|| Mutex::new(None));

/// 获取板块缓存文件路径
/// 与 Python get_sector_filename() 对齐: {meta_path}/blocks.{last_trading_day}
fn get_sector_filename() -> String {
    let current_date = crate::data::meta::Timestamp::now();
    let cache_date = crate::data::meta::calendar::last_trading_day(current_date).only_date();
    let filename = format!("blocks.{}", cache_date);
    let meta_path = crate::config::get_meta_path();
    format!("{}/{}", meta_path, filename)
}

/// 从CSV加载板块列表
fn load_sectors_from_csv(filename: &str) -> Option<Vec<Sector>> {
    let mut file = std::fs::File::open(filename).ok()?;
    let mut contents = String::new();
    file.read_to_string(&mut contents).ok()?;

    let mut rdr = csv::ReaderBuilder::new()
        .has_headers(true)
        .from_reader(contents.as_bytes());

    let mut sectors: Vec<Sector> = Vec::new();
    for result in rdr.records() {
        if let Ok(rec) = result {
            let name = rec.get(0).unwrap_or("").to_string();
            let code = rec.get(1).unwrap_or("").to_string();
            let sector_type: i32 = rec.get(2).and_then(|s| s.parse().ok()).unwrap_or(0);
            let count: i32 = rec.get(3).and_then(|s| s.parse().ok()).unwrap_or(0);
            let block = rec.get(4).unwrap_or("").to_string();
            // constituent_stocks 是JSON数组字符串
            let constituent_stocks_str = rec.get(5).unwrap_or("[]");
            let constituent_stocks: Vec<String> = serde_json::from_str(constituent_stocks_str)
                .unwrap_or_default();

            sectors.push(Sector {
                name,
                code,
                sector_type,
                count,
                block,
                constituent_stocks,
            });
        }
    }

    if sectors.is_empty() {
        None
    } else {
        Some(sectors)
    }
}

/// 获取板块列表
/// 与 Python get_sector_list() 对齐
pub fn get_sector_list() -> Vec<Sector> {
    // 先检查缓存
    {
        let cache = SECTOR_CACHE.lock().unwrap();
        if let Some(ref sectors) = *cache {
            return sectors.clone();
        }
    }

    // 尝试加载
    let filename = get_sector_filename();
    if let Some(sectors) = load_sectors_from_csv(&filename) {
        let mut cache = SECTOR_CACHE.lock().unwrap();
        *cache = Some(sectors.clone());
        return sectors;
    }

    Vec::new()
}
