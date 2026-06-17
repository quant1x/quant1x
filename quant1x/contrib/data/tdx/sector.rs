// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// sector — 板块列表加载与下载, 与 Python contrib/data/tdx/sector.py 对齐
// 作为 datasource 的本地代理, 从 level1 下载板块文件并解析为 CSV 缓存

use crate::data::schema::Sector;
use std::collections::HashMap;
use std::fs;
use std::io::{Read, Write};
use std::path::Path;
use std::sync::Mutex;
use once_cell::sync::Lazy;

use encoding::{DecoderTrap, Encoding};
use encoding::all::GBK;

use super::level1::std::block::BlockInfo;
use super::level1::std::block_meta::{
    BLOCK_DEFAULT, BLOCK_FENGGE, BLOCK_GAINIAN, BLOCK_ZHISHU,
};
use super::protocol::process_level1_stream;
use super::client::get_std_conn;

/// 内存缓存: 板块列表
static SECTOR_CACHE: Lazy<Mutex<Option<Vec<Sector>>>> = Lazy::new(|| Mutex::new(None));

/// 获取板块缓存文件路径
/// 与 Python get_sector_filename() 对齐: {meta_path}/blocks.{last_trading_day}
pub fn get_sector_filename() -> String {
    let current_date = crate::data::meta::Timestamp::now();
    let cache_date = crate::data::meta::calendar::last_trading_day(current_date).only_date();
    let filename = format!("blocks.{}", cache_date);
    let meta_path = crate::config::get_meta_path();
    format!("{}/{}", meta_path, filename)
}

// ============================================================
// 从 level1 下载原始板块文件
// ============================================================

/// 从 level1 连接获取板块原始数据, 对应 Python `_get_block_info_from_level1`
fn get_block_info_from_level1(filename: &str) -> Option<Vec<u8>> {
    let mut conn = match get_std_conn() {
        Ok(c) => c,
        Err(e) => {
            log::error!("sector: get_std_conn failed: {}", e);
            return None;
        }
    };

    let mut start: u32 = 0;
    let mut result: Vec<u8> = Vec::new();
    loop {
        let mut msg = BlockInfo::new(filename, start);
        match process_level1_stream(conn.stream(), &mut msg) {
            Ok(()) => {}
            Err(e) => {
                log::error!("sector: process_level1_stream for {} at offset {} failed: {}", filename, start, e);
                return None;
            }
        }
        if msg.size == 0 {
            return None;
        }
        if msg.size > 0 {
            result.extend_from_slice(&msg.data);
        }
        if msg.size < super::level1::std::block::BLOCK_CHUNKS_SIZE as u32 {
            break;
        }
        start += msg.size;
    }
    Some(result)
}

/// 下载原始板块文件到 meta_path, 对应 Python `download_block_raw_data`
fn download_block_raw_data(filename: &str) -> Option<String> {
    let meta_path = crate::config::get_meta_path();
    let _ = fs::create_dir_all(&meta_path);
    let filepath = format!("{}/{}", meta_path, filename);

    // 文件已存在且不需要更新, 跳过下载
    if Path::new(&filepath).exists() {
        if !crate::data::status::should_initialize_file(&filepath, crate::data::meta::exchange::Exchange::SSE) {
            log::debug!("sector: {} exists and is up-to-date, skip download", filename);
            return Some(filepath);
        }
    }

    let data = get_block_info_from_level1(filename)?;
    if data.is_empty() {
        return None;
    }

    match fs::write(&filepath, &data) {
        Ok(()) => Some(filepath),
        Err(e) => {
            log::error!("sector: failed to write {}: {}", filepath, e);
            None
        }
    }
}

// ============================================================
// 解析原始板块文件
// ============================================================

/// 原始板块记录
#[derive(Debug, Clone)]
struct RawBlockRecord {
    block_name: String,
    num: u16,
    block_type: u16,
    codes: Vec<String>,
}

