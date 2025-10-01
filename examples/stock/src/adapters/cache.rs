use crate::adapter::AdapterError;
use super::super::adapter::{Adapter, ScheduleConfig};
use std::any::Any;
use std::sync::Arc;

#[derive(Default)]
pub struct CacheAdapter;

impl Adapter for CacheAdapter {
    fn name(&self) -> &'static str {
        "CacheAdapter"
    }

    fn init(&self) -> Result<(), String> {
        println!("[{}] Refreshing cache...", self.name());
        Ok(())
    }

    fn schedule_config(&self) -> Option<ScheduleConfig> {
        Some(ScheduleConfig {
            name: "cache".into(),
            cron: "*/5 * * * * *".into(),
            timezone: None,
        })
    }

    fn as_any(&self) -> &dyn Any {
        self
    }

    fn as_any_arc(self: Arc<Self>) -> Arc<dyn Any + Send + Sync> {
        self as Arc<dyn Any + Send + Sync>
    }
}