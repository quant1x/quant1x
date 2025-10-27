use crate::cache::{self, DataAdapter, Kind};
use crate::Timestamp;
use std::sync::Arc;

/// DataXdxr: query level1 xdxr response and write local CSV cache
#[derive(Debug)]
pub struct DataXdxr;

impl cache::Schema for DataXdxr {
    fn kind(&self) -> Kind {
        cache::PLUGIN_MASK_BASE_DATA | 1
    }
    fn owner(&self) -> String {
        cache::DEFAULT_DATA_PROVIDER.to_string()
    }
    fn key(&self) -> String {
        "xdxr".to_string()
    }
    fn name(&self) -> String {
        "除权除息".to_string()
    }
    fn usage(&self) -> String {
        String::new()
    }
}

impl cache::DataAdapter for DataXdxr {
    fn print(&self, _code: &str, _dates: &[Timestamp]) {}

    fn update(&self, code: &str, _date: Timestamp) {
        // call into level1 client to fetch xdxr (if available)
        if let Some(resp) = crate::level1::fetch_xdxr(code) {
            if !resp.list.is_empty() {
                // write CSV file using C++ header order
                let filename = crate::config::get_xdxr_filename(code);
                if let Some(parent) = std::path::Path::new(&filename).parent() {
                    if let Err(e) = std::fs::create_dir_all(parent) {
                        log::error!("Failed to create parent dir {:?}: {}", parent, e);
                        return;
                    }
                }
                let tmp = format!("{}.tmp", filename);
                match std::fs::File::create(&tmp) {
                    Ok(f) => {
                        // Use csv::Writer + serde to serialize rows in the same column order as C++ header
                        let mut w = csv::WriterBuilder::new().has_headers(true).from_writer(f);
                        // if let Err(e) = w.write_record(crate::level1::xdxr::XdxrInfo::headers()) {
                        //     log::error!("Failed to write header to tmp file {}: {}", tmp, e);
                        //     return;
                        // }
                        for v in resp.list.iter() {
                            if let Err(e) = w.serialize(v) {
                                log::error!("Failed to serialize row to tmp file {}: {}", tmp, e);
                                return;
                            }
                        }
                        if let Err(e) = w.flush() {
                            log::error!("Failed to flush tmp file {}: {}", tmp, e);
                        }
                        if let Err(e) = std::fs::rename(&tmp, &filename) {
                            log::error!("Failed to rename {} -> {}: {}", tmp, filename, e);
                        }
                    }
                    Err(e) => {
                        log::error!("Failed to create tmp file {}: {}", tmp, e);
                    }
                }
            }
        } else {
            log::warn!("No XDXR response for {} (fetch failed)", code);
        }
    }
}

pub fn init() {
    let plugin = Arc::new(DataXdxr) as Arc<dyn DataAdapter>;
    cache::register(plugin);
}

/// Load XDXR CSV cache for a given security code. Returns an empty Vec on error or missing file.
pub fn load_xdxr(code: &str) -> Vec<crate::level1::XdxrInfo> {
    let filename = crate::config::get_xdxr_filename(code);
    let mut list: Vec<crate::level1::XdxrInfo> = Vec::new();
    if let Ok(f) = std::fs::File::open(&filename) {
        let mut rdr = csv::ReaderBuilder::new().has_headers(true).from_reader(f);
        match rdr
            .deserialize::<crate::level1::XdxrInfo>()
            .collect::<Result<Vec<_>, csv::Error>>()
        {
            Ok(v) => list = v,
            Err(e) => log::error!("[DataXdxr] failed to deserialize {}: {}", filename, e),
        }
    }
    list
}
