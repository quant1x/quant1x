use crate::exchange;
use chrono::{Datelike, NaiveDate};
use serde::{Deserialize, Serialize};
use std::path::Path;

const URL_TOP10_SHARE_HOLDER: &str = "https://datacenter-web.eastmoney.com/api/data/v1/get";

pub const HOLD_NUM_DAMPENED: i32 = -1; // 减少
pub const HOLD_NUM_UNCHANGED: i32 = 0; // 不变
pub const HOLD_NUM_NEWLY_ADDED: i32 = 1; // 新进/新增
pub const HOLD_NUM_INCREASE: i32 = 2; // 增加
pub const HOLD_NUM_UNKNOWN_CHANGES: i32 = -9; // 未知变化

#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct CirculatingShareholder {
    #[serde(rename = "SecurityCode")]
    pub security_code: String,
    #[serde(rename = "SecurityName")]
    pub security_name: String,
    #[serde(rename = "EndDate")]
    pub end_date: String,
    #[serde(rename = "UpdateDate")]
    pub update_date: String,
    #[serde(rename = "HolderType")]
    pub holder_type: String,
    #[serde(rename = "HolderName")]
    pub holder_name: String,
    #[serde(rename = "IsHoldOrg")]
    pub is_hold_org: String,
    #[serde(rename = "HolderRank")]
    pub holder_rank: i32,
    #[serde(rename = "HoldNum")]
    pub hold_num: i64,
    #[serde(rename = "FreeHoldNumRatio")]
    pub free_hold_num_ratio: f64,
    #[serde(rename = "HoldNumChange")]
    pub hold_num_change: i64,
    #[serde(rename = "HoldChangeName")]
    pub hold_change_name: String,
    #[serde(rename = "HoldChangeState")]
    pub hold_change_state: i32,
    #[serde(rename = "HoldChangeRatio")]
    pub hold_change_ratio: f64,
    #[serde(rename = "HoldRatio")]
    pub hold_ratio: f64,
    #[serde(rename = "HoldRatioChange")]
    pub hold_ratio_change: f64,
}

// Internal structs for JSON parsing
#[derive(Debug, Deserialize)]
struct RawStockHolder {
    success: Option<bool>,
    result: Option<RawResult>,
}

#[derive(Debug, Deserialize)]
struct RawResult {
    count: Option<i32>,
    data: Option<Vec<RawData>>,
}

#[derive(Debug, Deserialize)]
struct RawData {
    #[serde(rename = "SECUCODE")]
    secucode: Option<String>,
    #[serde(rename = "SECURITY_NAME_ABBR")]
    security_name_abbr: Option<String>,
    #[serde(rename = "END_DATE")]
    end_date: Option<String>,
    #[serde(rename = "UPDATE_DATE")]
    update_date: Option<String>,
    #[serde(rename = "HOLDER_NEWTYPE")]
    holder_newtype: Option<String>,
    #[serde(rename = "HOLDER_NAME")]
    holder_name: Option<String>,
    #[serde(rename = "IS_HOLDORG")]
    is_hold_org: Option<String>,
    #[serde(rename = "HOLDER_RANK")]
    holder_rank: Option<i32>,
    #[serde(rename = "HOLD_NUM")]
    hold_num: Option<i64>,
    #[serde(rename = "FREE_HOLDNUM_RATIO")]
    free_holdnum_ratio: Option<f64>,
    #[serde(rename = "XZCHANGE")]
    xzchange: Option<i64>,
    #[serde(rename = "HOLDNUM_CHANGE_NAME")]
    holdnum_change_name: Option<String>,
    #[serde(rename = "CHANGE_RATIO")]
    change_ratio: Option<f64>,
    #[serde(rename = "HOLD_RATIO")]
    hold_ratio: Option<f64>,
    #[serde(rename = "HOLD_RATIO_CHANGE")]
    hold_ratio_change: Option<f64>,
}

fn get_quarter_by_date(date_str: &str, diff: i32) -> (i32, i32, String) {
    let date = NaiveDate::parse_from_str(date_str, "%Y-%m-%d").unwrap_or_else(|_| {
        // Fallback to today if parse fails
        chrono::Local::now().date_naive()
    });

    let mut year = date.year();
    let month = date.month();
    let mut quarter = (month - 1) / 3 + 1;

    // Apply diff (backwards)
    let mut total_quarters = year * 4 + (quarter as i32) - 1;
    total_quarters -= diff;

    year = total_quarters / 4;
    quarter = (total_quarters % 4) as u32 + 1;

    let (end_month, end_day) = match quarter {
        1 => (3, 31),
        2 => (6, 30),
        3 => (9, 30),
        4 => (12, 31),
        _ => (12, 31),
    };

    let end_date = NaiveDate::from_ymd_opt(year, end_month, end_day).unwrap();
    (year, quarter as i32, end_date.format("%Y-%m-%d").to_string())
}