/// 解析原始板块二进制文件, 对应 Python `parse_raw_block_file`
fn parse_raw_block_file(block_filename: &str) -> Vec<RawBlockRecord> {
    let meta_path = crate::config::get_meta_path();
    let filepath = format!("{}/{}", meta_path, block_filename);
    if !Path::new(&filepath).exists() {
        return vec![];
    }

    let data = match fs::read(&filepath) {
        Ok(d) => d,
        Err(_) => return vec![],
    };

    // skip 384 bytes header
    if data.len() < 386 {
        return vec![];
    }
    let offset_after_header = 384;
    if data.len() < offset_after_header + 2 {
        return vec![];
    }
    let count = u16::from_le_bytes([data[offset_after_header], data[offset_after_header + 1]]) as usize;

    let mut records = Vec::with_capacity(count);
    let mut pos = offset_after_header + 2;

    for _ in 0..count {
        if pos + 2813 > data.len() {
            break;
        }
        let rec = &data[pos..pos + 2813];
        pos += 2813;

        // name: first 9 bytes
        let name_bytes = &rec[0..9];
        let name = extract_null_terminated_gbk(name_bytes);

        // num: 2 bytes at offset 9
        let num = u16::from_le_bytes([rec[9], rec[10]]);

        // block_type: 2 bytes at offset 11
        let block_type = u16::from_le_bytes([rec[11], rec[12]]);

        // codes: up to 400 codes, each 7 bytes, starting at offset 13
        let mut codes = Vec::new();
        let mut code_pos = 13;
        for _ in 0..400 {
            if code_pos + 7 > rec.len() {
                break;
            }
            let code_bytes = &rec[code_pos..code_pos + 7];
            code_pos += 7;
            let code = extract_null_terminated_ascii(code_bytes);
            if !code.is_empty() {
                codes.push(code);
            }
        }

        records.push(RawBlockRecord {
            block_name: name,
            num,
            block_type,
            codes,
        });
    }

    records
}

/// 从字节中提取以 \x00 结尾的 GBK 字符串
fn extract_null_terminated_gbk(data: &[u8]) -> String {
    let end = data.iter().position(|&b| b == 0).unwrap_or(data.len());
    let slice = &data[..end];
    GBK.decode(slice, DecoderTrap::Replace)
        .unwrap_or_else(|_| String::from_utf8_lossy(slice).to_string())
}

/// 从字节中提取以 \x00 结尾的 ASCII 字符串
fn extract_null_terminated_ascii(data: &[u8]) -> String {
    let end = data.iter().position(|&b| b == 0).unwrap_or(data.len());
    let slice = &data[..end];
    String::from_utf8_lossy(slice).to_string()
}

// ============================================================
// 解析配置文件 (tdxzs.cfg / tdxzs3.cfg)
// ============================================================

/// 配置文件中的板块索引条目
#[derive(Debug, Clone)]
struct BlockIndexEntry {
    name: String,
    code: String,
    block_type: i32,
    block: String,
}

/// 从配置文件加载板块索引, 对应 Python `get_block_info_from_config`
fn get_block_info_from_config(cfg_name: &str) -> Vec<BlockIndexEntry> {
    let meta_path = crate::config::get_meta_path();
    let filepath = format!("{}/{}", meta_path, cfg_name);
    if !Path::new(&filepath).exists() {
        return vec![];
    }

    let content = match fs::read_to_string(&filepath) {
        Ok(c) => c,
        Err(_) => {
            // try GBK
            match fs::read(&filepath) {
                Ok(bytes) => {
                    GBK.decode(&bytes, DecoderTrap::Replace)
                        .unwrap_or_default()
                }
                Err(_) => return vec![],
            }
        }
    };

    let mut entries = Vec::new();
    for line in content.lines() {
        let line = line.trim();
        if line.is_empty() {
            continue;
        }
        let parts: Vec<&str> = line.split('|').collect();
        if parts.len() < 4 {
            continue;
        }
        entries.push(BlockIndexEntry {
            name: parts[0].to_string(),
            code: parts[1].to_string(),
            block_type: parts[2].parse().unwrap_or(0),
            block: if parts.len() > 5 { parts[5].to_string() } else { String::new() },
        });
    }
    entries
}

// ============================================================
// 行业配置
// ============================================================

/// 行业信息, 对应 Python `IndustryInfo`
#[derive(Debug, Clone)]
struct IndustryInfo {
    market_id: i32,
    code: String,
    block: String,
    block5: String,
    xblock: String,
    xblock5: String,
}

/// 加载行业配置, 对应 Python `load_industry_blocks`
fn load_industry_blocks() -> Vec<IndustryInfo> {
    let meta_path = crate::config::get_meta_path();
    let filepath = format!("{}/tdxhy.cfg", meta_path);
    if !Path::new(&filepath).exists() {
        return vec![];
    }

    let content = match fs::read_to_string(&filepath) {
        Ok(c) => c,
        Err(_) => {
            match fs::read(&filepath) {
                Ok(bytes) => {
                    GBK.decode(&bytes, DecoderTrap::Replace)
                        .unwrap_or_default()
                }
                Err(_) => return vec![],
            }
        }
    };

    let mut out = Vec::new();
    for line in content.lines() {
        let line = line.trim();
        if line.is_empty() {
            continue;
        }
        let arr: Vec<&str> = line.split('|').collect();
        if arr.len() < 3 {
            continue;
        }
        let bc = arr.get(2).map(|s| s.to_string()).unwrap_or_default();
        let bc5 = if bc.len() >= 5 { bc[..5].to_string() } else { bc.clone() };
        let xbc5 = arr.get(5).map(|s| s.to_string()).unwrap_or_default();
        let xbc = if xbc5.len() >= 5 { xbc5[..5].to_string() } else { xbc5.clone() };
        let market_id = arr[0].parse::<i32>().unwrap_or(0);

        out.push(IndustryInfo {
            market_id,
            code: arr[1].to_string(),
            block: bc,
            block5: bc5,
            xblock: xbc,
            xblock5: xbc5,
        });
    }
    out
}

