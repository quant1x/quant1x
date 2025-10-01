use serde::Deserialize;
use chrono::Local;
use std::sync::*;
use crate::exchange::data_calendar;

const FMT_DATE: &str = "%Y-%m-%d";

#[allow(non_camel_case_types)]
#[derive(Deserialize)]
struct raw_calender {
    date: String,
    source: String,
}

pub fn load() -> Result<(), csv::Error>{
    let mut reader = csv::Reader::from_reader(data_calendar::CALENDAR_DATA.as_bytes());
    for record in reader.deserialize() {
        let record: raw_calender = record?;
        println!(
            "data = In {}, source={}.",
            record.date,
            record.source,
        );
    }
    Ok(())
}


static CALENDER: OnceLock<Vec<String>> = OnceLock::new();
//assert!(CALENDER.get().is_none());

/// 懒加载初始化日历
fn lazy_init_calender() -> Vec<String> {
    let mut list = Vec::new();
    let mut reader = csv::Reader::from_reader(data_calendar::CALENDAR_DATA.as_bytes());
    for row in reader.deserialize() {
        let record: raw_calender = row.unwrap();
        // println!(
        //     "data = In {}, source={}.",
        //     record.date,
        //     record.source,
        // );
        list.push(record.date);
    }
    list
}

fn get_list() -> &'static Vec<String> {
    std::thread::spawn(|| {
        CALENDER.get_or_init(|| { lazy_init_calender()});
    }).join().unwrap();
    CALENDER.get().unwrap()
}

/// 获取当前最后一个交易日
pub fn lastday_by(date : String) -> String {
    let list = get_list();
    let r = list.binary_search(&date.to_string());
    let pos = r.unwrap_or_else(|insertion_point| insertion_point - 1);
    let d = list.get(pos).unwrap().to_string();
    d
}

/// 获取当前最后一个交易日
pub fn lastday() -> String {
    let now = Local::now();
    let today = now.format(FMT_DATE).to_string();
    lastday_by(today)
}
