use chrono::{Local, Timelike};
use std::io::{self, Write};
use std::path::PathBuf;
use std::sync::{
    atomic::{AtomicBool, Ordering},
    Arc, Mutex,
};
use std::thread;
use std::time::Duration;

/// RollingOnce 提供与 C++ 等价的语义：
/// - 一个由互斥锁保护的共享内存 done 标志
/// - 一个 Do / do_once_try API，调用者传入闭包，在每个窗口内仅执行一次
/// - 一个 Reset 方法，可由调度器调用以允许下一个窗口的运行
///
/// 我们提供一个便利的工厂函数 `with_daily_reset`，它会启动一个后台线程，
/// 在本地时间指定的 (hour, minute) 每天调用 `reset()`。这对应于 C++ 的
/// `RollingOnce::create(name, hour, minute)`，该函数会在运行时注册一个重置任务。
pub struct RollingOnce {
    marker: PathBuf,
    done: AtomicBool,
    // 互斥锁用于保护闭包执行和内部重置逻辑
    m: Mutex<()>,
    // 后台重置线程的可选取消标志
    cancelled: Arc<AtomicBool>,
    // 之前我们保留了一个作业 id，在 drop 时移除；为避免跨 crate 的 uuid 类型问题
    // 我们不再在 drop 时移除作业，而是依赖进程生命周期。
}

impl RollingOnce {
    /// 创建一个新的 RollingOnce，会在 `marker` 处持久化标记，但不会创建任何
    /// 后台重置线程。若需每天重置，请使用 `with_daily_reset`。
    pub fn new(marker: PathBuf) -> Arc<Self> {
        Arc::new(Self {
            marker,
            done: AtomicBool::new(false),
            m: Mutex::new(()),
            cancelled: Arc::new(AtomicBool::new(false)),
            // 未存储 cron 作业 id
        })
    }

    /// 创建并启动一个后台线程，在本地时间的 hour:minute 每天调用 Reset。
    /// 返回所创建 RollingOnce 的 Arc。后台线程在 RollingOnce 被 drop 时停止（取消标志被设置）。
    /// 优先建议使用 `tokio_cron_scheduler` 注册每日 cron 任务来在本地时间的 `hour:minute` 每天调用
    /// `reset()`；如果 tokio 的 cron 调度器不可用（没有运行时），则回退到启动以前的后台线程实现。
    pub fn with_daily_reset(marker: PathBuf, hour: u32, minute: u32) -> Arc<Self> {
        let instance = Self::new(marker);

        // 启动后台线程，在指定的本地时间每天调用 reset()。
        let cancel = instance.cancelled.clone();
        let weak_inst = Arc::downgrade(&instance);
        thread::Builder::new()
            .name(format!("rolling-once-reset-{}:{}", hour, minute))
            .spawn(move || {
                while !cancel.load(Ordering::Relaxed) {
                    if let Some(inst) = weak_inst.upgrade() {
                        let now = Local::now();
                        // 计算下一次发生时间
                        let mut next = now
                            .with_hour(hour)
                            .and_then(|d| d.with_minute(minute))
                            .and_then(|d| d.with_second(0))
                            .unwrap_or(now);
                        if next <= now {
                            next = next + chrono::Duration::days(1);
                        }
                        let dur = next.signed_duration_since(now);
                        let secs = dur.num_seconds();
                        if secs > 0 {
                            let mut remaining = secs;
                            while remaining > 0 && !cancel.load(Ordering::Relaxed) {
                                // 分段睡眠以便可响应取消请求
                                let sleep_for = std::cmp::min(remaining, 60);
                                thread::sleep(Duration::from_secs(sleep_for as u64));
                                remaining -= sleep_for;
                            }
                        }
                        if cancel.load(Ordering::Relaxed) {
                            break;
                        }
                        inst.reset();
                    } else {
                        break;
                    }
                }
            })
            .ok();

        instance
    }

    /// 重置 once 状态，以便下一次 Do() 可以再次执行闭包。
    pub fn reset(&self) {
        // 加锁以等待任何正在进行的 Do 执行完成
        let _g = self.m.lock().unwrap();
        self.done.store(false, Ordering::Release);
        // 如果存在持久化的标记文件，也一并移除，这样基于持久化的检查（如果有）能看到重置
        let _ = std::fs::remove_file(&self.marker);
    }

