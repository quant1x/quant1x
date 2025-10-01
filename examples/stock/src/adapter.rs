use chrono_tz::Tz;
use thiserror::Error;
use std::any::{Any, TypeId};
use std::collections::HashMap;
use std::sync::{Arc, RwLock};

#[derive(Debug, Clone)]
pub struct ScheduleConfig {
    pub name: String,
    pub cron: String,
    pub timezone: Option<String>,
}

#[derive(Error, Debug)]
pub enum AdapterError {
    #[error("Invalid timezone: {0}")]
    InvalidTimezone(String),
}

pub trait Adapter: Any + Send + Sync {
    fn name(&self) -> &'static str;
    fn init(&self) -> Result<(), String>;
    fn schedule_config(&self) -> Option<ScheduleConfig>;

    // 新增转换方法
    fn as_any(&self) -> &dyn Any;
    fn as_any_arc(self: Arc<Self>) -> Arc<dyn Any + Send + Sync>;
}

lazy_static::lazy_static! {
    pub static ref ADAPTER_REGISTRY: Arc<RwLock<HashMap<TypeId, Arc<dyn Adapter>>>> = Arc::new(RwLock::new(HashMap::new()));
}

// 注册函数
pub fn register_adapter<T: Adapter + Default + 'static>() {
    let adapter = Arc::new(T::default());
    let type_id = TypeId::of::<T>();
    ADAPTER_REGISTRY
        .write()
        .unwrap()
        .insert(type_id, adapter);
}

// 获取函数
pub fn get_adapter<T: Adapter + 'static>() -> Option<Arc<T>> {
    ADAPTER_REGISTRY
        .read()
        .unwrap()
        .get(&TypeId::of::<T>())
        .and_then(|arc| arc.clone().as_any_arc().downcast().ok())
}

pub fn parse_timezone(tz: Option<&str>) -> Result<Tz, AdapterError> {
    tz.map(|tz_str| {
        tz_str.parse::<Tz>()
            .map_err(|_| AdapterError::InvalidTimezone(tz_str.to_string()))
    })
        .unwrap_or(Ok(Tz::UTC))
}