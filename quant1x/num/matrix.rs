//! 形状与构造 —— 一维拼接、二维转置、等差序列。

/// 形状。
pub fn shape(values: &[f64]) -> Vec<usize> {
    vec![values.len()]
}

/// 一维拼接。
pub fn concat1d(parts: &[&[f64]]) -> Vec<f64> {
    let mut out = Vec::new();
    for part in parts {
        out.extend_from_slice(part);
    }
    out
}

/// 二维转置；非矩形输入按最短行截断。
pub fn transpose2d(values: &[Vec<f64>]) -> Vec<Vec<f64>> {
    if values.is_empty() {
        return Vec::new();
    }
    let columns = values.iter().map(Vec::len).min().unwrap_or(0);
    (0..columns)
        .map(|column| values.iter().map(|row| row[column]).collect())
        .collect()
}

/// 全零序列。
pub fn zeros(length: usize) -> Vec<f64> {
    vec![0.0; length]
}

/// 全一序列。
pub fn ones(length: usize) -> Vec<f64> {
    vec![1.0; length]
}

/// 等差序列 `[start, stop)`。
pub fn arange(start: f64, stop: f64, step: f64) -> Vec<f64> {
    let mut out = Vec::new();
    if step == 0.0 {
        return out;
    }
    let mut value = start;
    if step > 0.0 {
        while value < stop {
            out.push(value);
            value += step;
        }
    } else {
        while value > stop {
            out.push(value);
            value += step;
        }
    }
    out
}

/// 等分序列，含两端。
pub fn linspace(start: f64, stop: f64, count: usize) -> Vec<f64> {
    if count == 0 {
        return Vec::new();
    }
    if count == 1 {
        return vec![start];
    }
    let step = (stop - start) / (count - 1) as f64;
    (0..count)
        .map(|index| start + step * index as f64)
        .collect()
}
