use once_cell::sync::Lazy;
use std::collections::BTreeMap;
use std::collections::HashSet;
use std::fmt::Debug;
use std::path::PathBuf;
use std::sync::{Arc, Condvar, Mutex};

use crate::Timestamp;

pub type Kind = u64;

pub const PLUGIN_MASK_BASE_DATA: Kind = 0x1000_0000_0000_0000;
pub const PLUGIN_MASK_FEATURE: Kind = 0x2000_0000_0000_0000;
pub const PLUGIN_MASK_STRATEGY: Kind = 0x3000_0000_0000_0000;

pub const DEFAULT_DATA_PROVIDER: &str = "engine";

/// 描述插件的 Schema
pub trait Schema: Send + Sync + Debug {
    fn kind(&self) -> Kind;
    fn owner(&self) -> String;
    fn key(&self) -> String;
    fn name(&self) -> String;
    fn usage(&self) -> String;
}

/// DataAdapter 特征：对应 C++ 中的 DataAdapter（包含 Schema + Update/Print）
pub trait DataAdapter: Schema + Send + Sync {
    fn print(&self, code: &str, dates: &[Timestamp]);
    fn update(&self, code: &str, date: Timestamp);
    /// 可选钩子：如果该适配器是 FeatureAdapter，则返回其 boxed 克隆。
    /// 默认实现返回 None；feature 适配器应重写并返回 Some(clone)。
    fn as_feature_clone(&self) -> Option<Box<dyn FeatureAdapter>> {
        None
    }
}

