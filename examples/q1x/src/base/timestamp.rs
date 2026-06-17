use chrono::{Local, TimeZone};
use chrono::NaiveTime;
use regex::Regex;
use std::fmt;
use thiserror::Error;

// 时间格式常量
pub const FORMAT_ONLY_DATE: &str = "%Y-%m-%d";
pub const FORMAT_ONLY_TIME: &str = "%H:%M:%S";
#[allow(dead_code)]
pub const FORMAT_FILE_DATE: &str = "%Y%m%d";
pub const FORMAT_DATETIME: &str = "%Y-%m-%d %H:%M:%S";
#[allow(dead_code)]
pub const FORMAT_TIMESTAMP: &str = "%Y-%m-%d %H:%M:%S.%f";

/// 时间转换错误类型
#[derive(Debug, Error)]
pub enum TimeError {
    #[error("Invalid timestamp format")]
    InvalidFormat,
    #[error("Parse error: {0}")]
    ParseError(#[from] chrono::ParseError),
}

/// 时间范围错误类型
#[derive(Debug, Error)]
pub enum TimeRangeError {
    #[error("Invalid time range format")]
    InvalidFormat,
    #[error("Parse error: {0}")]
    TimeError(#[from] TimeError),
}

/// 交易时段结构体
#[derive(Debug, PartialEq)]
pub struct TimeRange {
    begin: NaiveTime,
    end: NaiveTime,
}

impl TimeRange {
    pub fn new(time_range: &str) -> Result<Self, TimeRangeError> {
        let re = Regex::new(r"[~-]\s*").unwrap();
        let parts: Vec<&str> = re.split(time_range.trim()).collect();
        if parts.len() != 2 {
            return Err(TimeRangeError::InvalidFormat);
        }

        let parse_time = |s: &str| -> Result<NaiveTime, TimeError> {
            NaiveTime::parse_from_str(s.trim(), FORMAT_ONLY_TIME)
                .map_err(|_| TimeError::InvalidFormat)
        };

        let mut begin = parse_time(parts[0])?;
        let mut end = parse_time(parts[1])?;

        if begin > end {
            std::mem::swap(&mut begin, &mut end);
        }

        Ok(Self { begin, end })
    }

    /// 是否在交易中
    pub fn is_trading(&self, timestamp: Option<&str>) -> Result<bool, TimeError> {
        let time_str = match timestamp {
            Some(ts) => ts.trim(),
            None => &NaiveTime::now().format(FORMAT_ONLY_TIME).to_string(),
        };

        let time = NaiveTime::parse_from_str(time_str, FORMAT_ONLY_TIME)?;
        Ok(time >= self.begin && time <= self.end)
    }

    pub fn is_valid(&self) -> bool {
        !self.begin.format(FORMAT_ONLY_TIME).to_string().is_empty() &&
            !self.end.format(FORMAT_ONLY_TIME).to_string().is_empty()
    }

    /// 盘前
    pub fn is_session_pre(&self, timestamp: Option<&str>) -> Result<bool, TimeError> {
        let time_str = match timestamp {
            Some(ts) => ts.trim(),
            None => &NaiveTime::now().format(FORMAT_ONLY_TIME).to_string(),
        };

        let time = NaiveTime::parse_from_str(time_str, FORMAT_ONLY_TIME)?;
        Ok(time < self.begin)
    }

    /// 盘后
    pub fn is_session_post(&self, timestamp: Option<&str>) -> Result<bool, TimeError> {
        let time_str = match timestamp {
            Some(ts) => ts.trim(),
            None => &NaiveTime::now().format(FORMAT_ONLY_TIME).to_string(),
        };

        let time = NaiveTime::parse_from_str(time_str, FORMAT_ONLY_TIME)?;
        Ok(time > self.end)
    }
}

impl fmt::Display for TimeRange {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{} ~ {}", self.begin.format(FORMAT_ONLY_TIME), self.end.format(FORMAT_ONLY_TIME))
    }
}

/// 交易时段集合
#[derive(Debug)]
pub struct TradingSession {
    sessions: Vec<TimeRange>,
}

impl TradingSession {
    pub fn new(time_ranges: &str) -> Result<Self, TimeRangeError> {
        let re = Regex::new(r",\s*").unwrap();
        let parts: Vec<&str> = re.split(time_ranges.trim()).collect();

        let mut sessions = Vec::new();
        for part in parts {
            let tr = TimeRange::new(part)?;
            sessions.push(tr);
        }
        Ok(Self { sessions })
    }

