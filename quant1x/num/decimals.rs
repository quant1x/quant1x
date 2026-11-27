//! 小数位与变化率 —— 半程远离零。

/// 单值保留小数位（半程远离零，NaN 按 0 参与舍入）。
pub fn decimal(value: f64, digits: i32) -> f64 {
    let places = digits.max(0);
    let value = if value.is_nan() { 0.0 } else { value };
    let half = if value.is_sign_negative() { -5.0 } else { 5.0 };
    let scale = 10f64.powi(places);
    let wider = 10f64.powi(places + 1);
    ((value * wider + half) / 10.0).trunc() / scale
}

/// 对序列逐元素保留小数位。
pub fn decimal_series(values: &[f64], digits: i32) -> Vec<f64> {
    values.iter().map(|value| decimal(*value, digits)).collect()
}

/// 变化率 `current / base`。
pub fn change_rate(base: f64, current: f64) -> f64 {
    current / base
}

/// 净变化率百分比 `(current / base - 1) * 100`。
pub fn net_change_rate(base: f64, current: f64) -> f64 {
    (change_rate(base, current) - 1.0) * 100.0
}
