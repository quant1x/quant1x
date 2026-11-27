//! 移位、差分与累计 —— 越界补 NaN，累计顺序累加。

use crate::num::dtype::NAN;

/// 向后移位：取 N 周期前的值，越界为 NaN。
pub fn shift(values: &[f64], periods: i64) -> Vec<f64> {
    let mut out = vec![NAN; values.len()];
    if periods == 0 {
        out.copy_from_slice(values);
    } else if periods > 0 {
        let offset = periods as usize;
        if offset < values.len() {
            out[offset..].copy_from_slice(&values[..values.len() - offset]);
        }
    } else {
        let offset = (-periods) as usize;
        if offset < values.len() {
            out[..values.len() - offset].copy_from_slice(&values[offset..]);
        }
    }
    out
}

/// 引用 N 周期前的值（`ref` 是 Rust 关键字）。
pub fn ref_value(values: &[f64], periods: i64) -> Vec<f64> {
    shift(values, periods)
}

/// N 周期差分。
pub fn diff(values: &[f64], periods: i64) -> Vec<f64> {
    let lagged = shift(values, periods);
    (0..values.len()).map(|i| values[i] - lagged[i]).collect()
}

/// 累计和。
pub fn cumsum(values: &[f64]) -> Vec<f64> {
    let mut total = 0.0;
    values
        .iter()
        .map(|value| {
            total += *value;
            total
        })
        .collect()
}
