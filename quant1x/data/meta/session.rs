// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// 交易时段管理 - Rust 实现
// 对应 Python 版 quant1x/data/meta/session.py

use std::cmp::min;
use std::collections::HashMap;
use std::path::PathBuf;
use std::sync::{Arc, Mutex};

use once_cell::sync::Lazy;

use crate::config;
use crate::runtime::RollingOnce;

use super::calendar;
use super::exchange::Exchange;
use super::region::Region;
use super::timestamp::{
    Timestamp, MILLISECONDS_PER_MINUTE, PRE_MARKET_HOUR, PRE_MARKET_MINUTE,
};

// ==========================================
// 1. 权限位掩码 (Permission) - 全属性统一
// ==========================================
///
/// 全球统一交易状态位掩码
/// 所有状态信息用一个 u16 整数表示
///
/// 位分配:
/// - Bit 0-3: 订单操作权限
/// - Bit 4: 撮合机制
/// - Bit 5: 成交机制
/// - Bit 6: 统计标志 (计入交易分钟数)
/// - Bit 7: 状态性质 (临时/异常)
/// - Bit 8-15: 预留扩展
///
pub type Permission = u16;

/// 无权限
pub const PERM_NONE: Permission = 0;
/// 允许撤单
pub const PERM_CANCEL: Permission = 1 << 0;
/// 允许改单
pub const PERM_MODIFY: Permission = 1 << 1;
/// 允许市价单
pub const PERM_MARKET: Permission = 1 << 2;
/// 允许限价单
pub const PERM_LIMIT: Permission = 1 << 3;
/// 撮合中
pub const PERM_MATCHING: Permission = 1 << 4;
/// 会产生成交记录
pub const PERM_FILL: Permission = 1 << 5;
/// 计入交易分钟数
pub const PERM_OPEN: Permission = 1 << 6;
/// 临时状态 (可自动恢复)
pub const PERM_IS_TEMPORARY: Permission = 1 << 7;

// 常用组合
/// 撮合成交
pub const PERM_MATCHING_TRANSACTION: Permission = PERM_MATCHING | PERM_FILL;
/// 连续交易
pub const PERM_CONTINUOUS_TRADING: Permission =
    PERM_MARKET | PERM_LIMIT | PERM_CANCEL | PERM_MODIFY | PERM_OPEN | PERM_MATCHING_TRANSACTION;
/// 初始化阶段
pub const PERM_INITIALIZING: Permission = PERM_IS_TEMPORARY;
/// 盘前
pub const PERM_PRE_MARKET: Permission = PERM_IS_TEMPORARY | PERM_CANCEL | PERM_LIMIT;
/// 盘后
pub const PERM_AFTER_HOURS: Permission = PERM_IS_TEMPORARY | PERM_CANCEL | PERM_LIMIT;
/// 集合竞价
pub const PERM_CALL_AUCTION: Permission = PERM_LIMIT | PERM_MATCHING | PERM_IS_TEMPORARY;
/// 集合竞价, 可撤单阶段
pub const PERM_CALL_AUCTION_PRE: Permission = PERM_CALL_AUCTION | PERM_CANCEL;
/// 集合竞价, 不可撤单阶段
pub const PERM_CALL_AUCTION_ORDER: Permission = PERM_CALL_AUCTION;
/// 集合竞价, 随机对盘阶段
pub const PERM_CALL_AUCTION_FILL: Permission = PERM_CALL_AUCTION | PERM_FILL;
/// 只挂单不成交 (午间休市)
pub const PERM_ACCEPT_ORDER_ONLY: Permission = PERM_LIMIT;
/// 只读状态 (停牌)
pub const PERM_READ_ONLY: Permission = PERM_CANCEL;
/// 完全关闭
pub const PERM_CLOSED: Permission = PERM_NONE;
/// 紧急停牌
pub const PERM_EMERGENCY_HALT: Permission = PERM_OPEN;
/// 交易日休息时段
pub const PERM_LUNCH_BREAK: Permission = PERM_ACCEPT_ORDER_ONLY | PERM_IS_TEMPORARY;

/// Permission 辅助方法
pub trait PermissionExt {
    fn can_match(&self) -> bool;
    fn can_cancel(&self) -> bool;
    fn can_modify(&self) -> bool;
    fn can_market_order(&self) -> bool;
    fn can_limit_order(&self) -> bool;
    fn is_suspended(&self) -> bool;
    fn is_continuous_trading(&self) -> bool;
}

impl PermissionExt for Permission {
    fn can_match(&self) -> bool {
        (*self & PERM_MATCHING) != 0
    }

