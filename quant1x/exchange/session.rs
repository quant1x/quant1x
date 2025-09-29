use crate::timestamp::Timestamp;
use chrono::Datelike;
use once_cell::sync::Lazy;
use std::cmp::min;
use std::sync::Mutex;

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

// ts_today_init: approximate pre-market timestamp (today at pre-market hour)
fn init_ts_today() -> Timestamp {
    Timestamp::pre_market_time(
        Timestamp::now().to_datetime().year(),
        Timestamp::now().to_datetime().month(),
        Timestamp::now().to_datetime().day(),
    )
    .unwrap_or(Timestamp::now())
}

static TS_TODAY_INIT: Lazy<Mutex<Timestamp>> = Lazy::new(|| Mutex::new(init_ts_today()));

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

    let last_day = calendar::last_trading_day(*TS_TODAY_INIT.lock().unwrap());
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
    if ts < *TS_TODAY_INIT.lock().unwrap() {
        rs.before_init_time = true;
        return rs;
    }
    rs.status = MASK_ACTIVE; // pre-market

    rs.cache_after_init_time = true;

    // 5. trading not started
    if TS_TODAY_SESSION.lock().unwrap().is_trading_not_started(ts) {
        return rs;
    }

    rs.update_in_real_time = true;

    rs.status = TS_TODAY_SESSION.lock().unwrap().in_session(ts);
    if is_trading_disabled(rs.status) {
        rs.update_in_real_time = false;
    }
    rs
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
