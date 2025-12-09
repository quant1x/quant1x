use crate::exchange;
use serde::{Deserialize, Serialize};

const URL_EASTMONEY_NOTICES: &str = "https://np-anotice-stock.eastmoney.com/api/security/ann";
const URL_EASTMONEY_WARNING: &str = "https://datacenter.eastmoney.com/securities/api/data/get";
const EASTMONEY_NOTICES_PAGE_SIZE: i32 = 100;

const RISK_KEYWORDS: [&str; 11] = [
    "立案",
    "处罚",
    "冻结",
    "诉讼",
    "质押",
    "仲裁",
    "持股5%以上股东权益变动",
    "信用减值",
    "商誉减值",
    "重大风险",
    "退市风险",
];

#[derive(Debug, Clone, Default)]
pub struct NoticeDetail {
    pub code: String,
    pub name: String,
    pub display_time: String,
    pub notice_date: String,
    pub title: String,
    pub keywords: String,
    pub increase: i32,
    pub reduce: i32,
    pub holder_change: i32,
    pub risk: i32,
}

#[derive(Debug, Clone, Default)]
pub struct CompanyNotice {
    pub increase: i32,
    pub reduce: i32,
    pub risk: i32,
    pub risk_keywords: String,
}

#[derive(Debug, Clone, Copy)]
pub enum EMNoticeType {
    NoticeAll,
    NoticeUnused1,
    NoticeUnused2,
    NoticeUnused3,
    NoticeUnused4,
    NoticeWarning,
    NoticeUnused6,
    NoticeHolderChange,
}

impl EMNoticeType {
    pub fn as_str(&self) -> &'static str {
        match self {
            EMNoticeType::NoticeAll => "全部",
            EMNoticeType::NoticeUnused1 => "财务报告",
            EMNoticeType::NoticeUnused2 => "融资公告",
            EMNoticeType::NoticeUnused3 => "风险提示",
            EMNoticeType::NoticeUnused4 => "信息变更",
            EMNoticeType::NoticeWarning => "重大事项",
            EMNoticeType::NoticeUnused6 => "资产重组",
            EMNoticeType::NoticeHolderChange => "持股变动",
        }
    }
}

// Internal structs for JSON parsing
#[derive(Debug, Deserialize)]
struct RawNoticePackage {
    success: Option<i32>,
    #[serde(default)]
    data: Option<RawNoticeData>,
    #[serde(default)]
    error: Option<String>,
}

#[derive(Debug, Deserialize)]
struct RawNoticeData {
    #[serde(rename = "page_index")]
    page_index: Option<i32>,
    #[serde(rename = "page_size")]
    page_size: Option<i32>,
    #[serde(rename = "total_hits")]
    total_hits: Option<i32>,
    list: Option<Vec<RawNoticeItem>>,
}

#[derive(Debug, Deserialize)]
struct RawNoticeItem {
    #[serde(default)]
    codes: Vec<RawCodeInfo>,
    #[serde(default)]
    columns: Vec<RawColumnInfo>,
    #[serde(rename = "eiTime")]
    ei_time: Option<String>,
    #[serde(rename = "notice_date")]
    notice_date: Option<String>,
    #[serde(rename = "title_ch")]
    title_ch: Option<String>,
}

#[derive(Debug, Deserialize)]
struct RawCodeInfo {
    #[serde(rename = "market_code")]
    market_code: String,
    #[serde(rename = "stock_code")]
    stock_code: String,
    #[serde(rename = "short_name")]
    short_name: String,
}

#[derive(Debug, Deserialize)]
struct RawColumnInfo {
    #[serde(rename = "column_name")]
    column_name: String,
}

#[derive(Debug, Deserialize)]
pub struct RawWarning {
    pub success: Option<bool>,
    #[serde(rename = "hasNext")]
    pub has_next: Option<i32>,
    pub data: Option<Vec<Vec<WarningDetail>>>,
}

