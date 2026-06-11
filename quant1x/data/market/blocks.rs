use crate::std::BinaryStream;
use flate2::read::DeflateDecoder;
use once_cell::sync::Lazy;
use std::fs;
use std::fs::File;
use std::io::{self, BufRead, BufReader, Read};
use std::path::{Path, PathBuf};
use std::sync::Mutex;
use strsim::levenshtein;
use unicode_normalization::UnicodeNormalization;
use zip::ZipArchive;

// filenames and allowed lists (no magic strings elsewhere)
const META_ADDITIONAL_FILES: [&str; 2] = ["tdxhy.cfg", "zhb.zip"];
const ALLOWED_ZHB_FILES: [&str; 2] = ["tdxzs.cfg", "tdxzs3.cfg"];

fn extract_allowed_from_zip(
    zip_path: &Path,
    output_dir: &Path,
    allowed_files: &[&str],
) -> Result<(), String> {
    let file = File::open(zip_path)
        .map_err(|e| format!("failed to open zip {}: {}", zip_path.display(), e))?;
    let mut archive = ZipArchive::new(file)
        .map_err(|e| format!("failed to read zip {}: {}", zip_path.display(), e))?;
    let allowed_set: std::collections::HashSet<String> =
        allowed_files.iter().map(|s| s.to_string()).collect();

    for i in 0..archive.len() {
        let mut entry = archive.by_index(i).map_err(|e| e.to_string())?;
        let name = entry.name().to_string();
        let base_name = std::path::Path::new(&name)
            .file_name()
            .and_then(|s| s.to_str())
            .unwrap_or("")
            .to_string();
        let want = allowed_set.contains(&name) || allowed_set.contains(&base_name);

        if !want {
            continue;
        }

        // ensure output dir exists
        fs::create_dir_all(output_dir).map_err(|e| {
            format!(
                "failed to create output dir {}: {}",
                output_dir.display(),
                e
            )
        })?;

        let out_path = output_dir.join(&base_name);

        if entry.name().ends_with('/') {
            fs::create_dir_all(&out_path).map_err(|e| e.to_string())?;
            continue;
        }

        if let Some(parent) = out_path.parent() {
            if !parent.exists() {
                fs::create_dir_all(parent).map_err(|e| e.to_string())?;
            }
        }

        let mut outfile = fs::File::create(&out_path).map_err(|e| e.to_string())?;
        io::copy(&mut entry, &mut outfile).map_err(|e| e.to_string())?;

        #[cfg(unix)]
        {
            use std::os::unix::fs::PermissionsExt;
            if let Some(mode) = entry.unix_mode() {
                fs::set_permissions(&out_path, fs::Permissions::from_mode(mode)).ok();
            }
        }
        log::info!("blocks: extracted {} -> {}", name, out_path.display());
    }

    Ok(())
}

#[derive(Clone, Debug)]
pub struct BlockInfo {
    pub code: String,
    pub name: String,
    pub tp: u16,
    pub num: u16,
    pub block: String,
    pub constituent_stocks: Vec<String>,
    pub raw_name_hex: Option<String>,
}

static GLOBAL_SECTOR_LIST: Lazy<Mutex<Vec<BlockInfo>>> = Lazy::new(|| Mutex::new(Vec::new()));
static GLOBAL_SECTOR_MAP: Lazy<Mutex<std::collections::HashMap<String, BlockInfo>>> =
    Lazy::new(|| Mutex::new(std::collections::HashMap::new()));

fn default_block_path() -> PathBuf {
    PathBuf::from(crate::config::get_block_path())
}