/// 行业成分股列表, 对应 Python `industry_constituent_stock_list`
fn industry_constituent_stock_list(hys: &[IndustryInfo], block: &str) -> Vec<String> {
    let mut lst: Vec<String> = Vec::new();
    for v in hys {
        let matched = v.block5.starts_with(block)
            || v.xblock5.starts_with(block)
            || v.block5 == block
            || v.block == block
            || v.xblock5 == block
            || v.xblock == block;
        if matched {
            lst.push(v.code.clone());
        }
    }
    lst.sort();
    lst.dedup();
    lst
}

// ============================================================
// 解析并生成板块CSV缓存文件
// ============================================================

/// 解析所有原始文件并生成板块CSV, 对应 Python `parse_and_generate_block_file`
fn parse_and_generate_block_file() -> Option<String> {
    // 1) 加载 zs* 配置文件
    let bks_cfg = ["tdxzs.cfg", "tdxzs3.cfg"];
    let mut block_index: Vec<BlockIndexEntry> = Vec::new();
    let mut tmp_map: HashMap<String, BlockIndexEntry> = HashMap::new();
    for cfg in &bks_cfg {
        let bi = get_block_info_from_config(cfg);
        for v in bi {
            if tmp_map.contains_key(&v.code) {
                continue;
            }
            tmp_map.insert(v.code.clone(), v.clone());
            block_index.push(v);
        }
    }

    if block_index.is_empty() {
        log::warn!("sector: no block index entries found from config files");
        return None;
    }

    // block -> name mapping
    let block2name: HashMap<String, String> = block_index
        .iter()
        .filter(|v| !v.block.is_empty())
        .map(|v| (v.block.clone(), v.name.clone()))
        .collect();

    // 2) 解析原始板块文件
    let raw_files = [BLOCK_DEFAULT, BLOCK_GAINIAN, BLOCK_FENGGE, BLOCK_ZHISHU];
    let mut name2block: HashMap<String, RawBlockRecord> = HashMap::new();
    for f in &raw_files {
        let recs = parse_raw_block_file(f);
        for bk in recs {
            let block_name = if let Some(resolved) = block2name.get(&bk.block_name) {
                resolved.clone()
            } else {
                bk.block_name.clone()
            };
            name2block.insert(block_name, bk);
        }
    }

    // 3) code->hy mapping
    let mut code2hy: HashMap<String, String> = HashMap::new();
    for v in &block_index {
        if v.name != v.block {
            code2hy.insert(v.block.clone(), v.name.clone());
        }
    }

    // 4) industry blocks
    let hys = load_industry_blocks();

    // 5) 组装最终板块条目
    let mut rows: Vec<(String, String, i32, i32, String, Vec<String>)> = Vec::new();
    for v in &block_index {
        if let Some(info) = name2block.get(&v.name) {
            let mut entry_codes: Vec<String> = info.codes.iter()
                .filter(|s| s.len() >= 5)
                .cloned()
                .collect();
            entry_codes.sort();
            entry_codes.dedup();
            let count = entry_codes.len() as i32;
            rows.push((
                v.name.clone(),
                v.code.clone(),
                v.block_type,
                count,
                v.block.clone(),
                entry_codes,
            ));
            continue;
        }

        // fallback: industry mapping
        let bc = &v.block;
        let stock_list = industry_constituent_stock_list(&hys, bc);
        if !stock_list.is_empty() {
            rows.push((
                v.name.clone(),
                v.code.clone(),
                v.block_type,
                stock_list.len() as i32,
                v.block.clone(),
                stock_list,
            ));
        }
    }

    // 过滤空条目
    rows.retain(|r| !r.5.is_empty());

    if rows.is_empty() {
        return None;
    }

    // 写入CSV
    let out_fn = get_sector_filename();
    if let Some(parent) = Path::new(&out_fn).parent() {
        let _ = fs::create_dir_all(parent);
    }

    let mut wtr = csv::Writer::from_path(&out_fn).ok()?;
    for (name, code, btype, count, block, constituent_stocks) in &rows {
        let cs_json = serde_json::to_string(constituent_stocks).unwrap_or_else(|_| "[]".to_string());
        let _ = wtr.write_record(&[
            name,
            code,
            &btype.to_string(),
            &count.to_string(),
            block,
            &cs_json,
        ]);
    }
    let _ = wtr.flush();

    Some(out_fn)
}