/// FeatureAdapter 提供文件名和聚合相关的辅助方法
pub trait FeatureAdapter: DataAdapter {
    fn filename_for(&self, timestamp: Timestamp) -> String {
        // 默认实现与 C++ 的 FeatureAdapter::Filename 等价
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

// 全局插件注册表
static PLUGIN_MAP: Lazy<Mutex<BTreeMap<Kind, Arc<dyn DataAdapter>>>> =
    Lazy::new(|| Mutex::new(BTreeMap::new()));

/// 注册插件；如果已存在则 panic（等同于 ErrAlreadyExists 行为）
pub fn register(plugin: Arc<dyn DataAdapter>) {
    let kind = plugin.kind();
    let mut map = PLUGIN_MAP.lock().unwrap();
    if map.contains_key(&kind) {
        panic!("plugin already exists for kind {}", kind);
    }
    map.insert(kind, plugin);
}

/// 按 kind 获取插件（精确匹配）
pub fn get_data_adapter(kind: Kind) -> Option<Arc<dyn DataAdapter>> {
    let map = PLUGIN_MAP.lock().unwrap();
    map.get(&kind).cloned()
}

/// 返回与 mask 匹配的所有插件（mask==0 时返回全部）
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

/// 返回 key 在 keywords 中且匹配 plugin_type mask 的插件
pub fn plugins_with_name(plugin_type: Kind, keywords: &[String]) -> Vec<Arc<dyn DataAdapter>> {
    if keywords.is_empty() {
        return Vec::new();
    }
    let mut keyword_set: HashSet<String> = HashSet::new();
    for k in keywords {
        keyword_set.insert(k.clone());
    }

    let map = PLUGIN_MAP.lock().unwrap();
    let mut candidates: Vec<(Kind, Arc<dyn DataAdapter>)> = Vec::new();
    for (k, plugin) in map.iter() {
        if ((*k & plugin_type) == plugin_type) && keyword_set.contains(&plugin.key()) {
            candidates.push((*k, plugin.clone()));
        }
    }
    if candidates.is_empty() {
        return Vec::new();
    }
    candidates.sort_by_key(|(k, _)| *k);
    candidates.into_iter().map(|(_, p)| p).collect()
}

/// 按顺序更新给定的适配器列表。每个适配器决定其数据如何更新。
/// Feature 适配器（即 `as_feature_clone` 返回 Some 的适配器）会基于 codes 并行执行，
/// 其结果将被汇总并写入适配器指定的缓存文件（使用适配器提供的 headers/values）。
pub fn update_with_adapters(adapters: &[Arc<dyn DataAdapter>], feature_date: Timestamp) -> usize {
    use indicatif::{ProgressBar, ProgressStyle};

    let all_codes = crate::instruments::get_code_list();
    if all_codes.is_empty() {
        log::warn!("No codes found for update");
    }

    // 并发度默认值由每个 adapter 的配置决定（quant1x.yaml 中的 data.concurrency）

    let mut processed_adapters = 0usize;

    // 确保 base 适配器先于 feature 适配器运行：将其划分为两组
    let mut base_adapters: Vec<Arc<dyn DataAdapter>> = Vec::new();
    let mut feature_adapters: Vec<Arc<dyn DataAdapter>> = Vec::new();
    for adapter in adapters.iter() {
        // 根据适配器是否暴露 FeatureAdapter 克隆进行分类
        if adapter.as_feature_clone().is_some() {
            feature_adapters.push(adapter.clone());
        } else {
            base_adapters.push(adapter.clone());
        }
    }

    let ordered_iter = base_adapters
        .into_iter()
        .chain(feature_adapters.into_iter());

    // Determine a global concurrency limit based on level1 pool maximum connections.
    let pool_max = {
        if let Some(limit) = crate::level1::pool_max_connections() {
            limit
        } else {
            let fallback = std::cmp::min(crate::level1::config::MAX_CONNECTIONS.max(1), 5);
            log::info!(
                "[cache] level1 pool not initialized; falling back to concurrency limit {}",
                fallback
            );
            fallback
        }
    }
    .max(1);

    // Simple counting semaphore to bound concurrent network operations across all worker threads.
    #[derive(Debug)]
    struct SimpleSemaphore {
        mutex: Mutex<usize>,
        cvar: Condvar,
        max: usize,
    }

    impl SimpleSemaphore {
        fn new(max: usize) -> Self {
            Self {
                mutex: Mutex::new(0),
                cvar: Condvar::new(),
                max,
            }
        }

        fn acquire(&self) {
            let mut g = self.mutex.lock().unwrap();
            while *g >= self.max {
                g = self.cvar.wait(g).unwrap();
            }
            *g += 1;
        }

        fn release(&self) {
            let mut g = self.mutex.lock().unwrap();
            *g = g.saturating_sub(1);
            self.cvar.notify_one();
        }

        fn guard(self: &Arc<Self>) -> SemGuard {
            self.acquire();
            SemGuard {
                sem: Arc::clone(self),
            }
        }
    }

    #[derive(Clone)]
    struct SemGuard {
        sem: Arc<SimpleSemaphore>,
    }

    impl Drop for SemGuard {
        fn drop(&mut self) {
            self.sem.release();
        }
    }

    let sem = Arc::new(SimpleSemaphore::new(pool_max));

    for adapter in ordered_iter {
        processed_adapters += 1;
        let kind = adapter.kind();
        let module_name = format!("{}({})", adapter.key(), kind);
        log::info!("[update] plugin={}, start", module_name);

        // 为 codes 显示进度条
        let pb = ProgressBar::new(all_codes.len() as u64);
        pb.set_style(
            ProgressStyle::with_template("{spinner:.green} [{elapsed_precise}] {pos}/{len} {msg}")
                .unwrap(),
        );

        log::info!(
            "[cache] adapter {} using pool_max={} threads limit={}",
            module_name,
            pool_max,
            pool_max
        );

        // 检测是否为 feature 适配器
        if let Some(feature_prototype) = adapter.as_feature_clone() {
            // 使用 feature_date 初始化 feature 实例
            feature_prototype.init(feature_date);
            let cache_filename = feature_prototype.filename_for(feature_date);

            // 准备跨线程共享的结果容器
            use std::sync::{Arc as StdArc, Mutex as StdMutex};
            let results: StdArc<StdMutex<Vec<(String, Vec<String>)>>> =
                StdArc::new(StdMutex::new(Vec::new()));

            // 划分 codes（分块以供线程处理）
            let mut num_threads = crate::config::get_concurrency_for(&adapter.key());
            // Ensure business thread count does not exceed level1 pool capacity
            if num_threads == 0 || num_threads > pool_max {
                num_threads = pool_max.max(1);
            }
            let codes = all_codes.clone();
            log::info!(
                "[cache] adapter {} feature threads={} (requested vs capped)",
                module_name,
                num_threads
            );

            let chunk_size = (codes.len() + num_threads - 1) / num_threads;
            let mut handles = Vec::new();

            for t in 0..num_threads {
                let start = t * chunk_size;
                let end = std::cmp::min(start + chunk_size, codes.len());
                if start >= end {
                    continue;
                }
                let codes_slice = codes[start..end].to_vec();
                let adapter_clone = adapter.clone();
                let results_clone = results.clone();
                let pb_clone = pb.clone();

                let sem_clone = sem.clone();
                let handle = std::thread::spawn(move || {
                    for code in codes_slice.into_iter() {
                        // 为每个代码克隆一个 feature 实例
                        if let Some(feature_instance) = adapter_clone.as_feature_clone() {
                            // Acquire global semaphore before network/update work
                            let _g = sem_clone.guard();
                            // 执行更新
                            feature_instance.update(&code, feature_date);
                            let vals = feature_instance.values();
                            if !vals.is_empty() {
                                let mut guard = results_clone.lock().unwrap();
                                guard.push((code.clone(), vals));
                            }
                            // `_g` dropped here, releasing semaphore
                        } else {
                            // 不应发生 - 上面已判断为 feature 适配器
                        }
                        pb_clone.inc(1);
                    }
                });
                handles.push(handle);
            }

            // wait for threads
            for h in handles {
                let _ = h.join();
            }

            // aggregate and write CSV
            let mut all_data = Vec::new();
            // header
            all_data.push(feature_prototype.headers());
            let guard = results.lock().unwrap();
            // sort results by code order
            let mut ordered = guard.clone();
            let mut code_order = std::collections::HashMap::new();
            for (i, c) in all_codes.iter().enumerate() {
                code_order.insert(c.clone(), i);
            }
            ordered.sort_by_key(|(code, _)| *code_order.get(code).unwrap_or(&usize::MAX));
            for (_code, vals) in ordered.into_iter() {
                all_data.push(vals);
            }

            // write CSV
            if all_data.len() > 1 {
                if let Err(e) = std::fs::create_dir_all(
                    std::path::Path::new(&cache_filename)
                        .parent()
                        .unwrap_or(std::path::Path::new(".")),
                ) {
                    log::error!("Failed to create cache dir for {}: {}", cache_filename, e);
                } else {
                    match std::fs::File::create(&cache_filename) {
                        Ok(f) => {
                            let mut w = csv::Writer::from_writer(f);
                            for row in all_data.into_iter() {
                                if let Err(e) = w.write_record(row) {
                                    log::error!("Failed to write csv: {}", e);
                                    break;
                                }
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
            let mut num_threads = crate::config::get_concurrency_for(&adapter.key());
            // Ensure business thread count does not exceed level1 pool capacity
            if num_threads == 0 || num_threads > pool_max {
                num_threads = pool_max.max(1);
            }
            log::info!(
                "[cache] adapter {} base threads={} (requested vs capped)",
                module_name,
                num_threads
            );

            let chunk_size = (codes.len() + num_threads - 1) / num_threads;
            let mut handles = Vec::new();
            for t in 0..num_threads {
                let start = t * chunk_size;
                let end = std::cmp::min(start + chunk_size, codes.len());
                if start >= end {
                    continue;
                }
                let slice = codes[start..end].to_vec();
                let adapter_clone = adapter.clone();
                let pb_clone = pb.clone();
                let sem_clone = sem.clone();
                let handle = std::thread::spawn(move || {
                    for code in slice.into_iter() {
                        let _g = sem_clone.guard();
                        adapter_clone.update(&code, feature_date);
                        // `_g` released here
                        pb_clone.inc(1);
                    }
                });
                handles.push(handle);
            }
            for h in handles {
                let _ = h.join();
            }
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
        fn kind(&self) -> Kind {
            0x999
        }
        fn owner(&self) -> String {
            DEFAULT_DATA_PROVIDER.to_string()
        }
        fn key(&self) -> String {
            "dummy".to_string()
        }
        fn name(&self) -> String {
            "dummy".to_string()
        }
        fn usage(&self) -> String {
            "".to_string()
        }
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