// Decode a raw line (bytes) into a UTF-8 Rust String.
// Heuristic: try UTF-8, then GBK (using encoding_rs); choose the one with
// more Han characters as the likely correct decoding.
fn decode_line_bytes(raw: &[u8]) -> String {
    let mut line = raw.to_vec();
    if let Some(last) = line.last() {
        if *last == b'\r' {
            line.pop();
        }
    }
    if line.iter().all(|b| b.is_ascii_whitespace()) {
        return String::new();
    }

    // UTF-8 attempt
    let utf8_s = std::str::from_utf8(&line)
        .map(|s| s.to_string())
        .unwrap_or_default();
    // GBK attempt
    let (gbk_cow, _, _) = encoding_rs::GBK.decode(&line);
    let gbk_s = gbk_cow.to_string();

    // count Han characters in a string
    fn count_han(s: &str) -> usize {
        s.chars()
            .filter(|c| {
                let u = *c as u32;
                (u >= 0x4E00 && u <= 0x9FFF)
                    || (u >= 0x3400 && u <= 0x4DBF)
                    || (u >= 0xF900 && u <= 0xFAFF)
            })
            .count()
    }

    let utf8_han = count_han(&utf8_s);
    let gbk_han = count_han(&gbk_s);
    if gbk_han > utf8_han {
        gbk_s.trim().to_string()
    } else {
        utf8_s.trim().to_string()
    }
}

fn read_config_file(fname: &str) -> Vec<BlockInfo> {
    // returns vec of BlockInfo parsed from cfg (name, code, tp, block)
    let mut out: Vec<BlockInfo> = Vec::new();
    let mut path = default_block_path();
    path.push(fname);
    log::debug!(
        "blocks: attempting to read config file: {}",
        path.to_string_lossy()
    );
    if !path.exists() {
        log::warn!("blocks: config file not found: {}", path.to_string_lossy());
        return out;
    }
    if let Ok(f) = std::fs::File::open(path) {
        let mut reader = BufReader::new(f);
        let mut linebuf: Vec<u8> = Vec::new();
        while let Ok(n) = reader.read_until(b'\n', &mut linebuf) {
            if n == 0 {
                break;
            }
            // trim trailing LF and optional CR
            if linebuf.last() == Some(&b'\n') {
                linebuf.pop();
            }
            if linebuf.last() == Some(&b'\r') {
                linebuf.pop();
            }
            if linebuf.is_empty() {
                linebuf.clear();
                continue;
            }
            let line_slice = &linebuf[..];
            // To match C++ behavior exactly, decode each config line using GBK
            // and preserve the decoded content verbatim (do not trim) so that
            // field positions and leading/trailing spaces match the C++ split
            // semantics.
            let (decoded_cow, _, _) = encoding_rs::GBK.decode(line_slice);
            let s = decoded_cow.to_string();
            if s.is_empty() {
                linebuf.clear();
                continue;
            }
            let parts: Vec<&str> = s.split('|').collect();
            if parts.len() >= 6 {
                if let Ok(tp) = parts[2].parse::<u16>() {
                    out.push(BlockInfo {
                        code: parts[1].to_string(),
                        name: parts[0].to_string(),
                        tp,
                        num: 0,
                        block: parts[5].to_string(),
                        constituent_stocks: Vec::new(),
                        raw_name_hex: None,
                    });
                }
            }
            linebuf.clear();
        }
    }
    log::debug!("blocks: parsed {} entries from {}", out.len(), fname);
    out
}

fn load_index_block_infos() -> Vec<BlockInfo> {
    let mut bis: Vec<BlockInfo> = Vec::new();
    let names = ["tdxzs.cfg", "tdxzs3.cfg"];
    let mut seen = std::collections::HashSet::new();
    for n in names.iter() {
        let entries = read_config_file(n);
        for bi in entries.into_iter() {
            if seen.contains(&bi.code) {
                continue;
            }
            seen.insert(bi.code.clone());
            // Do not normalize/correct security code here; C++ does correction
            // later when assembling final block infos. Keep the raw code.
            bis.push(bi);
        }
    }
    bis
}

