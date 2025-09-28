use once_cell::sync::Lazy;
use std::collections::BTreeMap;
use std::collections::HashSet;
use std::fmt::Debug;
use std::path::PathBuf;
use std::sync::{Arc, Mutex};

use crate::Timestamp;

pub type Kind = u64;

pub const PLUGIN_MASK_BASE_DATA: Kind = 0x1000_0000_0000_0000;
pub const PLUGIN_MASK_FEATURE: Kind = 0x2000_0000_0000_0000;
pub const PLUGIN_MASK_STRATEGY: Kind = 0x3000_0000_0000_0000;

pub const DEFAULT_DATA_PROVIDER: &str = "engine";

/// Schema describing a plugin
pub trait Schema: Send + Sync + Debug {
    fn kind(&self) -> Kind;
    fn owner(&self) -> String;
    fn key(&self) -> String;
    fn name(&self) -> String;
    fn usage(&self) -> String;
}

/// DataAdapter trait mirrors C++ DataAdapter (Schema + Update/Print)
pub trait DataAdapter: Schema + Send + Sync {
    fn print(&self, code: &str, dates: &[Timestamp]);
    fn update(&self, code: &str, date: Timestamp);
    /// Optional hook: return a boxed FeatureAdapter clone if this adapter is a FeatureAdapter.
    /// Default implementation returns None; feature adapters should override and return Some(clone).
    fn as_feature_clone(&self) -> Option<Box<dyn FeatureAdapter>> { None }
}

/// FeatureAdapter provides filename and aggregation helpers
pub trait FeatureAdapter: DataAdapter {
    fn filename_for(&self, timestamp: Timestamp) -> String {
        // Default implementation mirrors C++ FeatureAdapter::Filename
        let key = self.key();
        let pos = key.find('/');
        let (cache_path, actual_key) = if let Some(p) = pos {
            (key[..p].to_string(), key[p + 1..].to_string())
        } else {
            ("flash".to_string(), key.clone())
        };

        let mut full = PathBuf::from(crate::config::default_cache_path());
        full.push(cache_path);
        let date = timestamp.only_date();
        let year = &date[..4.min(date.len())];
        full.push(year);
        full.push(format!("{}.{}", actual_key, date));
        full.to_string_lossy().to_string()
    }

    fn init(&self, _timestamp: Timestamp) {}
    fn clone_box(&self) -> Box<dyn FeatureAdapter>;
    fn headers(&self) -> Vec<String>;
    fn values(&self) -> Vec<String>;
}

// Global registry
static PLUGIN_MAP: Lazy<Mutex<BTreeMap<Kind, Arc<dyn DataAdapter>>>> = Lazy::new(|| Mutex::new(BTreeMap::new()));

/// Register a plugin; panics if already registered (mirrors ErrAlreadyExists)
pub fn register(plugin: Arc<dyn DataAdapter>) {
    let kind = plugin.kind();
    let mut map = PLUGIN_MAP.lock().unwrap();
    if map.contains_key(&kind) {
        panic!("plugin already exists for kind {}", kind);
    }
    map.insert(kind, plugin);
}

/// Get a plugin by kind (exact match)
pub fn get_data_adapter(kind: Kind) -> Option<Arc<dyn DataAdapter>> {
    let map = PLUGIN_MAP.lock().unwrap();
    map.get(&kind).cloned()
}

/// Return all plugins matching mask (if mask==0 return all)
pub fn plugins(mask: Kind) -> Vec<Arc<dyn DataAdapter>> {
    let map = PLUGIN_MAP.lock().unwrap();
    let mut result: Vec<Arc<dyn DataAdapter>> = Vec::new();
    for (k, v) in map.iter() {
        if mask == 0 || ((*k & mask) == mask) {
            result.push(v.clone());
        }
    }
    result
}