    fn can_cancel(&self) -> bool {
        (*self & PERM_CANCEL) != 0
    }

    fn can_modify(&self) -> bool {
        (*self & PERM_MODIFY) != 0
    }

    fn can_market_order(&self) -> bool {
        (*self & PERM_MARKET) != 0
    }

    fn can_limit_order(&self) -> bool {
        (*self & PERM_LIMIT) != 0
    }

    fn is_suspended(&self) -> bool {
        !self.can_match()
    }

    fn is_continuous_trading(&self) -> bool {
        (*self & PERM_OPEN) != 0
    }
}

// ======================================================================
// 时间状态枚举 (TimeStatus) - 使用掩码组合
// ======================================================================
pub type TimeStatus = u16;

pub const TS_OPEN: TimeStatus = PERM_OPEN;
pub const TS_CLOSED: TimeStatus = PERM_CLOSED;
pub const TS_PRE_MARKET: TimeStatus = PERM_PRE_MARKET;
pub const TS_AFTER_HOURS: TimeStatus = PERM_AFTER_HOURS;
pub const TS_SUSPEND: TimeStatus = PERM_LUNCH_BREAK;
pub const TS_CONTINUOUS_TRADING: TimeStatus = PERM_CONTINUOUS_TRADING;
pub const TS_TRADING: TimeStatus = TS_CONTINUOUS_TRADING;
pub const TS_CALL_AUCTION: TimeStatus = PERM_CALL_AUCTION;
/// 集合竞价, 订单输入阶段, 可撤单
pub const TS_AUCTION_ORDER_INPUT_PERIOD: TimeStatus = TS_CALL_AUCTION | PERM_CANCEL;
/// 集合竞价, 不可撤销阶段
pub const TS_AUCTION_NO_CANCELLATION_PERIOD: TimeStatus = TS_CALL_AUCTION;
/// 集合竞价, 竞价撮合/随机对盘阶段
pub const TS_AUCTION_MATCHING_FILL_PERIOD: TimeStatus = TS_CALL_AUCTION | PERM_FILL;
/// 集合竞价开盘阶段
pub const TS_AUCTION_MATCHING_TO_OPENING: TimeStatus = TS_CALL_AUCTION | PERM_FILL;
/// 集合竞价收盘阶段
pub const TS_AUCTION_MATCHING_TO_CLOSING: TimeStatus = TS_CALL_AUCTION | PERM_FILL;
/// 市场活跃但暂停交易
pub const TS_EXCHANGE_HALT_TRADING: TimeStatus = PERM_OPEN;

/// TimeStatus 辅助方法
pub trait TimeStatusExt {
    fn is_market_active(&self) -> bool;
    fn is_open(&self) -> bool;
    fn is_continuous_trading(&self) -> bool;
    fn is_trading_disabled(&self) -> bool;
    fn has_realtime_data(&self) -> bool;
}

impl TimeStatusExt for TimeStatus {
    fn is_market_active(&self) -> bool {
        self.has_realtime_data()
    }

    fn is_open(&self) -> bool {
        (*self & PERM_OPEN) == PERM_OPEN
    }

    fn is_continuous_trading(&self) -> bool {
        (*self & PERM_CONTINUOUS_TRADING) == PERM_CONTINUOUS_TRADING
    }

    fn is_trading_disabled(&self) -> bool {
        (*self & PERM_MATCHING) == 0
    }

    fn has_realtime_data(&self) -> bool {
        (*self & PERM_MATCHING) != 0
    }
}

// ==========================================
// 时区偏移辅助函数
// ==========================================

/// 计算目标时区与本地时区之间的标准时间差 (小时)
/// 对应 Python 的 get_timezone_offset_standard
fn get_timezone_offset_standard(target_zone: &str) -> i32 {
    // 本地时区偏移 (秒)
    let local_offset = chrono::Local::now().offset().local_minus_utc();
    // 目标时区偏移 (秒)
    let target_offset = match target_zone {
        "Asia/Shanghai" => 8 * 3600,
        "Asia/Hong_Kong" => 8 * 3600,
        "America/New_York" => -5 * 3600, // EST, 简化处理
        "Europe/London" => 0,
        "Europe/Berlin" => 1 * 3600,
        "Asia/Singapore" => 8 * 3600,
        "Asia/Tokyo" => 9 * 3600,
        _ => 0,
    };
    (target_offset - local_offset) / 3600
}

// ==========================================
// TimeRange - 时间范围
// ==========================================