    /// 在每个窗口内运行提供的闭包一次。这与 C++ 的 RollingOnce::Do 相同，闭包只有在 `done` 为 false 时才会被执行。
    /// 闭包可以返回 Result<T, E>，do_once_try 会向上传播该结果。如果闭包运行完成，
    /// 在闭包结束后会将 `done` 设为 true（即便闭包 panic，我们仍将 done 设为 true，以匹配 C++ 的行为）。
    pub fn do_once_try<F, T, E>(&self, f: F) -> Result<Option<T>, E>
    where
        F: FnOnce() -> Result<T, E>,
    {
        // 快速路径检查
        if self.done.load(Ordering::Acquire) {
            return Ok(None);
        }

        // 获取锁以确保只有一个运行者执行闭包
        let _guard = self.m.lock().unwrap();

        if self.done.load(Ordering::Relaxed) {
            return Ok(None);
        }

        // 执行闭包
        match f() {
            Ok(v) => {
                // 原子性地持久化标记
                if let Some(parent) = self.marker.parent() {
                    let _ = std::fs::create_dir_all(parent);
                }
                // 写入临时文件并重命名以保证原子性
                if let Ok(tmp) = tempfile::NamedTempFile::new_in(
                    self.marker
                        .parent()
                        .unwrap_or_else(|| std::path::Path::new(".")),
                ) {
                    let today = Local::now().format("%Y-%m-%d").to_string();
                    let _ = write!(tmp.as_file(), "{}", today);
                    let _ = tmp.persist(&self.marker);
                } else {
                    // fallback
                    // 回退方案：直接写文件
                    if let Ok(mut f) = std::fs::File::create(&self.marker) {
                        let today = Local::now().format("%Y-%m-%d").to_string();
                        let _ = write!(f, "{}", today);
                    }
                }

                self.done.store(true, Ordering::Release);
                Ok(Some(v))
            }
            Err(e) => {
                // 与 C++ 行为一致：即使闭包返回错误也将标记为 done，以避免重试风暴
                self.done.store(true, Ordering::Release);
                Err(e)
            }
        }
    }

    /// 便利方法：通过将今天的日期写入 marker 文件来标记已运行（与 do_once 的持久化语义相同）
    pub fn mark_run(&self) -> io::Result<()> {
        if let Some(parent) = self.marker.parent() {
            std::fs::create_dir_all(parent)?;
        }
        let mut f = std::fs::File::create(&self.marker)?;
        let today = Local::now().format("%Y-%m-%d").to_string();
        write!(f, "{}", today)?;
        Ok(())
    }
}

impl Drop for RollingOnce {
    fn drop(&mut self) {
        // 向回退线程发送取消信号
        self.cancelled.store(true, Ordering::Relaxed);
        // 我们有意不从全局调度器中移除 cron 作业以避免跨 crate 的 uuid 类型不匹配；
        // 调度器会在进程退出时被销毁。
    }
}
// chrono::Timelike 在需要的函数中已导入

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::atomic::{AtomicUsize, Ordering as AtomicOrdering};
    use std::thread;
    use tempfile::tempdir;

    #[test]
    fn test_do_once_persists_and_reset() {
        let td = tempdir().unwrap();
        let marker = td.path().join("calendar.updated");
        let ro = RollingOnce::new(marker.clone());

        // 第一次运行应被执行
        let r = ro.do_once_try(|| -> Result<i32, ()> { Ok(7) }).unwrap();
        assert_eq!(r, Some(7));

        // 应已写入标记文件
        assert!(marker.exists());

        // 第二次运行应被跳过
        let r2 = ro.do_once_try(|| -> Result<i32, ()> { Ok(8) }).unwrap();
        assert_eq!(r2, None);

        // 重置并再次运行
        ro.reset();
        let r3 = ro.do_once_try(|| -> Result<i32, ()> { Ok(9) }).unwrap();
        assert_eq!(r3, Some(9));
    }

    #[test]
    fn test_do_once_concurrent_runs_once() {
        let td = tempdir().unwrap();
        let marker = td.path().join("calendar2.updated");
        let ro = RollingOnce::new(marker.clone());

        let counter = Arc::new(AtomicUsize::new(0));
        let mut handles = Vec::new();
        for _ in 0..16 {
            let roclone = ro.clone();
            let c = counter.clone();
            handles.push(thread::spawn(move || {
                let _ = roclone.do_once_try(|| -> Result<(), ()> {
                    c.fetch_add(1, AtomicOrdering::SeqCst);
                    Ok(())
                });
            }));
        }
        for h in handles {
            let _ = h.join();
        }

        // 只有一个线程应增加计数器
        assert_eq!(counter.load(AtomicOrdering::SeqCst), 1);
    }
}
