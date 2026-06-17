use super::adapter::{self, Adapter, ScheduleConfig};
use std::any::TypeId;
use tokio_cron_scheduler::{Job, JobScheduler};
use chrono_tz::Tz;
use anyhow::Context;
use std::sync::Arc;

pub async fn start_scheduler() -> anyhow::Result<()> {
    let sched = JobScheduler::new().await?;

    // 获取适配器类型ID列表(避免长期持有锁)
    let adapter_ids = {
        let registry = adapter::ADAPTER_REGISTRY.read().unwrap();
        registry.keys().cloned().collect::<Vec<TypeId>>()
    };

    for type_id in adapter_ids {
        // 每次单独获取适配器实例
        let adapter = match adapter::ADAPTER_REGISTRY.read().unwrap().get(&type_id) {
            Some(a) => Arc::clone(a),
            None => continue,
        };

        let config = match adapter.schedule_config() {
            Some(c) => c,
            None => continue,
        };

        let tz = adapter::parse_timezone(config.timezone.as_deref())
            .context("Invalid timezone configuration")?;

        // let schedule = Schedule::from_str(&config.cron)
        //     .context("Invalid cron expression")?
        //     .with_timezone(&tz);
        if let Err(e) = adapter.init() {
            eprintln!("[{}] Init failed: {}", adapter.name(), e);
        }
        let job = Job::new_cron_job_async_tz(config.cron,tz, move |_uuid, _lock| {
            let adapter = Arc::clone(&adapter);
            Box::pin(async move {
                if let Err(e) = adapter.init() {
                    eprintln!("[{}] Init failed: {}", adapter.name(), e);
                }
            })
        })?;

        sched.add(job).await?;
    }

    sched.start().await?;
    Ok(())
}

fn get_adapter_by_type_id(type_id: TypeId) -> Option<Arc<dyn Adapter>> {
    adapter::ADAPTER_REGISTRY
        .read()
        .unwrap()
        .get(&type_id)
        .map(|arc| Arc::clone(arc))
}