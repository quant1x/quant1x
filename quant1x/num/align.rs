//! 序列对齐与重复 —— 按目标长度裁剪或补缺。
//!
//! 跨语言参考算法: Python spec 自身不调用 align / pad_to / repeat, 应用层请用
//! quant1x/pandas 的容器对齐; 本文件只为与 Python 的对齐口径逐位对照而存在。

use crate::num::dtype::NAN;

/// 对齐到指定长度：超长截断，不足补 `fill_value`。
pub fn align(values: &[f64], length: usize, fill_value: f64) -> Vec<f64> {
    if values.len() == length {
        return values.to_vec();
    }
    if values.len() > length {
        return values[..length].to_vec();
    }
    let mut out = vec![fill_value; length];
    out[..values.len()].copy_from_slice(values);
    out
}

/// 补齐到指定长度（与 [`align`] 同义）。
pub fn pad_to(values: &[f64], length: usize, fill_value: f64) -> Vec<f64> {
    align(values, length, fill_value)
}

/// 平铺到指定长度：常量重复，序列循环重复。
pub fn repeat(values: &[f64], length: usize) -> Vec<f64> {
    if values.is_empty() {
        return vec![NAN; length];
    }
    (0..length)
        .map(|index| values[index % values.len()])
        .collect()
}
