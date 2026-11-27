// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//! Series —— **一列同构数据的可移植容器**
//!
//! 对齐 `quant1x/pandas/series.py`：Python 用 pandas 承载是 spec，本文件用
//! `Vec<f64>` / `Vec<bool>` 复刻同一语义，数值算子全部委托 `crate::num`。
//!
//! 与 Python 的差异只在语法：Python 的运算符（`+` / `>` / `&`）在这里一一对应为
//! 命名方法（[`Series::add`] / [`Series::gt`] / [`Series::and`]），方便 Go 等无运算符
//! 重载的语言照抄同一契约。

use crate::num;

/// 一列数据：浮点或布尔（与 Python Series 的 float64 / bool 两种 dtype 对应）
#[derive(Clone, Debug)]
enum Data {
    Float(Vec<f64>),
    Bool(Vec<bool>),
}

/// 一列同构数据：值 + 名称
#[derive(Clone, Debug)]
pub struct Series {
    data: Data,
    name: Option<String>,
}

impl Series {
    /// 由 float64 序列构造
    pub fn new(values: Vec<f64>) -> Self {
        Series {
            data: Data::Float(values),
            name: None,
        }
    }

    /// 由布尔序列构造
    pub fn from_bools(values: Vec<bool>) -> Self {
        Series {
            data: Data::Bool(values),
            name: None,
        }
    }

    /// 设置列名
    pub fn with_name(mut self, name: impl Into<String>) -> Self {
        self.name = Some(name.into());
        self
    }

    /// 列名
    pub fn name(&self) -> Option<&str> {
        self.name.as_deref()
    }

