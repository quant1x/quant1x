// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.

/// K线数据结构体
#[derive(Debug, Clone, Default)]
pub struct Bar {
    /// 日期: YYYY-MM-DD, 用于查询和除权
    pub date: String,
    /// 开盘价
    pub open: f64,
    /// 收盘价
    pub close: f64,
    /// 最高价
    pub high: f64,
    /// 最低价
    pub low: f64,
    /// 成交量
    pub volume: f64,
    /// 成交额
    pub amount: f64,
    /// 上涨家数: 仅指数有效
    pub up: i32,
    /// 下跌家数: 仅指数有效
    pub down: i32,
    /// 时间戳: YYYY-MM-DD HH:MM:SS, 为该条数据的收盘时间
    pub timestamp: String,
    /// 复权次数: 0表示未复权, 大于0表示已复权的次数, 该字段用来校验复权
    pub adjustment_count: i32,
}

impl Bar {
    /// K线数据CSV头部
    pub fn headers() -> Vec<&'static str> {
        vec![
            "date", "open", "close", "high", "low", "volume", "amount",
            "up", "down", "timestamp", "adjustment_count",
        ]
    }
}
