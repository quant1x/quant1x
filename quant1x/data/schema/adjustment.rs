// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.

use crate::data::meta::{Exchange, Timestamp};

/// 除权除息类型枚举
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum XdxrCategory {
    /// 除权除息
    ExDividend = 1,
    /// 送股上市（无偿）
    BonusSharesListing = 2,
    /// 非流通股上市（受限股解禁）
    RestrictedSharesListing = 3,
    /// 未知股本变动
    UnspecifiedCapitalAdjustment = 4,
    /// 股本变化（保留，但慎用）
    GeneralCapitalAdjustment = 5,
    /// 增发新股
    NewShareIssuance = 6,
    /// 股份回购
    ShareRepurchase = 7,
    /// 增发新股上市
    NewSharesListing = 8,
    /// 转配股上市（中国特有）
    TransferredRightsSharesListing = 9,
    /// 可转债上市
    ConvertibleBondListing = 10,
    /// 拆股或合股
    StockSplitOrReverseSplit = 11,
    /// 非流通股缩股
    RestrictedSharesConsolidation = 12,
    /// 送认购权证
    IssueCallWarrants = 13,
    /// 送认沽权证
    IssuePutWarrants = 14,
}

impl XdxrCategory {
    /// 类型编号转中文字符串
    pub fn to_string(category: i32) -> String {
        match category {
            1 => "除权除息".to_string(),
            2 => "送配股上市".to_string(),
            3 => "非流通股上市".to_string(),
            4 => "未知股本变动".to_string(),
            5 => "股本变化".to_string(),
            6 => "增发新股".to_string(),
            7 => "股份回购".to_string(),
            8 => "增发新股上市".to_string(),
            9 => "转配股上市".to_string(),
            10 => "可转债上市".to_string(),
            11 => "扩缩股".to_string(),
            12 => "非流通股缩股".to_string(),
            13 => "送认购权证".to_string(),
            14 => "送认沽权证".to_string(),
            v => format!("Unknown({})", v),
        }
    }
}

/// 除权除息信息结构体
#[derive(Debug, Clone, Default)]
pub struct XdxrInfo {
    /// 日期 YYYY-MM-DD格式
    pub date: String,
    /// 类型编号
    pub category: i32,
    /// 类型名称
    pub name: String,
    /// 分红(元)
    pub fen_hong: f64,
    /// 分红币种
    pub dividend_currency: String,
    /// 配股价(元)
    pub pei_gu_jia: f64,
    /// 配股价币种
    pub rights_currency: String,
    /// 送转股(股)
    pub song_zhuan_gu: f64,
    /// 配股(股)
    pub pei_gu: f64,
    /// 缩股(股)
    pub suo_gu: f64,
    /// 除权前流通股(万股)
    pub qian_liu_tong: f64,
    /// 除权后流通股(万股)
    pub hou_liu_tong: f64,
    /// 除权前总股本(万股)
    pub qian_zong_gu_ben: f64,
    /// 除权后总股本(万股)
    pub hou_zong_gu_ben: f64,
    /// 权证份数
    pub fen_shu: f64,
    /// 行权价格(元)
    pub xing_quan_jia: f64,
}

impl XdxrInfo {
    /// 是否需要复权
    pub fn is_adjust(&self) -> bool {
        let count = self.fen_hong + self.pei_gu + self.song_zhuan_gu + self.suo_gu + self.fen_shu;
        count > 0.00
    }

    /// 计算除权因子 (m, a)
    pub fn adjust_factor(&self) -> (f64, f64) {
        let a = self.compute_monetary_adjustment();
        let b = self.compute_share_adjustment_ratio();

        if (1.0 + b).abs() > 1e-10 {
            let m = 1.0 / (1.0 + b);
            let aa = a * m;
            (m, aa)
        } else {
            (1.0, 0.0)
        }
    }

    /// 货币调整金额
    pub fn compute_monetary_adjustment(&self) -> f64 {
        (self.pei_gu * self.pei_gu_jia - self.fen_hong + self.fen_shu * self.xing_quan_jia) / 10.0
    }

    /// 股本调整比率
    pub fn compute_share_adjustment_ratio(&self) -> f64 {
        (self.song_zhuan_gu + self.pei_gu - self.suo_gu + self.fen_shu) / 10.0
    }

    /// 是否为股本变动
    pub fn is_capital_change(&self) -> bool {
        if matches!(
            self.category,
            1 | 11 | 12 | 13 | 14
        ) {
            return false;
        }
        self.hou_liu_tong > 0.0 && self.hou_zong_gu_ben > 0.0
    }
}

/// 除权除息条目
#[derive(Debug, Clone)]
pub struct XdxrEntry {
    /// 交易所
    pub exchange: Exchange,
    /// 证券代码
    pub ticker: String,
    /// 记录数
    pub count: i32,
    /// 除权除息记录列表
    pub list: Vec<XdxrInfo>,
}

/// 复权数据结构体
#[derive(Debug, Clone)]
pub struct CumulativeAdjustment {
    /// 复权日期
    pub timestamp: Timestamp,
    /// 乘性因子（Multiplier），处理比例调整（如送股）
    pub m: f64,
    /// 加性因子（Additive），处理平移调整（如分红）
    pub a: f64,
    /// 货币调整，用于价格复权（P' = P * (1 + ratio)）
    pub monetary_adjustment: f64,
    /// 股本调整比率，用于成交量复权（V' = V * (1 + ratio)）
    pub share_adjustment_ratio: f64,
    /// 本次复权调整的序号（从1开始），用于追踪应用顺序
    pub no: i32,
}

impl CumulativeAdjustment {
    pub fn new(
        timestamp: Timestamp,
        m: f64,
        a: f64,
        monetary_adjustment: f64,
        share_adjustment_ratio: f64,
        no: i32,
    ) -> Self {
        Self {
            timestamp,
            m,
            a,
            monetary_adjustment,
            share_adjustment_ratio,
            no,
        }
    }

    /// 格式化输出
    pub fn to_string(&self) -> String {
        format!(
            "{{no={},timestamp={},m={:.6},a={:.6},monetary_adjustment={:.6},share_adjustment_ratio={:.6}}}",
            self.no,
            self.timestamp.only_date(),
            self.m,
            self.a,
            self.monetary_adjustment,
            self.share_adjustment_ratio
        )
    }

    /// 复权
    pub fn apply(&self, price: f64) -> f64 {
        price * self.m + self.a
    }

    /// 还权
    pub fn inverse(&self, adjusted_price: f64) -> f64 {
        (adjusted_price - self.a) / self.m
    }
}