fn load_industry_blocks() -> Vec<(i32, String, String, String, String, String)> {
    // read tdxhy.cfg lines into tuples similar to C++ IndustryInfo
    let mut out = Vec::new();
    let mut path = default_block_path();
    path.push("tdxhy.cfg");
    if !path.exists() {
        return out;
    }
    if let Ok(f) = std::fs::File::open(&path) {
        let mut reader = BufReader::new(f);
        let mut linebuf: Vec<u8> = Vec::new();
        while let Ok(n) = reader.read_until(b'\n', &mut linebuf) {
            if n == 0 {
                break;
            }
            if linebuf.last() == Some(&b'\n') {
                linebuf.pop();
            }
            if linebuf.last() == Some(&b'\r') {
                linebuf.pop();
            }
            if linebuf.is_empty() {
                linebuf.clear();
                continue;
            }
            let line_slice = &linebuf[..];
            let (decoded_cow, _, _) = encoding_rs::GBK.decode(line_slice);
            let s = decoded_cow.to_string();
            if s.is_empty() {
                linebuf.clear();
                continue;
            }
            let parts: Vec<&str> = s.split('|').collect();
            if parts.len() >= 3 {
                if let Ok(market) = parts[0].parse::<i32>() {
                    // C++ ignores BeiJing market entries when loading industry blocks
                    if market == crate::exchange::MARKET_BEIJING as i32 {
                        linebuf.clear();
                        continue;
                    }
                    // Keep the raw code here; normalization (prefixing/correction)
                    // is performed later to match the Go pipeline semantics.
                    let code = parts[1].to_string();
                    let block = parts[2].to_string();
                    let block5 = if block.len() >= 5 {
                        block[..5].to_string()
                    } else {
                        block.clone()
                    };
                    let mut xblock = String::new();
                    let mut xblock5 = String::new();
                    if parts.len() >= 6 {
                        xblock5 = parts[5].to_string();
                        if xblock5.len() >= 6 {
                            xblock = xblock5[..5].to_string();
                        }
                    }
                    out.push((market, code, block, block5, xblock, xblock5));
                }
            }
            linebuf.clear();
        }
    }
    out
}

fn industry_constituent_stock_list(
    hys: &Vec<(i32, String, String, String, String, String)>,
    block: &str,
) -> Vec<String> {
    let mut list = Vec::new();
    for v in hys.iter() {
        let block5 = &v.3;
        let xblock5 = &v.5;
        if block5.starts_with(block) || xblock5.starts_with(block) {
            list.push(v.1.clone());
        } else if block5 == block || &v.2 == block || xblock5 == block || &v.4 == block {
            list.push(v.1.clone());
        }
    }
    list.sort();
    list
}

