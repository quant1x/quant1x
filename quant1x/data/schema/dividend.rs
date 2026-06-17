// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.

use std::collections::HashMap;

// ================= 枚举定义 =================

/// 市场类型
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MarketType {
    AShare,
    HkShare,
    UsShare,
    UkShare,
    SgShare,
    Fund,
    Reits,
    Other,
}

impl MarketType {
    /// 转为中文字符串
    pub fn to_cn_string(&self) -> &'static str {
        match self {
            MarketType::AShare => "A 股",
            MarketType::HkShare => "港股",
            MarketType::UsShare => "美股",
            MarketType::UkShare => "英股",
            MarketType::SgShare => "新加坡",
            MarketType::Fund => "基金",
            MarketType::Reits => "REITs",
            MarketType::Other => "其他",
        }
    }
}

/// 分红类型
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DividendType {
    Cash,
    Special,
    Property,
    None_,
}

impl DividendType {
    /// 转为中文字符串
    pub fn to_cn_string(&self) -> &'static str {
        match self {
            DividendType::Cash => "现金分红",
            DividendType::Special => "特别分红",
            DividendType::Property => "实物分红",
            DividendType::None_ => "无分红",
        }
    }
}

/// 红股类型
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum BonusType {
    BonusIssue,
    StockDividend,
    Capitalization,
    None_,
}

impl BonusType {
    /// 转为中文字符串
    pub fn to_cn_string(&self) -> &'static str {
        match self {
            BonusType::BonusIssue => "红股发行",
            BonusType::StockDividend => "股票分红",
            BonusType::Capitalization => "资本化发行",
            BonusType::None_ => "无红股",
        }
    }
}

/// 公司行为类型
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ActionType {
    Dividend,
    Bonus,
    Split,
    ReverseSplit,
    Consolidation,
    RightsIssue,
    Mixed,
    SpinOff,
}

impl ActionType {
    /// 转为中文字符串
    pub fn to_cn_string(&self) -> &'static str {
        match self {
            ActionType::Dividend => "分红",
            ActionType::Bonus => "送红股",
            ActionType::Split => "拆股",
            ActionType::ReverseSplit => "缩股/合股",
            ActionType::Consolidation => "股份合并",
            ActionType::RightsIssue => "供股/配股",
            ActionType::Mixed => "混合方案",
            ActionType::SpinOff => "分拆上市",
        }
    }
}

// ================= 核心数据模型(扁平化, 含 Bonus)=================

/// 除权除息记录 - 扁平化设计
///
/// 明确区分: 
/// - Dividend(现金分红)
/// - Bonus(送红股)
/// - Split(拆股)
/// - Consolidation(缩股/合并)
///
/// 金额与币种独立存储
#[derive(Debug, Clone)]
pub struct DividendAdjustmentRecord {
    // ===== 基础信息 =====
    /// 股票代码
    pub symbol: String,
    /// 市场类型
    pub market: MarketType,
    /// 行为类型
    pub action_type: ActionType,

    // ===== 日期字段 =====
    /// 公告日期
    pub announcement_date: Option<String>,
    /// 股权登记日
    pub record_date: Option<String>,
    /// 除权除息日(核心)
    pub ex_date: Option<String>,
    /// 派息/到账日
    pub payment_date: Option<String>,

    // ===== Dividend 专用字段(现金分红)=====
    /// 每股现金分红金额
    pub dividend_amount: Option<f64>,
    /// 分红币种 (CNY/HKD/USD)
    pub dividend_currency: Option<String>,
    /// 分红类型
    pub dividend_type: DividendType,

    // ===== Bonus 专用字段(送红股)=====
    /// 红股比例 (如 10 送 3 -> 0.3)
    pub bonus_ratio: Option<f64>,
    /// 红股类型
    pub bonus_type: BonusType,

    // ===== Split 专用字段(拆股)=====
    /// 拆股比例 (1 拆 5 -> 5.0)
    pub split_ratio: Option<f64>,

    // ===== Rights Issue 专用字段(供股/配股)=====
    /// 配股比例
    pub rights_ratio: Option<f64>,
    /// 配股价
    pub rights_price: Option<f64>,
    /// 配股价币种
    pub rights_currency: Option<String>,

    // ===== Consolidation 专用字段(缩股/合并)=====
    /// 缩股比例 (10 合 1 -> 0.1)
    pub consolidation_ratio: Option<f64>,
    /// 合并基数 (10)
    pub consolidation_base: Option<i64>,
    /// 合并目标 (1)
    pub consolidation_target: Option<i64>,

    // ===== 其他字段 =====
    /// 原始方案描述
    pub raw_description: String,
    /// 额外信息
    pub extra_info: HashMap<String, String>,
}