/// 时间范围, 用 ~ 或 - 间隔 HH:MM:SS
#[derive(Debug, Clone)]
pub struct TimeRange {
    pub begin: Timestamp,
    pub end: Timestamp,
    pub status: TimeStatus,
    pub reg: Region,
}

impl TimeRange {
    /// 从字符串构造 TimeRange
    /// 格式: "HH:MM:SS ~ HH:MM:SS" 或 "HH:MM:SS - HH:MM:SS"
    pub fn new(time_range: &str, status: TimeStatus) -> Self {
        Self::new_with_region(time_range, status, Region::CN)
    }

    /// 从字符串构造 TimeRange, 指定区域
    pub fn new_with_region(time_range: &str, status: TimeStatus, reg: Region) -> Self {
        let zone_offset_hours = -get_timezone_offset_standard(reg.timezone());

        let time_range = time_range.trim();

        // 按 ~ 或 - 分割
        let parts: Vec<&str> = if time_range.contains('~') {
            time_range.split('~').collect()
        } else if time_range.contains('-') {
            // 注意: 时间本身包含 - 号，如 "09:30:00 - 11:30:00"
            // 找到中间的分隔符位置
            let mid = time_range.find(" - ").unwrap_or_else(|| {
                // fallback: 在第二个时间格式的冒号后面找 -
                let bytes = time_range.as_bytes();
                let mut idx = 0;
                let mut colon_count = 0;
                for (i, &b) in bytes.iter().enumerate() {
                    if b == b':' {
                        colon_count += 1;
                    }
                    if colon_count >= 4 && b == b'-' {
                        idx = i;
                        break;
                    }
                }
                idx
            });
            if mid > 0 {
                vec![time_range[..mid].trim(), time_range[mid + 1..].trim()]
            } else {
                vec![]
            }
        } else {
            vec![]
        };

        if parts.len() != 2 {
            panic!("非法的时间格式: {}", time_range);
        }

        let begin_str = parts[0].trim();
        let end_str = parts[1].trim();

        let begin = Timestamp::parse_time(begin_str)
            .unwrap_or_else(|_| panic!("无法解析开始时间: {}", begin_str))
            .offset(
                zone_offset_hours,
                0,
                0,
                0,
            );
        let end = Timestamp::parse_time(end_str)
            .unwrap_or_else(|_| panic!("无法解析结束时间: {}", end_str))
            .offset(
                zone_offset_hours,
                0,
                0,
                0,
            );

        let (begin, end) = if begin > end {
            (end, begin)
        } else {
            (begin, end)
        };

        Self {
            begin,
            end,
            status,
            reg,
        }
    }

    /// 直接从两个 Timestamp 构造 TimeRange
    pub fn from_timestamps(begin: Timestamp, end: Timestamp, status: TimeStatus, reg: Region) -> Self {
        let (begin, end) = if begin > end {
            (end, begin)
        } else {
            (begin, end)
        };
        Self {
            begin,
            end,
            status,
            reg,
        }
    }

    /// 判断给定时间戳是否在本交易时段内
    pub fn in_range(&self, timestamp: Option<&Timestamp>) -> Option<TimeStatus> {
        let now;
        let ts = match timestamp {
            Some(t) => t,
            None => {
                now = Timestamp::now();
                &now
            }
        };
        if self.begin <= *ts && *ts < self.end {
            Some(self.status)
        } else {
            None
        }
    }

    /// 判断给定时间戳是否在连续竞价交易中
    pub fn is_trading(&self, timestamp: Option<&Timestamp>) -> bool {
        match self.in_range(timestamp) {
            Some(status) => (status & TS_TRADING) == TS_TRADING,
            None => false,
        }
    }

    /// 时段是否有效
    pub fn is_valid(&self) -> bool {
        !self.begin.is_empty() && !self.end.is_empty()
    }

    /// 是否盘前 (给定时间戳 < begin)
    pub fn is_session_pre(&self, timestamp: Option<&Timestamp>) -> bool {
        let now;
        let ts = match timestamp {
            Some(t) => t,
            None => {
                now = Timestamp::now();
                &now
            }
        };
        *ts < self.begin
    }

    /// 是否盘中
    pub fn is_session_reg(&self, timestamp: Option<&Timestamp>) -> bool {
        self.is_trading(timestamp)
    }

    /// 是否盘后 (给定时间戳 >= end)
    pub fn is_session_post(&self, timestamp: Option<&Timestamp>) -> bool {
        let now;
        let ts = match timestamp {
            Some(t) => t,
            None => {
                now = Timestamp::now();
                &now
            }
        };
        *ts >= self.end
    }