// ============================================================
// 同步板块文件(下载 + 解析 + 生成CSV)
// ============================================================

/// 同步所有板块文件, 对应 Python `sync_block_files`
pub fn sync_block_files() -> Option<String> {
    log::info!("sector: sync_block_files start");

    // 行业配置
    download_block_raw_data("tdxhy.cfg");

    // 下载 zip 并解压
    if let Some(zhb) = download_block_raw_data("zhb.zip") {
        if let Ok(file) = fs::File::open(&zhb) {
            if let Ok(mut archive) = zip::ZipArchive::new(file) {
                let need_files = ["tdxzs.cfg", "tdxzs3.cfg"];
                for i in 0..archive.len() {
                    if let Ok(mut file) = archive.by_index(i) {
                        let base_opt = file.enclosed_name()
                            .and_then(|n| n.file_name().map(|s| s.to_string_lossy().to_string()));
                        if let Some(base) = base_opt {
                            if need_files.contains(&base.as_str()) {
                                let meta_path = crate::config::get_meta_path();
                                let out_path = format!("{}/{}", meta_path, base);
                                if let Ok(mut out) = fs::File::create(&out_path) {
                                    let _ = std::io::copy(&mut file, &mut out);
                                    log::debug!("sector: extracted {} from zhb.zip", base);
                                }
                            }
                        }
                    }
                }
            } else {
                log::warn!("sector: failed to open zhb.zip as zip archive");
            }
        } else {
            log::warn!("sector: failed to open zhb.zip file");
        }
    }

    // 下载标准板块文件
    for fname in &["block.dat", "block_gn.dat", "block_fg.dat", "block_zs.dat"] {
        download_block_raw_data(fname);
    }

    // 解析并生成CSV
    let result = parse_and_generate_block_file();
    log::info!("sector: sync_block_files done, result={:?}", result);
    result
}

// ============================================================
// 缓存加载
// ============================================================

