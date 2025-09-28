use once_cell::sync::Lazy;
use std::sync::Mutex;
use std::path::PathBuf;
use std::fs::File;
use std::io::{BufRead, BufReader};

#[derive(Clone, Debug)]
pub struct BlockInfo {
    pub code: String,
    pub name: String,
    pub tp: u16,
    pub num: u16,
    pub block: String,
    pub constituent_stocks: Vec<String>,
}

static GLOBAL_SECTOR_LIST: Lazy<Mutex<Vec<BlockInfo>>> = Lazy::new(|| Mutex::new(Vec::new()));
static GLOBAL_SECTOR_MAP: Lazy<Mutex<std::collections::HashMap<String, BlockInfo>>> = Lazy::new(|| Mutex::new(std::collections::HashMap::new()));

fn default_block_path() -> PathBuf {
    PathBuf::from(crate::config::get_block_path())
}

fn read_config_file(fname: &str) -> Vec<(String,String,u16,String)> {
    // returns vec of (name, code, type, blockcode)
    let mut out = Vec::new();
    let mut path = default_block_path();
    path.push(fname);
    if !path.exists() { return out; }
    if let Ok(f) = File::open(path) {
        let reader = BufReader::new(f);
        for line in reader.lines().flatten() {
            // C++ code uses GBK->UTF8; config files in repo may already be UTF-8.
            let s = line.trim().to_string();
            if s.is_empty() { continue; }
            let parts: Vec<&str> = s.split('|').collect();
            if parts.len() >= 6 {
                if let Ok(tp) = parts[2].parse::<u16>() {
                    out.push((parts[0].to_string(), parts[1].to_string(), tp, parts[5].to_string()));
                }
            }
        }
    }
    out
}

fn load_index_block_infos() -> Vec<BlockInfo> {
    let mut bis = Vec::new();
    let names = ["tdxzs.cfg", "tdxzs3.cfg"];
    let mut seen = std::collections::HashSet::new();
    for n in names.iter() {
        for (name, code, tp, block) in read_config_file(n) {
            if seen.contains(&code) { continue; }
            seen.insert(code.clone());
            bis.push(BlockInfo {
                code: crate::exchange::correct_security_code(&code),
                name,
                tp,
                num: 0,
                block,
                constituent_stocks: Vec::new(),
            });
        }
    }
    bis
}

fn load_industry_blocks() -> Vec<(i32,String,String,String,String,String)> {
    // read tdxhy.cfg lines into tuples similar to C++ IndustryInfo
    let mut out = Vec::new();
    let mut path = default_block_path();
    path.push("tdxhy.cfg");
    if !path.exists() { return out; }
    if let Ok(f) = File::open(path) {
        let reader = BufReader::new(f);
        for line in reader.lines().flatten() {
            let s = line.trim().to_string();
            if s.is_empty() { continue; }
            let parts: Vec<&str> = s.split('|').collect();
            if parts.len() >= 3 {
                if let Ok(market) = parts[0].parse::<i32>() {
                    let code = crate::exchange::correct_security_code(parts[1]);
                    let block = parts[2].to_string();
                    let block5 = if block.len() >=5 { block[..5].to_string() } else { block.clone() };
                    let mut xblock = String::new();
                    let mut xblock5 = String::new();
                    if parts.len() >= 6 {
                        xblock5 = parts[5].to_string();
                        if xblock5.len() >= 6 { xblock = xblock5[..5].to_string(); }
                    }
                    out.push((market, code, block, block5, xblock, xblock5));
                }
            }
        }
    }
    out
}

fn industry_constituent_stock_list(hys: &Vec<(i32,String,String,String,String,String)>, block: &str) -> Vec<String> {
    let mut list = Vec::new();
    for v in hys.iter() {
        let block5 = &v.3;
        let xblock5 = &v.4;
        if block5.starts_with(block) || xblock5.starts_with(block) {
            list.push(v.1.clone());
        } else if block5 == block || &v.2 == block || xblock5 == block || &v.4 == block {
            list.push(v.1.clone());
        }
    }
    list.sort();
    list
}

fn parse_and_generate_block_file() -> Vec<BlockInfo> {
    let mut block_infos = load_index_block_infos();
    let mut block2name = std::collections::HashMap::new();
    for b in block_infos.iter() {
        block2name.insert(b.block.clone(), b.name.clone());
    }

    // parse raw block files (not implemented: level1 raw parsing). We'll skip that
    // and rely on config files and industry files (tdxhy.cfg) to build constituents.
    let hys = load_industry_blocks();
    for bi in block_infos.iter_mut() {
        // first try to find by block code mapping (we don't currently use the name here)
        if block2name.get(&bi.block).is_some() {
            // intentionally unused: future ports may populate additional fields
        }
        let stock_list = industry_constituent_stock_list(&hys, &bi.block);
        if !stock_list.is_empty() {
            bi.num = stock_list.len() as u16;
            bi.constituent_stocks = stock_list;
        }
    }
    block_infos.retain(|b| !b.constituent_stocks.is_empty());
    block_infos
}

pub fn sync_block_files() -> Vec<BlockInfo> {
    let list = parse_and_generate_block_file();
    let mut guard_list = GLOBAL_SECTOR_LIST.lock().unwrap();
    let mut guard_map = GLOBAL_SECTOR_MAP.lock().unwrap();
    guard_list.clear();
    guard_map.clear();
    for b in list.iter() {
        guard_map.insert(b.code.clone(), b.clone());
        guard_list.push(b.clone());
    }
    guard_list.clone()
}

pub fn get_sector_list() -> Vec<BlockInfo> {
    // lazy init
    let guard = GLOBAL_SECTOR_LIST.lock().unwrap();
    if guard.is_empty() {
        drop(guard);
        sync_block_files()
    } else {
        guard.clone()
    }
}

pub fn get_sector_map() -> std::collections::HashMap<String, BlockInfo> {
    let guard = GLOBAL_SECTOR_MAP.lock().unwrap();
    if guard.is_empty() {
        drop(guard);
        let _ = sync_block_files();
    }
    GLOBAL_SECTOR_MAP.lock().unwrap().clone()
}

pub fn get_sector_info(code: &str) -> Option<BlockInfo> {
    let map = get_sector_map();
    map.get(code).cloned()
}