    /// 计算时段总时长 (分钟)
    pub fn get_duration_minutes(&self) -> i64 {
        let start_minutes = self.begin.value() / MILLISECONDS_PER_MINUTE;
        let end_minutes = self.end.value() / MILLISECONDS_PER_MINUTE;

        if end_minutes > start_minutes {
            end_minutes - start_minutes
        } else {
            (24 * 60 - start_minutes) + end_minutes
        }
    }

    /// 时段已经开始多少分钟
    pub fn get_elapsed_minutes(&self, current_time: &Timestamp) -> i64 {
        let current = min(*current_time, self.end);
        let start = min(self.begin, current);
        let current_minutes = current.value() / MILLISECONDS_PER_MINUTE;
        let start_minutes = start.value() / MILLISECONDS_PER_MINUTE;

        if current_minutes >= start_minutes {
            current_minutes - start_minutes
        } else {
            0
        }
    }
}

// ==========================================
// TradingSession - 交易时段
// ==========================================

/// 交易时段
#[derive(Debug, Clone)]
pub struct TradingSession {
    pub sessions: Vec<TimeRange>,
    /// 最早开始时间
    pub earliest_start: Timestamp,
    /// 最晚结束时间
    pub latest_end: Timestamp,
    /// 收盘时间点
    pub closing_time: Timestamp,
}

impl Default for TradingSession {
    fn default() -> Self {
        Self {
            sessions: Vec::new(),
            earliest_start: Timestamp::parse_time("23:59:59").unwrap_or(Timestamp::zero()),
            latest_end: Timestamp::parse_time("00:00:00").unwrap_or(Timestamp::zero()),
            closing_time: Timestamp::parse_time("00:00:00").unwrap_or(Timestamp::zero()),
        }
    }
}

impl TradingSession {
    /// 从 TimeRange 列表构造
    pub fn new(sessions: Vec<TimeRange>) -> Self {
        let mut ts = Self::default();
        ts.sessions = sessions;
        ts.update_time_bounds();
        ts
    }

    /// 从字符串构造 (兼容旧的字符串构造方式)
    /// 格式: "09:30:00 ~ 11:30:00, 13:00:00 ~ 15:00:00"
    pub fn from_str(time_range_str: &str) -> Self {
        let mut sessions = Vec::new();
        for part in time_range_str.split(',') {
            let part = part.trim();
            if !part.is_empty() {
                sessions.push(TimeRange::new(part, TS_TRADING));
            }
        }
        Self::new(sessions)
    }

    /// 更新交易时段的时间边界
    pub fn update_time_bounds(&mut self) {
        if self.sessions.is_empty() {
            self.earliest_start =
                Timestamp::parse_time("23:59:59").unwrap_or(Timestamp::zero());
            self.latest_end =
                Timestamp::parse_time("00:00:00").unwrap_or(Timestamp::zero());
            self.closing_time =
                Timestamp::parse_time("00:00:00").unwrap_or(Timestamp::zero());
            return;
        }

        self.earliest_start =
            Timestamp::parse_time("23:59:59").unwrap_or(Timestamp::zero());
        self.latest_end =
            Timestamp::parse_time("00:00:00").unwrap_or(Timestamp::zero());
        self.closing_time =
            Timestamp::parse_time("00:00:00").unwrap_or(Timestamp::zero());

        for session in &self.sessions {
            if session.begin < self.earliest_start {
                self.earliest_start = session.begin;
            }
            if session.end > self.latest_end {
                self.latest_end = session.end;
                if session.status.is_open() {
                    self.closing_time = session.end;
                }
            }
        }
    }

    /// 添加交易时段
    pub fn add_session(&mut self, range: TimeRange) {
        self.sessions.push(range);
        self.update_time_bounds();
    }

    /// 判断当前时间的状态
    pub fn check_status(&self, timestamp: Option<&Timestamp>) -> TimeStatus {
        let now;
        let ts = match timestamp {
            Some(t) => t,
            None => {
                now = Timestamp::now();
                &now
            }
        };

        for session in &self.sessions {
            if let Some(status) = session.in_range(Some(ts)) {
                return status;
            }
        }

        // 不在任何交易时段内
        if *ts < self.earliest_start {
            return TS_PRE_MARKET;
        }

        if *ts < self.latest_end {
            return TS_EXCHANGE_HALT_TRADING;
        }

        TS_CLOSED
    }

    /// 是否交易中
    pub fn is_trading(&self, timestamp: Option<&Timestamp>) -> bool {
        let status = self.check_status(timestamp);
        (status & TS_TRADING) == TS_TRADING
    }