    /// 元素类型: `float64` 或 `bool`
    pub fn dtype(&self) -> &'static str {
        match self.data {
            Data::Float(_) => "float64",
            Data::Bool(_) => "bool",
        }
    }

    /// 长度
    pub fn len(&self) -> usize {
        match &self.data {
            Data::Float(values) => values.len(),
            Data::Bool(values) => values.len(),
        }
    }

    /// 是否为空
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }

    /// 浮点视图（布尔列返回 None）
    pub fn float_values(&self) -> Option<&[f64]> {
        match &self.data {
            Data::Float(values) => Some(values),
            Data::Bool(_) => None,
        }
    }

    /// 布尔视图（浮点列返回 None）
    pub fn bool_values(&self) -> Option<&[bool]> {
        match &self.data {
            Data::Bool(values) => Some(values),
            Data::Float(_) => None,
        }
    }

    /// 收敛为 float64 序列（布尔转 0/1）
    pub fn to_f64(&self) -> Vec<f64> {
        match &self.data {
            Data::Float(values) => values.clone(),
            Data::Bool(values) => values
                .iter()
                .map(|&value| if value { 1.0 } else { 0.0 })
                .collect(),
        }
    }

    /// 收敛为布尔序列（非零且非 NaN 为真）
    pub fn to_bool(&self) -> Vec<bool> {
        match &self.data {
            Data::Float(values) => values
                .iter()
                .map(|&value| !value.is_nan() && value != 0.0)
                .collect(),
            Data::Bool(values) => values.clone(),
        }
    }

    /// 按位置取值（浮点）
    pub fn get_f64(&self, index: usize) -> Option<f64> {
        match &self.data {
            Data::Float(values) => values.get(index).copied(),
            Data::Bool(values) => values
                .get(index)
                .map(|&value| if value { 1.0 } else { 0.0 }),
        }
    }

    /// 按位置取值（布尔）
    pub fn get_bool(&self, index: usize) -> Option<bool> {
        match &self.data {
            Data::Bool(values) => values.get(index).copied(),
            Data::Float(values) => values
                .get(index)
                .map(|&value| !value.is_nan() && value != 0.0),
        }
    }

    /// 按位置赋值（浮点）
    pub fn set_f64(&mut self, index: usize, value: f64) {
        if let Data::Float(values) = &mut self.data {
            if index < values.len() {
                values[index] = value;
            }
        }
    }

    /// 按位置赋值（布尔）
    pub fn set_bool(&mut self, index: usize, value: bool) {
        if let Data::Bool(values) = &mut self.data {
            if index < values.len() {
                values[index] = value;
            }
        }
    }

    /// 复制
    pub fn copy(&self) -> Series {
        self.clone()
    }

    /// 切片
    pub fn slice(&self, start: usize, end: usize) -> Series {
        match &self.data {
            Data::Float(values) => {
                let end = end.min(values.len());
                let start = start.min(end);
                Series::new(values[start..end].to_vec())
            }
            Data::Bool(values) => {
                let end = end.min(values.len());
                let start = start.min(end);
                Series::from_bools(values[start..end].to_vec())
            }
        }
    }

    /// 移位: 浮点越界补 NaN, 布尔越界补 false
    pub fn shift(&self, periods: i64) -> Series {
        match &self.data {
            Data::Float(values) => Series::new(num::shift(values, periods)),
            Data::Bool(values) => {
                let n = values.len();
                let mut out = vec![false; n];
                let offset = periods.unsigned_abs() as usize;
                if offset < n {
                    if periods > 0 {
                        out[offset..].copy_from_slice(&values[..n - offset]);
                    } else if periods < 0 {
                        out[..n - offset].copy_from_slice(&values[offset..]);
                    }
                } else if periods == 0 {
                    out.copy_from_slice(values);
                }
                Series::from_bools(out)
            }
        }
    }

    /// 差分: 本值减 N 周期前的值
    pub fn diff(&self, periods: i64) -> Series {
        Series::new(num::diff(&self.to_f64(), periods))
    }

    /// 累计和
    pub fn cumsum(&self) -> Series {
        Series::new(num::cumsum(&self.to_f64()))
    }

    /// 填充缺失值
    pub fn fill_na(&self, value: f64) -> Series {
        Series::new(num::fill(&self.to_f64(), value))
    }

    /// 逐元素映射
    pub fn apply(&self, func: impl Fn(f64) -> f64) -> Series {
        let values = self.to_f64();
        Series::new(values.into_iter().map(func).collect())
    }

    // -- 四则（方法即契约） ----------------------------------------------
    fn binary(&self, other: &Series, op: fn(&[f64], &[f64]) -> Vec<f64>) -> Series {
        Series::new(op(&self.to_f64(), &other.to_f64()))
    }

    fn binary_scalar(&self, value: f64, op: fn(&[f64], f64) -> Vec<f64>) -> Series {
        Series::new(op(&self.to_f64(), value))
    }

    /// 逐元素加
    pub fn add(&self, other: &Series) -> Series {
        self.binary(other, num::elementwise::add)
    }

    /// 逐元素减
    pub fn sub(&self, other: &Series) -> Series {
        self.binary(other, num::elementwise::sub)
    }

    /// 逐元素乘
    pub fn mul(&self, other: &Series) -> Series {
        self.binary(other, num::elementwise::mul)
    }

    /// 逐元素除
    pub fn div(&self, other: &Series) -> Series {
        self.binary(other, num::elementwise::div)
    }

    /// 序列与标量加
    pub fn add_scalar(&self, value: f64) -> Series {
        self.binary_scalar(value, num::elementwise::add_scalar)
    }

    /// 序列与标量减
    pub fn sub_scalar(&self, value: f64) -> Series {
        self.binary_scalar(value, num::elementwise::sub_scalar)
    }

    /// 序列与标量乘
    pub fn mul_scalar(&self, value: f64) -> Series {
        self.binary_scalar(value, num::elementwise::mul_scalar)
    }

    /// 序列与标量除
    pub fn div_scalar(&self, value: f64) -> Series {
        self.binary_scalar(value, num::elementwise::div_scalar)
    }

    // -- 比较（返回布尔 Series） ------------------------------------------
    fn compare(&self, other: &Series, op: fn(&[f64], &[f64]) -> Vec<bool>) -> Series {
        Series::from_bools(op(&self.to_f64(), &other.to_f64()))
    }

    /// 大于
    pub fn gt(&self, other: &Series) -> Series {
        self.compare(other, num::logic::gt)
    }

    /// 大于等于
    pub fn gte(&self, other: &Series) -> Series {
        self.compare(other, num::logic::gte)
    }

    /// 小于
    pub fn lt(&self, other: &Series) -> Series {
        self.compare(other, num::logic::lt)
    }

    /// 小于等于
    pub fn lte(&self, other: &Series) -> Series {
        self.compare(other, num::logic::lte)
    }

    /// 等于
    pub fn equal(&self, other: &Series) -> Series {
        self.compare(other, num::logic::equal)
    }

    /// 不等于
    pub fn not_equal(&self, other: &Series) -> Series {
        self.compare(other, num::logic::not_equal)
    }

    /// 大于标量
    pub fn gt_scalar(&self, value: f64) -> Series {
        Series::from_bools(num::logic::gt(&self.to_f64(), &vec![value; self.len()]))
    }

    // -- 逻辑（布尔 Series） ---------------------------------------------
    /// 逻辑与
    pub fn and(&self, other: &Series) -> Series {
        Series::from_bools(num::logic::logic_and(&self.to_bool(), &other.to_bool()))
    }

    /// 逻辑或
    pub fn or(&self, other: &Series) -> Series {
        Series::from_bools(num::logic::logic_or(&self.to_bool(), &other.to_bool()))
    }

    /// 逻辑非
    pub fn not(&self) -> Series {
        Series::from_bools(num::logic::logic_not(&self.to_bool()))
    }

    // -- 归约 -------------------------------------------------------------
    /// 求和
    pub fn sum(&self) -> f64 {
        num::reduce::sum_value(&self.to_f64())
    }

    /// 均值
    pub fn mean(&self) -> f64 {
        num::reduce::mean(&self.to_f64())
    }

    /// 中位数
    pub fn median(&self) -> f64 {
        num::reduce::median(&self.to_f64())
    }

    /// 最小值
    pub fn min(&self) -> f64 {
        num::reduce::min_value(&self.to_f64())
    }

    /// 最大值
    pub fn max(&self) -> f64 {
        num::reduce::max_value(&self.to_f64())
    }

    /// 最小值下标
    pub fn argmin(&self) -> i64 {
        num::reduce::argmin(&self.to_f64()).map(|index| index as i64).unwrap_or(-1)
    }

    /// 最大值下标
    pub fn argmax(&self) -> i64 {
        num::reduce::argmax(&self.to_f64()).map(|index| index as i64).unwrap_or(-1)
    }

    /// 总体方差
    pub fn var(&self) -> f64 {
        num::reduce::variance(&self.to_f64())
    }

    /// 总体标准差
    pub fn std(&self) -> f64 {
        num::reduce::std(&self.to_f64())
    }

    // -- 滚动 / EWM -------------------------------------------------------
    /// 绑定滚动窗口
    pub fn rolling(&self, window: i64) -> Rolling<'_> {
        Rolling {
            series: self,
            window,
        }
    }

    /// 绑定指数加权系数
    pub fn ewm(&self, alpha: f64) -> Ewm<'_> {
        Ewm {
            series: self,
            alpha,
        }
    }

    /// 由周期换算 alpha 后绑定
    pub fn ewm_span(&self, span: i64) -> Ewm<'_> {
        let period = if span <= 0 { 1.0 } else { span as f64 };
        Ewm {
            series: self,
            alpha: 2.0 / (period + 1.0),
        }
    }
}

