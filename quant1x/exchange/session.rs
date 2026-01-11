use crate::timestamp::Timestamp;
use crate::timestamp::{PRE_MARKET_HOUR, PRE_MARKET_MINUTE};
use crate::config;
use chrono::Datelike;
use once_cell::sync::Lazy;
use std::cmp::min;
use std::sync::Mutex;
use std::sync::MutexGuard;
use std::path::PathBuf;
use std::sync::Arc;
use crate::runtime::RollingOnce;

pub const MASK_CLOSED: u8 = 0x00;
pub const MASK_ACTIVE: u8 = 0x01;
pub const MASK_TRADING: u8 = 0x02;
pub const MASK_CALL_AUCTION: u8 = 0x04;
pub const MASK_ORDER: u8 = 0x08;
pub const MASK_CANCELABLE: u8 = 0x10;
pub const MASK_OPENING: u8 = 0x20;
pub const MASK_CLOSING: u8 = 0x40;
pub const MASK_HALT: u8 = 0x80;

pub type TimeStatus = u8;

pub fn is_trading_disabled(status: TimeStatus) -> bool {
    status == MASK_CLOSED || status == MASK_HALT || (status & MASK_HALT) != 0
}

#[derive(Debug, Clone)]
pub struct TimeRange {
    begin: Timestamp,
    end: Timestamp,
    status: TimeStatus,
}

impl TimeRange {
    pub fn new(begin: Timestamp, end: Timestamp, status: TimeStatus) -> Self {
        // C++ floors begin and ceils end
        let b = begin.floor();
        let e = end.ceil();
        Self {
            begin: b,
            end: e,
            status,
        }
    }

    pub fn in_range(&self, ts: Timestamp) -> Option<TimeStatus> {
        if self.begin <= ts && ts < self.end {
            Some(self.status)
        } else {
            None
        }
    }

    pub fn begin(&self) -> Timestamp {
        self.begin
    }
    pub fn end(&self) -> Timestamp {
        self.end
    }

    pub fn minutes(&self, timestamp: Option<Timestamp>) -> i32 {
        let mut seconds: i64 = 0;
        let in_status = self.status == (MASK_ACTIVE | MASK_ORDER | MASK_TRADING)
            || self.status == (MASK_CALL_AUCTION | MASK_CLOSING);
        let mut ts_close_phase = self.end;
        if in_status {
            // if close phase, use floor
            if self.status == (MASK_CALL_AUCTION | MASK_CLOSING) {
                ts_close_phase = ts_close_phase.floor();
            }
            if timestamp.is_none() || timestamp.unwrap().value() == 0 {
                seconds = (ts_close_phase.value() - self.begin.value()) / 1000;
            } else if let Some(t) = timestamp {
                if self.in_range(t).is_some() {
                    let min_ts = min(Timestamp::new(ts_close_phase.value()), t);
                    seconds = (min_ts.value() - self.begin.value()) / 1000;
                }
            }
        }
        let minutes = ((seconds as i32) + 59) / 60;
        minutes
    }
}

#[derive(Debug, Clone)]
pub struct TradingSession {
    sessions: Vec<TimeRange>,
    earliest_start: Timestamp,
    latest_end: Timestamp,
}

impl TradingSession {
    pub fn new(sessions: Vec<TimeRange>) -> Self {
        let mut ts = Self {
            sessions,
            earliest_start: Timestamp::new(i64::MAX),
            latest_end: Timestamp::new(i64::MIN),
        };
        ts.update_time_bounds();
        ts
    }

    fn update_time_bounds(&mut self) {
        if self.sessions.is_empty() {
            self.earliest_start = Timestamp::new(i64::MAX);
            self.latest_end = Timestamp::new(i64::MIN);
            return;
        }
        self.earliest_start = Timestamp::new(i64::MAX);
        self.latest_end = Timestamp::new(i64::MIN);
        for s in &self.sessions {
            if s.begin < self.earliest_start {
                self.earliest_start = s.begin
            }
            if s.end > self.latest_end {
                self.latest_end = s.end
            }
        }
    }

    pub fn in_session(&self, ts: Timestamp) -> TimeStatus {
        for s in &self.sessions {
            if let Some(status) = s.in_range(ts) {
                return status;
            }
        }
        if ts < self.earliest_start {
            return MASK_ACTIVE; /* ExchangePreMarket */
        }
        if ts < self.latest_end {
            return MASK_ACTIVE | MASK_HALT; /* ExchangeHaltTrading */
        }
        MASK_CLOSED
    }

    pub fn is_trading_not_started(&self, ts: Timestamp) -> bool {
        ts < self.earliest_start
    }

    pub fn minutes(&self, timestamp: Option<Timestamp>) -> i32 {
        let mut minutes = 0;
        for s in &self.sessions {
            minutes += s.minutes(timestamp);
        }
        minutes
    }
}