/// Return plugins whose Key() is in keywords and which match plugin_type mask
pub fn plugins_with_name(plugin_type: Kind, keywords: &[String]) -> Vec<Arc<dyn DataAdapter>> {
    if keywords.is_empty() { return Vec::new(); }
    let mut keyword_set: HashSet<String> = HashSet::new();
    for k in keywords { keyword_set.insert(k.clone()); }

    let map = PLUGIN_MAP.lock().unwrap();
    let mut candidates: Vec<(Kind, Arc<dyn DataAdapter>)> = Vec::new();
    for (k, plugin) in map.iter() {
        if ((*k & plugin_type) == plugin_type) && keyword_set.contains(&plugin.key()) {
            candidates.push((*k, plugin.clone()));
        }
    }
    if candidates.is_empty() { return Vec::new(); }
    candidates.sort_by_key(|(k, _)| *k);
    candidates.into_iter().map(|(_, p)| p).collect()
}

/// Update a list of adapters in order. Each adapter decides how to update its data.
/// Feature adapters (those returning Some from `as_feature_clone`) will be initialized and
/// executed in parallel across codes; their results will be aggregated and written to a
/// per-adapter cache filename using the adapter-provided headers/values.
pub fn update_with_adapters(adapters: &[Arc<dyn DataAdapter>], feature_date: Timestamp) -> usize {
    use indicatif::{ProgressBar, ProgressStyle};

    let all_codes = crate::exchange::get_code_list();
    if all_codes.is_empty() {
        log::warn!("No codes found for update");
    }

    // concurrency default
    let default_concurrency = std::thread::available_parallelism().map(|n| n.get()).unwrap_or(4);

    let mut processed_adapters = 0usize;

    for adapter in adapters.iter() {
        processed_adapters += 1;
        let kind = adapter.kind();
        let module_name = format!("{}({})", adapter.key(), kind);
        log::info!("[update] plugin={}, start", module_name);

        // show a progress bar for codes
        let pb = ProgressBar::new(all_codes.len() as u64);
        pb.set_style(ProgressStyle::with_template("{spinner:.green} [{elapsed_precise}] {pos}/{len} {msg}").unwrap());

        // detect feature adapter
        if let Some(feature_prototype) = adapter.as_feature_clone() {
            // initialize feature with feature_date
            feature_prototype.init(feature_date);
            let cache_filename = feature_prototype.filename_for(feature_date);

            // prepare results container shared across threads
            use std::sync::{Arc as StdArc, Mutex as StdMutex};
            let results: StdArc<StdMutex<Vec<(String, Vec<String>)>>> = StdArc::new(StdMutex::new(Vec::new()));

            // partition codes
            let num_threads = std::cmp::min(default_concurrency, 8);
            let codes = all_codes.clone();
            let chunk_size = (codes.len() + num_threads - 1) / num_threads;
            let mut handles = Vec::new();

            for t in 0..num_threads {
                let start = t * chunk_size;
                let end = std::cmp::min(start + chunk_size, codes.len());
                if start >= end { continue; }
                let codes_slice = codes[start..end].to_vec();
                let adapter_clone = adapter.clone();
                let results_clone = results.clone();
                let pb_clone = pb.clone();

                let handle = std::thread::spawn(move || {
                    for code in codes_slice.into_iter() {
                        // clone a per-code feature instance
                        if let Some(feature_instance) = adapter_clone.as_feature_clone() {
                            // do the update
                            feature_instance.update(&code, feature_date);
                            let vals = feature_instance.values();
                            if !vals.is_empty() {
                                let mut guard = results_clone.lock().unwrap();
                                guard.push((code.clone(), vals));
                            }
                        } else {
                            // shouldn't happen - adapter was recognized as feature above
                        }
                        pb_clone.inc(1);
                    }
                });
                handles.push(handle);
            }

            // wait for threads
            for h in handles { let _ = h.join(); }

            // aggregate and write CSV
            let mut all_data = Vec::new();
            // header
            all_data.push(feature_prototype.headers());
            let guard = results.lock().unwrap();
            // sort results by code order
            let mut ordered = guard.clone();
            let mut code_order = std::collections::HashMap::new();
            for (i, c) in all_codes.iter().enumerate() { code_order.insert(c.clone(), i); }
            ordered.sort_by_key(|(code, _)| *code_order.get(code).unwrap_or(&usize::MAX));
            for (_code, vals) in ordered.into_iter() {
                all_data.push(vals);
            }

            // write CSV
            if all_data.len() > 1 {
                if let Err(e) = std::fs::create_dir_all(std::path::Path::new(&cache_filename).parent().unwrap_or(std::path::Path::new("."))) {
                    log::error!("Failed to create cache dir for {}: {}", cache_filename, e);
                } else {
                    match std::fs::File::create(&cache_filename) {
                        Ok(f) => {
                            let mut w = csv::Writer::from_writer(f);
                            for row in all_data.into_iter() {
                                if let Err(e) = w.write_record(row) { log::error!("Failed to write csv: {}", e); break; }
                            }
                            let _ = w.flush();
                        }
                        Err(e) => log::error!("Failed to create file {}: {}", cache_filename, e),
                    }
                }
            } else {
                log::warn!("No feature data for adapter {}", module_name);
            }

            pb.finish_with_message(format!("{} done", module_name));
        } else {
            // base adapter: process codes potentially concurrently
            let codes = all_codes.clone();
            let num_threads = std::cmp::min(default_concurrency, 8);
            let chunk_size = (codes.len() + num_threads - 1) / num_threads;
            let mut handles = Vec::new();
            for t in 0..num_threads {
                let start = t * chunk_size;
                let end = std::cmp::min(start + chunk_size, codes.len());
                if start >= end { continue; }
                let slice = codes[start..end].to_vec();
                let adapter_clone = adapter.clone();
                let pb_clone = pb.clone();
                let handle = std::thread::spawn(move || {
                    for code in slice.into_iter() {
                        adapter_clone.update(&code, feature_date);
                        pb_clone.inc(1);
                    }
                });
                handles.push(handle);
            }
            for h in handles { let _ = h.join(); }
            pb.finish_with_message(format!("{} done", module_name));
        }

        log::info!("[update] plugin={}, end", module_name);
    }

    processed_adapters
}