#[derive(Debug, Deserialize, Clone)]
pub struct WarningDetail {
    #[serde(rename = "EVENT_TYPE")]
    pub event_type: Option<String>,
    #[serde(rename = "SPECIFIC_EVENTTYPE")]
    pub specific_event_type: Option<String>,
    #[serde(rename = "NOTICE_DATE")]
    pub notice_date: Option<String>,
}

pub fn stock_notices(
    security_code: &str,
    begin_date: &str,
    end_date: &str,
    page_number: i32,
) -> Result<(Vec<NoticeDetail>, i32), String> {
    let fixed_begin_date = crate::Timestamp::parse(begin_date)
        .map(|ts| ts.only_date())
        .unwrap_or_else(|_| begin_date.to_string());
    let fixed_end_date = if end_date.is_empty() {
        crate::Timestamp::now().only_date()
    } else {
        crate::Timestamp::parse(end_date)
            .map(|ts| ts.only_date())
            .unwrap_or_else(|_| end_date.to_string())
    };

    let (market_id, _, code) = exchange::detect_market(security_code);
    let stock_list = format!("{},{}", code, market_id);

    let client = reqwest::blocking::Client::new();
    let params = [
        ("sr", "-1"),
        ("page_size", &EASTMONEY_NOTICES_PAGE_SIZE.to_string()),
        ("page_index", &page_number.to_string()),
        ("ann_type", "A"),
        ("client_source", "web"),
        ("f_node", "0"),
        ("s_node", "0"),
        ("begin_time", &fixed_begin_date),
        ("end_time", &fixed_end_date),
        ("stock_list", &stock_list),
    ];

    let resp = client
        .get(URL_EASTMONEY_NOTICES)
        .query(&params)
        .send()
        .map_err(|e| format!("Request failed: {}", e))?;

    if !resp.status().is_success() {
        return Err(format!("Request failed with status: {}", resp.status()));
    }

    let raw: RawNoticePackage = resp
        .json()
        .map_err(|e| format!("JSON parse error: {}", e))?;

    if raw.success != Some(1) {
        return Err("API returned error".to_string());
    }

    let mut notices = Vec::new();
    let mut pages = 0;

    if let Some(data) = raw.data {
        pages = get_pages(EASTMONEY_NOTICES_PAGE_SIZE, data.total_hits.unwrap_or(0));
        if let Some(list) = data.list {
            for item in list {
                if item.codes.is_empty() || item.columns.is_empty() {
                    continue;
                }

                let code_info = &item.codes[0];
                let market_code_val = code_info.market_code.parse::<u8>().unwrap_or(0);
                let security_code_str =
                    exchange::security_code(market_code_val, &code_info.stock_code);

                let mut notice = NoticeDetail {
                    code: security_code_str,
                    name: code_info.short_name.clone(),
                    display_time: item.ei_time.clone().unwrap_or_default(),
                    notice_date: item.notice_date.clone().unwrap_or_default(),
                    title: item.title_ch.clone().unwrap_or_default(),
                    ..Default::default()
                };

                let mut notice_keywords = Vec::new();
                let mut check_risk = |content: &str| {
                    if content.contains("减持") {
                        notice_keywords.push("减持".to_string());
                        notice.reduce += 1;
                    }
                    if content.contains("增持") {
                        notice_keywords.push("增持".to_string());
                        notice.increase += 1;
                    }
                    if content.contains("控制人变更") {
                        notice_keywords.push("控制人变更".to_string());
                        notice.holder_change += 1;
                    }
                    for keyword in RISK_KEYWORDS.iter() {
                        if content.contains(keyword) {
                            notice_keywords.push(keyword.to_string());
                            notice.risk += 1;
                        }
                    }
                };

                for col in &item.columns {
                    check_risk(&col.column_name);
                }
                check_risk(&notice.title);

                if !notice_keywords.is_empty() {
                    notice.keywords = notice_keywords.join(",");
                }

                notices.push(notice);
            }
        }
    }

    Ok((notices, pages))
}

