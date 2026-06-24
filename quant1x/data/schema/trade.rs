// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.

/// 交易方向
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Direction {
    /// 主动买入
    Buy = 0,
    /// 主动卖出
    Sell = 1,
    /// 中性盘
    Neutral = 2,
}

/// 逐笔交易数据结构体
#[derive(Debug, Clone, Default)]
pub struct Transaction {
    /// 时间
    pub time: String,
    /// 价格
    pub price: f64,
    /// 成交量
    pub volume: i64,
    /// 成交笔数
    pub num: i64,
    /// 成交额
    pub amount: f64,
    /// 交易方向
    pub direction: i32,
}

impl Transaction {
    /// 逐笔交易数据CSV头部
    pub fn headers() -> Vec<&'static str> {
        vec!["time", "price", "volume", "num", "amount", "direction"]
    }

    /// 转为扁平字典
    pub fn to_map(&self) -> std::collections::HashMap<&'static str, String> {
        let mut m = std::collections::HashMap::new();
        m.insert("time", self.time.clone());
        m.insert("price", self.price.to_string());
        m.insert("volume", self.volume.to_string());
        m.insert("num", self.num.to_string());
        m.insert("amount", self.amount.to_string());
        m.insert("direction", self.direction.to_string());
        m
    }
}