fn init_session() -> TradingSession {
    let now = Timestamp::midnight();
    let tr1 = TimeRange::new(
        now.offset(9, 15, 0, 0),
        now.offset(9, 20, 0, 0),
        MASK_CALL_AUCTION | MASK_OPENING | MASK_ORDER,
    );
    let tr2 = TimeRange::new(
        now.offset(9, 20, 0, 0),
        now.offset(9, 25, 0, 0),
        MASK_CALL_AUCTION | MASK_OPENING,
    );
    let tr3 = TimeRange::new(now.offset(9, 25, 0, 0), now.offset(9, 29, 0, 0), MASK_HALT);
    let tr4 = TimeRange::new(
        now.offset(9, 30, 0, 0),
        now.offset(11, 29, 0, 0),
        MASK_ACTIVE | MASK_ORDER | MASK_TRADING,
    );
    let tr5 = TimeRange::new(
        now.offset(13, 0, 0, 0),
        now.offset(14, 56, 0, 0),
        MASK_ACTIVE | MASK_ORDER | MASK_TRADING,
    );
    let tr6 = TimeRange::new(
        now.offset(14, 57, 0, 0),
        now.offset(15, 0, 0, 0),
        MASK_CALL_AUCTION | MASK_CLOSING,
    );
    TradingSession::new(vec![tr1, tr2, tr3, tr4, tr5, tr6])
}

static TS_TODAY_SESSION: Lazy<Mutex<TradingSession>> = Lazy::new(|| Mutex::new(init_session()));

// Separate RollingOnce instance for reinitializing today's TradingSession daily.
static TS_TODAY_SESSION_ONCE: Lazy<Arc<RollingOnce>> = Lazy::new(|| {
    let mut marker = PathBuf::from(config::get_meta_path());
    marker.push("calendar.updated");
    let ro = RollingOnce::with_daily_reset(marker, PRE_MARKET_HOUR as u32, PRE_MARKET_MINUTE as u32);
    // execute once immediately to set TS_TODAY_SESSION to today's session
    let _ = ro.do_once_try(|| -> Result<(), ()> {
        let mut s = TS_TODAY_SESSION.lock().unwrap();
        *s = init_session();
        Ok(())
    });
    ro
});


// ts_today_init: approximate pre-market timestamp (today at pre-market hour)
fn init_ts_today() -> Timestamp {
    Timestamp::pre_market_time(
        Timestamp::now().to_datetime().year(),
        Timestamp::now().to_datetime().month(),
        Timestamp::now().to_datetime().day(),
    )
    .unwrap_or(Timestamp::now())
}

// Use the existing runtime's RollingOnce to schedule a daily reset and ensure
// the Do/DoOnce semantics are used to provide today's pre-market timestamp once per day.
// TS_TODAY_INIT holds the pre-market timestamp for today (initialized by RollingOnce.do_once)
static TS_TODAY_INIT: Lazy<Mutex<Timestamp>> = Lazy::new(|| Mutex::new(Timestamp::zero()));

// Use the existing runtime's RollingOnce to schedule a daily reset and ensure
// the Do/DoOnce semantics are used for initializing `TS_TODAY_INIT` once per day.
static TS_TODAY_ONCE: Lazy<Arc<RollingOnce>> = Lazy::new(|| {
    let mut marker = PathBuf::from(config::get_meta_path());
    marker.push("session.updated");
    let ro = RollingOnce::with_daily_reset(marker, PRE_MARKET_HOUR as u32, PRE_MARKET_MINUTE as u32);
    // execute once immediately to set TS_TODAY_INIT to today's pre-market time
    let _ = ro.do_once_try(|| -> Result<(), ()> {
        let mut t = TS_TODAY_INIT.lock().unwrap();
        *t = init_ts_today();
        Ok(())
    });
    ro
});

#[derive(Debug, Clone)]
pub struct RuntimeStatus {
    pub before_last_trade_day: bool,
    pub is_holiday: bool,
    pub before_init_time: bool,
    pub cache_after_init_time: bool,
    pub update_in_real_time: bool,
    pub status: TimeStatus,
}

impl Default for RuntimeStatus {
    fn default() -> Self {
        Self {
            before_last_trade_day: false,
            is_holiday: false,
            before_init_time: false,
            cache_after_init_time: false,
            update_in_real_time: false,
            status: MASK_CLOSED,
        }
    }
}

// NOTE: We don't yet port the full calendar.last_trading_day logic; use today as last trading day for now.
use crate::exchange::calendar;

