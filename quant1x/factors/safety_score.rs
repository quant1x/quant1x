use crate::data::market::detect_symbol;
use once_cell::sync::Lazy;
use serde::Deserialize;
use std::collections::HashMap;
use std::sync::Mutex;

// Constants
const URL_RISK_ASSESSMENT: &str = "http://page3.tdx.com.cn:7615/site/pcwebcall_static/bxb/json/";
const DEFAULT_SAFETY_SCORE: i32 = 100;
const DEFAULT_SAFETY_SCORE_OF_NOT_FOUND: i32 = 100;
const DEFAULT_SAFETY_SCORE_OF_IGNORE: i32 = 0;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum RiskCategoryType {
    Financial,      // 财务类风险
    Market,         // 市场类风险
    Trading,        // 交易类风险
    STAndDelisting, // ST风险和退市
    Unknown,        // 未知类型
}

impl RiskCategoryType {
    pub fn from_str(category_name: &str) -> Self {
        match category_name {
            "财务类风险" => RiskCategoryType::Financial,
            "市场类风险" => RiskCategoryType::Market,
            "交易类风险" => RiskCategoryType::Trading,
            "ST风险和退市" => RiskCategoryType::STAndDelisting,
            _ => RiskCategoryType::Unknown,
        }
    }

    pub fn as_str(&self) -> &'static str {
        match self {
            RiskCategoryType::Financial => "财务类风险",
            RiskCategoryType::Market => "市场类风险",
            RiskCategoryType::Trading => "交易类风险",
            RiskCategoryType::STAndDelisting => "ST风险和退市",
            RiskCategoryType::Unknown => "未知类型",
        }
    }
}

#[derive(Debug, Default, Deserialize, Clone)]
pub struct CommonLxId {
    #[serde(default)]
    pub fs: i32,
    #[serde(default)]
    pub level: i32,
    #[serde(default)]
    pub trig: i32,
    #[serde(default)]
    pub pos: i32,
    #[serde(default)]
    pub id: i32,
    #[serde(default)]
    pub lx: String,
    #[serde(default)]
    pub trigyy: String,
}

#[derive(Debug, Default, Deserialize, Clone)]
pub struct SafetyItem {
    #[serde(default)]
    pub fs: i32,
    #[serde(default)]
    pub trigyy: String,
    #[serde(default)]
    pub trig: i32,
    #[serde(default)]
    pub id: i32,
    #[serde(default)]
    pub lx: String,
    #[serde(default, rename = "commonlxid")]
    pub details: Vec<CommonLxId>,
}

#[derive(Debug, Default, Deserialize, Clone)]
pub struct RiskCategory {
    #[serde(default)]
    pub name: String,
    #[serde(default, rename = "rows")]
    pub rows: Vec<SafetyItem>,
}

#[derive(Debug, Default, Deserialize, Clone)]
pub struct SafetyReport {
    #[serde(default)]
    pub total: i32,
    #[serde(default)]
    pub name: String,
    #[serde(default)]
    pub num: i32,
    #[serde(default, rename = "data")]
    pub data: Vec<RiskCategory>,
}

// Global cache for safety scores
static MAP_SAFETY_SCORE: Lazy<Mutex<HashMap<String, i32>>> =
    Lazy::new(|| Mutex::new(HashMap::new()));

pub fn get_safety_score(security_code: &str) -> (i32, String) {
    let inst = detect_symbol(security_code);
    if !inst.instrument_type.is_stock() {
        return (DEFAULT_SAFETY_SCORE, "".to_string());
    }
    // TODO: Implement exchange::is_need_ignore
    // if exchange::is_need_ignore(security_code) {
    //     return (DEFAULT_SAFETY_SCORE_OF_IGNORE, "".to_string());
    // }

    let pure_code = if security_code.len() > 6 {
        // Try to extract the 6-digit code
        // This is a simplified version of exchange::detect_market
        let len = security_code.len();
        if len >= 6 {
            &security_code[len - 6..]
        } else {
            security_code
        }
    } else {
        security_code
    };

    if pure_code.len() != 6 {
        return (DEFAULT_SAFETY_SCORE, "".to_string());
    }

    let url = format!("{}{}.json", URL_RISK_ASSESSMENT, pure_code);

    let response = reqwest::blocking::get(&url);

    let mut score = DEFAULT_SAFETY_SCORE;
    let mut detail = String::new();

    match response {
        Ok(resp) => {
            if !resp.status().is_success() {
                score = DEFAULT_SAFETY_SCORE_OF_NOT_FOUND;
            } else {
                match resp.json::<SafetyReport>() {
                    Ok(report) => {
                        let mut tmp_score = 100;
                        let mut risk_categories = Vec::new();

                        for data in report.data {
                            let category = data.name;
                            for v in data.rows {
                                let mut details_vec = Vec::new();
                                if v.trig == 1 {
                                    tmp_score -= v.fs;
                                    for common in v.details {
                                        if common.trig == 1 {
                                            details_vec.push(common.trigyy);
                                        }
                                    }
                                }

                                if !details_vec.is_empty() {
                                    let risk_item = format!(
                                        "{}:{}({}):{}",
                                        category,
                                        v.lx,
                                        details_vec.len(),
                                        details_vec.join("|||")
                                    );
                                    risk_categories.push(risk_item);
                                }
                            }
                        }
                        score = tmp_score;
                        if !risk_categories.is_empty() {
                            detail = format!("[{}]", risk_categories.join(";"));
                        }

                        // Update cache
                        if let Ok(mut map) = MAP_SAFETY_SCORE.lock() {
                            map.insert(security_code.to_string(), score);
                        }
                    }
                    Err(e) => {
                        log::error!("[safety-score] JSON parse error: {}", e);
                        // Read from cache
                        if let Ok(map) = MAP_SAFETY_SCORE.lock() {
                            if let Some(&cached_score) = map.get(security_code) {
                                score = cached_score;
                            }
                        }
                    }
                }
            }
        }
        Err(e) => {
            log::error!("[safety-score] Request error: {}", e);
            if let Ok(map) = MAP_SAFETY_SCORE.lock() {
                if let Some(&cached_score) = map.get(security_code) {
                    score = cached_score;
                }
            }
        }
    }

    (score, detail)
}