    /// 时段是否有效
    pub fn is_valid(&self) -> bool {
        self.sessions.iter().all(|item| item.is_valid())
    }

    /// 交易是否尚未开始
    pub fn is_trading_not_started(&self, timestamp: Option<&Timestamp>) -> bool {
        let now;
        let ts = match timestamp {
            Some(t) => t,
            None => {
                now = Timestamp::now();
                &now
            }
        };
        *ts < self.earliest_start
    }

    /// 交易是否已结束
    pub fn is_trading_ended(&self, timestamp: Option<&Timestamp>) -> bool {
        let now;
        let ts = match timestamp {
            Some(t) => t,
            None => {
                now = Timestamp::now();
                &now
            }
        };
        *ts > self.latest_end
    }

    /// 计算当前时间距离最近的交易时间的分钟数
    pub fn minutes(&self, timestamp: Option<&Timestamp>) -> i64 {
        let now;
        let ts = match timestamp {
            Some(t) => t,
            None => {
                now = Timestamp::now();
                &now
            }
        };
        self.sessions
            .iter()
            .filter(|tr| tr.status.is_open())
            .map(|tr| tr.get_elapsed_minutes(ts))
            .sum()
    }

    /// 当日可交易时段总时长 (分钟)
    pub fn get_trading_minutes(&self) -> i64 {
        self.sessions
            .iter()
            .filter(|tr| tr.status.is_open())
            .map(|tr| tr.get_duration_minutes())
            .sum()
    }
}

// ==========================================
// 各市场交易时段初始化
// ==========================================

/// 初始化 A 股交易时段
pub fn init_cn_session() -> TradingSession {
    // 9:15~9:20, 开盘集合竞价, 可撤单
    let tr1 = TimeRange::new("09:15:00 ~ 09:20:00", TS_AUCTION_ORDER_INPUT_PERIOD);
    // 9:20~9:25, 开盘集合竞价, 不可撤单
    let tr2 = TimeRange::new("09:20:00 ~ 09:25:00", TS_AUCTION_MATCHING_TO_OPENING);
    // 9:25~9:30, 休市
    let tr3 = TimeRange::new("09:25:00 ~ 09:30:00", TS_SUSPEND);
    // 9:30~11:30, 连续竞价
    let tr4 = TimeRange::new("09:30:00 ~ 11:30:00", TS_TRADING);
    // 13:00~14:57, 连续竞价
    let tr5 = TimeRange::new("13:00:00 ~ 14:57:00", TS_TRADING);
    // 14:57~15:00, 收盘集合竞价
    let tr6 = TimeRange::new("14:57:00 ~ 15:00:00", TS_AUCTION_MATCHING_TO_CLOSING | PERM_OPEN);

    TradingSession::new(vec![tr1, tr2, tr3, tr4, tr5, tr6])
}

/// 初始化港股交易时段
pub fn init_hk_session() -> TradingSession {
    // 1. 输入买卖盘时段：9:00-9:15
    let tr1 = TimeRange::new("09:00:00 ~ 09:15:00", TS_AUCTION_ORDER_INPUT_PERIOD);
    // 2. 不可取消时段：9:15-9:20
    let tr2 = TimeRange::new("09:15:00 ~ 09:20:00", TS_AUCTION_NO_CANCELLATION_PERIOD);
    // 3. 随机对盘时段：9:20-9:22
    let tr3 = TimeRange::new("09:20:00 ~ 09:22:00", TS_AUCTION_MATCHING_TO_OPENING);
    // 4. 暂停时段：9:22-9:30
    let tr4 = TimeRange::new("09:22:00 ~ 09:30:00", TS_SUSPEND);
    // 5. 连续交易：9:30-12:00
    let tr5 = TimeRange::new("09:30:00 ~ 12:00:00", TS_CONTINUOUS_TRADING);
    // 6. 午间休市：12:00-13:00
    let tr6 = TimeRange::new("12:00:00 ~ 13:00:00", TS_SUSPEND);
    // 7. 连续交易：13:00-16:00
    let tr7 = TimeRange::new("13:00:00 ~ 16:00:00", TS_CONTINUOUS_TRADING);
    // 8. 收盘竞价 - 参考价定价阶段 (16:00-16:01)
    let tr8 = TimeRange::new("16:00:00 ~ 16:01:00", TS_AUCTION_ORDER_INPUT_PERIOD);
    // 9. 收盘竞价 - 输入订单阶段 (16:01-16:06)
    let tr9 = TimeRange::new("16:01:00 ~ 16:06:00", TS_AUCTION_ORDER_INPUT_PERIOD);
    // 10. 收盘竞价 - 不可撤销阶段 (16:06-16:08)
    let tr10 = TimeRange::new("16:06:00 ~ 16:08:00", TS_AUCTION_NO_CANCELLATION_PERIOD);
    // 11. 收盘竞价 - 随机收盘 (16:06-16:10)
    let tr11 = TimeRange::new("16:06:00 ~ 16:10:00", TS_AUCTION_MATCHING_TO_CLOSING);

    TradingSession::new(vec![tr1, tr2, tr3, tr4, tr5, tr6, tr7, tr8, tr9, tr10, tr11])
}