/// Convenience: pick adapters from registry according to mask and optional keywords and run update.
pub fn update_all_mask(mask: Kind, keywords: Option<&[String]>, feature_date: Timestamp) -> usize {
    let adapters = if let Some(ks) = keywords {
        plugins_with_name(mask, ks)
    } else {
        plugins(mask)
    };
    update_with_adapters(&adapters, feature_date)
}

// a small helper to make a boxed clone of a FeatureAdapter (if available)
pub fn clone_feature_adapter(a: &dyn FeatureAdapter) -> Box<dyn FeatureAdapter> {
    a.clone_box()
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::timestamp::Timestamp as Ts;

    #[derive(Debug)]
    struct DummyAdapter;

    impl Schema for DummyAdapter {
        fn kind(&self) -> Kind { 0x999 }
        fn owner(&self) -> String { DEFAULT_DATA_PROVIDER.to_string() }
        fn key(&self) -> String { "dummy".to_string() }
        fn name(&self) -> String { "dummy".to_string() }
        fn usage(&self) -> String { "".to_string() }
    }
    impl DataAdapter for DummyAdapter {
        fn print(&self, _code: &str, _dates: &[Timestamp]) {}
        fn update(&self, _code: &str, _date: Timestamp) {}
    }

    #[test]
    fn test_register_and_plugins() {
    let d = std::sync::Arc::new(DummyAdapter) as Arc<dyn DataAdapter>;
    register(d);
        let all = plugins(0);
        assert!(!all.is_empty());
    }
}
