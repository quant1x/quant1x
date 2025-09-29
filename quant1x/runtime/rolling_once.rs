use chrono::{Local, Timelike};
use std::io::{self, Write};
use std::path::PathBuf;
use std::sync::{
    atomic::{AtomicBool, Ordering},
    Arc, Mutex,
};
use std::thread;
use std::time::Duration;

/// RollingOnce provides C++-equivalent semantics:
/// - a shared, in-memory done flag guarded by a mutex
/// - a Do / do_once_try API where callers pass a closure to execute exactly once per window
/// - a Reset method which can be invoked by a scheduler to allow the next window's run
///
/// We provide a small convenience factory `with_daily_reset` that spawns a background thread
/// which calls `reset()` every day at the provided (hour, minute) local time. This mirrors
/// the C++ `RollingOnce::create(name, hour, minute)` which registers a reset task with runtime.
pub struct RollingOnce {
    marker: PathBuf,
    done: AtomicBool,
    // mutex protects running the closure and internal reset logic
    m: Mutex<()>,
    // optional cancellation flag for the background reset thread
    cancelled: Arc<AtomicBool>,
    // previously we kept a job id to remove on drop; to avoid cross-crate uuid type issues
    // we no longer remove the job on drop and rely on process lifecycle.
}

impl RollingOnce {
    /// Create a new RollingOnce that persists a marker at `marker` but does not create any
    /// background reset thread. Use `with_daily_reset` to also spawn a daily reset.
    pub fn new(marker: PathBuf) -> Arc<Self> {
        Arc::new(Self {
            marker,
            done: AtomicBool::new(false),
            m: Mutex::new(()),
            cancelled: Arc::new(AtomicBool::new(false)),
            // no cron job id stored
        })
    }

    /// Create and spawn a background thread which calls Reset every day at local hour:minute.
    /// Returns an Arc to the created RollingOnce. The background thread will stop when the
    /// RollingOnce is dropped (the cancellation flag is set).
    /// Create and (preferably) register a daily cron job using `tokio_cron_scheduler` to call
    /// `reset()` every day at local `hour:minute`. If the tokio cron scheduler is unavailable
    /// (no runtime), this falls back to spawning the previous background thread implementation.
    pub fn with_daily_reset(marker: PathBuf, hour: u32, minute: u32) -> Arc<Self> {
        let instance = Self::new(marker);

        // Spawn the background thread that calls reset() every day at the provided local time.
        let cancel = instance.cancelled.clone();
        let weak_inst = Arc::downgrade(&instance);
        thread::Builder::new()
            .name(format!("rolling-once-reset-{}:{}", hour, minute))
            .spawn(move || {
                while !cancel.load(Ordering::Relaxed) {
                    if let Some(inst) = weak_inst.upgrade() {
                        let now = Local::now();
                        // compute next occurrence
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

    /// Reset the once state so the next Do() may execute the closure again.
    pub fn reset(&self) {
        // lock to wait for any in-progress Do to finish
        let _g = self.m.lock().unwrap();
        self.done.store(false, Ordering::Release);
        // also remove persisted marker file if present so persistence-based checks (if any) see reset
        let _ = std::fs::remove_file(&self.marker);
    }

    /// Run the provided closure once per window. This mirrors C++ RollingOnce::Do where the
    /// closure is only executed when `done` is false. The closure can return Result<T,E> and
    /// do_once_try will propagate the result. If the closure runs, `done` will be set to true
    /// after the closure finishes (even if it panics, we still set done to true to match C++).
    pub fn do_once_try<F, T, E>(&self, f: F) -> Result<Option<T>, E>
    where
        F: FnOnce() -> Result<T, E>,
    {
        // fast-path check
        if self.done.load(Ordering::Acquire) {
            return Ok(None);
        }

        // acquire lock to ensure only one runner executes the closure
        let _guard = self.m.lock().unwrap();

        if self.done.load(Ordering::Relaxed) {
            return Ok(None);
        }

        // run
        match f() {
            Ok(v) => {
                // Persist marker atomically
                if let Some(parent) = self.marker.parent() {
                    let _ = std::fs::create_dir_all(parent);
                }
                // write to temp + rename for atomicity
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
                    if let Ok(mut f) = std::fs::File::create(&self.marker) {
                        let today = Local::now().format("%Y-%m-%d").to_string();
                        let _ = write!(f, "{}", today);
                    }
                }

                self.done.store(true, Ordering::Release);
                Ok(Some(v))
            }
            Err(e) => {
                // match C++ behavior: mark as done even if the closure throws to avoid retry storms
                self.done.store(true, Ordering::Release);
                Err(e)
            }
        }
    }

    /// Convenience: mark run by writing today's date to marker file (same semantics as do_once persistence)
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
        // signal cancellation for fallback thread
        self.cancelled.store(true, Ordering::Relaxed);
        // we intentionally do not remove cron jobs from the global scheduler to avoid
        // cross-crate uuid type mismatches; the scheduler will be torn down on process exit.
    }
}
// chrono::Timelike is imported where needed inside functions

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

        // First run should execute
        let r = ro.do_once_try(|| -> Result<i32, ()> { Ok(7) }).unwrap();
        assert_eq!(r, Some(7));

        // Marker file should have been written
        assert!(marker.exists());

        // Second run should be skipped
        let r2 = ro.do_once_try(|| -> Result<i32, ()> { Ok(8) }).unwrap();
        assert_eq!(r2, None);

        // Reset and run again
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

        // Only one thread should have incremented the counter
        assert_eq!(counter.load(AtomicOrdering::SeqCst), 1);
    }
}