pub fn share_holder(security_code: &str, date: &str, diff: i32) -> Vec<CirculatingShareholder> {
    let mut list = Vec::new();

    let (_market_id, _flag, code) = exchange::detect_market(security_code);
    let (_, _, quarter_end_date) = get_quarter_by_date(date, diff);

    let client = reqwest::blocking::Client::new();
    let filter = format!("(SECURITY_CODE=\"{}\")(END_DATE='{}')", code, quarter_end_date);
    
    let params = [
        ("sortColumns", "HOLDER_RANK"),
        ("sortTypes", "1"),
        ("pageSize", "10"),
        ("pageNumber", "1"),
        ("reportName", "RPT_F10_EH_FREEHOLDERS"),
        ("columns", "ALL"),
        ("source", "WEB"),
        ("client", "WEB"),
        ("filter", &filter),
    ];

    let resp = match client.get(URL_TOP10_SHARE_HOLDER).query(&params).send() {
        Ok(r) => r,
        Err(e) => {
            log::error!("[share-holder] Request failed: {}", e);
            return list;
        }
    };

    if !resp.status().is_success() {
        return list;
    }

    let raw: RawStockHolder = match resp.json() {
        Ok(v) => v,
        Err(e) => {
            log::error!("[share-holder] JSON parse error: {}", e);
            return list;
        }
    };

    if raw.success != Some(true) {
        return list;
    }

    if let Some(result) = raw.result {
        if result.count.unwrap_or(0) == 0 {
            return list;
        }
        if let Some(data) = result.data {
            for v in data {
                let mut shareholder = CirculatingShareholder {
                    security_code: v.secucode.unwrap_or_default(),
                    security_name: v.security_name_abbr.unwrap_or_default(),
                    end_date: crate::Timestamp::parse(&v.end_date.unwrap_or_default())
                        .map(|ts| ts.only_date())
                        .unwrap_or_default(),
                    update_date: crate::Timestamp::parse(&v.update_date.unwrap_or_default())
                        .map(|ts| ts.only_date())
                        .unwrap_or_default(),
                    holder_type: v.holder_newtype.unwrap_or_default(),
                    holder_name: v.holder_name.unwrap_or_default(),
                    is_hold_org: v.is_hold_org.unwrap_or_default(),
                    holder_rank: v.holder_rank.unwrap_or(0),
                    hold_num: v.hold_num.unwrap_or(0),
                    free_hold_num_ratio: v.free_holdnum_ratio.unwrap_or(0.0),
                    hold_num_change: v.xzchange.unwrap_or(0),
                    hold_change_name: v.holdnum_change_name.clone().unwrap_or_default(),
                    hold_change_state: 0,
                    hold_change_ratio: v.change_ratio.unwrap_or(0.0),
                    hold_ratio: v.hold_ratio.unwrap_or(0.0),
                    hold_ratio_change: v.hold_ratio_change.unwrap_or(0.0),
                };

                // Correct security code
                let (_mid, mflag, mcode) = exchange::detect_market(&shareholder.security_code);
                shareholder.security_code = format!("{}{}", mflag, mcode);

                // HoldChangeState
                shareholder.hold_change_state = match shareholder.hold_change_name.as_str() {
                    "新进" => HOLD_NUM_NEWLY_ADDED,
                    "增加" => HOLD_NUM_INCREASE,
                    "减少" => HOLD_NUM_DAMPENED,
                    "不变" => HOLD_NUM_UNCHANGED,
                    _ => {
                        log::warn!(
                            "[share-holder] WARNING: {}: {}, 变化状态未知: {}",
                            shareholder.security_name,
                            shareholder.security_code,
                            shareholder.hold_change_name
                        );
                        HOLD_NUM_UNKNOWN_CHANGES
                    }
                };

                list.push(shareholder);
            }
        }
    }

    // Sort by HolderRank
    list.sort_by(|a, b| a.holder_rank.cmp(&b.holder_rank));

    list
}

fn cache_share_holder(security_code: &str, date: &str, diff: i32) -> Vec<CirculatingShareholder> {
    let (_, _, last) = get_quarter_by_date(date, diff);
    let filename = crate::config::top10_holders_filename(security_code, &last);

    if Path::new(&filename).exists() {
        if let Ok(mut rdr) = csv::Reader::from_path(&filename) {
            let mut list = Vec::new();
            for result in rdr.deserialize() {
                if let Ok(record) = result {
                    list.push(record);
                }
            }
            if !list.is_empty() {
                return list;
            }
        }
    }

    let list = share_holder(security_code, &last, 0); // diff is 0 because last is already the target date
    if !list.is_empty() {
        if let Ok(mut wtr) = csv::Writer::from_path(&filename) {
            for item in &list {
                let _ = wtr.serialize(item);
            }
        }
    }

    list
}

pub fn get_cache_share_holder(security_code: &str, date: &str, mut diff: i32) -> Vec<CirculatingShareholder> {
    let mut list = Vec::new();

    while diff < 4 {
        let tmp_list = cache_share_holder(security_code, date, diff);
        if tmp_list.is_empty() {
            diff += 1;
            continue;
        }
        list = tmp_list;
        break;
    }

    list
}

#[derive(Debug, Default, Clone)]
pub struct ShareHolderSummary {
    pub free_capital: f64,
    pub top10_capital: f64,
    pub top10_change: f64,
    pub change_capital: f64,
    pub increase_ratio: f64,
    pub reduction_ratio: f64,
    pub quarterly_year_quarter: String,
}

pub fn get_share_holder_summary(security_code: &str, date: &str) -> Option<ShareHolderSummary> {
    let list = get_cache_share_holder(security_code, date, 0);
    if list.is_empty() {
        return None;
    }
    
    let mut summary = ShareHolderSummary::default();
    
    for holder in &list {
        summary.top10_capital += holder.hold_num as f64;
        summary.top10_change += holder.hold_num_change as f64;
        
        if holder.hold_change_state == HOLD_NUM_INCREASE || holder.hold_change_state == HOLD_NUM_NEWLY_ADDED {
             summary.change_capital += holder.hold_num_change as f64;
        } else if holder.hold_change_state == HOLD_NUM_DAMPENED {
             summary.change_capital += holder.hold_num_change as f64;
        }
    }
    
    if !list.is_empty() {
        summary.quarterly_year_quarter = list[0].end_date.clone();
    }
    
    Some(summary)
}
