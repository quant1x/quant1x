//! 缺失值填充 —— Rust 侧返回新序列（不原地修改）。

/// 把 NaN 替换为指定值。
pub fn fill(values: &[f64], value: f64) -> Vec<f64> {
    values
        .iter()
        .map(|item| if item.is_nan() { value } else { *item })
        .collect()
}

/// 填充缺失值（与 [`fill`] 同义）。
pub fn fill_na(values: &[f64], value: f64) -> Vec<f64> {
    fill(values, value)
}

/// 逐元素判断是否为 NaN。
pub fn is_na(values: &[f64]) -> Vec<bool> {
    values.iter().map(|value| value.is_nan()).collect()
}
