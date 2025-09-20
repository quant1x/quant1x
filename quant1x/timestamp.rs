use chrono::{DateTime, Datelike, Local, NaiveDate, NaiveDateTime, TimeZone, Timelike};
use std::fmt;

// 时间常量
pub const SECONDS_PER_MINUTE: i64 = 60;
pub const SECONDS_PER_HOUR: i64 = 60 * SECONDS_PER_MINUTE;
pub const SECONDS_PER_DAY: i64 = 24 * SECONDS_PER_HOUR;
pub const MILLISECONDS_PER_SECOND: i64 = 1000;
pub const MILLISECONDS_PER_MINUTE: i64 = SECONDS_PER_MINUTE * MILLISECONDS_PER_SECOND;
pub const MILLISECONDS_PER_HOUR: i64 = SECONDS_PER_HOUR * MILLISECONDS_PER_SECOND;
pub const MILLISECONDS_PER_DAY: i64 = SECONDS_PER_DAY * MILLISECONDS_PER_SECOND;

// 盘前时间配置（对应C++中的config::cn_pre_market_*）
pub const PRE_MARKET_HOUR: u32 = 9;
pub const PRE_MARKET_MINUTE: u32 = 0;
pub const PRE_MARKET_SECOND: u32 = 0;

/// 本地时间戳，单位毫秒
/// 
/// 这个结构体提供了与C++ timestamp类相同的API和功能，
/// 用于处理本地时间戳的各种操作。
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub struct Timestamp {
    ms: i64,
}

impl Timestamp {
    /// 创建新的时间戳
    /// 
    /// # Arguments
    /// * `ms` - 毫秒时间戳
    /// 
    /// # Examples
    /// ```
    /// use quant1x_std::Timestamp;
    /// let ts = Timestamp::new(1640995200000);
    /// ```
    pub fn new(ms: i64) -> Self {
        Self { ms }
    }

    /// 从chrono::DateTime创建时间戳
    /// 
    /// # Arguments
    /// * `dt` - chrono::DateTime对象
    pub fn from_datetime(dt: DateTime<Local>) -> Self {
        Self::new(dt.timestamp_millis())
    }

    /// 从字符串解析创建时间戳
    /// 
    /// # Arguments
    /// * `s` - 时间字符串
    pub fn from_string(s: &str) -> Result<Self, chrono::ParseError> {
        Self::parse(s)
    }

    /// 从年月日时分秒毫秒创建时间戳
    /// 
    /// # Arguments
    /// * `year` - 年
    /// * `month` - 月
    /// * `day` - 日
    /// * `hour` - 时
    /// * `minute` - 分
    /// * `second` - 秒
    /// * `millisecond` - 毫秒
    pub fn from_date(
        year: i32,
        month: u32,
        day: u32,
        hour: u32,
        minute: u32,
        second: u32,
        millisecond: u32,
    ) -> Option<Self> {
        let naive_date = NaiveDate::from_ymd_opt(year, month, day)?;
        let naive_time = naive_date
            .and_hms_milli_opt(hour, minute, second, millisecond)?;
        let dt = Local.from_local_datetime(&naive_time).single()?;
        Some(Self::from_datetime(dt))
    }

    /// 获取毫秒时间戳值
    pub fn value(&self) -> i64 {
        self.ms
    }

    /// 创建盘前时间戳
    /// 
    /// # Arguments
    /// * `year` - 年
    /// * `month` - 月
    /// * `day` - 日
    pub fn pre_market_time(year: i32, month: u32, day: u32) -> Option<Self> {
        Self::from_date(year, month, day, PRE_MARKET_HOUR, PRE_MARKET_MINUTE, PRE_MARKET_SECOND, 0)
    }

    /// 当前时间戳
    pub fn now() -> Self {
        Self::from_datetime(Local::now())
    }

    /// 零值时间戳
    pub fn zero() -> Self {
        Self::new(0)
    }

