//! 逻辑与比较 —— 非零即真，NaN 比较恒为假。

use crate::num::dtype::NAN;

fn right_at(values: &[f64], index: usize) -> f64 {
    values.get(index).copied().unwrap_or(NAN)
}

/// 大于。
pub fn gt(left: &[f64], right: &[f64]) -> Vec<bool> {
    (0..left.len())
        .map(|i| left[i] > right_at(right, i))
        .collect()
}

/// 大于等于。
pub fn gte(left: &[f64], right: &[f64]) -> Vec<bool> {
    (0..left.len())
        .map(|i| left[i] >= right_at(right, i))
        .collect()
}

/// 小于。
pub fn lt(left: &[f64], right: &[f64]) -> Vec<bool> {
    (0..left.len())
        .map(|i| left[i] < right_at(right, i))
        .collect()
}

/// 小于等于。
pub fn lte(left: &[f64], right: &[f64]) -> Vec<bool> {
    (0..left.len())
        .map(|i| left[i] <= right_at(right, i))
        .collect()
}

/// 相等。
pub fn equal(left: &[f64], right: &[f64]) -> Vec<bool> {
    (0..left.len())
        .map(|i| left[i] == right_at(right, i))
        .collect()
}

/// 不等。
pub fn not_equal(left: &[f64], right: &[f64]) -> Vec<bool> {
    (0..left.len())
        .map(|i| left[i] != right_at(right, i))
        .collect()
}

/// 数值转布尔：非零且非 NaN 为真。
pub fn to_bools(values: &[f64]) -> Vec<bool> {
    values
        .iter()
        .map(|value| !value.is_nan() && *value != 0.0)
        .collect()
}

/// 逐元素逻辑与，不足部分视为假。
pub fn logic_and(left: &[bool], right: &[bool]) -> Vec<bool> {
    let length = left.len().max(right.len());
    (0..length)
        .map(|i| left.get(i).copied().unwrap_or(false) && right.get(i).copied().unwrap_or(false))
        .collect()
}

/// 逐元素逻辑或，不足部分视为假。
pub fn logic_or(left: &[bool], right: &[bool]) -> Vec<bool> {
    let length = left.len().max(right.len());
    (0..length)
        .map(|i| left.get(i).copied().unwrap_or(false) || right.get(i).copied().unwrap_or(false))
        .collect()
}

/// 逐元素逻辑非。
pub fn logic_not(values: &[bool]) -> Vec<bool> {
    values.iter().map(|value| !value).collect()
}

/// 是否全真。
pub fn all_of(values: &[bool]) -> bool {
    values.iter().all(|value| *value)
}

/// 是否存在真值。
pub fn any_of(values: &[bool]) -> bool {
    values.iter().any(|value| *value)
}

/// 统计真值个数。
pub fn count_true(values: &[bool]) -> usize {
    values.iter().filter(|value| **value).count()
}