pub fn stock_warning(security_code: &str, page_number: i32) -> Result<RawWarning, String> {
    let (_market_id, flag, code) = exchange::detect_market(security_code);
    let flag_upper = flag.to_uppercase();
    // C++: std::get<2>(marketInfo) + "." + flag + ",02"
    // detect_market returns (id, flag, code). So it is code + "." + flag + ",02"
    let params_val = format!("{}.{},02", code, flag_upper);

    let client = reqwest::blocking::Client::new();
    let params = [
        ("type", "RTP_F10_DETAIL"),
        ("params", &params_val),
        ("p", &page_number.to_string()),
        ("ann_type", "A"),
        ("source", "HSF10"),
        ("client", "PC"),
    ];

    let resp = client
        .get(URL_EASTMONEY_WARNING)
        .query(&params)
        .send()
        .map_err(|e| format!("Request failed: {}", e))?;

    if !resp.status().is_success() {
        return Err(format!("Request failed with status: {}", resp.status()));
    }

    let warning: RawWarning = resp
        .json()
        .map_err(|e| format!("JSON parse error: {}", e))?;

    if warning.success != Some(true) {
        return Err("API returned error".to_string());
    }

    Ok(warning)
}

fn get_annual_report_date(
    year: &str,
    events: &[WarningDetail],
) -> (Option<String>, Option<String>) {
    let mut annual_report_date = None;
    let mut quarterly_report_date = None;

    for v in events {
        let notice_date = v.notice_date.as_deref().unwrap_or("");
        let date = crate::Timestamp::parse(notice_date)
            .map(|ts| ts.only_date())
            .unwrap_or_default();
        if date.len() < 4 {
            continue;
        }
        let tmp_year = &date[0..4];

        let event_type = v.event_type.as_deref().unwrap_or("");
        if event_type != "报表披露" {
            continue;
        }

        let specific_event_type = v.specific_event_type.as_deref().unwrap_or("");

        if annual_report_date.is_none()
            && (specific_event_type == "年报披露" || specific_event_type == "年报预披露")
            && tmp_year >= year
        {
            annual_report_date = Some(date.clone());
        } else if quarterly_report_date.is_none()
            && (specific_event_type.contains("季报披露")
                || specific_event_type.contains("季报预披露"))
        {
            quarterly_report_date = Some(date.clone());
        }

        if annual_report_date.is_some() && quarterly_report_date.is_some() {
            break;
        }
        if tmp_year < year {
            break;
        }
    }
    (annual_report_date, quarterly_report_date)
}

pub fn notice_date_for_report(code: &str, date: &str) -> (String, String) {
    let fixed_date = crate::Timestamp::parse(date)
        .map(|ts| ts.only_date())
        .unwrap_or_else(|_| date.to_string());
    if fixed_date.len() < 4 {
        return (String::new(), String::new());
    }
    let year = &fixed_date[0..4];
    let mut page_no = 1;
    let mut annual_report_date = String::new();
    let mut quarterly_report_date = String::new();

    loop {
        match stock_warning(code, page_no) {
            Ok(warning) => {
                if let Some(data) = warning.data {
                    for events in data {
                        let (tmp_annual, tmp_quarterly) = get_annual_report_date(year, &events);
                        if annual_report_date.is_empty() {
                            if let Some(d) = tmp_annual {
                                annual_report_date = d;
                            }
                        }
                        if quarterly_report_date.is_empty() {
                            if let Some(d) = tmp_quarterly {
                                quarterly_report_date = d;
                            }
                        }
                        if !annual_report_date.is_empty() && !quarterly_report_date.is_empty() {
                            break;
                        }
                    }
                }

                if !annual_report_date.is_empty() && !quarterly_report_date.is_empty() {
                    break;
                }
                if warning.has_next.unwrap_or(0) > 0 {
                    page_no += 1;
                } else {
                    break;
                }
            }
            Err(_) => break,
        }
    }

    (annual_report_date, quarterly_report_date)
}