    /// 解析日期时间字符串
    /// 解析日期时间字符串 - 主要用于日期格式，也支持包含时间
    /// 
    /// 支持多种格式：
    /// - "2022-01-01 15:30:45"
    /// - "2022-01-01"
    /// - "2022/01/01 15:30:45"
    /// - "20220101" 
    /// - ISO 8601 等格式
    pub fn parse(s: &str) -> Result<Self, chrono::ParseError> {
        // 偏向日期格式，但也支持包含时间的格式
        let formats = [
            "%Y-%m-%d %H:%M:%S%.f",    // 完整日期时间+毫秒
            "%Y-%m-%d %H:%M:%S",       // 完整日期时间
            "%Y-%m-%d",                // 仅日期
            "%Y%m%d",                  // 紧凑日期格式
            "%Y/%m/%d %H:%M:%S%.f",    // 斜杠分隔的日期时间+毫秒
            "%Y/%m/%d %H:%M:%S",       // 斜杠分隔的日期时间
            "%Y/%m/%d",                // 斜杠分隔的日期
            "%m/%d/%Y %H:%M:%S",       // 美式日期时间
            "%H:%M:%S %d-%m-%Y",       // 时间在前的格式
            "%Y%m%d %H%M%S",           // 紧凑日期时间
            "%Y-%m-%dT%H:%M:%SZ",      // ISO 8601 UTC
            "%Y-%m-%dT%H:%M:%S%z",     // ISO 8601 with timezone
        ];

        // 尝试日期时间格式
        for format in &formats {
            if let Ok(naive_dt) = NaiveDateTime::parse_from_str(s, format) {
                if let Some(dt) = Local.from_local_datetime(&naive_dt).single() {
                    return Ok(Self::from_datetime(dt));
                }
            }
        }

        // 尝试仅日期格式
        let date_formats = ["%Y-%m-%d", "%Y/%m/%d", "%Y%m%d"];
        for format in &date_formats {
            if let Ok(naive_date) = NaiveDate::parse_from_str(s, format) {
                if let Some(naive_dt) = naive_date.and_hms_opt(0, 0, 0) {
                    if let Some(dt) = Local.from_local_datetime(&naive_dt).single() {
                        return Ok(Self::from_datetime(dt));
                    }
                }
            }
        }

        // 如果所有格式都失败，创建一个错误
        Err(chrono::NaiveDateTime::parse_from_str("invalid", "%Y-%m-%d").unwrap_err())
    }

    /// 解析时间字符串 - 主要用于时间格式，但也兼容完整日期时间
    /// 
    /// 设计目的：用户关注时分秒时使用，但不限制输入格式
    /// 既支持纯时间，也支持包含日期的格式
    /// 
    /// # Arguments
    /// * `s` - 时间字符串，如 "14:30:45" 或 "2022-01-01 14:30:45"
    pub fn parse_time(s: &str) -> Result<Self, chrono::ParseError> {
        // 既支持纯时间，也支持包含日期的格式
        let all_formats = [
            // 纯时间格式
            "%H:%M:%S%.f",             // 时间+毫秒
            "%H:%M:%S",                // 标准时间
            "%H:%M",                   // 时分
            "%H%M%S",                  // 紧凑时间
            "%H%M",                    // 紧凑时分
            // 完整日期时间格式（兼容性）
            "%Y-%m-%d %H:%M:%S%.f",    // 完整日期时间+毫秒
            "%Y-%m-%d %H:%M:%S",       // 完整日期时间
            "%Y-%m-%d",                // 仅日期
            "%Y%m%d",                  // 紧凑日期
            "%Y/%m/%d %H:%M:%S",       // 斜杠日期时间
            "%m/%d/%Y %H:%M:%S",       // 美式日期时间
            "%H:%M:%S %d-%m-%Y",       // 时间在前
            "%Y%m%d %H%M%S",           // 紧凑日期时间
            "%Y-%m-%dT%H:%M:%SZ",      // ISO 8601 UTC
            "%Y-%m-%dT%H:%M:%S%z",     // ISO 8601 with timezone
        ];
        
        // 首先尝试纯时间格式
        let time_only_formats = ["%H:%M:%S%.f", "%H:%M:%S", "%H:%M", "%H%M%S", "%H%M"];
        for format in &time_only_formats {
            if let Ok(time) = chrono::NaiveTime::parse_from_str(s, format) {
                let today = Local::now().date_naive();
                if let Some(dt) = today.and_time(time).and_local_timezone(Local).single() {
                    return Ok(Self::from_datetime(dt));
                }
            }
        }
        
        // 然后尝试完整的日期时间格式
        for format in &all_formats[5..] { // 跳过已经尝试过的时间格式
            if let Ok(naive_dt) = NaiveDateTime::parse_from_str(s, format) {
                if let Some(dt) = Local.from_local_datetime(&naive_dt).single() {
                    return Ok(Self::from_datetime(dt));
                }
            }
        }
        
        // 尝试仅日期格式
        let date_formats = ["%Y-%m-%d", "%Y/%m/%d", "%Y%m%d"];
        for format in &date_formats {
            if let Ok(naive_date) = NaiveDate::parse_from_str(s, format) {
                if let Some(naive_dt) = naive_date.and_hms_opt(0, 0, 0) {
                    if let Some(dt) = Local.from_local_datetime(&naive_dt).single() {
                        return Ok(Self::from_datetime(dt));
                    }
                }
            }
        }
        
        Err(chrono::NaiveTime::parse_from_str("invalid", "%H:%M:%S").unwrap_err())
    }