    pub fn is_trading(&self, timestamp: Option<&str>) -> Result<bool, TimeError> {
        for session in &self.sessions {
            if session.is_trading(timestamp)? {
                return Ok(true);
            }
        }
        Ok(false)
    }

    pub fn is_valid(&self) -> bool {
        self.sessions.iter().all(|tr| tr.is_valid())
    }
}

impl fmt::Display for TradingSession {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "[{}]",
            self.sessions
                .iter()
                .map(|tr| tr.to_string())
                .collect::<Vec<String>>()
                .join(", ")
        )
    }
}

/// 时间转换函数
// pub fn seconds_to_timestamp(seconds: i64) -> String {
//     NaiveDateTime::from_timestamp_opt(seconds, 0)
//         .map(|dt| dt.format(FORMAT_DATETIME).to_string())
//         .unwrap_or_else(|| {
//             eprintln!("Invalid timestamp: {}", seconds);
//             "Invalid timestamp".to_string()
//         })
// }

// 安全且无警告的时间转换方法
pub fn seconds_to_timestamp(seconds: i64) -> String {
    // 创建本地时间对象
    let dt = Local.timestamp_opt(seconds, 0)
        .single()
        .expect("无效时间戳");
    //println!("{}", dt);
    // 提取无时区信息的时间
    let dt1 = dt.naive_local();
    //println!("{}", dt1);
    //println!("{}", Local::now().format(FORMAT_DATETIME).to_string());
    //println!("{}", dt.offset());
    dt1.format(FORMAT_DATETIME).to_string()
}


use std::time::{SystemTime, UNIX_EPOCH};

// 直接本地时间转换(无需UTC)
pub fn local_timestamp_millis() -> i64 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .expect("Time went backwards")
        .as_millis() as i64
}

// // 将秒数转换为本地时间
// pub fn seconds_to_local_datetime(seconds: i64) -> NaiveDateTime {
//     let d = std::time::Duration::from_secs(seconds as u64);
//     let systime = UNIX_EPOCH + d;
//     systime
//         .duration_since(UNIX_EPOCH)
//         .map(|dur| NaiveDateTime::from_timestamp_opt(dur.as_secs() as i64, 0))
//         .unwrap()
//         .expect("Invalid timestamp")
// }


/// 扩展NaiveTime的now()方法
trait NaiveTimeExt {
    fn now() -> NaiveTime;
}

impl NaiveTimeExt for NaiveTime {
    fn now() -> Self {
        chrono::Local::now().naive_local().time()
    }
}

/// 单元测试模块
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_time_conversion() {
        let now = chrono::Local::now().timestamp();
        let timestamp = seconds_to_timestamp(now);
        assert!(timestamp.len() >= 19);
        let timestamp = 1697004000; // 北京时间2023-10-10 12:00:00
        //println!("Timestamp: {}", timestamp_to_local(timestamp));
        println!("Timestamp: {}", seconds_to_timestamp(timestamp));
    }

    #[test]
    fn test_time_range_parsing() {
        let tr = TimeRange::new("09:30:00 ~ 14:56:30").unwrap();
        assert_eq!(tr.begin.format(FORMAT_ONLY_TIME).to_string(), "09:30:00");
        assert_eq!(tr.end.format(FORMAT_ONLY_TIME).to_string(), "14:56:30");

        let tr = TimeRange::new("14:56:30 - 09:30:00").unwrap();
        assert_eq!(tr.begin.format(FORMAT_ONLY_TIME).to_string(), "09:30:00");
        assert_eq!(tr.end.format(FORMAT_ONLY_TIME).to_string(), "14:56:30");
    }

    #[test]
    fn test_trading_status() {
        let tr = TimeRange::new("09:30:00 ~ 14:56:30").unwrap();
        //assert!(tr.is_trading().unwrap());
        assert!(tr.is_trading(Some("12:00:00")).unwrap());
        assert!(!tr.is_trading(Some("08:00:00")).unwrap());
        assert!(!tr.is_trading(Some("15:00:00")).unwrap());
    }

    #[test]
    fn test_trading_session() {
        let ts = TradingSession::new("11:30:00 ~ 09:15:00, 15:00:00 - 13:00:00").unwrap();
        assert!(ts.is_trading(Some("10:00:00")).unwrap());
        assert!(ts.is_trading(Some("14:00:00")).unwrap());
        assert!(!ts.is_trading(Some("12:00:00")).unwrap());
    }

    #[test]
    fn test_edge_cases() {
        let tr = TimeRange::new("23:00:00 ~ 01:00:00").unwrap();
        assert!(!tr.is_trading(Some("23:30:00")).unwrap());
        assert!(!tr.is_trading(Some("00:30:00")).unwrap());
    }
}