fn parse_and_generate_block_file() -> Result<Vec<BlockInfo>, String> {
    // Ensure the two auxiliary resources are present. Per C++ changes, the
    // embedded .inc export was removed; instead we attempt to download the
    // following files from level1 when missing: `tdxhy.cfg` and `zhb.zip`.
    // If `zhb.zip` is downloaded we only extract the two index cfg files
    // `tdxzs.cfg` and `tdxzs3.cfg` from it.
    let block_path = default_block_path();
    for fname in META_ADDITIONAL_FILES.iter() {
        let mut p = block_path.clone();
        p.push(fname);
        if !p.exists() {
            log::info!(
                "blocks: auxiliary file {} not found locally, attempting download",
                fname
            );
            if download_block_file(fname) {
                log::info!("blocks: downloaded {}", fname);
                if *fname == "zhb.zip" {
                    let zip_path = p.clone();
                    // extract only allowed files into block path
                    match extract_allowed_from_zip(&zip_path, &block_path, &ALLOWED_ZHB_FILES) {
                        Ok(_) => log::info!(
                            "blocks: extracted allowed files from {}",
                            zip_path.display()
                        ),
                        Err(e) => {
                            log::warn!("blocks: failed to extract {}: {}", zip_path.display(), e)
                        }
                    }
                }
            } else {
                log::warn!("blocks: failed to download auxiliary file {}", fname);
            }
        }
    }

    let mut block_infos = load_index_block_infos();
    log::debug!(
        "blocks: loaded {} index block entries from tdxzs files",
        block_infos.len()
    );
    let mut block2name = std::collections::HashMap::new();
    for b in block_infos.iter() {
        block2name.insert(b.block.clone(), b.name.clone());
    }

    // parse raw block files (block.dat etc) to build a name->BlockInfo map like C++
    // use literal filenames (the C++ names are in level1/block_meta.h but the
    // Rust module is private). These match the C++ constants.
    let bks = ["block.dat", "block_gn.dat", "block_fg.dat", "block_zs.dat"];
    let mut name2block: std::collections::HashMap<String, BlockInfo> =
        std::collections::HashMap::new();
    for filename in bks.iter() {
        // ensure the raw block file exists locally; attempt download from level1 server if missing
        let mut path = default_block_path();
        path.push(filename);
        if !path.exists() {
            log::info!(
                "blocks: raw file {} not found locally, attempting download",
                filename
            );
            if !download_block_file(filename) {
                log::warn!("blocks: failed to download {}", filename);
                continue;
            }
        }
        let parsed = parse_block_raw_data(filename)?;
        // debug removed: parsed entries from raw block file
        if parsed.is_empty() {
            continue;
        }
        for bk in parsed.into_iter() {
            // C++ behavior: initialize blockName with parsed.name and then
            // try to map that name via block2name. Do not prefer parsed.block
            // (the C++ code looks up block2Name using the parsed name).
            let mut insert_name = bk.name.clone();
            if let Some(mapped) = block2name.get(&bk.name) {
                insert_name = mapped.clone();
            }
            name2block.insert(insert_name, bk);
        }
    }

    // 行业板块数据 (C++-equivalent simple assembly)
    let hys = load_industry_blocks();

    for block_info in block_infos.iter_mut() {
        let bn = block_info.name.clone();
        // Strict C++ behavior: prefer exact name-based mapping via name2block
        if let Some(_info) = name2block.get(&bn) {
            let mut list: Vec<String> = Vec::new();
            for symbol in &_info.constituent_stocks {
                if symbol.len() < 5 {
                    continue;
                }
                let (market_id, _prefix, _x2) = crate::exchange::detect_market(symbol);
                if market_id == crate::exchange::MARKET_BEIJING {
                    continue;
                }
                // Keep the raw symbol here (no prefix added). Normalization
                // will occur as a separate post-processing step, mirroring
                // the Go implementation which corrects codes later.
                list.push(symbol.clone());
            }
            block_info.num = _info.num;
            block_info.constituent_stocks = list;
            continue;
        }
        // Fallback to industry blocks like C++
        let stock_list = industry_constituent_stock_list(&hys, &block_info.block);
        if !stock_list.is_empty() {
            block_info.num = stock_list.len() as u16;
            block_info.constituent_stocks = stock_list;
        }
    }

    // Normalize block and constituent codes to the canonical exchange format
    // after assembly (this mirrors Go's `loadCacheBlockInfos` which
    // corrects codes when loading the final CSV). Doing normalization
    // here keeps the Rust flow consistent with Go.
        for b in block_infos.iter_mut() {
            b.code = crate::exchange::correct_security_code(&b.code);
            for i in 0..b.constituent_stocks.len() {
                let corrected = crate::exchange::correct_security_code(&b.constituent_stocks[i]);
                b.constituent_stocks[i] = corrected;
            }
        }

        block_infos.retain(|b| !b.constituent_stocks.is_empty());

        // Persist CSV cache under meta path as blocks.<date> (same as Go).
        if !block_infos.is_empty() {
            let cache_date = crate::meta::last_trading_day(crate::meta::Timestamp::now()).only_date();
            let mut csv_path = std::path::PathBuf::from(crate::config::get_meta_path());
            let filename = format!("blocks.{}", cache_date);
            csv_path.push(filename);
            if let Some(parent) = csv_path.parent() {
                if let Err(e) = std::fs::create_dir_all(parent) {
                    log::warn!("blocks: failed to create meta dir {:?}: {}", parent, e);
                }
            }
            let tmp = format!("{}.tmp", csv_path.to_string_lossy());
            match std::fs::File::create(&tmp) {
                Ok(f) => {
                    let mut w = csv::WriterBuilder::new().has_headers(true).from_writer(f);
                    // header order: name,code,type,count,block,constituent_stocks
                    if let Err(e) = w.write_record(&[
                        "name",
                        "code",
                        "type",
                        "count",
                        "block",
                        "constituent_stocks",
                    ])
                    {
                        log::error!("blocks: failed to write header to {}: {}", tmp, e);
                    } else {
                        for b in block_infos.iter() {
                            // Serialize constituent list as JSON string (same as C++ writer)
                            let constituents = match serde_json::to_string(&b.constituent_stocks) {
                                Ok(s) => s,
                                Err(_) => String::new(),
                            };
                            let record = [
                                b.name.clone(),
                                b.code.clone(),
                                b.tp.to_string(),
                                b.num.to_string(),
                                b.block.clone(),
                                constituents,
                            ];
                            if let Err(e) = w.write_record(record.iter()) {
                                log::error!("blocks: failed to write record to {}: {}", tmp, e);
                            }
                        }
                    }
                    if let Err(e) = w.flush() {
                        log::warn!("blocks: failed to flush {}: {}", tmp, e);
                    }
                    if let Err(e) = std::fs::rename(&tmp, &csv_path) {
                        log::warn!("blocks: failed to rename {} -> {}: {}", tmp, csv_path.display(), e);
                    } else {
                        log::debug!("blocks: wrote block csv: {}", csv_path.display());
                    }
                }
                Err(e) => {
                    log::warn!("blocks: failed to create tmp csv {}: {}", tmp, e);
                }
            }
        }

    Ok(block_infos)
}