    /// 获取当天零点的时间戳
    pub fn start_of_day(&self) -> Self {
        let dt = self.to_datetime();
        let start_of_day = dt.date_naive().and_hms_opt(0, 0, 0).unwrap();
        let start_dt = Local.from_local_datetime(&start_of_day).single().unwrap();
        Self::from_datetime(start_dt)
    }

    /// 当前时间的零点整
    pub fn midnight() -> Self {
        Self::now().start_of_day()
    }

    /// 今天的指定时间
    /// 
    /// # Arguments
    /// * `hour` - 时
    /// * `minute` - 分
    /// * `second` - 秒
    /// * `millisecond` - 毫秒
    pub fn today(&self, hour: u32, minute: u32, second: u32, millisecond: u32) -> Option<Self> {
        let dt = self.to_datetime();
        let date = dt.date_naive();
        let new_time = date.and_hms_milli_opt(hour, minute, second, millisecond)?;
        let new_dt = Local.from_local_datetime(&new_time).single()?;
        Some(Self::from_datetime(new_dt))
    }

    /// 当天自0点开始的偏移时间
    /// 
    /// # Arguments
    /// * `hour` - 小时偏移
    /// * `minute` - 分钟偏移
    /// * `second` - 秒偏移
    /// * `millisecond` - 毫秒偏移
    pub fn since(&self, hour: u32, minute: u32, second: u32, millisecond: u32) -> Self {
        let start_of_day = self.start_of_day();
        start_of_day.offset(hour as i32, minute as i32, second as i32, millisecond as i32)
    }

    /// 偏移指定时间
    /// 
    /// # Arguments
    /// * `hour` - 小时偏移
    /// * `minute` - 分钟偏移
    /// * `second` - 秒偏移
    /// * `millisecond` - 毫秒偏移
    pub fn offset(&self, hour: i32, minute: i32, second: i32, millisecond: i32) -> Self {
        let offset_ms = hour as i64 * MILLISECONDS_PER_HOUR
            + minute as i64 * MILLISECONDS_PER_MINUTE
            + second as i64 * MILLISECONDS_PER_SECOND
            + millisecond as i64;
        Self::new(self.ms + offset_ms)
    }

    /// 转换为盘前时间
    pub fn pre_market_time_from_current(&self) -> Option<Self> {
        let _dt = self.to_datetime();
        self.today(PRE_MARKET_HOUR, PRE_MARKET_MINUTE, PRE_MARKET_SECOND, 0)
    }

    /// 调整到分钟的开始（秒和毫秒归零）
    pub fn floor(&self) -> Self {
        let dt = self.to_datetime();
        let floored = dt
            .with_second(0).unwrap()
            .with_nanosecond(0).unwrap();
        Self::from_datetime(floored)
    }

    /// 调整到分钟的结束（59秒999毫秒）
    pub fn ceil(&self) -> Self {
        let dt = self.to_datetime();
        let ceiled = dt
            .with_second(59).unwrap()
            .with_nanosecond(999_000_000).unwrap();
        Self::from_datetime(ceiled)
    }

    /// 提取年月日
    pub fn extract(&self) -> (i32, u32, u32) {
        let dt = self.to_datetime();
        (dt.year(), dt.month(), dt.day())
    }

