use crate::base::config::QUANT1X_CACHE_CONFIG;
use crate::base::timestamp::*;
use chrono::Local;
use const_format::formatcp;
use serde::Deserialize;
use std::string::ToString;
use std::sync::Mutex;
use std::sync::OnceLock;

// 交易日历缓存
static TRADE_CALENDAR: OnceLock<Mutex<Vec<String>>> = OnceLock::new();

#[allow(non_camel_case_types)]
#[derive(Deserialize)]
#[allow(dead_code)]
struct raw_calender {
    date: String,
    source: String,
}

/// 加载交易日历
fn lazy_load_calendar() -> Vec<String> {
    let path = QUANT1X_CACHE_CONFIG.meta_path("calendar"); // 修改为实际路径
    let mut list = Vec::new();
    let mut reader = csv::Reader::from_path(path).unwrap();
    for row in reader.deserialize() {
        let record: raw_calender = row.unwrap();
        list.push(record.date);
    }
    list
}

fn get_calendar_list() -> Vec<String> {
    let list = TRADE_CALENDAR.get_or_init(|| {
        Mutex::new(lazy_load_calendar())
    }).lock().unwrap();
    list.clone()
}

fn search_calendar_by(list: &Vec<String>, date : &str) -> (usize, String) {
    let s = String::from(date);
    // 二分查找
    let pos = match list.binary_search_by(|c| {
        c.cmp(&s)
    }) {
        Ok(index) | Err(index) => index,
    };
    (pos, list[pos].clone())
}

/// 获取指定的begin和end之间的有效交易日列表
pub fn trade_date_range(begin :&str, end :&str) -> Vec<String> {
    let list = get_calendar_list();
    let (bp, _) = search_calendar_by(&list, begin);
    let (mut ep, ed) = search_calendar_by(&list, end);
    let today = get_today();
    // 范围内最后一个交易日不是当天或者非盘前, ep+1, 返回的交易日是需要包含最后一天的
    if today != ed || !is_session_pre() {
        ep += 1;
    }
    // 防止ep越界
    if ep > list.len() {
        ep = list.len();
    }
    list[bp..ep].to_vec()
}


pub fn get_today() -> String {
    let now = Local::now();
    now.format(FORMAT_ONLY_DATE).to_string()
}

/// 获取最近交易日(字符串格式)
pub fn last_trade_date() -> String {
    let list = get_calendar_list();

    //let today = Local::today().naive_local();
    let today = get_today();
    //println!("Today: {}", today);
    // 二分查找
    let pos = match list.binary_search_by(|date| {
        date.cmp(&today)
    }) {
        Ok(index) | Err(index) => index,
    };

    let selected_date = if list.is_empty() {
        today.clone()
    } else if pos == 0 {
        list[0].clone()
    } else if pos >= list.len() {
        list.last().unwrap().clone()
    } else {
        list[pos].clone()
    };
    //println!("Selected date: {}", selected_date);
    let is_pre = is_session_pre();
    if selected_date > today {
        list[pos-1].clone()
    } else if selected_date == today && is_pre {
        // 盘前判断
        if pos > 0 {
            list[pos-1].clone()
        } else {
            selected_date
        }
    } else {
        selected_date
    }
}

static EXCHANGE_START_TIME:&str= "09:15:00";
static EXCHANGE_END_TIME:&str= "15:00:59";
const TIME_RANGE_FORMAT: &str = formatcp!("{EXCHANGE_START_TIME}-{EXCHANGE_END_TIME}");
static A_SHARE_TRADE_SESSION: OnceLock<TimeRange> = OnceLock::new();

fn trade_session() -> &'static TimeRange {
    A_SHARE_TRADE_SESSION.get_or_init(||  {
        let result = TimeRange::new(TIME_RANGE_FORMAT);
        match result {
            Ok(tr) => tr,
            Err(e) => {
                panic!("{}", e);
            }
        }
})
}

/// 是否盘前(时间判断)
// fn is_session_pre() -> bool {
//     let now = Local::now();
//     let now = now.format("%H:%M:%S").to_string();
//     now.as_str() < "09:15:00"
// }


fn is_session_pre() -> bool {
    let now = Local::now();
    let binding = now.format(FORMAT_ONLY_TIME).to_string();
    let tm = binding.as_str();
    //let tm = "09:15:59";
    //println!("tm is {}", tm);
    let is_pre = trade_session().is_session_pre(Some(tm));
    is_pre.unwrap_or_else(|_| false)

}

// 单元测试
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_last_trade_date() {
        // 模拟交易日历
        let mock_calendar = vec![
            "2023-10-01".to_string(),
            "2023-10-02".to_string(),
            "2023-10-05".to_string(),
        ];

        // 测试正常情况
        TRADE_CALENDAR.get_or_init(|| Mutex::new(mock_calendar.clone()));
        assert_eq!(last_trade_date(), "2023-10-05");

        // 测试盘前情况
        // 需要模拟时间, 这里仅演示逻辑
    }

    #[test]
    fn test_get_last_trade_date() {
        let date = last_trade_date();
        println!("{}", date);
    }

    #[test]
    fn test_search_calendar() {
        let list = get_calendar_list();
        let d = "2025-03-22".to_string();
        let (pos, date) = search_calendar_by(&list, &d);
        println!("pos: {}, date: {}", pos, date);
    }

    #[test]
    fn test_trade_date() {
        let list = trade_date_range("2025-03-08", "2025-03-21");
        println!("{:?}", list);
    }
}