use chrono::{Datelike, Local, NaiveDate, NaiveDateTime, NaiveTime, ParseResult, Timelike};

#[allow(non_camel_case_types)]
pub type timestamp = i64;

/// 默认的时间戳格式, 按照十进制表示的年月日时按分秒以及3位毫秒
pub const LAYOUT_TIMESTAMP: &str = "%Y%m%d%H%M%S%3f";
/// 日期格式: YYYYmmdd
pub const LAYOUT_DATE: &str = "%Y%m%d";
/// 日期格式: YYYYmmdd, 长度8
pub const LENGTH_DATE: usize = 8;
/// 日期格式: YYYY-mm-dd
pub const LAYOUT_ONLYDATE: &str = "%Y-%m-%d";
/// 日期格式: YYYY-mm-dd, 长度10
pub const LENGTH_ONLYDATE: usize = 10;
/// 日期时间格式: YYYY-mm-dd HH:MM:SS
pub const LAYOUT_DATETIME: &str = "%Y-%m-%d %H:%M:%S";
pub const LENGTH_DATETIME: usize = 19;
/// 默认的时间戳格式, 按照十进制表示的年月日时按分秒以及3位毫秒
pub const LAYOUT_FULLTIME: &str = "%Y-%m-%d %H:%M:%S%.3f";
pub const LENGTH_FULLTIME: usize = 23;

/// 获得当前的时间戳
pub fn now() -> timestamp {
    v3_now()
}


pub fn v3_now() -> timestamp {
    let local_time = Local::now();
    local_time.timestamp_millis()
}

pub fn v2_now() -> timestamp {
    let local_time = Local::now();
    //let t1 :i64 = 20240712183319929;
    const CY: i64 =             100000000000000000i64;
    let mut ts:i64 = 0i64;
    let mut step:i64 = 10000;
    ts += local_time.year() as i64 * (CY/step);
    //step/= 10000;
    step*=100;
    ts += local_time.month() as i64 *(CY/step);
    step*=100;
    ts += local_time.day() as i64 *(CY/step);
    step*=100;
    ts += local_time.hour() as i64 *(CY/step);
    step*=100;
    ts += local_time.minute() as i64 *(CY/step);
    step*=100;
    ts += local_time.second() as i64 *(CY/step);
    ts += local_time.timestamp_millis() %1000;
    ts
}

pub fn v1_now() -> timestamp {
    let local_time = Local::now();
    let ts = local_time.format(LAYOUT_TIMESTAMP);
    ts.to_string().parse::<timestamp>().unwrap()
}

pub fn format(time : timestamp, fmt: &str) -> String {
    let tmp = datetime_from_string(time.to_string().as_str(), LAYOUT_TIMESTAMP);
    match tmp {
        Ok(datetime) => { datetime.format(fmt).to_string() }
        Err(_) => { time.to_string() }
    }
}

const ZERO_DATE: NaiveDate = NaiveDate::MIN;
const ZERO_TIME: NaiveTime = NaiveTime::MIN;

pub fn local_from_string(date:&str, fmt:&str) -> ParseResult<NaiveDateTime> {
    //Local.datetime_from_str(date, fmt).unwrap()
    NaiveDateTime::parse_from_str(date, fmt)
}

/// 只解析日期
pub fn date_from_string(date: &str, fmt: &str) -> ParseResult<NaiveDateTime> {
    let tmp = NaiveDate::parse_from_str(date, fmt)?;
    let datetime = tmp.and_time(ZERO_TIME);
    Ok(datetime)
}

/// 只解析时间
pub fn time_from_string(time: &str, fmt: &str) -> ParseResult<NaiveDateTime> {
    let tmp = NaiveTime::parse_from_str(time, fmt)?;
    let datetime = ZERO_DATE.clone();
    Ok(datetime.and_time(tmp))
}

/// 只解析日期时间
fn datetime_from_string(time: &str, fmt: &str) -> ParseResult<NaiveDateTime> {
    NaiveDateTime::parse_from_str(time, fmt)
}

pub fn parse(date: &str) -> ParseResult<NaiveDateTime> {
    let length = date.len();
    // match length {
    //     LENGTH_DATE => parse_from_string(date, LAYOUT_DATE),
    //     LENGTH_ONLYDATE => parse_from_string(date, LAYOUT_ONLYDATE),
    //     LENGTH_DATETIME => parse_from_string(date, LAYOUT_DATETIME),
    //     LENGTH_FULLTIME => parse_from_string(date, LAYOUT_FULLTIME),
    //     _ => { date.parse().unwrap() }
    // }
    let result: ParseResult<NaiveDateTime>;
    if length == LENGTH_DATE {
        result = date_from_string(date, LAYOUT_DATE);
    } else if length == LENGTH_ONLYDATE {
        result = date_from_string(date, LAYOUT_ONLYDATE);
    } else if length == LENGTH_DATETIME {
        result = datetime_from_string(date, LAYOUT_DATETIME);
    } else if length == LENGTH_FULLTIME {
        result = datetime_from_string(date, LAYOUT_FULLTIME);
    } else {
        result = datetime_from_string(date, LAYOUT_FULLTIME);
    }
    result
}

pub fn fix_date(date: &String) -> String {
    let t = parse(date);
    match t {
        Ok(datetime) => { datetime.format(LAYOUT_ONLYDATE).to_string() }
        Err(_) => { date.to_string() }
    }
}


#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn timestamp_format() {
        let fmt = LAYOUT_ONLYDATE;
        let t = now();
        println!("{}", t);
        let result = format(t, fmt);
        println!("{}", result)
    }

    #[test]
    fn datetime_fix_date_for_date_1() {
        let target = "2020-01-02";
        let text = "2020-01-02";
        let t1 = fix_date(&text.to_string());
        assert_eq!(target, t1)
    }

    #[test]
    fn datetime_fix_date_for_date_2() {
        let target = "2020-01-02";
        let text = "20200102";
        let t1 = fix_date(&text.to_string());
        assert_eq!(target, t1)
    }

    #[test]
    fn datetime_fix_date_for_date_3() {
        let target = "2020-01-02";
        let text = "2020-01-02 01:02:03";
        let t1 = fix_date(&text.to_string());
        assert_eq!(target, t1)
    }

    #[test]
    fn datetime_fix_date_for_datetime() {
        let target = "2020-01-02";
        let text = "2020-01-02 01:02:03";
        let t1 = fix_date(&text.to_string());
        println!("{}", t1);
        assert_eq!(target, t1)
    }

    #[test]
    fn datetime_date_v3() {
        let text = "2020-01-02";
        let tmp = fix_date(&text.to_string());
        println!("{}", tmp)
    }
}