pub fn check_trading_timestamp(last_modified: Option<Timestamp>) -> RuntimeStatus {
    let mut rs = RuntimeStatus::default();
    let now = Timestamp::now();
    let ts = last_modified.unwrap_or(now);

    // Ensure the rolling initializer is active and let it set the module variable.
    let ro = &*TS_TODAY_ONCE;
    let _ = ro.do_once_try(|| -> Result<(), ()> {
        let mut t = TS_TODAY_INIT.lock().unwrap();
        *t = init_ts_today();
        Ok(())
    });
    let ts_today_init = *TS_TODAY_INIT.lock().unwrap();
    let last_day = calendar::last_trading_day(ts_today_init);
    // 1. timestamp before last trading day
    if ts < last_day {
        rs.before_last_trade_day = true;
        return rs;
    }

    // 2. if today != last_day => holiday
    let today = now;
    if !today.is_same_date(&last_day) {
        rs.is_holiday = true;
        return rs;
    }

    // 3. before init
    if ts < ts_today_init {
        rs.before_init_time = true;
        return rs;
    }
    rs.status = MASK_ACTIVE; // pre-market

    rs.cache_after_init_time = true;

    // 5. trading not started
    if get_today_session().is_trading_not_started(ts) {
        return rs
    }

    rs.update_in_real_time = true;

    rs.status = get_today_session().in_session(ts);
    if is_trading_disabled(rs.status) {
        rs.update_in_real_time = false;
    }
    rs
}

/// 返回对全局 `TS_TODAY_SESSION` 的互斥锁保护的访问，语义上与 Go/C++ 的 `GetTodaySession()` 一致。
pub fn get_today_session() -> MutexGuard<'static, TradingSession> {
    // Ensure the session RollingOnce has run for today before returning the session.
    let ro = &*TS_TODAY_SESSION_ONCE;
    let _ = ro.do_once_try(|| -> Result<(), ()> {
        let mut s = TS_TODAY_SESSION.lock().unwrap();
        *s = init_session();
        Ok(())
    });
    TS_TODAY_SESSION.lock().unwrap()
}

pub fn can_update_in_realtime(last_modified: Option<Timestamp>) -> (bool, TimeStatus) {
    let rs = check_trading_timestamp(last_modified);
    (rs.update_in_real_time, rs.status)
}

pub fn can_initialize(last_modified: Option<Timestamp>) -> bool {
    let rs = check_trading_timestamp(last_modified);
    if rs.before_last_trade_day {
        return true;
    }
    if rs.is_holiday {
        return false;
    }
    if rs.before_init_time {
        return false;
    }
    !rs.cache_after_init_time
}


#[cfg(test)]
mod tests {
    use super::*;
    use chrono::Timelike;

    #[test]
    fn test_init_session_bounds() {
        let s = init_session();
        let earliest_dt = s.earliest_start.to_datetime();
        let latest_dt = s.latest_end.to_datetime();
        assert_eq!(earliest_dt.hour(), 9);
        assert_eq!(earliest_dt.minute(), 15);
        assert_eq!(latest_dt.hour(), 15);
        assert_eq!(latest_dt.minute(), 0);
    }

    #[test]
    fn test_check_trading_timestamp_states() {
        // Use init_ts_today() to deterministically compute pre-market time
        let ts_today_init = init_ts_today();

        // timestamp before init: depending on calendar.last_trading_day it may be
        // classified as BeforeLastTradeDay (if last trading day is after `before`),
        // otherwise BeforeInitTime. Accept both possibilities.
        let before = ts_today_init.offset(-1, 0, 0, 0);
        let rs = check_trading_timestamp(Some(before));
        let last_day = calendar::last_trading_day(ts_today_init);
        if before < last_day {
            assert!(rs.before_last_trade_day);
        } else if rs.is_holiday {
            // acceptable: calendar indicates today != last trading day
            assert!(rs.is_holiday);
        } else {
            assert!(rs.before_init_time);
        }

        // timestamp at pre-market should set CacheAfterInitTime (and pre-market status)
        let rs2 = check_trading_timestamp(Some(ts_today_init));
        if rs2.before_last_trade_day || rs2.is_holiday || rs2.before_init_time {
            // acceptable outcomes depending on calendar semantics
        } else {
            assert!(rs2.cache_after_init_time);
            assert_eq!(rs2.status, MASK_ACTIVE);
        }

        // timestamp during trading (09:31) should be update_in_real_time and trading status
        let midnight = ts_today_init.start_of_day();
        let t_0931 = midnight.offset(9, 31, 0, 0);
        let rs3 = check_trading_timestamp(Some(t_0931));
        if rs3.before_last_trade_day || rs3.is_holiday || rs3.before_init_time {
            // acceptable outcomes
        } else {
            assert!(rs3.update_in_real_time);
            assert_eq!(rs3.status, MASK_ACTIVE | MASK_ORDER | MASK_TRADING);
        }
    }
}