impl DividendAdjustmentRecord {
    /// 创建默认记录
    pub fn new(symbol: String, market: MarketType, action_type: ActionType) -> Self {
        Self {
            symbol,
            market,
            action_type,
            announcement_date: None,
            record_date: None,
            ex_date: None,
            payment_date: None,
            dividend_amount: None,
            dividend_currency: None,
            dividend_type: DividendType::None_,
            bonus_ratio: None,
            bonus_type: BonusType::None_,
            split_ratio: None,
            rights_ratio: None,
            rights_price: None,
            rights_currency: None,
            consolidation_ratio: None,
            consolidation_base: None,
            consolidation_target: None,
            raw_description: String::new(),
            extra_info: HashMap::new(),
        }
    }

    /// 是否有现金分红
    pub fn has_cash_dividend(&self) -> bool {
        self.dividend_amount.is_some() && self.dividend_amount.unwrap() > 0.0
    }

    /// 是否有送红股
    pub fn has_bonus(&self) -> bool {
        self.bonus_ratio.is_some() && self.bonus_ratio.unwrap() > 0.0
    }

    /// 是否有拆股
    pub fn has_split(&self) -> bool {
        self.split_ratio.is_some() && self.split_ratio.unwrap() > 1.0
    }

    /// 是否有缩股/合并
    pub fn has_consolidation(&self) -> bool {
        if let Some(ratio) = self.consolidation_ratio {
            if ratio < 1.0 {
                return true;
            }
        }
        self.consolidation_base.is_some() && self.consolidation_target.is_some()
    }

    /// 是否有供股/配股
    pub fn has_rights_issue(&self) -> bool {
        self.rights_ratio.is_some() && self.rights_ratio.unwrap() > 0.0
    }

    /// 获取缩股因子
    pub fn get_consolidation_factor(&self) -> Option<f64> {
        if let Some(ratio) = self.consolidation_ratio {
            return Some(ratio);
        }
        if let (Some(base), Some(target)) = (self.consolidation_base, self.consolidation_target) {
            if base > 0 {
                return Some(target as f64 / base as f64);
            }
        }
        None
    }

    /// 获取红股因子 (1 + bonus_ratio)
    pub fn get_bonus_factor(&self) -> f64 {
        if self.has_bonus() {
            1.0 + self.bonus_ratio.unwrap()
        } else {
            1.0
        }
    }

    /// 获取拆股因子
    pub fn get_split_factor(&self) -> f64 {
        if self.has_split() {
            self.split_ratio.unwrap()
        } else {
            1.0
        }
    }

    /// 获取除权除息因子(用于复权计算)
    ///
    /// 返回: (price_factor, share_factor, cash_dividend)
    pub fn get_adjustment_factor(&self) -> (f64, f64, f64) {
        let mut price_factor = 1.0_f64;
        let mut share_factor = 1.0_f64;
        let mut cash_dividend = 0.0_f64;

        // 1. 现金分红
        if self.has_cash_dividend() {
            cash_dividend = self.dividend_amount.unwrap();
        }

        // 2. Bonus 红股(股份扩张, 价格下降)
        if self.has_bonus() {
            let bonus_factor = self.get_bonus_factor();
            price_factor /= bonus_factor;
            share_factor *= bonus_factor;
        }

        // 3. Split 拆股(股份扩张, 价格下降)
        if self.has_split() {
            let split_factor = self.get_split_factor();
            price_factor /= split_factor;
            share_factor *= split_factor;
        }

        // 4. Consolidation 缩股/合并(股份收缩, 价格上升)
        if self.has_consolidation() {
            if let Some(cf) = self.get_consolidation_factor() {
                if cf > 0.0 {
                    price_factor /= cf;
                    share_factor *= cf;
                }
            }
        }

        (price_factor, share_factor, cash_dividend)
    }

    /// 获取除权除息描述文本
    pub fn get_adjustment_description(&self) -> String {
        let mut parts: Vec<String> = Vec::new();

        if self.has_cash_dividend() {
            let currency = self.dividend_currency.as_deref().unwrap_or("");
            parts.push(format!(
                "派息{}{}",
                self.dividend_amount.unwrap(),
                currency
            ));
        }

        if self.has_bonus() {
            parts.push(format!(
                "送红股{:.1}股/10 股",
                self.bonus_ratio.unwrap() * 10.0
            ));
        }

        if self.has_split() {
            parts.push(format!("拆股 1 拆{}", self.split_ratio.unwrap()));
        }

        if self.has_consolidation() {
            if let (Some(base), Some(target)) =
                (self.consolidation_base, self.consolidation_target)
            {
                parts.push(format!("合并{}合{}", base, target));
            } else {
                parts.push("缩股".to_string());
            }
        }

        if self.has_rights_issue() {
            parts.push(format!(
                "供股{:.1}股/10 股",
                self.rights_ratio.unwrap() * 10.0
            ));
        }

        if parts.is_empty() {
            "无".to_string()
        } else {
            parts.join(" + ")
        }
    }
}

// ================= 核心处理类 =================

/// 分红除权除息数据处理中心
///
/// 功能: 
/// 1. 统一存储 A 股, 港股, 美股, 英股等多市场数据
/// 2. 明确区分 Dividend(现金), Bonus(红股), Split(拆股)
/// 3. 金额与币种独立存储
/// 4. 支持复权计算
#[derive(Debug, Clone, Default)]
pub struct DividendAdjustment {
    pub records: Vec<DividendAdjustmentRecord>,
}