pub fn get_one_notice(security_code: &str, current_date: &str) -> CompanyNotice {
    let mut notice = CompanyNotice::default();
    if !exchange::assert_stock_by_security_code(security_code) {
        return notice;
    }

    let timestamp =
        crate::Timestamp::parse(current_date).unwrap_or_else(|_| crate::Timestamp::now());
    // offset -24 * 30 hours? C++: timestamp.offset(-24 * 30)
    // Assuming offset takes hours in C++, but in Rust Timestamp implementation it might differ.
    // Let's assume we need to go back 30 days.
    // If Timestamp doesn't have offset, we might need to use chrono or similar logic.
    // Looking at Timestamp implementation in previous context, it wraps chrono::NaiveDateTime or similar.
    // Let's assume we can calculate it.
    // Since I don't have full Timestamp API, I'll use a safe approximation or try to use what's available.
    // C++: timestamp = timestamp.offset(-24 * 30);
    // Let's assume 30 days ago.

    // Hack: parse to chrono, subtract, format back.
    // Or use crate::exchange::calendar if available.
    // For now, let's try to use a simple calculation if Timestamp exposes it.
    // If not, I'll just use current_date as end_date and maybe a fixed start date?
    // No, the logic requires a window.

    // Let's try to use chrono directly since it is a dependency.
    let dt =
        chrono::NaiveDate::parse_from_str(&timestamp.only_date(), "%Y-%m-%d").unwrap_or_default();
    let begin_date_dt = dt - chrono::Duration::days(30);
    let begin_date = begin_date_dt.format("%Y-%m-%d").to_string();

    let end_date = current_date.to_string();
    let mut pages_count = 1;

    let mut tmp_notice: Option<NoticeDetail> = None;
    let mut page_no = 1;

    loop {
        if page_no > pages_count {
            break;
        }

        match stock_notices(security_code, &begin_date, &end_date, page_no) {
            Ok((list, pages)) => {
                if pages_count < pages {
                    pages_count = pages;
                }
                if list.is_empty() {
                    break;
                }

                for v in &list {
                    if let Some(ref mut t_notice) = tmp_notice {
                        t_notice.name = v.name.clone();
                        if t_notice.notice_date < v.notice_date {
                            t_notice.display_time = v.display_time.clone();
                            t_notice.notice_date = v.notice_date.clone();
                        }
                        t_notice.title = v.title.clone();

                        let mut keywords = t_notice.keywords.clone();
                        if !v.keywords.is_empty() {
                            if keywords.is_empty() {
                                keywords = v.keywords.clone();
                            } else {
                                keywords = format!("{},{}", keywords, v.keywords);
                            }
                        }

                        // Unique keywords
                        let mut tmp_arr: Vec<&str> = keywords.split(',').collect();
                        tmp_arr.sort();
                        tmp_arr.dedup();
                        t_notice.keywords = tmp_arr.join(",");

                        t_notice.increase += v.increase;
                        t_notice.reduce += v.reduce;
                        t_notice.holder_change += v.holder_change;
                        t_notice.risk += v.risk;
                    } else {
                        tmp_notice = Some(v.clone());
                    }
                }

                if list.len() < EASTMONEY_NOTICES_PAGE_SIZE as usize {
                    break;
                }
            }
            Err(_) => break,
        }

        page_no += 1;

        // Safety break to prevent infinite loops if logic is wrong
        if page_no > 100 {
            break;
        }
    }

    if let Some(t_notice) = tmp_notice {
        notice.increase = t_notice.increase;
        notice.reduce = t_notice.reduce;
        notice.risk = t_notice.risk;
        notice.risk_keywords = t_notice.keywords;
    }

    notice
}

fn get_pages(page_size: i32, total_hits: i32) -> i32 {
    (total_hits + page_size - 1) / page_size
}
