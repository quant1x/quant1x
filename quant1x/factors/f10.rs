use crate::config;
use crate::data::kline;
use crate::data::market::detect_symbol;
use crate::data::meta::Timestamp;
use crate::data::xdxr;
use crate::factors::financial_report;
use crate::factors::notice;
use crate::factors::safety_score;
use crate::factors::share_holder;
use crate::contrib::data::tdx::level1;
use crate::std::numeric;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct F10 {
    // 日期
    #[serde(rename = "Date")]
    pub date: String,
    // 代码
    #[serde(rename = "Code")]
    pub code: String,
    // 名称
    #[serde(rename = "SecurityName")]
    pub security_name: String,
    // 是否次新股
    #[serde(rename = "SubNew")]
    pub sub_new: bool,
    // 是否两融标的
    #[serde(rename = "MarginTradingTarget")]
    pub margin_trading_target: bool,
    // 每手单位
    #[serde(rename = "VolUnit")]
    pub vol_unit: i32,
    // 小数点
    #[serde(rename = "DecimalPoint")]
    pub decimal_point: i32,
    // 上市日期
    #[serde(rename = "IpoDate")]
    pub ipo_date: String,
    // 更新日期
    #[serde(rename = "UpdateDate")]
    pub update_date: String,
    // 总股本
    #[serde(rename = "TotalCapital")]
    pub total_capital: f64,
    // 流通股本
    #[serde(rename = "Capital")]
    pub capital: f64,
    // 自由流通股本
    #[serde(rename = "FreeCapital")]
    pub free_capital: f64,
    // 前十大流通股东总股本
    #[serde(rename = "Top10Capital")]
    pub top10_capital: f64,
    // 前十大流通股东总股本变化
    #[serde(rename = "Top10Change")]
    pub top10_change: f64,
    // 前十大流通股东持仓变化
    #[serde(rename = "ChangeCapital")]
    pub change_capital: f64,
    // 当期增持比例
    #[serde(rename = "IncreaseRatio")]
    pub increase_ratio: f64,
    // 当期减持比例
    #[serde(rename = "ReductionRatio")]
    pub reduction_ratio: f64,
    // 当前市场处于哪个季报期
    #[serde(rename = "QuarterlyYearQuarter")]
    pub quarterly_year_quarter: String,
    // 最新报告期
    #[serde(rename = "QDate")]
    pub q_date: String,
    // 年报披露日期
    #[serde(rename = "AnnualReportDate")]
    pub annual_report_date: String,
    // 最新季报披露日期
    #[serde(rename = "QuarterlyReportDate")]
    pub quarterly_report_date: String,
    // 当期营业总收入
    #[serde(rename = "TotalOperateIncome")]
    pub total_operate_income: f64,
    // 每股净资产
    #[serde(rename = "BPS")]
    pub bps: f64,
    // 每股收益
    #[serde(rename = "BasicEPS")]
    pub basic_eps: f64,
    // 每股收益(扣除)
    #[serde(rename = "DeductBasicEPS")]
    pub deduct_basic_eps: f64,
    // 通达信安全分
    #[serde(rename = "SafetyScore")]
    pub safety_score: i32,
    // 公告-增持
    #[serde(rename = "Increases")]
    pub increases: i32,
    // 公告-减持
    #[serde(rename = "Reduces")]
    pub reduces: i32,
    // 公告-风险数
    #[serde(rename = "Risk")]
    pub risk: i32,
    // 公告-风险关键词
    #[serde(rename = "RiskKeywords")]
    pub risk_keywords: String,
    // 更新时间
    #[serde(rename = "UpdateTime")]
    pub update_time: String,
    // 样本状态
    #[serde(rename = "State")]
    pub state: u64,
}

impl F10 {
    /// 计算自由换手率
    pub fn turn_z(&self, v: f64) -> f64 {
        let mut free_capital = self.free_capital;
        if free_capital == 0.0 {
            free_capital = self.capital;
        }
        if free_capital.abs() < f64::EPSILON {
            return 0.00;
        }
        let mut turnover_rate_z = numeric::change_rate(free_capital, v);
        turnover_rate_z *= 10000.0;
        numeric::decimal(turnover_rate_z)
    }

    /// 是否财报披露前夕
    pub fn is_reporting_risk_period(&self) -> bool {
        if self.annual_report_date.is_empty() || self.quarterly_report_date.is_empty() {
            return false;
        }
        // TODO: Implement date comparison logic if needed, currently just returning false as placeholder
        // or porting the logic from C++ if dependencies are available.
        // C++ uses exchange::trading_days_between which might need porting.
        false
    }
}

fn get_ipo_date(security_code: &str, _feature_date: &str) -> String {
    // Use load_klines from datasets::kline
    // Note: checkout_klines in C++ might do more, but here we just load from cache
    let filename = config::get_kline_filename(security_code, true);
    let kls = kline::load_klines(&filename);
    if kls.is_empty() {
        return String::new();
    }
    kls[0].date.clone()
}