/// 滚动窗口
pub struct Rolling<'a> {
    series: &'a Series,
    window: i64,
}

impl Rolling<'_> {
    /// 滚动求和
    pub fn sum(&self) -> Series {
        Series::new(num::rolling::rolling_sum(
            &self.series.to_f64(),
            self.window,
        ))
    }

    /// 滚动均值
    pub fn mean(&self) -> Series {
        Series::new(num::rolling::rolling_mean(
            &self.series.to_f64(),
            self.window,
        ))
    }

    /// 滚动总体标准差
    pub fn std(&self) -> Series {
        Series::new(num::rolling::rolling_std(
            &self.series.to_f64(),
            self.window,
        ))
    }

    /// 滚动最小值
    pub fn min(&self) -> Series {
        Series::new(num::rolling::rolling_min(
            &self.series.to_f64(),
            self.window,
        ))
    }

    /// 滚动最大值
    pub fn max(&self) -> Series {
        Series::new(num::rolling::rolling_max(
            &self.series.to_f64(),
            self.window,
        ))
    }

    /// 逐窗口归约
    pub fn apply(&self, func: impl Fn(&[f64]) -> f64) -> Series {
        Series::new(num::rolling::rolling_apply(
            &self.series.to_f64(),
            self.window,
            func,
        ))
    }
}

/// 指数加权窗口
pub struct Ewm<'a> {
    series: &'a Series,
    alpha: f64,
}

impl Ewm<'_> {
    /// 指数加权均值
    pub fn mean(&self) -> Series {
        Series::new(num::ewm::ewm_mean(&self.series.to_f64(), self.alpha))
    }
}
