// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//! ndarray —— **对标 numpy.ndarray 的一维可移植容器**
//!
//! 与 `quant1x/num/ndarray.py` 的方法表一致：用 `Vec<f64>` / `Vec<bool>` 复刻同一语义，
//! 算子全部委托 `crate::num`。Go 无运算符重载，故契约以命名方法为准（`add` / `gt` / `and` …）。

use crate::num;

/// 一维数组：浮点或布尔
#[derive(Clone, Debug)]
enum Data {
    Float(Vec<f64>),
    Bool(Vec<bool>),
}

/// 一维数值数组
#[derive(Clone, Debug)]
pub struct NdArray {
    data: Data,
}

impl NdArray {
    /// 由 float64 序列构造
    pub fn new(values: Vec<f64>) -> Self {
        NdArray {
            data: Data::Float(values),
        }
    }

    /// 由布尔序列构造
    pub fn from_bools(values: Vec<bool>) -> Self {
        NdArray {
            data: Data::Bool(values),
        }
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

    /// 收敛为 float64 序列（布尔转 0/1）
    pub fn to_f64(&self) -> Vec<f64> {
        match &self.data {
            Data::Float(values) => values.clone(),
            Data::Bool(values) => values.iter().map(|&v| if v { 1.0 } else { 0.0 }).collect(),
        }
    }

    /// 收敛为布尔序列（非零且非 NaN 为真）
    pub fn to_bool(&self) -> Vec<bool> {
        match &self.data {
            Data::Float(values) => values.iter().map(|&v| !v.is_nan() && v != 0.0).collect(),
            Data::Bool(values) => values.clone(),
        }
    }

    /// 按位置取浮点值
    pub fn get_f64(&self, index: usize) -> Option<f64> {
        match &self.data {
            Data::Float(values) => values.get(index).copied(),
            Data::Bool(values) => values.get(index).map(|&v| if v { 1.0 } else { 0.0 }),
        }
    }

    /// 按位置取布尔值
    pub fn get_bool(&self, index: usize) -> Option<bool> {
        match &self.data {
            Data::Bool(values) => values.get(index).copied(),
            Data::Float(values) => values.get(index).map(|&v| !v.is_nan() && v != 0.0),
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
    pub fn copy(&self) -> NdArray {
        self.clone()
    }

    /// 切片
    pub fn slice(&self, start: usize, end: usize) -> NdArray {
        match &self.data {
            Data::Float(values) => {
                let end = end.min(values.len());
                let start = start.min(end);
                NdArray::new(values[start..end].to_vec())
            }
            Data::Bool(values) => {
                let end = end.min(values.len());
                let start = start.min(end);
                NdArray::from_bools(values[start..end].to_vec())
            }
        }
    }

    /// 类型转换: `float64` / `bool`
    pub fn astype(&self, dtype: &str) -> Result<NdArray, String> {
        match dtype {
            "bool" => Ok(NdArray::from_bools(self.to_bool())),
            "float64" => Ok(NdArray::new(self.to_f64())),
            other => Err(format!("不支持的 dtype: {other}")),
        }
    }

    // -- 构造 -------------------------------------------------------------
    /// 全零
    pub fn zeros(length: usize) -> NdArray {
        NdArray::new(vec![0.0; length])
    }

    /// 全一
    pub fn ones(length: usize) -> NdArray {
        NdArray::new(vec![1.0; length])
    }

    /// 定值填充
    pub fn full(length: usize, value: f64) -> NdArray {
        NdArray::new(vec![value; length])
    }

    /// 等差序列
    pub fn arange(start: f64, stop: f64, step: f64) -> NdArray {
        NdArray::new(num::matrix::arange(start, stop, step))
    }

    /// 等分序列
    pub fn linspace(start: f64, stop: f64, count: usize) -> NdArray {
        NdArray::new(num::matrix::linspace(start, stop, count))
    }

    // -- 四则 -------------------------------------------------------------
    fn binary(&self, other: &NdArray, op: fn(&[f64], &[f64]) -> Vec<f64>) -> NdArray {
        NdArray::new(op(&self.to_f64(), &other.to_f64()))
    }

    fn binary_scalar(&self, value: f64, op: fn(&[f64], f64) -> Vec<f64>) -> NdArray {
        NdArray::new(op(&self.to_f64(), value))
    }

    /// 逐元素加
    pub fn add(&self, other: &NdArray) -> NdArray {
        self.binary(other, num::elementwise::add)
    }
    /// 逐元素减
    pub fn sub(&self, other: &NdArray) -> NdArray {
        self.binary(other, num::elementwise::sub)
    }
    /// 逐元素乘
    pub fn mul(&self, other: &NdArray) -> NdArray {
        self.binary(other, num::elementwise::mul)
    }
    /// 逐元素除
    pub fn div(&self, other: &NdArray) -> NdArray {
        self.binary(other, num::elementwise::div)
    }
    /// 数组与标量加
    pub fn add_scalar(&self, value: f64) -> NdArray {
        self.binary_scalar(value, num::elementwise::add_scalar)
    }
    /// 数组与标量减
    pub fn sub_scalar(&self, value: f64) -> NdArray {
        self.binary_scalar(value, num::elementwise::sub_scalar)
    }
    /// 数组与标量乘
    pub fn mul_scalar(&self, value: f64) -> NdArray {
        self.binary_scalar(value, num::elementwise::mul_scalar)
    }
    /// 数组与标量除
    pub fn div_scalar(&self, value: f64) -> NdArray {
        self.binary_scalar(value, num::elementwise::div_scalar)
    }

    /// 逐元素取大
    pub fn maximum(&self, other: &NdArray) -> NdArray {
        self.binary(other, num::elementwise::maximum)
    }
    /// 逐元素取小
    pub fn minimum(&self, other: &NdArray) -> NdArray {
        self.binary(other, num::elementwise::minimum)
    }

    // -- 一元 -------------------------------------------------------------
    /// 绝对值
    pub fn abs(&self) -> NdArray {
        NdArray::new(num::elementwise::abs(&self.to_f64()))
    }
    /// 平方根
    pub fn sqrt(&self) -> NdArray {
        NdArray::new(num::elementwise::sqrt(&self.to_f64()))
    }
    /// 自然对数
    pub fn log(&self) -> NdArray {
        NdArray::new(num::elementwise::log(&self.to_f64()))
    }
    /// 幂
    pub fn pow(&self, exponent: f64) -> NdArray {
        NdArray::new(num::elementwise::pow(&self.to_f64(), exponent))
    }

    // -- 比较 -------------------------------------------------------------
    fn compare(&self, other: &NdArray, op: fn(&[f64], &[f64]) -> Vec<bool>) -> NdArray {
        NdArray::from_bools(op(&self.to_f64(), &other.to_f64()))
    }

    fn compare_scalar(&self, value: f64, op: fn(&[f64], &[f64]) -> Vec<bool>) -> NdArray {
        let left = self.to_f64();
        let right = vec![value; left.len()];
        NdArray::from_bools(op(&left, &right))
    }

    /// 大于
    pub fn gt(&self, other: &NdArray) -> NdArray {
        self.compare(other, num::logic::gt)
    }
    /// 大于等于
    pub fn gte(&self, other: &NdArray) -> NdArray {
        self.compare(other, num::logic::gte)
    }
    /// 小于
    pub fn lt(&self, other: &NdArray) -> NdArray {
        self.compare(other, num::logic::lt)
    }
    /// 小于等于
    pub fn lte(&self, other: &NdArray) -> NdArray {
        self.compare(other, num::logic::lte)
    }
    /// 等于
    pub fn equal(&self, other: &NdArray) -> NdArray {
        self.compare(other, num::logic::equal)
    }
    /// 不等于
    pub fn not_equal(&self, other: &NdArray) -> NdArray {
        self.compare(other, num::logic::not_equal)
    }
    /// 大于标量
    pub fn gt_scalar(&self, value: f64) -> NdArray {
        self.compare_scalar(value, num::logic::gt)
    }
    /// 小于标量
    pub fn lt_scalar(&self, value: f64) -> NdArray {
        self.compare_scalar(value, num::logic::lt)
    }
    /// 等于标量
    pub fn equal_scalar(&self, value: f64) -> NdArray {
        self.compare_scalar(value, num::logic::equal)
    }

    // -- 逻辑 -------------------------------------------------------------
    /// 逻辑与
    pub fn logic_and(&self, other: &NdArray) -> NdArray {
        NdArray::from_bools(num::logic::logic_and(&self.to_bool(), &other.to_bool()))
    }
    /// 逻辑或
    pub fn logic_or(&self, other: &NdArray) -> NdArray {
        NdArray::from_bools(num::logic::logic_or(&self.to_bool(), &other.to_bool()))
    }
    /// 逻辑非
    pub fn logic_not(&self) -> NdArray {
        NdArray::from_bools(num::logic::logic_not(&self.to_bool()))
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
        num::reduce::argmin(&self.to_f64())
            .map(|i| i as i64)
            .unwrap_or(-1)
    }
    /// 最大值下标
    pub fn argmax(&self) -> i64 {
        num::reduce::argmax(&self.to_f64())
            .map(|i| i as i64)
            .unwrap_or(-1)
    }
    /// 总体方差
    pub fn var(&self) -> f64 {
        num::reduce::variance(&self.to_f64())
    }
    /// 总体标准差
    pub fn std(&self) -> f64 {
        num::reduce::std(&self.to_f64())
    }
    /// 累计和
    pub fn cumsum(&self) -> NdArray {
        NdArray::new(num::cumsum(&self.to_f64()))
    }

    // -- 移位 / 差分 / 填充 ----------------------------------------------
    /// 移位: 越界补 NaN
    pub fn shift(&self, periods: i64) -> NdArray {
        NdArray::new(num::shift(&self.to_f64(), periods))
    }
    /// 差分
    pub fn diff(&self, periods: i64) -> NdArray {
        NdArray::new(num::diff(&self.to_f64(), periods))
    }
    /// 填充缺失值
    pub fn fill_na(&self, value: f64) -> NdArray {
        NdArray::new(num::fill(&self.to_f64(), value))
    }

    // -- 滚动 / EWM -------------------------------------------------------
    /// 滚动求和
    pub fn rolling_sum(&self, window: i64) -> NdArray {
        NdArray::new(num::rolling::rolling_sum(&self.to_f64(), window))
    }
    /// 滚动均值
    pub fn rolling_mean(&self, window: i64) -> NdArray {
        NdArray::new(num::rolling::rolling_mean(&self.to_f64(), window))
    }
    /// 滚动总体标准差
    pub fn rolling_std(&self, window: i64) -> NdArray {
        NdArray::new(num::rolling::rolling_std(&self.to_f64(), window))
    }
    /// 滚动最小值
    pub fn rolling_min(&self, window: i64) -> NdArray {
        NdArray::new(num::rolling::rolling_min(&self.to_f64(), window))
    }
    /// 滚动最大值
    pub fn rolling_max(&self, window: i64) -> NdArray {
        NdArray::new(num::rolling::rolling_max(&self.to_f64(), window))
    }
    /// 滚动平均绝对偏差
    pub fn rolling_avedev(&self, window: i64) -> NdArray {
        NdArray::new(num::rolling::rolling_avedev(&self.to_f64(), window))
    }
    /// 逐窗口归约
    pub fn rolling_apply(&self, window: i64, func: impl Fn(&[f64]) -> f64) -> NdArray {
        NdArray::new(num::rolling::rolling_apply(&self.to_f64(), window, func))
    }
    /// 指数加权均值
    pub fn ewm_mean(&self, alpha: f64) -> NdArray {
        NdArray::new(num::ewm::ewm_mean(&self.to_f64(), alpha))
    }
}
