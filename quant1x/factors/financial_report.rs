use serde::{Deserialize, Serialize};
use std::error::Error;
use std::fs;
use std::path::Path;

use crate::config;

#[derive(Debug, Clone, Serialize, Deserialize, Default)]
pub struct QuarterlyReport {
    #[serde(rename = "SECURITY_CODE")]
    pub security_code: String,
    #[serde(rename = "UPDATE_DATE")]
    pub update_date: Option<String>,
    #[serde(rename = "REPORTDATE")]
    pub report_date: Option<String>,
    #[serde(rename = "NOTICE_DATE")]
    pub notice_date: Option<String>,
    #[serde(rename = "ISNEW")]
    pub is_new: Option<String>,
    #[serde(rename = "ORG_CODE")]
    pub org_code: Option<String>,
    #[serde(rename = "TRADE_MARKET_ZJG")]
    pub trade_market_zjg: Option<String>,
    #[serde(rename = "QDATE")]
    pub q_date: Option<String>,
    #[serde(rename = "DATATYPE")]
    pub data_type: Option<String>,
    #[serde(rename = "DATAYEAR")]
    pub data_year: Option<String>,
    #[serde(rename = "DATEMMDD")]
    pub date_mmdd: Option<String>,
    #[serde(rename = "EITIME")]
    pub eitime: Option<String>,
    #[serde(rename = "SECUCODE")]
    pub secu_code: Option<String>,
    #[serde(rename = "SECURITY_NAME_ABBR")]
    pub security_name_abbr: Option<String>,
    #[serde(rename = "TRADE_MARKET_CODE")]
    pub trade_market_code: Option<String>,
    #[serde(rename = "TRADE_MARKET")]
    pub trade_market: Option<String>,
    #[serde(rename = "SECURITY_TYPE_CODE")]
    pub security_type_code: Option<String>,
    #[serde(rename = "SECURITY_TYPE")]
    pub security_type: Option<String>,
    #[serde(rename = "BASIC_EPS")]
    pub basic_eps: Option<f64>,
    #[serde(rename = "DEDUCT_BASIC_EPS")]
    pub deduct_basic_eps: Option<f64>,
    #[serde(rename = "BPS")]
    pub bps: Option<f64>,
    #[serde(rename = "TOTAL_OPERATE_INCOME")]
    pub total_operate_income: Option<f64>,
    #[serde(rename = "PARENT_NETPROFIT")]
    pub parent_netprofit: Option<f64>,
    #[serde(rename = "WEIGHTAVG_ROE")]
    pub weight_avg_roe: Option<f64>,
    #[serde(rename = "YSTZ")]
    pub ystz: Option<f64>,
    #[serde(rename = "SJLTZ")]
    pub sjltz: Option<f64>,
    #[serde(rename = "MGJYXJJE")]
    pub mgjyxjje: Option<f64>,
    #[serde(rename = "XSMLL")]
    pub xsmll: Option<f64>,
    #[serde(rename = "YSHZ")]
    pub yshz: Option<f64>,
    #[serde(rename = "SJLHZ")]
    pub sjlhz: Option<f64>,
    #[serde(rename = "ASSIGNDSCRPT")]
    pub assign_dscrpt: Option<String>,
    #[serde(rename = "PAYYEAR")]
    pub pay_year: Option<String>,
    #[serde(rename = "PUBLISHNAME")]
    pub publish_name: Option<String>,
    #[serde(rename = "ZXGXL")]
    pub zxgxl: Option<f64>,
    #[serde(rename = "BOARD_NAME")]
    pub board_name: Option<String>,
    #[serde(rename = "ORI_BOARD_CODE")]
    pub ori_board_code: Option<String>,
    #[serde(rename = "BOARD_CODE")]
    pub board_code: Option<String>,
}

#[derive(Debug, Deserialize)]
struct ApiResponse {
    version: Option<String>,
    result: Option<ApiResult>,
    success: Option<bool>,
    message: Option<String>,
    code: Option<i32>,
}

#[derive(Debug, Deserialize)]
struct ApiResult {
    pages: Option<i32>,
    data: Option<Vec<QuarterlyReport>>,
    count: Option<i32>,
}

const URL_QUARTERLY_REPORT_ALL: &str = "https://datacenter-web.eastmoney.com/api/data/v1/get";
const PAGE_SIZE: i32 = 50;

pub fn fetch_quarterly_reports(
    feature_date: &str,
    page_no: i32,
) -> Result<(Vec<QuarterlyReport>, i32), Box<dyn Error>> {
    let filter = format!("(REPORTDATE='{}')", feature_date);
    let params = [
        ("sortColumns", "REPORTDATE"),
        ("sortTypes", "-1"),
        ("pageSize", &PAGE_SIZE.to_string()),
        ("pageNumber", &page_no.to_string()),
        ("columns", "ALL"),
        ("filter", &filter),
        ("reportName", "RPT_LICO_FN_CPD"),
    ];

    let client = reqwest::blocking::Client::new();
    let resp = client.get(URL_QUARTERLY_REPORT_ALL).query(&params).send()?;

    if !resp.status().is_success() {
        return Err(format!("HTTP error: {}", resp.status()).into());
    }

    let text = resp.text()?;
    let api_resp: ApiResponse = serde_json::from_str(&text)?;

    if let Some(result) = api_resp.result {
        let data = result.data.unwrap_or_default();
        let pages = result.pages.unwrap_or(0);
        Ok((data, pages))
    } else {
        Ok((Vec::new(), 0))
    }
}

