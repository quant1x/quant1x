use std::marker::PhantomData;
use std::path::Path;

use serde::de::DeserializeOwned;
use serde::Serialize;

use crate::data::meta::instrument::Instrument;
use crate::meta::Timestamp;

/// 文件存储接口（专用于单一数据类型 T）
///
/// 提供抽象的文件存储生命周期：初始化 → 更新 → 加载/保存。
/// 实现者只需实现文件名生成和是否需要初始化/更新的判断逻辑。
pub trait FileStorage<T: Serialize + DeserializeOwned + Clone>: Send + Sync {
    /// 返回文件名
    fn file_name(&self) -> String;

    /// 判断是否需要初始化
    fn should_initialize(&self, timestamp: Option<Timestamp>) -> bool;

    /// 判断是否需要更新
    fn should_update(&self, timestamp: Option<Timestamp>) -> bool;

    /// 更新数据（无参，类型已固定）
    fn update(&self);

    /// 加载数据
    fn load(&self) -> Vec<T> {
        let filename = self.file_name();
        csv_to_vec::<T>(&filename).unwrap_or_default()
    }

    /// 保存数据
    fn save(&self, data: &[T]) {
        let filename = self.file_name();
        let _ = vec_to_csv(&filename, data);
    }

    /// 检出数据（自动更新 + 加载）
    fn checkout(&self) -> Vec<T> {
        let ts = Timestamp::now();
        if self.should_initialize(Some(ts)) || self.should_update(Some(ts)) {
            self.update();
        }
        self.load()
    }
}

/// 基础数据文件存储类
///
/// 用于存储与具体证券标的（Instrument）关联的基础数据。
pub struct BasedataFileStorage<T: Serialize + DeserializeOwned + Clone> {
    inst: Instrument,
    _marker: PhantomData<T>,
}

impl<T: Serialize + DeserializeOwned + Clone + Send + Sync> BasedataFileStorage<T> {
    pub fn new(inst: Instrument) -> Self {
        Self {
            inst,
            _marker: PhantomData,
        }
    }

    /// 返回关联的证券标的
    pub fn instrument(&self) -> &Instrument {
        &self.inst
    }
}

/// 元数据文件存储类
///
/// 用于存储与数据类型绑定的元数据，文件名自动生成为 "{TypeName}.csv"。
pub struct MetaFileStorage<T: Serialize + DeserializeOwned + Clone> {
    _marker: PhantomData<T>,
}

impl<T: Serialize + DeserializeOwned + Clone + Send + Sync> MetaFileStorage<T> {
    pub fn new() -> Self {
        Self {
            _marker: PhantomData,
        }
    }

    /// 返回基于类型名称自动生成的文件名
    pub fn default_file_name() -> String {
        let type_name = std::any::type_name::<T>();
        // 去除模块路径前缀，只保留类型名称
        let simple_name = type_name.rsplit("::").next().unwrap_or(type_name);
        format!("{}.csv", simple_name)
    }
}

// ============================================================
// CSV 读写辅助函数
// ============================================================

/// 展开路径中的 `~` 为 home 目录
fn expand_path(filename: &str) -> String {
    if let Some(rest) = filename.strip_prefix("~/") {
        if let Some(home) = dirs::home_dir() {
            return format!("{}/{}", home.to_string_lossy(), rest);
        }
    }
    if filename == "~" {
        if let Some(home) = dirs::home_dir() {
            return home.to_string_lossy().to_string();
        }
    }
    filename.to_string()
}

/// 从 CSV 文件读取数据到 Vec<T>
fn csv_to_vec<T: DeserializeOwned>(filename: &str) -> Result<Vec<T>, csv::Error> {
    let path = expand_path(filename);
    if !Path::new(&path).exists() {
        return Ok(Vec::new());
    }
    let mut reader = csv::ReaderBuilder::new()
        .has_headers(true)
        .from_path(&path)?;
    let mut result = Vec::new();
    for record in reader.deserialize() {
        let item: T = record?;
        result.push(item);
    }
    Ok(result)
}

/// 将 Vec<T> 写入 CSV 文件
fn vec_to_csv<T: Serialize>(filename: &str, data: &[T]) -> Result<(), csv::Error> {
    let path = expand_path(filename);
    if let Some(parent) = Path::new(&path).parent() {
        std::fs::create_dir_all(parent).ok();
    }
    let mut writer = csv::WriterBuilder::new()
        .has_headers(true)
        .from_path(&path)?;
    for item in data {
        writer.serialize(item)?;
    }
    writer.flush()?;
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use serde::Deserialize;

    #[derive(Debug, Clone, Serialize, Deserialize, PartialEq)]
    struct TestRecord {
        name: String,
        value: f64,
    }

    #[test]
    fn test_csv_roundtrip() {
        let data = vec![
            TestRecord {
                name: "a".into(),
                value: 1.0,
            },
            TestRecord {
                name: "b".into(),
                value: 2.0,
            },
        ];
        let tmp = std::env::temp_dir().join("test_storage.csv");
        let filename = tmp.to_string_lossy().to_string();
        vec_to_csv(&filename, &data).unwrap();
        let loaded: Vec<TestRecord> = csv_to_vec(&filename).unwrap();
        assert_eq!(data, loaded);
        std::fs::remove_file(&filename).ok();
    }

    #[test]
    fn test_meta_file_storage_default_name() {
        let name = MetaFileStorage::<TestRecord>::default_file_name();
        assert!(name.ends_with(".csv"));
        assert!(name.contains("TestRecord"));
    }
}
