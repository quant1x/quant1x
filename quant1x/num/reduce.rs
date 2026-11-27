//! 归约 —— 顺序累加、总体方差、NaN 传播。
//!
//! 归约顺序是语义的一部分：一律从左到右顺序累加，不做分块求和。

use crate::num::dtype::NAN;

/// 求和；空输入为 0。
pub fn sum_value(values: &[f64]) -> f64 {
    let mut total = 0.0;
    for value in values {
        total += *value;
    }
    total
}

/// 均值；空输入为 NaN。
pub fn mean(values: &[f64]) -> f64 {
    if values.is_empty() {
        return NAN;
    }
    sum_value(values) / values.len() as f64
}

/// 中位数；空输入或含 NaN 为 NaN。
pub fn median(values: &[f64]) -> f64 {
    if values.is_empty() || values.iter().any(|value| value.is_nan()) {
        return NAN;
    }
    let mut ordered = values.to_vec();
    ordered.sort_by(|left, right| left.partial_cmp(right).unwrap_or(std::cmp::Ordering::Equal));
    let middle = ordered.len() / 2;
    if ordered.len() % 2 == 0 {
        (ordered[middle - 1] + ordered[middle]) / 2.0
    } else {
        ordered[middle]
    }
}

/// 最小值；空输入或含 NaN 为 NaN。
pub fn min_value(values: &[f64]) -> f64 {
    if values.is_empty() || values.iter().any(|value| value.is_nan()) {
        return NAN;
    }
    values.iter().copied().fold(f64::INFINITY, f64::min)
}

/// 最大值；空输入或含 NaN 为 NaN。
pub fn max_value(values: &[f64]) -> f64 {
    if values.is_empty() || values.iter().any(|value| value.is_nan()) {
        return NAN;
    }
    values.iter().copied().fold(f64::NEG_INFINITY, f64::max)
}

/// 最小值下标；空输入或含 NaN 为 None，并列取第一个。
pub fn argmin(values: &[f64]) -> Option<usize> {
    if values.is_empty() || values.iter().any(|value| value.is_nan()) {
        return None;
    }
    let mut best = 0usize;
    for (index, value) in values.iter().enumerate() {
        if *value < values[best] {
            best = index;
        }
    }
    Some(best)
}

/// 最大值下标；空输入或含 NaN 为 None，并列取第一个。
pub fn argmax(values: &[f64]) -> Option<usize> {
    if values.is_empty() || values.iter().any(|value| value.is_nan()) {
        return None;
    }
    let mut best = 0usize;
    for (index, value) in values.iter().enumerate() {
        if *value > values[best] {
            best = index;
        }
    }
    Some(best)
}

/// 总体方差；空输入为 0。
pub fn variance(values: &[f64]) -> f64 {
    if values.is_empty() {
        return 0.0;
    }
    let average = mean(values);
    let mut total = 0.0;
    for value in values {
        let delta = *value - average;
        total += delta * delta;
    }
    total / values.len() as f64
}

/// 总体标准差；空输入为 0。
pub fn std(values: &[f64]) -> f64 {
    variance(values).sqrt()
}

/// 总体协方差；长度不一致或空输入为 0。
pub fn covariance(left: &[f64], right: &[f64]) -> f64 {
    if left.len() != right.len() || left.is_empty() {
        return 0.0;
    }
    let mean_left = mean(left);
    let mean_right = mean(right);
    let mut total = 0.0;
    for index in 0..left.len() {
        total += (left[index] - mean_left) * (right[index] - mean_right);
    }
    total / left.len() as f64
}
