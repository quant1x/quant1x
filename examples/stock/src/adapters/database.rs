use std::sync::Arc;
use std::any::Any;
use crate::adapter::ScheduleConfig;
use crate::adapter::Adapter;

#[derive(Default)]
pub struct DatabaseAdapter;

impl Adapter for DatabaseAdapter {
    fn name(&self) -> &'static str {
        "Database"
    }

    fn init(&self) -> Result<(), String> {
        println!("[{}] Initializing database...", self.name());
        Ok(())
    }

    fn schedule_config(&self) -> Option<ScheduleConfig> {
        Some(ScheduleConfig {
            name: "database".into(),
            cron: "0 0 9 * * *".into(),
            timezone: Some("Asia/Shanghai".into()),
        })
    }

    fn as_any(&self) -> &dyn Any {
        self
    }

    fn as_any_arc(self: Arc<Self>) -> Arc<dyn Any + Send + Sync> {
        self as Arc<dyn Any + Send + Sync>
    }
}