/// 初始化美股交易时段
pub fn init_us_session() -> TradingSession {
    let tr1 = TimeRange::new_with_region("04:00:00 ~ 09:30:00", TS_PRE_MARKET, Region::US);
    let tr2 = TimeRange::new_with_region("09:30:00 ~ 16:00:00", TS_TRADING, Region::US);
    let tr3 = TimeRange::new_with_region("16:00:00 ~ 20:00:00", TS_AFTER_HOURS, Region::US);

    TradingSession::new(vec![tr1, tr2, tr3])
}

// ==========================================
// 全局交易时段管理
// ==========================================

/// 全局交易时段映射
static TRADING_HOURS_MAP: Lazy<Mutex<HashMap<String, TradingSession>>> =
    Lazy::new(|| Mutex::new(HashMap::new()));

/// 默认中国市场时段
fn default_trading_session() -> TradingSession {
    init_cn_session()
}

/// 每日交易时段初始化 RollingOnce
static TS_TODAY_SESSION_ONCE: Lazy<Arc<RollingOnce>> = Lazy::new(|| {
    let mut marker = PathBuf::from(config::get_meta_path());
    marker.push("sessions_init.updated");
    RollingOnce::with_daily_reset(marker, PRE_MARKET_HOUR, PRE_MARKET_MINUTE)
});

/// 初始化各市场交易时段信息
fn ts_today_session_init() {
    let mut map = TRADING_HOURS_MAP.lock().unwrap();
    map.insert(Region::CN.as_str().to_lowercase(), init_cn_session());
    map.insert(Region::HK.as_str().to_lowercase(), init_hk_session());
    map.insert(Region::US.as_str().to_lowercase(), init_us_session());
}

/// 获取指定交易所当天的交易时段信息
pub fn latest_session_by_exchange(exchange: Exchange) -> TradingSession {
    let _ = TS_TODAY_SESSION_ONCE.do_once_try(|| -> Result<(), ()> {
        ts_today_session_init();
        Ok(())
    });

    let key = exchange.region().as_str().to_lowercase();
    let map = TRADING_HOURS_MAP.lock().unwrap();
    match map.get(&key) {
        Some(session) => session.clone(),
        None => {
            log::warn!("Unsupported exchange: {:?}, using default CN session", exchange);
            default_trading_session()
        }
    }
}

// ==========================================
// RuntimeStatus 与 check_trading_timestamp
// ==========================================

/// 运行时状态
#[derive(Debug, Clone, Default)]
pub struct RuntimeStatus {
    /// 最后交易日前
    pub before_last_trade_day: bool,
    /// 是否节假日休市
    pub is_holiday: bool,
    /// 初始化时间前
    pub before_init_time: bool,
    /// 缓存在初始化时间之后
    pub cache_after_init_time: bool,
    /// 是否可以实时更新
    pub update_in_real_time: bool,
    /// 当前状态
    pub status: TimeStatus,
}

// ==========================================
// ts_today_init - 盘前初始化时间戳
// ==========================================

static TS_TODAY_INIT: Lazy<Mutex<Timestamp>> = Lazy::new(|| Mutex::new(Timestamp::zero()));

static TS_TODAY_ONCE: Lazy<Arc<RollingOnce>> = Lazy::new(|| {
    let mut marker = PathBuf::from(config::get_meta_path());
    marker.push("session.updated");
    RollingOnce::with_daily_reset(marker, PRE_MARKET_HOUR, PRE_MARKET_MINUTE)
});

/// 获取今天的盘前初始化时间戳 (每日仅计算一次)
pub fn get_today() -> Timestamp {
    let _ = TS_TODAY_ONCE.do_once_try(|| -> Result<(), ()> {
        let now = Timestamp::now();
        let pre_market = now.pre_market_time_from_current().unwrap_or(now);
        let mut t = TS_TODAY_INIT.lock().unwrap();
        *t = pre_market;
        Ok(())
    });

    *TS_TODAY_INIT.lock().unwrap()
}