/// 加载缓存板块数据, 必要时触发同步
/// 对应 Python `load_cache_block_infos`
fn load_cache_block_infos() {
    let bk_filename = get_sector_filename();

    // 如果CSV不存在或需要更新, 先同步
    if !Path::new(&bk_filename).exists() || crate::data::status::should_initialize_file(&bk_filename, crate::data::meta::exchange::Exchange::SSE) {
        log::info!("sector: cache missing or outdated, triggering sync_block_files");
        match sync_block_files() {
            Some(_) => {}
            None => {
                log::warn!("sector: sync_block_files returned None");
            }
        }
    }

    // 从CSV加载
    let sectors = load_sectors_from_csv(&bk_filename).unwrap_or_default();
    let mut cache = SECTOR_CACHE.lock().unwrap();
    *cache = Some(sectors);
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

// ============================================================
// 公共API
// ============================================================

/// 获取板块列表
/// 与 Python get_sector_list() 对齐
/// 首次调用时会自动触发板块文件下载和解析
pub fn get_sector_list() -> Vec<Sector> {
    // 先检查缓存
    {
        let cache = SECTOR_CACHE.lock().unwrap();
        if let Some(ref sectors) = *cache {
            return sectors.clone();
        }
    }

    // 触发加载(含首次同步)
    load_cache_block_infos();

    let cache = SECTOR_CACHE.lock().unwrap();
    cache.as_ref().map(|s| s.clone()).unwrap_or_default()
}

// ============================================================
// 测试
// ============================================================

#[cfg(test)]
mod tests {
    use super::*;

    /// 测试 get_sector_list 基本功能
    /// 需要网络连接从 level1 下载板块数据
    /// 标记为 #[ignore] 避免 CI 环境因无网络而失败
    #[test]
    #[ignore]
    fn test_get_sector_list() {
        let _ = env_logger::try_init();
        let sectors = get_sector_list();
        log::info!("[sector test] get_sector_list returned {} sectors", sectors.len());
        assert!(!sectors.is_empty(), "sector list should not be empty");
        for s in &sectors {
            assert!(!s.name.is_empty(), "sector name should not be empty");
            assert!(!s.code.is_empty(), "sector code should not be empty");
            assert!(s.count >= 0, "sector count should be non-negative");
            log::debug!(
                "[sector test] name={}, code={}, type={}, count={}",
                s.name, s.code, s.sector_type, s.count
            );
        }
    }

    /// 测试缓存命中: 第二次调用应该直接返回缓存
    #[test]
    #[ignore]
    fn test_get_sector_list_cached() {
        let _ = env_logger::try_init();
        // 第一次调用: 触发下载+解析
        let sectors1 = get_sector_list();
        assert!(!sectors1.is_empty());
        // 第二次调用: 应该命中缓存
        let sectors2 = get_sector_list();
        assert!(!sectors2.is_empty());
        assert_eq!(sectors1.len(), sectors2.len());
        log::info!("[sector test] cache hit OK, {} sectors", sectors1.len());
    }

    /// 测试 sync_block_files 直接同步
    #[test]
    #[ignore]
    fn test_sync_block_files() {
        let _ = env_logger::try_init();
        let result = sync_block_files();
        assert!(result.is_some(), "sync_block_files should return a file path");
        let path = result.unwrap();
        log::info!("[sector test] sync_block_files produced: {}", path);
        assert!(Path::new(&path).exists(), "output file should exist");
    }

    /// 测试 get_sector_filename 格式
    #[test]
    fn test_get_sector_filename_format() {
        let filename = get_sector_filename();
        // 应包含 "blocks." 格式
        assert!(filename.contains("blocks."), "filename should contain 'blocks.'");
        log::info!("[sector test] sector filename: {}", filename);
    }

    /// 测试 extract_null_terminated_gbk
    #[test]
    fn test_extract_null_terminated_gbk() {
        // 简单 GBK 编码的 "测试" 后跟 null 终止符
        let data = [0xb2, 0xe2, 0xca, 0xd4, 0x00];
        let result = extract_null_terminated_gbk(&data);
        assert_eq!(result, "测试");
    }

    /// 测试 extract_null_terminated_ascii
    #[test]
    fn test_extract_null_terminated_ascii() {
        let data = [b'h', b'e', b'l', b'l', b'o', 0x00, b'x'];
        let result = extract_null_terminated_ascii(&data);
        assert_eq!(result, "hello");

        // 无 null 终止符
        let data = [b'a', b'b', b'c'];
        let result = extract_null_terminated_ascii(&data);
        assert_eq!(result, "abc");

        // 空数据
        let data: [u8; 0] = [];
        let result = extract_null_terminated_ascii(&data);
        assert_eq!(result, "");
    }

    /// 测试 parse_raw_block_file 解析不存在的文件
    #[test]
    fn test_parse_raw_block_file_nonexistent() {
        let recs = parse_raw_block_file("nonexistent.dat");
        assert!(recs.is_empty(), "should return empty for nonexistent file");
    }

    /// 测试 load_industry_blocks 加载不存在的文件
    #[test]
    fn test_load_industry_blocks_nonexistent() {
        let hys = load_industry_blocks();
        // 如果没有 tdxhy.cfg, 应返回空
        // 如果存在(测试环境), 至少有数据
        log::info!("[sector test] load_industry_blocks returned {} entries", hys.len());
    }

    /// 测试 industry_constituent_stock_list 基本逻辑
    #[test]
    fn test_industry_constituent_stock_list() {
        let hys = vec![
            IndustryInfo {
                market_id: 0,
                code: "000001".to_string(),
                block: "BK0001".to_string(),
                block5: "BK000".to_string(),
                xblock: "XB0001".to_string(),
                xblock5: "XB000".to_string(),
            },
            IndustryInfo {
                market_id: 0,
                code: "000002".to_string(),
                block: "BK0002".to_string(),
                block5: "BK000".to_string(),
                xblock: "XB0002".to_string(),
                xblock5: "XB000".to_string(),
            },
            IndustryInfo {
                market_id: 1,
                code: "600000".to_string(),
                block: "BK0001".to_string(),
                block5: "BK000".to_string(),
                xblock: "XB0001".to_string(),
                xblock5: "XB000".to_string(),
            },
        ];

        // block 精确匹配
        let result = industry_constituent_stock_list(&hys, "BK0001");
        assert_eq!(result, vec!["000001", "600000"]);

        // block5 前缀匹配
        let result = industry_constituent_stock_list(&hys, "BK000");
        assert_eq!(result, vec!["000001", "000002", "600000"]);

        // 无匹配
        let result = industry_constituent_stock_list(&hys, "NOTEXIST");
        assert!(result.is_empty());
    }
}
