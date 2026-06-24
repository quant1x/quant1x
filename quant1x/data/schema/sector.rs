// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.

/// 板块信息结构体
#[derive(Debug, Clone, Default)]
pub struct Sector {
    /// 板块名称
    pub name: String,
    /// 板块代码
    pub code: String,
    /// 板块类型
    pub sector_type: i32,
    /// 成分股数量
    pub count: i32,
    /// 板块标识
    pub block: String,
    /// 成分股列表
    pub constituent_stocks: Vec<String>,
}