    /// 格式化为字符串
    /// 
    /// # Arguments
    /// * `layout` - 格式字符串，默认为 "%Y-%m-%d %H:%M:%S"
    pub fn to_string_with_layout(&self, layout: &str) -> String {
        let dt = self.to_datetime();
        dt.format(layout).to_string()
    }

    /// 格式化为字符串（以秒为单位，截断毫秒）
    /// 
    /// # Arguments
    /// * `layout` - 格式字符串，默认为 "%H:%M:%S"
    pub fn to_string_as_time_in_seconds(&self, layout: &str) -> String {
        let dt = self.to_datetime();
        // 截断到秒
        let truncated = dt.with_nanosecond(0).unwrap();
        truncated.format(layout).to_string()
    }

    /// 返回仅日期部分
    pub fn only_date(&self) -> String {
        self.to_string_with_layout("%Y-%m-%d")
    }

    /// 返回缓存日期格式
    pub fn cache_date(&self) -> String {
        self.to_string_with_layout("%Y%m%d")
    }

    /// 返回仅时间部分
    pub fn only_time(&self) -> String {
        self.to_string_with_layout("%H:%M:%S")
    }

    /// 返回YYYYMMDD格式的整数
    pub fn yyyymmdd(&self) -> u32 {
        let dt = self.to_datetime();
        (dt.year() as u32) * 10000 + dt.month() * 100 + dt.day()
    }

    /// 是否为空（零值）
    pub fn is_empty(&self) -> bool {
        self.ms == 0
    }

    /// 检查是否与另一个时间戳在同一天
    /// 
    /// # Arguments
    /// * `other` - 另一个时间戳
    pub fn is_same_date(&self, other: &Self) -> bool {
        let dt1 = self.to_datetime();
        let dt2 = other.to_datetime();
        dt1.date_naive() == dt2.date_naive()
    }

    /// 转换为chrono::DateTime
    pub fn to_datetime(&self) -> DateTime<Local> {
        Local.timestamp_millis_opt(self.ms).single().unwrap_or_else(|| {
            // 如果转换失败，返回epoch时间
            Local.timestamp_millis_opt(0).single().unwrap()
        })
    }

    /// 转换为Unix毫秒时间戳
    pub fn unix_millis(&self) -> i64 {
        self.ms
    }
}

// 实现Display trait，对应C++的operator<<
impl fmt::Display for Timestamp {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.to_string_with_layout("%Y-%m-%d %H:%M:%S%.3f"))
    }
}

// 实现From trait用于类型转换
impl From<i64> for Timestamp {
    fn from(ms: i64) -> Self {
        Self::new(ms)
    }
}

impl From<Timestamp> for i64 {
    fn from(ts: Timestamp) -> Self {
        ts.value()
    }
}