fn download_block_file(fname: &str) -> bool {
    // Downloads a block file from level1 in chunks and saves it to the block path.
    // Returns true on success.
    let mut total: Vec<u8> = Vec::new();
    let mut offset: u32 = 0;
    let chunk_size = crate::contrib::data::tdx::standard::block_meta::BLOCK_CHUNKS_SIZE;
    loop {
        match crate::contrib::data::tdx::standard::block_info::fetch_block_info(fname, offset) {
            Some(resp) => {
                // log per-chunk diagnostic: header size and actual data length
                log::info!(
                    "blocks: chunk for {} @ offset {} -> header.size={} data.len={}",
                    fname,
                    offset,
                    resp.size,
                    resp.data.len()
                );

                if resp.size == 0 || resp.data.is_empty() {
                    log::warn!(
                        "blocks: empty chunk received for {} offset {}",
                        fname,
                        offset
                    );
                    break;
                }

                if (resp.data.len() as u32) != resp.size {
                    log::warn!(
                        "blocks: size header ({}) != actual data.len ({}) for {} offset {}",
                        resp.size,
                        resp.data.len(),
                        fname,
                        offset
                    );
                }

                total.extend_from_slice(&resp.data);

                // if the returned chunk (header size) is less than chunk_size, C++ treats that as final
                if resp.size < chunk_size {
                    log::info!(
                        "blocks: final chunk detected for {} (header.size {} < chunk_size {})",
                        fname,
                        resp.size,
                        chunk_size
                    );
                    break;
                }

                // advance offset by the Size field from response (C++ advances by response.Size)
                offset = offset.saturating_add(resp.size);
            }
            None => {
                log::error!(
                    "blocks: level1 fetch returned error for {} offset {}",
                    fname,
                    offset
                );
                return false;
            }
        }
    }

    if total.is_empty() {
        log::warn!("blocks: no data downloaded for {}", fname);
        return false;
    }

    // write to disk
    let mut path = default_block_path();
    if let Err(e) = std::fs::create_dir_all(&path) {
        log::error!("blocks: failed to create block path {:?}: {}", path, e);
        return false;
    }
    path.push(fname);
    match std::fs::write(&path, &total) {
        Ok(_) => {
            log::info!(
                "blocks: saved {} ({} bytes)",
                path.to_string_lossy(),
                total.len()
            );
            true
        }
        Err(e) => {
            log::error!("blocks: failed to write {}: {}", path.to_string_lossy(), e);
            false
        }
    }
}