impl DividendAdjustment {
    /// 添加一条除权除息记录
    pub fn add_record(&mut self, record: DividendAdjustmentRecord) {
        self.records.push(record);
    }

    /// 批量添加记录
    pub fn add_records(&mut self, records: Vec<DividendAdjustmentRecord>) {
        self.records.extend(records);
    }

    /// 获取某标的的所有现金分红记录
    pub fn get_dividend_records(&self, symbol: &str) -> Vec<&DividendAdjustmentRecord> {
        self.records
            .iter()
            .filter(|r| r.symbol == symbol && r.has_cash_dividend())
            .collect()
    }

    /// 获取某标的的所有送红股记录
    pub fn get_bonus_records(&self, symbol: &str) -> Vec<&DividendAdjustmentRecord> {
        self.records
            .iter()
            .filter(|r| r.symbol == symbol && r.has_bonus())
            .collect()
    }

    /// 获取某标的的所有除权除息记录
    pub fn get_all_records(&self, symbol: &str) -> Vec<&DividendAdjustmentRecord> {
        self.records
            .iter()
            .filter(|r| r.symbol == symbol)
            .collect()
    }

    /// 获取某市场的所有记录
    pub fn get_by_market(&self, market: MarketType) -> Vec<&DividendAdjustmentRecord> {
        self.records
            .iter()
            .filter(|r| r.market == market)
            .collect()
    }

    /// 获取某类型的公司行为记录
    pub fn get_by_action_type(&self, action_type: ActionType) -> Vec<&DividendAdjustmentRecord> {
        self.records
            .iter()
            .filter(|r| r.action_type == action_type)
            .collect()
    }

    /// 获取某时间段内的记录
    pub fn get_by_ex_date_range(
        &self,
        start_date: &str,
        end_date: &str,
    ) -> Vec<&DividendAdjustmentRecord> {
        self.records
            .iter()
            .filter(|r| {
                r.ex_date
                    .as_ref()
                    .map_or(false, |d| d.as_str() >= start_date && d.as_str() <= end_date)
            })
            .collect()
    }

    /// 计算除息后的理论价格
    pub fn calculate_ex_dividend_price(
        &self,
        symbol: &str,
        price: f64,
        ex_date: &str,
    ) -> f64 {
        let records = self.get_all_records(symbol);
        let target = records.iter().find(|r| {
            r.ex_date.as_ref().map_or(false, |d| d.as_str() == ex_date)
        });

        match target {
            Some(record) => {
                let (price_factor, _, cash_dividend) = record.get_adjustment_factor();
                let adjusted = (price - cash_dividend)
                    / if price_factor > 0.0 { price_factor } else { 1.0 };
                (adjusted * 100.0).round() / 100.0
            }
            None => price,
        }
    }

    /// 计算除权后的持股数量
    pub fn calculate_adjusted_shares(
        &self,
        symbol: &str,
        shares: i64,
        ex_date: &str,
    ) -> i64 {
        let records = self.get_all_records(symbol);
        let target = records.iter().find(|r| {
            r.ex_date.as_ref().map_or(false, |d| d.as_str() == ex_date)
        });

        match target {
            Some(record) => {
                let (_, share_factor, _) = record.get_adjustment_factor();
                (shares as f64 * share_factor) as i64
            }
            None => shares,
        }
    }

    /// 计算某标的在时间段内的现金分红总收入
    pub fn get_total_dividend_income(
        &self,
        symbol: &str,
        shares: i64,
        start_date: Option<&str>,
        end_date: Option<&str>,
        exchange_rate: f64,
    ) -> f64 {
        let records = self.get_dividend_records(symbol);
        let mut total = 0.0_f64;

        for r in records {
            if let Some(sd) = start_date {
                if r.ex_date.as_ref().map_or(true, |d| d.as_str() < sd) {
                    continue;
                }
            }
            if let Some(ed) = end_date {
                if r.ex_date.as_ref().map_or(true, |d| d.as_str() > ed) {
                    continue;
                }
            }
            if r.has_cash_dividend() {
                total += r.dividend_amount.unwrap() * shares as f64 * exchange_rate;
            }
        }

        (total * 100.0).round() / 100.0
    }

    /// 计算某标的在时间段内的送红股总数
    pub fn get_total_bonus_shares(
        &self,
        symbol: &str,
        shares: i64,
        start_date: Option<&str>,
        end_date: Option<&str>,
    ) -> i64 {
        let records = self.get_bonus_records(symbol);
        let mut total_bonus = 0_i64;

        for r in records {
            if let Some(sd) = start_date {
                if r.ex_date.as_ref().map_or(true, |d| d.as_str() < sd) {
                    continue;
                }
            }
            if let Some(ed) = end_date {
                if r.ex_date.as_ref().map_or(true, |d| d.as_str() > ed) {
                    continue;
                }
            }
            if r.has_bonus() {
                total_bonus += (shares as f64 * r.bonus_ratio.unwrap()) as i64;
            }
        }

        total_bonus
    }
}