impl From<DateTime<Local>> for Timestamp {
    fn from(dt: DateTime<Local>) -> Self {
        Self::from_datetime(dt)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_timestamp_creation() {
        let ts = Timestamp::new(1640995200000);
        assert_eq!(ts.value(), 1640995200000);
    }

    #[test]
    fn test_timestamp_now() {
        let ts = Timestamp::now();
        assert!(ts.value() > 0);
    }

    #[test]
    fn test_timestamp_zero() {
        let ts = Timestamp::zero();
        assert_eq!(ts.value(), 0);
        assert!(ts.is_empty());
    }

    #[test]
    fn test_timestamp_from_date() {
        let ts = Timestamp::from_date(2022, 1, 1, 0, 0, 0, 0).unwrap();
        let (year, month, day) = ts.extract();
        assert_eq!(year, 2022);
        assert_eq!(month, 1);
        assert_eq!(day, 1);
    }

    #[test]
    fn test_timestamp_parse() {
        let ts = Timestamp::parse("2022-06-15 14:30:45").unwrap();
        let (year, month, day) = ts.extract();
        assert_eq!(year, 2022);
        assert_eq!(month, 6);
        assert_eq!(day, 15);
    }

    #[test]
    fn test_timestamp_parse_comprehensive() {
        // 测试 parse() 函数 - 主要用于日期格式，也支持时间
        
        // 完整日期时间格式
        let ts1 = Timestamp::parse("2022-06-15 14:30:45").unwrap();
        let (year, month, day) = ts1.extract();
        assert_eq!(year, 2022);
        assert_eq!(month, 6);
        assert_eq!(day, 15);
        
        // 仅日期格式
        let ts2 = Timestamp::parse("2022-06-15").unwrap();
        let (year, month, day) = ts2.extract();
        assert_eq!(year, 2022);
        assert_eq!(month, 6);
        assert_eq!(day, 15);
        
        // 紧凑日期格式
        let ts3 = Timestamp::parse("20220615").unwrap();
        let (year, month, day) = ts3.extract();
        assert_eq!(year, 2022);
        assert_eq!(month, 6);
        assert_eq!(day, 15);
        
        // 斜杠分隔格式
        let ts4 = Timestamp::parse("2022/06/15 14:30:45").unwrap();
        let (year, month, day) = ts4.extract();
        assert_eq!(year, 2022);
        assert_eq!(month, 6);
        assert_eq!(day, 15);
    }

    #[test]  
    fn test_timestamp_parse_time_comprehensive() {
        // 测试 parse_time() 函数 - 主要用于时间格式，但也兼容完整日期时间
        
        // 纯时间格式 - 应该配合今天的日期
        let now = chrono::Local::now();
        let today_year = now.year();
        let today_month = now.month();
        let today_day = now.day();
        
        let ts1 = Timestamp::parse_time("14:30:45").unwrap();
        let dt1 = ts1.to_datetime();
        assert_eq!(dt1.hour(), 14);
        assert_eq!(dt1.minute(), 30);
        assert_eq!(dt1.second(), 45);
        // 应该是今天的日期
        assert_eq!(dt1.year(), today_year);
        assert_eq!(dt1.month(), today_month);
        assert_eq!(dt1.day(), today_day);
        
        // 紧凑时间格式
        let ts2 = Timestamp::parse_time("143045").unwrap();
        let dt2 = ts2.to_datetime();
        assert_eq!(dt2.hour(), 14);
        assert_eq!(dt2.minute(), 30);
        assert_eq!(dt2.second(), 45);
        
        // 兼容完整日期时间格式
        let ts3 = Timestamp::parse_time("2022-06-15 14:30:45").unwrap();
        let (year, month, day) = ts3.extract();
        assert_eq!(year, 2022);
        assert_eq!(month, 6);
        assert_eq!(day, 15);
        let dt3 = ts3.to_datetime();
        assert_eq!(dt3.hour(), 14);
        assert_eq!(dt3.minute(), 30);
        assert_eq!(dt3.second(), 45);
        
        // 兼容仅日期格式
        let ts4 = Timestamp::parse_time("2022-06-15").unwrap();
        let (year, month, day) = ts4.extract();
        assert_eq!(year, 2022);
        assert_eq!(month, 6);
        assert_eq!(day, 15);
        
        // 兼容紧凑日期格式
        let ts5 = Timestamp::parse_time("20220615").unwrap();
        let (year, month, day) = ts5.extract();
        assert_eq!(year, 2022);
        assert_eq!(month, 6);
        assert_eq!(day, 15);
    }

    #[test]
    fn test_parse_flexibility_and_tolerance() {
        // 测试解析函数的容错性和灵活性 - 这是这次修改的核心特性
        
        println!("=== 测试 parse() 的灵活性 ===");
        
        // parse() 主要用于日期，但也能处理时间
        let test_cases_parse = vec![
            ("2022-06-15", "仅日期"),
            ("2022-06-15 14:30:45", "完整日期时间"),
            ("20220615", "紧凑日期"),
            ("2022/06/15", "斜杠日期"),
            ("2022/06/15 14:30:45", "斜杠日期时间"),
        ];
        
        for (input, desc) in test_cases_parse {
            match Timestamp::parse(input) {
                Ok(ts) => {
                    println!("✓ parse('{}') [{}] -> {}", input, desc, ts.to_string());
                    let (year, month, day) = ts.extract();
                    assert_eq!(year, 2022);
                    assert_eq!(month, 6); 
                    assert_eq!(day, 15);
                },
                Err(e) => panic!("❌ parse('{}') [{}] failed: {}", input, desc, e),
            }
        }
        
        println!("\n=== 测试 parse_time() 的灵活性 ===");
        
        // parse_time() 主要用于时间，但也能处理完整日期时间
        let test_cases_parse_time = vec![
            ("14:30:45", "纯时间 - 应配合今天"),
            ("143045", "紧凑时间 - 应配合今天"),
            ("14:30", "时分 - 应配合今天"),
            ("2022-06-15 14:30:45", "完整日期时间 - 兼容性"),
            ("2022-06-15", "仅日期 - 兼容性"),
            ("20220615", "紧凑日期 - 兼容性"),
        ];
        
        for (input, desc) in test_cases_parse_time {
            match Timestamp::parse_time(input) {
                Ok(ts) => {
                    println!("✓ parse_time('{}') [{}] -> {}", input, desc, ts.to_string());
                    let dt = ts.to_datetime();
                    
                    if input.starts_with("2022") {
                        // 包含日期的情况
                        let (year, month, day) = ts.extract();
                        assert_eq!(year, 2022);
                        assert_eq!(month, 6);
                        assert_eq!(day, 15);
                    } else {
                        // 纯时间的情况，应该是今天的日期
                        let now = chrono::Local::now();
                        assert_eq!(dt.year(), now.year());
                        assert_eq!(dt.month(), now.month());
                        assert_eq!(dt.day(), now.day());
                    }
                    
                    if input.contains(":") || input.len() == 6 {
                        // 包含时间的情况
                        if input.contains("14") {
                            assert_eq!(dt.hour(), 14);
                        }
                        if input.contains("30") {
                            assert_eq!(dt.minute(), 30);
                        }
                    }
                },
                Err(e) => panic!("❌ parse_time('{}') [{}] failed: {}", input, desc, e),
            }
        }
        
        println!("\n=== 测试用户使用场景 ===");
        
        // 用户场景1: 只关注时分秒，不关注日期
        let time_only = Timestamp::parse_time("14:30:45").unwrap();
        println!("用户场景1 - 只关注时间: parse_time('14:30:45') -> {}", time_only.to_string());
        
        // 用户场景2: 传入完整日期时间给parse_time，不应该失败
        let full_datetime = Timestamp::parse_time("2022-06-15 14:30:45").unwrap();
        println!("用户场景2 - 完整日期时间给parse_time: parse_time('2022-06-15 14:30:45') -> {}", full_datetime.to_string());
        
        // 验证两种场景都成功了
        assert_eq!(time_only.to_datetime().hour(), 14);
        assert_eq!(time_only.to_datetime().minute(), 30);
        assert_eq!(time_only.to_datetime().second(), 45);
        
        assert_eq!(full_datetime.to_datetime().hour(), 14);
        assert_eq!(full_datetime.to_datetime().minute(), 30);
        assert_eq!(full_datetime.to_datetime().second(), 45);
        
        println!("✓ 所有用户场景测试通过！");
    }

    #[test]
    fn test_timestamp_formatting() {
        let ts = Timestamp::from_date(2022, 6, 15, 14, 30, 45, 123).unwrap();
        assert_eq!(ts.only_date(), "2022-06-15");
        assert_eq!(ts.only_time(), "14:30:45");
        assert_eq!(ts.yyyymmdd(), 20220615);
    }

    #[test]
    fn test_timestamp_operations() {
        let ts = Timestamp::from_date(2022, 6, 15, 14, 30, 45, 123).unwrap();
        
        // 测试今天9点
        let nine_am = ts.today(9, 0, 0, 0).unwrap();
        assert_eq!(nine_am.only_time(), "09:00:00");
        
        // 测试偏移2小时
        let offset = ts.offset(2, 0, 0, 0);
        assert_eq!(offset.only_time(), "16:30:45");
    }

    #[test]
    fn test_timestamp_comparison() {
        let ts1 = Timestamp::new(1000);
        let ts2 = Timestamp::new(2000);
        
        assert!(ts1 < ts2);
        assert!(ts2 > ts1);
        assert_eq!(ts1, ts1);
        assert_ne!(ts1, ts2);
    }

    #[test]
    fn test_same_date() {
        let ts1 = Timestamp::from_date(2022, 6, 15, 8, 0, 0, 0).unwrap();
        let ts2 = Timestamp::from_date(2022, 6, 15, 20, 0, 0, 0).unwrap();
        let ts3 = Timestamp::from_date(2022, 6, 16, 8, 0, 0, 0).unwrap();
        
        assert!(ts1.is_same_date(&ts2));
        assert!(!ts1.is_same_date(&ts3));
    }
}