fn parse_block_raw_data(fname: &str) -> Result<Vec<BlockInfo>, String> {
    let mut out = Vec::new();
    let mut path = default_block_path();
    path.push(fname);
    log::debug!(
        "blocks: attempting to parse raw block file: {}",
        path.to_string_lossy()
    );
    if !path.exists() {
        log::warn!(
            "blocks: raw block file not found: {}",
            path.to_string_lossy()
        );
        return Ok(out);
    }
    let data = match std::fs::read(&path) {
        Ok(d) => d,
        Err(e) => {
            log::error!("blocks: failed to read {}: {}", path.to_string_lossy(), e);
            return Ok(out);
        }
    };
    if data.len() < 4 {
        return Ok(out);
    }
    let mut bs = BinaryStream::from_vec(data);
    // mirror C++: skip header bytes
    bs.skip(384);
    // read u16 count (ensure enough bytes available)
    if bs.position() + 2 > bs.data().len() {
        return Ok(out);
    }
    let count = bs.get_u16()? as usize;
    for _ in 0..count {
        // tmpBuf1[2813]
        let mut tmp1 = vec![0u8; 2813];
        if bs.position() + tmp1.len() > bs.data().len() {
            break;
        }
        bs.get_byte_array(&mut tmp1)?;
        // name: first 9 bytes; C++ forms a std::string from these raw bytes then
        // runs gbk_to_utf8 on that string. To match that exactly, decode the raw
        // 9 bytes using GBK (do not first run UTF-8 lossy conversion).
        let raw_name_bytes = &tmp1[..9];
        // find NUL within the 9 bytes to trim, but keep the raw bytes for GBK decode
        let name_nul = raw_name_bytes.iter().position(|&b| b == 0).unwrap_or(9);
        let (gbk_cow, _, _) = encoding_rs::GBK.decode(&raw_name_bytes[..name_nul]);
        let name = gbk_cow.to_string().trim().to_string();
        // record raw hex of the name bytes (for diagnostics)
        let raw_name_hex = raw_name_bytes[..name_nul]
            .iter()
            .map(|b| format!("{:02X}", b))
            .collect::<Vec<String>>()
            .join("");
        // num and type are stored at offset 9 and 11 in tmp1 (per C++ layout)
        let mut bs1 = BinaryStream::from_vec(tmp1.clone());
        bs1.seek(9);
        // ensure tmp1 had enough bytes for these reads
        if bs1.position() + 4 > bs1.data().len() {
            break;
        }
        let num = bs1.get_u16()?;
        let tp = bs1.get_u16()?;
        // tmpBuf2[400*7] comes from tmp1 (embedded inside tmpBuf1) per C++ layout
        let mut tmp2 = vec![0u8; 400 * 7];
        if bs1.position() + tmp2.len() > bs1.data().len() {
            break;
        }
        bs1.get_byte_array(&mut tmp2)?;
        let mut bs2 = BinaryStream::from_vec(tmp2.clone());
        let mut constituents = Vec::new();
        // guard against malformed num > available slots
        let available = tmp2.len() / 7;
        let to_read = std::cmp::min(num as usize, available);
        for _j in 0..to_read {
            // each symbol fixed 7 bytes
            let sym = bs2.get_string(7)?;
            constituents.push(sym);
        }
        out.push(BlockInfo {
            code: String::new(),
            name,
            tp,
            num,
            block: String::new(),
            constituent_stocks: constituents,
            raw_name_hex: Some(raw_name_hex),
        });
    }
    Ok(out)
}

pub fn sync_block_files() -> Vec<BlockInfo> {
    let list = match parse_and_generate_block_file() {
        Ok(l) => l,
        Err(e) => {
            log::error!("blocks: failed to parse/generate block files: {}", e);
            return Vec::new();
        }
    };
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
