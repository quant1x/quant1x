//! 滚动窗口 —— 完整窗口才计算，窗口内含 NaN 即 NaN。
//!
//! 归约函数按窗口内从左到右顺序执行，不做增量维护。

use crate::num::dtype::NAN;
use crate::num::window::{Window, as_window};

fn sequential_sum(values: &[f64]) -> f64 {
    let mut total = 0.0;
    for value in values {
        total += *value;
    }
    total
}

/// 枚举窗口：返回每个位置的有效切片范围 `(begin, end)`。
fn blocks(x: &[f64], window: &Window, min_periods: Option<usize>) -> Vec<Option<(usize, usize)>> {
    let sizes = as_window(window, x.len());
    (0..x.len())
        .map(|index| {
            let size = sizes[index];
            if size <= 0 {
                return None;
            }
            let available = (index + 1).min(size as usize);
            let threshold = min_periods.unwrap_or(size as usize);
            if available < threshold {
                return None;
            }
            Some((index + 1 - available, index + 1))
        })
        .collect()
}

/// 返回每个位置的窗口；无效位置为空数组。
pub fn rolling(x: &[f64], window: impl Into<Window>) -> Vec<Vec<f64>> {
    let window = window.into();
    blocks(x, &window, None)
        .into_iter()
        .map(|block| match block {
            Some((begin, end)) => x[begin..end].to_vec(),
            None => Vec::new(),
        })
        .collect()
}

/// 逐窗口归约；无效窗口为 NaN。
pub fn rolling_apply<F>(x: &[f64], window: impl Into<Window>, apply: F) -> Vec<f64>
where
    F: Fn(&[f64]) -> f64,
{
    let window = window.into();
    let mut out = vec![NAN; x.len()];
    for (index, block) in blocks(x, &window, None).into_iter().enumerate() {
        if let Some((begin, end)) = block {
            let chunk = &x[begin..end];
            if chunk.iter().any(|value| value.is_nan()) {
                continue;
            }
            out[index] = apply(chunk);
        }
    }
    out
}

/// 滚动求和。
pub fn rolling_sum(x: &[f64], window: impl Into<Window>) -> Vec<f64> {
    rolling_apply(x, window, sequential_sum)
}

/// 滚动均值。
pub fn rolling_mean(x: &[f64], window: impl Into<Window>) -> Vec<f64> {
    rolling_apply(x, window, |block| {
        sequential_sum(block) / block.len() as f64
    })
}

fn two_pass_std(block: &[f64]) -> f64 {
    let count = block.len() as f64;
    let average = sequential_sum(block) / count;
    let mut total = 0.0;
    for value in block {
        let delta = *value - average;
        total += delta * delta;
    }
    (total / count).sqrt()
}

/// 滚动标准差（总体口径）。
pub fn rolling_std(x: &[f64], window: impl Into<Window>) -> Vec<f64> {
    rolling_apply(x, window, two_pass_std)
}

/// 滚动最小值。
pub fn rolling_min(x: &[f64], window: impl Into<Window>) -> Vec<f64> {
    rolling_apply(x, window, |block| {
        block.iter().copied().fold(f64::INFINITY, f64::min)
    })
}

/// 滚动最大值。
pub fn rolling_max(x: &[f64], window: impl Into<Window>) -> Vec<f64> {
    rolling_apply(x, window, |block| {
        block.iter().copied().fold(f64::NEG_INFINITY, f64::max)
    })
}

fn mean_absolute_deviation(block: &[f64]) -> f64 {
    let count = block.len() as f64;
    let average = sequential_sum(block) / count;
    let mut total = 0.0;
    for value in block {
        total += (*value - average).abs();
    }
    total / count
}

/// 滚动平均绝对偏差。
pub fn rolling_avedev(x: &[f64], window: impl Into<Window>) -> Vec<f64> {
    rolling_apply(x, window, mean_absolute_deviation)
}

fn hhvbars(block: &[f64]) -> f64 {
    let mut position = 0usize;
    for index in 1..block.len() {
        if block[index] >= block[position] {
            position = index;
        }
    }
    (block.len() - 1 - position) as f64
}

fn llvbars(block: &[f64]) -> f64 {
    let mut position = 0usize;
    for index in 1..block.len() {
        if block[index] <= block[position] {
            position = index;
        }
    }
    (block.len() - 1 - position) as f64
}

/// 最高值到窗口末端的周期数，并列取最近。
pub fn rolling_hhvbars(x: &[f64], window: impl Into<Window>) -> Vec<f64> {
    rolling_apply(x, window, hhvbars)
}

/// 最低值到窗口末端的周期数，并列取最近。
pub fn rolling_llvbars(x: &[f64], window: impl Into<Window>) -> Vec<f64> {
    rolling_apply(x, window, llvbars)
}
