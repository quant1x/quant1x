//! 逐元素运算 —— 与 NumPy 同序的加减乘除、取极值与条件选择。
//!
//! NaN 按 IEEE-754 传播：任一操作数为 NaN，结果即 NaN；除零得 `±inf`。
//! 二元算子按**第一个操作数**的长度对齐，第二个短则补 NaN、长则截断。

use crate::num::dtype::NAN;

fn right_at(values: &[f64], index: usize) -> f64 {
    values.get(index).copied().unwrap_or(NAN)
}

/// 绝对值。
pub fn abs(x: &[f64]) -> Vec<f64> {
    x.iter().map(|value| value.abs()).collect()
}

/// 平方根，负数得 NaN。
pub fn sqrt(x: &[f64]) -> Vec<f64> {
    x.iter().map(|value| value.sqrt()).collect()
}

/// 自然对数，非正数得 NaN。
pub fn log(x: &[f64]) -> Vec<f64> {
    x.iter().map(|value| value.ln()).collect()
}

/// 幂。
pub fn pow(x: &[f64], exponent: f64) -> Vec<f64> {
    x.iter().map(|value| value.powf(exponent)).collect()
}

/// 逐元素加。
pub fn add(left: &[f64], right: &[f64]) -> Vec<f64> {
    (0..left.len())
        .map(|i| left[i] + right_at(right, i))
        .collect()
}

/// 逐元素减。
pub fn sub(left: &[f64], right: &[f64]) -> Vec<f64> {
    (0..left.len())
        .map(|i| left[i] - right_at(right, i))
        .collect()
}

/// 逐元素乘。
pub fn mul(left: &[f64], right: &[f64]) -> Vec<f64> {
    (0..left.len())
        .map(|i| left[i] * right_at(right, i))
        .collect()
}

/// 逐元素除，除零得 `±inf` 或 NaN。
pub fn div(left: &[f64], right: &[f64]) -> Vec<f64> {
    (0..left.len())
        .map(|i| left[i] / right_at(right, i))
        .collect()
}

/// 序列与标量加。
pub fn add_scalar(x: &[f64], value: f64) -> Vec<f64> {
    x.iter().map(|item| item + value).collect()
}

/// 序列与标量减。
pub fn sub_scalar(x: &[f64], value: f64) -> Vec<f64> {
    x.iter().map(|item| item - value).collect()
}

/// 序列与标量乘。
pub fn mul_scalar(x: &[f64], value: f64) -> Vec<f64> {
    x.iter().map(|item| item * value).collect()
}

/// 序列与标量除。
pub fn div_scalar(x: &[f64], value: f64) -> Vec<f64> {
    x.iter().map(|item| item / value).collect()
}

/// 逐元素取大，任一侧 NaN 则结果 NaN。
pub fn maximum(left: &[f64], right: &[f64]) -> Vec<f64> {
    (0..left.len())
        .map(|i| {
            let other = right_at(right, i);
            if left[i].is_nan() || other.is_nan() {
                NAN
            } else {
                left[i].max(other)
            }
        })
        .collect()
}

/// 逐元素取小，任一侧 NaN 则结果 NaN。
pub fn minimum(left: &[f64], right: &[f64]) -> Vec<f64> {
    (0..left.len())
        .map(|i| {
            let other = right_at(right, i);
            if left[i].is_nan() || other.is_nan() {
                NAN
            } else {
                left[i].min(other)
            }
        })
        .collect()
}

/// 序列与标量取大。
pub fn maximum_scalar(x: &[f64], value: f64) -> Vec<f64> {
    x.iter()
        .map(|item| if item.is_nan() { NAN } else { item.max(value) })
        .collect()
}

/// 序列与标量取小。
pub fn minimum_scalar(x: &[f64], value: f64) -> Vec<f64> {
    x.iter()
        .map(|item| if item.is_nan() { NAN } else { item.min(value) })
        .collect()
}

/// 条件选择：`condition` 为真取 `true_value`，否则取 `false_value`。
///
/// 三个操作数按最长对齐，缺位为 NaN（条件缺位视为假）。
pub fn where_select(condition: &[bool], true_value: &[f64], false_value: &[f64]) -> Vec<f64> {
    let length = condition.len().max(true_value.len()).max(false_value.len());
    (0..length)
        .map(|i| {
            let flag = condition.get(i).copied().unwrap_or(false);
            let source = if flag { true_value } else { false_value };
            source.get(i).copied().unwrap_or(NAN)
        })
        .collect()
}
