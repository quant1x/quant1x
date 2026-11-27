// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//! DataFrame —— **多列容器的可移植实现**
//!
//! 对齐 `quant1x/pandas/dataframe.py`：Python 用 pandas 承载是 spec，本文件用
//! 「列名 → Series」的有序表复刻同一语义。只做按列名存取，不做计算，数值语义在
//! [`Series`] 与 `crate::num`。
//!
//! 与 Python 的差异：Python 的 `df[name]` / `df[name] = value` / `df[names]` 在这里
//! 分别对应 [`DataFrame::get`] / [`DataFrame::set`] / [`DataFrame::select`]。

use std::fmt;

use super::series::Series;

/// 多列容器：列名按插入序保存
#[derive(Clone, Debug, Default)]
pub struct DataFrame {
    columns: Vec<(String, Series)>,
}

impl DataFrame {
    /// 空表
    pub fn new() -> Self {
        DataFrame {
            columns: Vec::new(),
        }
    }

    /// 由「列名 + 列」构造；所有列必须等长且列名不重复
    pub fn from_columns(columns: Vec<(String, Series)>) -> Result<Self, String> {
        let mut frame = DataFrame::new();
        for (name, values) in columns {
            frame.set(name, values)?;
        }
        Ok(frame)
    }

    /// 列名（插入序）
    pub fn columns(&self) -> Vec<String> {
        self.columns.iter().map(|(name, _)| name.clone()).collect()
    }

    /// 列数
    pub fn column_count(&self) -> usize {
        self.columns.len()
    }

    /// 行数（首列决定，无列为 0）
    pub fn len(&self) -> usize {
        self.columns.first().map_or(0, |(_, values)| values.len())
    }

    /// 是否为空表
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }

    /// `(行数, 列数)`
    pub fn shape(&self) -> (usize, usize) {
        (self.len(), self.column_count())
    }

    /// 是否已有该列
    pub fn contains(&self, name: &str) -> bool {
        self.columns.iter().any(|(column, _)| column == name)
    }

    /// 取列
    pub fn get(&self, name: &str) -> Option<&Series> {
        self.columns
            .iter()
            .find(|(column, _)| column == name)
            .map(|(_, values)| values)
    }

    /// 按给定列序取子集
    pub fn select(&self, names: &[&str]) -> Result<DataFrame, String> {
        let mut frame = DataFrame::new();
        for name in names {
            let values = self
                .get(name)
                .ok_or_else(|| format!("DataFrame: 列不存在 {}", name))?;
            frame.columns.push(((*name).to_string(), values.clone()));
        }
        Ok(frame)
    }

    /// 写列：已存在则原位替换，否则追加；列长必须与现有行数一致
    pub fn set(&mut self, name: impl Into<String>, values: Series) -> Result<(), String> {
        let name = name.into();
        let rows = self.len();
        if !self.columns.is_empty() && values.len() != rows {
            return Err(format!(
                "DataFrame: 列长不一致, 期望 {} 实际 {}",
                rows,
                values.len()
            ));
        }
        match self.columns.iter_mut().find(|(column, _)| *column == name) {
            Some(slot) => slot.1 = values,
            None => self.columns.push((name, values)),
        }
        Ok(())
    }

    /// 深拷贝
    pub fn copy(&self) -> DataFrame {
        DataFrame {
            columns: self.columns.clone(),
        }
    }
}

impl fmt::Display for DataFrame {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        let (rows, columns) = self.shape();
        write!(
            formatter,
            "DataFrame(shape=({}, {}), columns=[",
            rows, columns
        )?;
        for (index, (name, _)) in self.columns.iter().enumerate() {
            if index > 0 {
                write!(formatter, ", ")?;
            }
            write!(formatter, "{}", name)?;
        }
        write!(formatter, "])")
    }
}
