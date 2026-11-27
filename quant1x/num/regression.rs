//! 一元线性回归 —— 均值中心化最小二乘闭式解。

use crate::num::dtype::NAN;

/// 返回窗口内 `y = slope * t + intercept` 的斜率与截距，`t` 取 `0..n-1`。
///
/// 窗口小于 2 或含 NaN 时返回 `(NaN, NaN)`。
pub fn linear_regression(values: &[f64]) -> (f64, f64) {
    let count = values.len();
    if count < 2 || values.iter().any(|value| value.is_nan()) {
        return (NAN, NAN);
    }
    let mean_t = (count - 1) as f64 / 2.0;
    let mut total = 0.0;
    for value in values {
        total += *value;
    }
    let mean_y = total / count as f64;

    let mut numerator = 0.0;
    let mut denominator = 0.0;
    for (index, value) in values.iter().enumerate() {
        let offset = index as f64 - mean_t;
        numerator += offset * (*value - mean_y);
        denominator += offset * offset;
    }
    let slope = numerator / denominator;
    let intercept = mean_y - slope * mean_t;
    (slope, intercept)
}

/// 回归斜率。
pub fn slope(values: &[f64]) -> f64 {
    linear_regression(values).0
}

/// 回归线在最后一个点上的预测值。
pub fn forcast(values: &[f64]) -> f64 {
    let (slope, intercept) = linear_regression(values);
    if slope.is_nan() {
        return NAN;
    }
    intercept + slope * (values.len() as f64 - 1.0)
}
