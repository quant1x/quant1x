// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

/// 公司信息块
#[derive(Debug, Clone, Default)]
pub struct CompanyInfoChunk {
    /// 标题
    pub title: String,
    /// 文件名
    pub filename: String,
    /// 偏移量
    pub offset: i64,
    /// 大小
    pub size: i64,
}