fn get_finance_info(security_code: &str, feature_date: &str) -> (f64, f64, String, String) {
    let mut capital = 0.0;
    let mut total_capital = 0.0;
    let mut ipo_date = String::new();
    let mut update_date = String::new();
    let base_date = 19901219; // datasets::market_first_date.yyyymmdd()

    // Try to fetch from level1
    if let Ok(mut conn) = crate::contrib::data::tdx::client::get_std_conn() {
        let mut msg = level1::FinanceInfoRequest::new(security_code);

        // Use stream() to get the stream
        if let Ok(_) = crate::contrib::data::tdx::protocol::process_level1_stream(conn.stream(), &mut msg) {
            let info = msg.info;
            // Check if response is valid (assuming non-zero capital means valid)
            if info.liu_tong_gu_ben > 0.0 && info.zong_gu_ben > 0.0 {
                capital = info.liu_tong_gu_ben;
                total_capital = info.zong_gu_ben;
            }

            if info.ipo_date >= base_date {
                ipo_date = Timestamp::from_yyyymmdd_int(info.ipo_date).to_string();
            } else {
                ipo_date = get_ipo_date(security_code, feature_date);
            }

            if info.updated_date >= base_date {
                update_date = Timestamp::from_yyyymmdd_int(info.updated_date).to_string();
            }
        }
    }

    (capital, total_capital, ipo_date, update_date)
}

struct F10SecurityInfo {
    total_capital: f64,
    capital: f64,
    vol_unit: i32,
    decimal_point: i32,
    name: String,
    ipo_date: String,
    sub_new: bool,
    update_date: String,
}

fn checkout_security_basic_info(security_code: &str, feature_date: &str) -> F10SecurityInfo {
    let mut info = F10SecurityInfo {
        total_capital: 0.0,
        capital: 0.0,
        vol_unit: 100,
        decimal_point: 2,
        name: String::new(),
        ipo_date: String::new(),
        sub_new: false,
        update_date: String::new(),
    };

    let mut list = xdxr::load_xdxr(security_code);
    // Sort descending by date
    list.sort_by(|a, b| b.date.cmp(&a.date));

    // Find first capital change <= feature_date
    let xdxr = list
        .iter()
        .find(|v| v.is_capital_change() && feature_date >= v.date.as_str());

    if let Some(v) = xdxr {
        info.total_capital = v.hou_zonggu * 10000.0; // config::TenThousand
        info.capital = v.hou_liutong * 10000.0;
    } else {
        let (cap, total_cap, ipo, update) = get_finance_info(security_code, feature_date);
        info.capital = cap;
        info.total_capital = total_cap;
        info.ipo_date = ipo;
        info.update_date = update;
    }

    if info.ipo_date.is_empty() {
        info.ipo_date = get_ipo_date(security_code, feature_date);
    }

    // Basic info from data::market::detect_symbol (replaces deprecated crate::market)
    let inst = detect_symbol(security_code);
    if inst.can_construct_symbol() {
        info.vol_unit = inst.lot_size;
        info.decimal_point = inst.price_precision;
        info.name = inst.name;
    }

    // SubNew logic
    if !info.ipo_date.is_empty() {
        // TODO: Implement sub-new logic (e.g., IPO within 1 year)
        // For now, just placeholder
    }

    info
}

pub fn get_f10(security_code: &str, feature_date: &str) -> F10 {
    let mut f10 = F10::default();
    f10.code = security_code.to_string();
    f10.date = feature_date.to_string();

    let basic_info = checkout_security_basic_info(security_code, feature_date);
    f10.security_name = basic_info.name;
    f10.vol_unit = basic_info.vol_unit;
    f10.decimal_point = basic_info.decimal_point;
    f10.ipo_date = basic_info.ipo_date;
    f10.sub_new = basic_info.sub_new;
    f10.update_date = basic_info.update_date;
    f10.total_capital = basic_info.total_capital;
    f10.capital = basic_info.capital;

    // Share holder info
    if let Some(holder) = share_holder::get_share_holder_summary(security_code, feature_date) {
        f10.free_capital = holder.free_capital;
        f10.top10_capital = holder.top10_capital;
        f10.top10_change = holder.top10_change;
        f10.change_capital = holder.change_capital;
        f10.increase_ratio = holder.increase_ratio;
        f10.reduction_ratio = holder.reduction_ratio;
        f10.quarterly_year_quarter = holder.quarterly_year_quarter;
    }

    // Financial report
    if let Some(report) =
        financial_report::get_quarterly_report_summary(security_code, feature_date)
    {
        f10.q_date = report.q_date;
        f10.bps = report.bps;
        f10.basic_eps = report.basic_eps;
        f10.total_operate_income = report.total_operate_income;
        f10.deduct_basic_eps = report.deduct_basic_eps;
    }

    // Notice
    let notice = notice::get_one_notice(security_code, feature_date);
    f10.increases = notice.increase;
    f10.reduces = notice.reduce;
    f10.risk = notice.risk;
    f10.risk_keywords = notice.risk_keywords;

    // Safety Score
    let (score, _) = safety_score::get_safety_score(security_code);
    f10.safety_score = score;

    f10
}
