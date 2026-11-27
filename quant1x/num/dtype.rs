//! 类型与缺失值 —— float64 是唯一数值类型，NaN 是唯一缺失标记。

/// 缺失值。
pub const NAN: f64 = f64::NAN;

/// 判断缺失。
pub fn is_nan(value: f64) -> bool {
    value.is_nan()
}

/// 生成与输入等长的全 NaN 序列。
pub fn default_like(values: &[f64]) -> Vec<f64> {
    vec![NAN; values.len()]
}

/// 取最后一个值；空序列返回 NaN。
pub fn to_scalar(values: &[f64]) -> f64 {
    values.last().copied().unwrap_or(NAN)
}