pub fn fetch_quarterly_reports_by_security_code(
    security_code: &str,
    page_no: i32,
) -> Result<(Vec<QuarterlyReport>, i32), Box<dyn Error>> {
    let filter = format!("(SECURITY_CODE=\"{}\")", security_code);
    let params = [
        ("sortColumns", "REPORTDATE"),
        ("sortTypes", "-1"),
        ("pageSize", &PAGE_SIZE.to_string()),
        ("pageNumber", &page_no.to_string()),
        ("columns", "ALL"),
        ("filter", &filter),
        ("reportName", "RPT_LICO_FN_CPD"),
    ];

    let client = reqwest::blocking::Client::new();
    let resp = client.get(URL_QUARTERLY_REPORT_ALL).query(&params).send()?;

    if !resp.status().is_success() {
        return Err(format!("HTTP error: {}", resp.status()).into());
    }

    let text = resp.text()?;
    let api_resp: ApiResponse = serde_json::from_str(&text)?;

    if let Some(result) = api_resp.result {
        let data = result.data.unwrap_or_default();
        let pages = result.pages.unwrap_or(0);
        Ok((data, pages))
    } else {
        Ok((Vec::new(), 0))
    }
}

pub fn load_quarterly_reports(date: &str) -> Result<(), Box<dyn Error>> {
    let filename = config::reports_filename(date);
    let path = Path::new(&filename);

    if path.exists() {
        // Already cached
        return Ok(());
    }

    // Ensure directory exists
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent)?;
    }

    let mut all_reports = Vec::new();
    let mut page = 1;
    loop {
        let (reports, total_pages) = fetch_quarterly_reports(date, page)?;
        if reports.is_empty() {
            break;
        }
        all_reports.extend(reports);
        if page >= total_pages {
            break;
        }
        page += 1;
    }

    // Save to CSV
    let mut wtr = csv::Writer::from_path(path)?;
    for report in all_reports {
        wtr.serialize(report)?;
    }
    wtr.flush()?;

    Ok(())
}

#[derive(Debug, Clone, Default)]
pub struct QuarterlyReportSummary {
    pub q_date: String,
    pub bps: f64,
    pub basic_eps: f64,
    pub total_operate_income: f64,
    pub deduct_basic_eps: f64,
}

impl QuarterlyReportSummary {
    pub fn from_report(report: &QuarterlyReport) -> Self {
        Self {
            q_date: report.q_date.clone().unwrap_or_default(),
            bps: report.bps.unwrap_or(0.0),
            basic_eps: report.basic_eps.unwrap_or(0.0),
            total_operate_income: report.total_operate_income.unwrap_or(0.0),
            deduct_basic_eps: report.deduct_basic_eps.unwrap_or(0.0),
        }
    }
}

pub fn get_quarterly_report_summary(
    security_code: &str,
    date: &str,
) -> Option<QuarterlyReportSummary> {
    let filename = config::reports_filename(date);
    let path = Path::new(&filename);

    if !path.exists() {
        // Try to load if not exists
        if let Err(e) = load_quarterly_reports(date) {
            log::error!("Failed to load quarterly reports for {}: {}", date, e);
            return None;
        }
    }

    let mut rdr = match csv::Reader::from_path(path) {
        Ok(r) => r,
        Err(e) => {
            log::error!("Failed to read CSV {}: {}", filename, e);
            return None;
        }
    };

    for result in rdr.deserialize() {
        let report: QuarterlyReport = match result {
            Ok(r) => r,
            Err(_) => continue,
        };
        if report.security_code == security_code {
            return Some(QuarterlyReportSummary::from_report(&report));
        }
    }

    None
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_quarterly_report_struct() {
        let json = r#"{
            "SECURITY_CODE": "301381",
            "SECURITY_NAME_ABBR": "赛维时代",
            "TRADE_MARKET_CODE": "069001002002",
            "TRADE_MARKET": "深交所创业板",
            "SECURITY_TYPE_CODE": "058001001",
            "SECURITY_TYPE": "A股",
            "UPDATE_DATE": "2025-04-26 00:00:00",
            "REPORTDATE": "2025-03-31 00:00:00",
            "BASIC_EPS": 0.1175,
            "TOTAL_OPERATE_INCOME": 2458280774.74
        }"#;

        let report: QuarterlyReport = serde_json::from_str(json).unwrap();
        assert_eq!(report.security_code, "301381");
        assert_eq!(report.basic_eps, Some(0.1175));
        assert_eq!(report.total_operate_income, Some(2458280774.74));
    }
}