/// 检查交易时间戳状态
pub fn check_trading_timestamp(
    exchange: Exchange,
    last_modified: Option<Timestamp>,
) -> RuntimeStatus {
    log::debug!(
        "check_trading_timestamp called with exchange={:?}, last_modified={:?}",
        exchange,
        last_modified
    );

    let mut rs = RuntimeStatus::default();
    rs.status = TS_CLOSED;

    let now = Timestamp::now();
    let ts = last_modified.unwrap_or(now);

    log::debug!("check_trading_timestamp: {}", ts);

    let last_day = calendar::last_trading_day(now);

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
    let ts_today = get_today();
    if ts < ts_today {
        rs.before_init_time = true;
        return rs;
    }

    rs.status = TS_PRE_MARKET;
    rs.cache_after_init_time = true;

    // 5. trading not started
    let session = latest_session_by_exchange(exchange);
    if session.is_trading_not_started(Some(&ts)) {
        return rs;
    }

    rs.update_in_real_time = true;

    rs.status = session.check_status(Some(&ts));
    if rs.status.is_trading_disabled() {
        rs.update_in_real_time = false;
    }

    rs
}

/// 是否可以初始化
pub fn can_initialize(exchange: Exchange, last_modified: Option<Timestamp>) -> bool {
    let rs = check_trading_timestamp(exchange, last_modified);
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

// ==========================================
// 测试
// ==========================================

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_permission_constants() {
        assert_eq!(PERM_NONE, 0);
        assert_eq!(PERM_CANCEL, 1);
        assert_eq!(PERM_MODIFY, 2);
        assert_eq!(PERM_MARKET, 4);
        assert_eq!(PERM_LIMIT, 8);
        assert_eq!(PERM_MATCHING, 16);
        assert_eq!(PERM_FILL, 32);
        assert_eq!(PERM_OPEN, 64);
        assert_eq!(PERM_IS_TEMPORARY, 128);
    }

    #[test]
    fn test_permission_methods() {
        assert!(!PERM_NONE.can_match());
        assert!(!PERM_NONE.can_cancel());
        assert!(PERM_CANCEL.can_cancel());
        assert!(PERM_CONTINUOUS_TRADING.can_market_order());
        assert!(PERM_CONTINUOUS_TRADING.can_limit_order());
        assert!(PERM_CONTINUOUS_TRADING.can_match());
        assert!(PermissionExt::is_continuous_trading(&PERM_CONTINUOUS_TRADING));
        assert!(!PermissionExt::is_continuous_trading(&PERM_CLOSED));
    }

    #[test]
    fn test_timestatus_methods() {
        assert!(TS_CONTINUOUS_TRADING.is_open());
        assert!(TimeStatusExt::is_continuous_trading(&TS_CONTINUOUS_TRADING));
        assert!(TS_CONTINUOUS_TRADING.is_market_active());
        assert!(!TS_CONTINUOUS_TRADING.is_trading_disabled());
        assert!(TS_CLOSED.is_trading_disabled());
        assert!(!TS_SUSPEND.is_market_active());
    }

    #[test]
    fn test_time_range_creation() {
        let tr = TimeRange::new("09:30:00 ~ 11:30:00", TS_TRADING);
        assert!(tr.is_valid());
        assert!(tr.begin < tr.end);
    }

    #[test]
    fn test_time_range_in_range() {
        let tr = TimeRange::new("09:30:00 ~ 11:30:00", TS_TRADING);

        // 在范围内
        let ts_in = Timestamp::parse_time("10:00:00").unwrap();
        assert_eq!(tr.in_range(Some(&ts_in)), Some(TS_TRADING));

        // 不在范围内
        let ts_out = Timestamp::parse_time("12:00:00").unwrap();
        assert_eq!(tr.in_range(Some(&ts_out)), None);
    }

    #[test]
    fn test_time_range_is_trading() {
        let tr = TimeRange::new("09:30:00 ~ 11:30:00", TS_TRADING);
        let ts_in = Timestamp::parse_time("10:00:00").unwrap();
        assert!(tr.is_trading(Some(&ts_in)));

        let ts_out = Timestamp::parse_time("12:00:00").unwrap();
        assert!(!tr.is_trading(Some(&ts_out)));
    }

    #[test]
    fn test_time_range_duration() {
        let tr = TimeRange::new("09:30:00 ~ 11:30:00", TS_TRADING);
        let duration = tr.get_duration_minutes();
        assert_eq!(duration, 120); // 2 hours
    }

    #[test]
    fn test_time_range_elapsed() {
        let tr = TimeRange::new("09:30:00 ~ 11:30:00", TS_TRADING);
        let ts = Timestamp::parse_time("10:00:00").unwrap();
        let elapsed = tr.get_elapsed_minutes(&ts);
        assert_eq!(elapsed, 30); // 30 minutes
    }

    #[test]
    fn test_time_range_session_pre_post() {
        let tr = TimeRange::new("09:30:00 ~ 11:30:00", TS_TRADING);
        let ts_pre = Timestamp::parse_time("09:00:00").unwrap();
        assert!(tr.is_session_pre(Some(&ts_pre)));

        let ts_reg = Timestamp::parse_time("10:00:00").unwrap();
        assert!(tr.is_session_reg(Some(&ts_reg)));

        let ts_post = Timestamp::parse_time("12:00:00").unwrap();
        assert!(tr.is_session_post(Some(&ts_post)));
    }

    #[test]
    fn test_trading_session_from_str() {
        let session = TradingSession::from_str("09:30:00 ~ 11:30:00, 13:00:00 ~ 15:00:00");
        assert_eq!(session.sessions.len(), 2);
        assert!(session.is_valid());
    }

    #[test]
    fn test_trading_session_check_status() {
        let session = TradingSession::from_str("09:30:00 ~ 11:30:00, 13:00:00 ~ 15:00:00");

        // 盘中
        let ts_trading = Timestamp::parse_time("10:00:00").unwrap();
        assert_eq!(session.check_status(Some(&ts_trading)), TS_TRADING);

        // 休市 (午间)
        let ts_lunch = Timestamp::parse_time("12:00:00").unwrap();
        assert_eq!(session.check_status(Some(&ts_lunch)), TS_EXCHANGE_HALT_TRADING);

        // 盘前
        let ts_pre = Timestamp::parse_time("09:00:00").unwrap();
        assert_eq!(session.check_status(Some(&ts_pre)), TS_PRE_MARKET);

        // 盘后
        let ts_post = Timestamp::parse_time("15:30:00").unwrap();
        assert_eq!(session.check_status(Some(&ts_post)), TS_CLOSED);
    }

    #[test]
    fn test_init_cn_session() {
        let session = init_cn_session();
        assert_eq!(session.sessions.len(), 6);
        assert!(session.is_valid());

        // 测试具体时段
        let ts = Timestamp::parse_time("09:16:00").unwrap();
        let status = session.check_status(Some(&ts));
        assert_eq!(status, TS_AUCTION_ORDER_INPUT_PERIOD);

        let ts2 = Timestamp::parse_time("10:00:00").unwrap();
        assert!(session.is_trading(Some(&ts2)));

        // 测试 minutes
        let trading_minutes = session.get_trading_minutes();
        assert!(trading_minutes > 0);
    }

    #[test]
    fn test_init_hk_session() {
        let session = init_hk_session();
        assert_eq!(session.sessions.len(), 11);
        assert!(session.is_valid());

        let ts = Timestamp::parse_time("09:16:00").unwrap();
        let status = session.check_status(Some(&ts));
        assert_eq!(status, TS_AUCTION_NO_CANCELLATION_PERIOD);
    }

    #[test]
    fn test_init_us_session() {
        let session = init_us_session();
        assert_eq!(session.sessions.len(), 3);
        assert!(session.is_valid());

        let ts_pre = Timestamp::parse_time("05:00:00").unwrap();
        let status = session.check_status(Some(&ts_pre));
        assert_eq!(status, TS_PRE_MARKET);

        let ts_trading = Timestamp::parse_time("10:00:00").unwrap();
        assert!(session.is_trading(Some(&ts_trading)));

        let ts_post = Timestamp::parse_time("17:00:00").unwrap();
        let status = session.check_status(Some(&ts_post));
        assert_eq!(status, TS_AFTER_HOURS);
    }

    #[test]
    fn test_latest_session_by_exchange() {
        let cn_session = latest_session_by_exchange(Exchange::SSE);
        assert_eq!(cn_session.sessions.len(), 6);

        let hk_session = latest_session_by_exchange(Exchange::HKEX);
        assert_eq!(hk_session.sessions.len(), 11);

        let us_session = latest_session_by_exchange(Exchange::USA);
        assert_eq!(us_session.sessions.len(), 3);
    }

    #[test]
    fn test_check_trading_timestamp_holiday() {
        // 使用一个周末日期来测试假日检测
        let rs = check_trading_timestamp(Exchange::SSE, None);
        // 至少应该有一个非 None 的状态
        assert!(rs.status == TS_CLOSED || rs.is_holiday || rs.before_init_time || rs.before_last_trade_day);
    }
}
