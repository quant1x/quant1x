//! formula —— **通达信 / 同花顺公式函数, 语义以 quant1x/formula/formula.py 为准**
//!
//! 与 C++ `quant1x/formula/formula.h` 逐函数对应; 容器用 `Vec<f64>` (数值) 与
//! `Vec<bool>` (布尔), 全程显式循环, 归约顺序与 Python / C++ 一致。
//!
//! 口径:
//! - 滚动窗口必须完整且不含 NaN, 否则该位置为 NaN;
//! - EMA / SMA 为 `y_t = (1-alpha)*y_{t-1} + alpha*x_t`, 首值作种子, NaN 跳过;
//! - SLOPE / FORCAST 用均值中心化最小二乘闭式解, 不做列缩放;
//! - RD 为半程远离零, 不是银行家舍入。
//!
//! 命名: `ref` / `const` / `if` / `std` 是 Rust 关键字或标准库名, 改用
//! `ref_value` / `const_series` / `if_select` / `stddev`, 与 C++ 同名。

use crate::num;

/// 数值序列
pub type Series = Vec<f64>;

/// 布尔序列
pub type Bools = Vec<bool>;

// ---------------------------------------------------------------------------
// 基础工具
// ---------------------------------------------------------------------------

/// 缺失值
pub fn nan_value() -> f64 {
    f64::NAN
}

/// 判断缺失
pub fn is_nan(value: f64) -> bool {
    value.is_nan()
}

/// 判断真值: 非零且非 NaN
pub fn is_true(value: f64) -> bool {
    !value.is_nan() && value != 0.0
}

/// 构造全 NaN 序列
pub fn make_series(size: usize) -> Series {
    vec![f64::NAN; size]
}

/// 构造指定值序列
pub fn filled_series(size: usize, value: f64) -> Series {
    vec![value; size]
}

/// 构造全假布尔序列
pub fn make_bools(size: usize) -> Bools {
    vec![false; size]
}

/// 布尔序列转数值
pub fn as_series(values: &[bool]) -> Series {
    values
        .iter()
        .map(|&value| if value { 1.0 } else { 0.0 })
        .collect()
}

/// 数值序列转布尔
pub fn as_bools(values: &Series) -> Bools {
    values.iter().map(|&value| is_true(value)).collect()
}

/// 顺序求和
fn sequential_sum(values: &[f64]) -> f64 {
    let mut total = 0.0;
    for &value in values {
        total += value;
    }
    total
}

/// 窗口内是否含 NaN
fn window_has_nan(values: &[f64]) -> bool {
    values.iter().any(|&value| value.is_nan())
}

/// 逐窗口归约: 完整窗口且无 NaN 才计算
fn rolling_apply<F>(x: &[f64], window: &num::Window, apply: F) -> Series
where
    F: Fn(&[f64]) -> f64,
{
    let sizes = num::as_window(window, x.len());
    let mut out = make_series(x.len());
    for (index, &size) in sizes.iter().enumerate() {
        if size <= 0 || size as usize > index + 1 {
            continue;
        }
        let begin = index + 1 - size as usize;
        let end = index + 1;
        let block = &x[begin..end];
        if window_has_nan(block) {
            continue;
        }
        out[index] = apply(block);
    }
    out
}

/// 常量窗口展开: 交给 `num::as_window` 按序列长度展开
fn constant_window(value: i64) -> num::Window {
    num::Window::Scalar(value)
}

// ---------------------------------------------------------------------------
// 逐元素运算
// ---------------------------------------------------------------------------

/// 绝对值
pub fn abs(x: &[f64]) -> Series {
    x.iter().map(|value| value.abs()).collect()
}

/// 平方根
pub fn sqrt(x: &[f64]) -> Series {
    x.iter().map(|value| value.sqrt()).collect()
}

/// 自然对数
pub fn log(x: &[f64]) -> Series {
    x.iter().map(|value| value.ln()).collect()
}

/// 整数次幂
pub fn pow(x: &[f64], exponent: f64) -> Series {
    x.iter().map(|value| value.powf(exponent)).collect()
}

/// 逐元素加
pub fn add(left: &[f64], right: &[f64]) -> Series {
    let length = left.len().min(right.len());
    let mut out = make_series(left.len());
    for index in 0..length {
        out[index] = left[index] + right[index];
    }
    out
}

/// 逐元素减
pub fn sub(left: &[f64], right: &[f64]) -> Series {
    let length = left.len().min(right.len());
    let mut out = make_series(left.len());
    for index in 0..length {
        out[index] = left[index] - right[index];
    }
    out
}

/// 逐元素乘
pub fn mul(left: &[f64], right: &[f64]) -> Series {
    let length = left.len().min(right.len());
    let mut out = make_series(left.len());
    for index in 0..length {
        out[index] = left[index] * right[index];
    }
    out
}

/// 逐元素除
pub fn div(left: &[f64], right: &[f64]) -> Series {
    let length = left.len().min(right.len());
    let mut out = make_series(left.len());
    for index in 0..length {
        out[index] = left[index] / right[index];
    }
    out
}

/// 序列与标量运算
pub fn add_scalar(x: &[f64], value: f64) -> Series {
    x.iter().map(|item| item + value).collect()
}

/// 序列与标量减
pub fn sub_scalar(x: &[f64], value: f64) -> Series {
    x.iter().map(|item| item - value).collect()
}

/// 序列与标量乘
pub fn mul_scalar(x: &[f64], value: f64) -> Series {
    x.iter().map(|item| item * value).collect()
}

/// 序列与标量除
pub fn div_scalar(x: &[f64], value: f64) -> Series {
    x.iter().map(|item| item / value).collect()
}

/// 标量与序列加
pub fn scalar_add(value: f64, x: &[f64]) -> Series {
    x.iter().map(|item| value + item).collect()
}

/// 标量与序列减
pub fn scalar_sub(value: f64, x: &[f64]) -> Series {
    x.iter().map(|item| value - item).collect()
}

/// 标量与序列乘
pub fn scalar_mul(value: f64, x: &[f64]) -> Series {
    x.iter().map(|item| value * item).collect()
}

/// 标量与序列除
pub fn scalar_div(value: f64, x: &[f64]) -> Series {
    x.iter().map(|item| value / item).collect()
}

/// 取大: 任一侧 NaN 则结果 NaN
pub fn maximum(left: &[f64], right: &[f64]) -> Series {
    let length = left.len().min(right.len());
    let mut out = make_series(left.len());
    for index in 0..length {
        let a = left[index];
        let b = right[index];
        out[index] = if a.is_nan() || b.is_nan() {
            f64::NAN
        } else {
            a.max(b)
        };
    }
    out
}

/// 取小: 任一侧 NaN 则结果 NaN
pub fn minimum(left: &[f64], right: &[f64]) -> Series {
    let length = left.len().min(right.len());
    let mut out = make_series(left.len());
    for index in 0..length {
        let a = left[index];
        let b = right[index];
        out[index] = if a.is_nan() || b.is_nan() {
            f64::NAN
        } else {
            a.min(b)
        };
    }
    out
}

/// 序列与标量取大
pub fn maximum_scalar(x: &[f64], value: f64) -> Series {
    maximum(x, &filled_series(x.len(), value))
}

/// 序列与标量取小
pub fn minimum_scalar(x: &[f64], value: f64) -> Series {
    minimum(x, &filled_series(x.len(), value))
}

/// 大于
pub fn greater(left: &[f64], right: &[f64]) -> Bools {
    let length = left.len().min(right.len());
    (0..length)
        .map(|index| left[index] > right[index])
        .collect()
}

/// 大于等于
pub fn greater_equal(left: &[f64], right: &[f64]) -> Bools {
    let length = left.len().min(right.len());
    (0..length)
        .map(|index| left[index] >= right[index])
        .collect()
}

/// 小于
pub fn less(left: &[f64], right: &[f64]) -> Bools {
    let length = left.len().min(right.len());
    (0..length)
        .map(|index| left[index] < right[index])
        .collect()
}

/// 小于等于
pub fn less_equal(left: &[f64], right: &[f64]) -> Bools {
    let length = left.len().min(right.len());
    (0..length)
        .map(|index| left[index] <= right[index])
        .collect()
}

/// 相等
pub fn equal(left: &[f64], right: &[f64]) -> Bools {
    let length = left.len().min(right.len());
    (0..length)
        .map(|index| left[index] == right[index])
        .collect()
}

/// 不等
pub fn not_equal(left: &[f64], right: &[f64]) -> Bools {
    let length = left.len().min(right.len());
    (0..length)
        .map(|index| left[index] != right[index])
        .collect()
}

/// 序列与标量比较
pub fn greater_scalar(x: &[f64], value: f64) -> Bools {
    greater(x, &filled_series(x.len(), value))
}

/// 大于等于 (标量)
pub fn greater_equal_scalar(x: &[f64], value: f64) -> Bools {
    greater_equal(x, &filled_series(x.len(), value))
}

/// 小于 (标量)
pub fn less_scalar(x: &[f64], value: f64) -> Bools {
    less(x, &filled_series(x.len(), value))
}

/// 小于等于 (标量)
pub fn less_equal_scalar(x: &[f64], value: f64) -> Bools {
    less_equal(x, &filled_series(x.len(), value))
}

/// 相等 (标量)
pub fn equal_scalar(x: &[f64], value: f64) -> Bools {
    equal(x, &filled_series(x.len(), value))
}

/// 逐元素逻辑与
pub fn logic_and(left: &[bool], right: &[bool]) -> Bools {
    let length = left.len().min(right.len());
    (0..length)
        .map(|index| left[index] && right[index])
        .collect()
}

/// 逐元素逻辑或
pub fn logic_or(left: &[bool], right: &[bool]) -> Bools {
    let length = left.len().min(right.len());
    (0..length)
        .map(|index| left[index] || right[index])
        .collect()
}

/// 逐元素逻辑非
pub fn logic_not(x: &[bool]) -> Bools {
    x.iter().map(|value| !value).collect()
}

/// 条件选择: 条件为真取真值
pub fn if_select(condition: &[bool], true_value: &[f64], false_value: &[f64]) -> Series {
    let mut out = make_series(condition.len());
    for index in 0..condition.len() {
        out[index] = if condition[index] {
            true_value.get(index).copied().unwrap_or(f64::NAN)
        } else {
            false_value.get(index).copied().unwrap_or(f64::NAN)
        };
    }
    out
}

/// 条件选择的标量版本
pub fn if_select_scalar(condition: &[bool], true_value: f64, false_value: f64) -> Series {
    condition
        .iter()
        .map(|&flag| if flag { true_value } else { false_value })
        .collect()
}

/// 条件选择: 真值为序列, 假值为标量
pub fn if_select_false_scalar(condition: &[bool], true_value: &[f64], false_value: f64) -> Series {
    if_select(
        condition,
        true_value,
        &filled_series(condition.len(), false_value),
    )
}

/// 条件选择: 真值为标量, 假值为序列
pub fn if_select_true_scalar(condition: &[bool], true_value: f64, false_value: &[f64]) -> Series {
    if_select(
        condition,
        &filled_series(condition.len(), true_value),
        false_value,
    )
}

/// 条件选择: 两侧都是布尔序列
pub fn if_select_bools(condition: &[bool], true_value: &[bool], false_value: &[bool]) -> Bools {
    let mut out = make_bools(condition.len());
    for index in 0..condition.len() {
        out[index] = if condition[index] {
            true_value.get(index).copied().unwrap_or(false)
        } else {
            false_value.get(index).copied().unwrap_or(false)
        };
    }
    out
}

/// 把数值序列转成逐位置窗口: NaN 记为 -1, 其余四舍五入
pub fn series_to_window(values: &[f64]) -> num::Window {
    num::Window::Series(
        values
            .iter()
            .map(|&value| {
                if value.is_nan() {
                    -1
                } else {
                    value.round() as i64
                }
            })
            .collect(),
    )
}

// ---------------------------------------------------------------------------
// 移位 / 差分 / 累计
// ---------------------------------------------------------------------------

/// 取 N 周期前的值
pub fn ref_value(x: &[f64], periods: i64) -> Series {
    ref_window(x, &constant_window(periods))
}

/// 取逐位置窗口前的值
pub fn ref_window(x: &[f64], window: &num::Window) -> Series {
    let periods = num::as_window(window, x.len());
    let mut out = make_series(x.len());
    for (index, &period) in periods.iter().enumerate() {
        if period >= 0 {
            if period as usize > index {
                continue;
            }
            out[index] = x[index - period as usize];
        } else {
            let offset = (-period) as usize;
            if index + offset >= x.len() {
                continue;
            }
            out[index] = x[index + offset];
        }
    }
    out
}

/// N 周期差分
pub fn diff(x: &[f64], periods: i64) -> Series {
    sub(x, &ref_value(x, periods))
}

/// 累计和
pub fn cumsum(x: &[f64]) -> Series {
    let mut out = make_series(x.len());
    let mut total = 0.0;
    for (index, &value) in x.iter().enumerate() {
        total += value;
        out[index] = total;
    }
    out
}

/// 求 N 周期累计和, 窗口 <= 0 表示从首个值开始累计
pub fn sum(x: &[f64], window: impl Into<num::Window>) -> Series {
    let window = window.into();
    let sizes = num::as_window(&window, x.len());
    let mut out = make_series(x.len());
    for (index, &size) in sizes.iter().enumerate() {
        if size > 0 {
            if size as usize > index + 1 {
                continue;
            }
            let begin = index + 1 - size as usize;
            let block = &x[begin..index + 1];
            if window_has_nan(block) {
                continue;
            }
            out[index] = sequential_sum(block);
        } else {
            let block = &x[..index + 1];
            if window_has_nan(block) {
                continue;
            }
            out[index] = sequential_sum(block);
        }
    }
    out
}

/// 常量序列: 用最后一个值铺满
pub fn const_series(x: &[f64]) -> Series {
    if x.is_empty() {
        return Series::new();
    }
    filled_series(x.len(), x[x.len() - 1])
}

// ---------------------------------------------------------------------------
// 极值与滚动窗口
// ---------------------------------------------------------------------------

fn window_max(block: &[f64]) -> f64 {
    let mut result = block[0];
    for &value in &block[1..] {
        if value > result {
            result = value;
        }
    }
    result
}

fn window_min(block: &[f64]) -> f64 {
    let mut result = block[0];
    for &value in &block[1..] {
        if value < result {
            result = value;
        }
    }
    result
}

fn window_hhvbars(block: &[f64]) -> f64 {
    let mut position = 0;
    for index in 1..block.len() {
        if block[index] >= block[position] {
            position = index;
        }
    }
    (block.len() - 1 - position) as f64
}

fn window_llvbars(block: &[f64]) -> f64 {
    let mut position = 0;
    for index in 1..block.len() {
        if block[index] <= block[position] {
            position = index;
        }
    }
    (block.len() - 1 - position) as f64
}

/// N 周期最高值
pub fn hhv(x: &[f64], window: impl Into<num::Window>) -> Series {
    rolling_apply(x, &window.into(), window_max)
}

/// N 周期最低值
pub fn llv(x: &[f64], window: impl Into<num::Window>) -> Series {
    rolling_apply(x, &window.into(), window_min)
}

/// N 周期最高点位置
pub fn hhvbars(x: &[f64], window: i64) -> Series {
    rolling_apply(x, &constant_window(window), window_hhvbars)
}

/// N 周期最低点位置
pub fn llvbars(x: &[f64], window: i64) -> Series {
    rolling_apply(x, &constant_window(window), window_llvbars)
}

// ---------------------------------------------------------------------------
// 均线族
// ---------------------------------------------------------------------------

/// 指数加权均值: y_t = (1-alpha)*y_{t-1} + alpha*x_t, 首值作种子, NaN 跳过
pub fn ewm_mean(x: &[f64], alpha: f64) -> Series {
    let mut out = make_series(x.len());
    let mut seeded = false;
    let mut previous = f64::NAN;
    let decay = 1.0 - alpha;
    for (index, &current) in x.iter().enumerate() {
        if current.is_nan() {
            if seeded {
                out[index] = previous;
            }
            continue;
        }
        if !seeded {
            previous = current;
            seeded = true;
            out[index] = previous;
            continue;
        }
        let weighted_previous = decay * previous;
        let weighted_current = alpha * current;
        previous = weighted_previous + weighted_current;
        out[index] = previous;
    }
    out
}

fn window_mean(block: &[f64]) -> f64 {
    sequential_sum(block) / block.len() as f64
}

fn window_std(block: &[f64]) -> f64 {
    let count = block.len() as f64;
    let average = sequential_sum(block) / count;
    let mut total = 0.0;
    for &value in block {
        let delta = value - average;
        total += delta * delta;
    }
    (total / count).sqrt()
}

fn window_avedev(block: &[f64]) -> f64 {
    let count = block.len() as f64;
    let average = sequential_sum(block) / count;
    let mut total = 0.0;
    for &value in block {
        total += (value - average).abs();
    }
    total / count
}

fn window_regression(block: &[f64]) -> (f64, f64) {
    let count = block.len();
    if count < 2 || window_has_nan(block) {
        return (f64::NAN, f64::NAN);
    }
    let mean_t = (count - 1) as f64 / 2.0;
    let mean_y = sequential_sum(block) / count as f64;
    let mut numerator = 0.0;
    let mut denominator = 0.0;
    for (index, &value) in block.iter().enumerate() {
        let offset = index as f64 - mean_t;
        numerator += offset * (value - mean_y);
        denominator += offset * offset;
    }
    let slope = numerator / denominator;
    let intercept = mean_y - slope * mean_t;
    (slope, intercept)
}

/// N 日简单移动平均
pub fn ma(x: &[f64], window: impl Into<num::Window>) -> Series {
    rolling_apply(x, &window.into(), window_mean)
}

/// 指数移动平均: alpha = 2/(N+1)
pub fn ema(x: &[f64], span: i64) -> Series {
    let period = if span <= 0 { 1.0 } else { span as f64 };
    ewm_mean(x, 2.0 / (period + 1.0))
}

/// 中国式 SMA: alpha = M/N
pub fn sma(x: &[f64], span: i64, weight: i64) -> Series {
    let period = if span == 0 { 1.0 } else { span as f64 };
    let mass = if weight == 0 { 1.0 } else { weight as f64 };
    ewm_mean(x, mass / period)
}

/// 加权移动平均
pub fn wma(x: &[f64], window: i64) -> Series {
    rolling_apply(x, &constant_window(window), |block| {
        let mut weighted = 0.0;
        let mut total = 0.0;
        for (position, &value) in block.iter().enumerate() {
            let mass = (position + 1) as f64;
            weighted += mass * value;
            total += mass;
        }
        weighted / total
    })
}

/// 动态移动平均: 常量平滑因子
pub fn dma(x: &[f64], alpha: f64) -> Series {
    ewm_mean(x, alpha)
}

/// 动态移动平均: 逐位置平滑因子, NaN 按 1 处理
pub fn dma_series(x: &[f64], weights: &[f64]) -> Series {
    let mut out = make_series(x.len());
    if x.is_empty() {
        return out;
    }
    out[0] = x[0];
    for index in 1..x.len() {
        let mut alpha = weights.get(index).copied().unwrap_or(1.0);
        if alpha.is_nan() {
            alpha = 1.0;
        }
        out[index] = alpha * x[index] + (1.0 - alpha) * out[index - 1];
    }
    out
}

/// N 日总体标准差
pub fn stddev(x: &[f64], window: i64) -> Series {
    rolling_apply(x, &constant_window(window), window_std)
}

/// 平均绝对偏差
pub fn avedev(x: &[f64], window: impl Into<num::Window>) -> Series {
    rolling_apply(x, &window.into(), window_avedev)
}

/// 线性回归斜率
pub fn slope(x: &[f64], window: i64) -> Series {
    rolling_apply(x, &constant_window(window), |block| {
        window_regression(block).0
    })
}

/// 线性回归预测值
pub fn forcast(x: &[f64], window: impl Into<num::Window>) -> Series {
    rolling_apply(x, &window.into(), |block| {
        let (value, intercept) = window_regression(block);
        if value.is_nan() {
            return f64::NAN;
        }
        intercept + value * (block.len() as f64 - 1.0)
    })
}

// ---------------------------------------------------------------------------
// 1 级函数
// ---------------------------------------------------------------------------

/// 从前 A 日到前 B 日是否一直满足条件
pub fn last(condition: &[bool], far: i64, near: i64) -> Bools {
    let mut out = make_bools(condition.len());
    let window = (far + 1) as usize;
    for index in 0..condition.len() {
        if index + 1 < window {
            continue;
        }
        let mut good = true;
        for offset in near as usize..window {
            if !condition[index - offset] {
                good = false;
                break;
            }
        }
        out[index] = good;
    }
    out
}

/// 最近 N 天满足条件的天数
pub fn count(condition: &[bool], window: impl Into<num::Window>) -> Series {
    sum(&as_series(condition), window)
}

/// 最近 N 天是否全为真
pub fn every(condition: &[bool], window: i64) -> Bools {
    let total = count(condition, window);
    total.iter().map(|&value| value == window as f64).collect()
}

/// 最近 N 天是否存在真值
pub fn exist(condition: &[bool], window: i64) -> Bools {
    let total = count(condition, window);
    total.iter().map(|&value| value > 0.0).collect()
}

/// 信号过滤: 条件成立后 N 个周期置 0, 返回 0/1 序列
pub fn filter(condition: &[bool], window: i64) -> Series {
    let mut out = as_series(condition);
    for index in 0..out.len() {
        if !is_true(out[index]) {
            continue;
        }
        let end = (index + 1 + window as usize).min(out.len());
        for cursor in index + 1..end {
            out[cursor] = 0.0;
        }
    }
    out
}

/// 上一次条件成立到当前的周期数
pub fn barslast(condition: &[bool]) -> Series {
    let mut out = make_series(condition.len());
    let mut previous: i64 = 0;
    for (index, &flag) in condition.iter().enumerate() {
        previous = if flag { 0 } else { previous + 1 };
        out[index] = previous as f64;
    }
    out
}

/// 连续满足条件的周期数
pub fn barslastcount(condition: &[bool]) -> Series {
    let mut out = make_series(condition.len());
    let mut counter: i64 = 0;
    for (index, &flag) in condition.iter().enumerate() {
        counter = if flag { counter + 1 } else { 0 };
        out[index] = counter as f64;
    }
    out
}

/// N 周期内第一次条件成立到现在的周期数
pub fn barssincen(condition: &[bool], window: i64) -> Series {
    let mut out = filled_series(condition.len(), 0.0);
    if window <= 0 {
        return out;
    }
    let size = window as usize;
    for index in 0..condition.len() {
        if index + 1 < size {
            continue;
        }
        let begin = index + 1 - size;
        let mut position = 0;
        while position < size && !condition[begin + position] {
            position += 1;
        }
        if position == size {
            position = 0;
        }
        if position != 0 || condition[begin] {
            out[index] = (size - 1 - position) as f64;
        }
    }
    out
}

/// 向上金叉
pub fn cross(fast: &[f64], slow: &[f64]) -> Bools {
    let previous = less(&ref_value(fast, 1), &ref_value(slow, 1));
    let current = greater_equal(fast, slow);
    logic_and(&previous, &current)
}

/// 维持 N 周期后上穿
pub fn longcross(fast: &[f64], slow: &[f64], window: i64) -> Bools {
    logic_and(&last(&less(fast, slow), window, 1), &greater(fast, slow))
}

/// 条件成立时取值, 否则沿用上一次
pub fn valuewhen(condition: &[bool], values: &[f64]) -> Series {
    let mut out = make_series(condition.len());
    let mut previous = f64::NAN;
    let mut seeded = false;
    for (index, &flag) in condition.iter().enumerate() {
        if flag && index < values.len() && !values[index].is_nan() {
            previous = values[index];
            seeded = true;
        }
        out[index] = if seeded { previous } else { f64::NAN };
    }
    out
}

/// 区间判断
pub fn between(values: &[f64], low: f64, high: f64) -> Bools {
    let left = logic_and(&greater_scalar(values, low), &less_scalar(values, high));
    let right = logic_and(&less_scalar(values, low), &greater_scalar(values, high));
    logic_or(&left, &right)
}

/// 当前值距区间最高点的周期数
pub fn toprange(x: &[f64]) -> Series {
    let mut out = filled_series(x.len(), 0.0);
    for index in 1..x.len() {
        let mut cursor = 0;
        while cursor < index && x[index - 1 - cursor] < x[index] {
            cursor += 1;
        }
        out[index] = if cursor == index { 0.0 } else { cursor as f64 };
    }
    out
}

/// 当前值距区间最低点的周期数
pub fn lowrange(x: &[f64]) -> Series {
    let mut out = filled_series(x.len(), 0.0);
    for index in 1..x.len() {
        let mut cursor = 0;
        while cursor < index && x[index - 1 - cursor] > x[index] {
            cursor += 1;
        }
        out[index] = if cursor == index { 0.0 } else { cursor as f64 };
    }
    out
}

// ---------------------------------------------------------------------------
// 舍入与涨停价
// ---------------------------------------------------------------------------

/// 半程远离零的舍入
pub fn decimal_one(value: f64, digits: i32) -> f64 {
    let places = digits.max(0);
    let mut input = value;
    if input.is_nan() {
        input = 0.0;
    }
    let half = if input.is_sign_negative() { -5.0 } else { 5.0 };
    let scale = 10f64.powi(places);
    let wider = 10f64.powi(places + 1);
    ((input * wider + half) / 10.0).trunc() / scale
}

/// 对序列保留指定位数
pub fn rd(x: &[f64], digits: i32) -> Series {
    x.iter().map(|&value| decimal_one(value, digits)).collect()
}

/// 十进制字面量: 数值 = digits * 10^exponent
struct DecimalParts {
    digits: String,
    exponent: i32,
}

/// 把 double 的最短往返表示拆成十进制字面量
fn parse_shortest(value: f64) -> DecimalParts {
    let negative = value.is_sign_negative();
    let mut text = format!("{}", value.abs());
    let mut exponent: i32 = 0;
    if let Some(pos) = text.find(['e', 'E']) {
        exponent = text[pos + 1..].parse::<i32>().unwrap_or(0);
        text.truncate(pos);
    }
    let mut fraction = 0;
    if let Some(dot) = text.find('.') {
        fraction = (text.len() - dot - 1) as i32;
        text.remove(dot);
    }
    let digits = text.trim_start_matches('0');
    DecimalParts {
        digits: if digits.is_empty() {
            "0".to_string()
        } else {
            digits.to_string()
        },
        exponent: exponent - fraction,
    }
    .with_sign(negative)
}

impl DecimalParts {
    fn with_sign(mut self, negative: bool) -> Self {
        // 负数标记编码在 digits 的前缀, 供 quantize_cents 使用
        if negative && self.digits != "0" {
            self.digits.insert(0, '-');
        }
        self
    }

    fn negative(&self) -> bool {
        self.digits.starts_with('-')
    }

    fn magnitude(&self) -> &str {
        self.digits.trim_start_matches('-')
    }
}

/// 十进制数字串相乘
fn multiply_digits(left: &str, right: &str) -> String {
    let left: Vec<u8> = left.bytes().map(|b| b - b'0').collect();
    let right: Vec<u8> = right.bytes().map(|b| b - b'0').collect();
    let mut acc = vec![0u32; left.len() + right.len()];
    for i in (0..left.len()).rev() {
        for j in (0..right.len()).rev() {
            acc[i + j + 1] += left[i] as u32 * right[j] as u32;
        }
    }
    for k in (1..acc.len()).rev() {
        acc[k - 1] += acc[k] / 10;
        acc[k] %= 10;
    }
    let out: String = acc.iter().map(|d| (b'0' + *d as u8) as char).collect();
    let trimmed = out.trim_start_matches('0');
    if trimmed.is_empty() {
        "0".to_string()
    } else {
        trimmed.to_string()
    }
}

/// 按分四舍五入(半程远离零)
fn quantize_cents(digits: &str, exponent: i32, negative: bool) -> f64 {
    let target = exponent + 2;
    let mut scaled: Vec<u8> = digits.bytes().collect();
    if target >= 0 {
        scaled.extend(std::iter::repeat(b'0').take(target as usize));
    } else {
        let drop = (-target) as usize;
        let dropped = if drop <= scaled.len() {
            scaled[scaled.len() - drop]
        } else {
            b'0'
        };
        if drop > scaled.len() {
            scaled = vec![b'0'];
        } else {
            scaled.truncate(scaled.len() - drop);
            if dropped >= b'5' {
                let mut cursor = scaled.len() as isize - 1;
                while cursor >= 0 {
                    let pos = cursor as usize;
                    if scaled[pos] == b'9' {
                        scaled[pos] = b'0';
                        cursor -= 1;
                    } else {
                        scaled[pos] += 1;
                        break;
                    }
                }
                if cursor < 0 {
                    scaled.insert(0, b'1');
                }
            }
            if scaled.is_empty() {
                scaled = vec![b'0'];
            }
        }
    }

    let text = if scaled.len() <= 2 {
        let mut s = String::from("0.");
        s.push_str(&"0".repeat(2 - scaled.len()));
        s.push_str(std::str::from_utf8(&scaled).unwrap());
        s
    } else {
        let split = scaled.len() - 2;
        let head = std::str::from_utf8(&scaled[..split]).unwrap();
        let tail = std::str::from_utf8(&scaled[split..]).unwrap();
        format!("{}.{}", head, tail)
    };
    let signed = if negative { format!("-{}", text) } else { text };
    signed.parse::<f64>().unwrap_or(f64::NAN)
}

/// 涨停价: 前收盘价乘(1+比例), 按分四舍五入
pub fn ztprice_one(price: f64, ratio: f64) -> f64 {
    if ratio <= 0.0 || price.is_nan() {
        return price;
    }
    let base = parse_shortest(price);
    let factor = parse_shortest(1.0 + ratio);
    let negative = base.negative() != factor.negative();
    let digits = multiply_digits(base.magnitude(), factor.magnitude());
    quantize_cents(&digits, base.exponent + factor.exponent, negative)
}

/// 涨停价: 序列版本
pub fn ztprice(price: &[f64], ratio: f64) -> Series {
    if ratio <= 0.0 {
        return price.to_vec();
    }
    price
        .iter()
        .map(|&value| ztprice_one(value, ratio))
        .collect()
}

// ---------------------------------------------------------------------------
// 2 级指标
// ---------------------------------------------------------------------------

/// MACD: 返回 (DIF, DEA, MACD)
pub fn macd(close: &[f64], fast: i64, slow: i64, signal: i64) -> (Series, Series, Series) {
    let dif = sub(&ema(close, fast), &ema(close, slow));
    let dea = ema(&dif, signal);
    let histogram = mul_scalar(&sub(&dif, &dea), 2.0);
    (rd(&dif, 3), rd(&dea, 3), rd(&histogram, 3))
}

/// KDJ: 返回 (K, D, J)
pub fn kdj(
    close: &[f64],
    high: &[f64],
    low: &[f64],
    period: i64,
    m1: i64,
    m2: i64,
) -> (Series, Series, Series) {
    let rsv = mul_scalar(
        &div(
            &sub(close, &llv(low, period)),
            &sub(&hhv(high, period), &llv(low, period)),
        ),
        100.0,
    );
    let k = ema(&rsv, m1 * 2 - 1);
    let d = ema(&k, m2 * 2 - 1);
    let j = sub(&mul_scalar(&k, 3.0), &mul_scalar(&d, 2.0));
    (k, d, j)
}

/// RSI
pub fn rsi(close: &[f64], period: i64) -> Series {
    let delta = sub(close, &ref_value(close, 1));
    let up = sma(&maximum_scalar(&delta, 0.0), period, 1);
    let down = sma(&abs(&delta), period, 1);
    rd(&mul_scalar(&div(&up, &down), 100.0), 3)
}

/// W&R: 返回 (WR, WR1)
pub fn wr(
    close: &[f64],
    high: &[f64],
    low: &[f64],
    period: i64,
    short_period: i64,
) -> (Series, Series) {
    let one = |span: i64| {
        mul_scalar(
            &div(
                &sub(&hhv(high, span), close),
                &sub(&hhv(high, span), &llv(low, span)),
            ),
            100.0,
        )
    };
    (rd(&one(period), 3), rd(&one(short_period), 3))
}

/// BIAS: 返回 (BIAS1, BIAS2, BIAS3)
pub fn bias(close: &[f64], l1: i64, l2: i64, l3: i64) -> (Series, Series, Series) {
    let one = |span: i64| {
        let average = ma(close, span);
        mul_scalar(&div(&sub(close, &average), &average), 100.0)
    };
    (rd(&one(l1), 3), rd(&one(l2), 3), rd(&one(l3), 3))
}

/// BOLL: 返回 (UPPER, MID, LOWER)
pub fn boll(close: &[f64], period: i64, width: f64) -> (Series, Series, Series) {
    let mid = ma(close, period);
    let band = mul_scalar(&stddev(close, period), width);
    (
        rd(&add(&mid, &band), 3),
        rd(&mid, 3),
        rd(&sub(&mid, &band), 3),
    )
}

/// PSY: 返回 (PSY, PSYMA)
pub fn psy(close: &[f64], period: i64, average_period: i64) -> (Series, Series) {
    let rising = greater(close, &ref_value(close, 1));
    let value = mul_scalar(&div_scalar(&count(&rising, period), period as f64), 100.0);
    (rd(&value, 3), rd(&ma(&value, average_period), 3))
}

/// CCI
pub fn cci(close: &[f64], high: &[f64], low: &[f64], period: i64) -> Series {
    let typical = div_scalar(&add(&add(high, low), close), 3.0);
    div(
        &sub(&typical, &ma(&typical, period)),
        &mul_scalar(&avedev(&typical, period), 0.015),
    )
}

/// ATR
pub fn atr(close: &[f64], high: &[f64], low: &[f64], period: i64) -> Series {
    let previous = ref_value(close, 1);
    let true_range = maximum(
        &maximum(&sub(high, low), &abs(&sub(&previous, high))),
        &abs(&sub(&previous, low)),
    );
    ma(&true_range, period)
}

/// BBI
pub fn bbi(close: &[f64], m1: i64, m2: i64, m3: i64, m4: i64) -> Series {
    div_scalar(
        &add(
            &add(&ma(close, m1), &ma(close, m2)),
            &add(&ma(close, m3), &ma(close, m4)),
        ),
        4.0,
    )
}

/// DMI: 返回 (PDI, MDI, ADX, ADXR)
pub fn dmi(
    close: &[f64],
    high: &[f64],
    low: &[f64],
    m1: i64,
    m2: i64,
) -> (Series, Series, Series, Series) {
    let previous = ref_value(close, 1);
    let true_range = sum(
        &maximum(
            &maximum(&sub(high, low), &abs(&sub(high, &previous))),
            &abs(&sub(low, &previous)),
        ),
        m1,
    );
    let high_delta = sub(high, &ref_value(high, 1));
    let low_delta = sub(&ref_value(low, 1), low);
    let up = logic_and(
        &greater_scalar(&high_delta, 0.0),
        &greater(&high_delta, &low_delta),
    );
    let down = logic_and(
        &greater_scalar(&low_delta, 0.0),
        &greater(&low_delta, &high_delta),
    );
    let dmp = sum(&if_select_false_scalar(&up, &high_delta, 0.0), m1);
    let dmm = sum(&if_select_false_scalar(&down, &low_delta, 0.0), m1);
    let pdi = div(&mul_scalar(&dmp, 100.0), &true_range);
    let mdi = div(&mul_scalar(&dmm, 100.0), &true_range);
    let adx = ma(
        &mul_scalar(&div(&abs(&sub(&mdi, &pdi)), &add(&pdi, &mdi)), 100.0),
        m2,
    );
    let adxr = div_scalar(&add(&adx, &ref_value(&adx, m2)), 2.0);
    (pdi, mdi, adx, adxr)
}

/// TAQ: 返回 (UP, MID, DOWN)
pub fn taq(high: &[f64], low: &[f64], period: i64) -> (Series, Series, Series) {
    let up = hhv(high, period);
    let down = llv(low, period);
    let mid = div_scalar(&add(&up, &down), 2.0);
    (up, mid, down)
}

/// KTN: 返回 (UPPER, MID, LOWER)
pub fn ktn(
    close: &[f64],
    high: &[f64],
    low: &[f64],
    period: i64,
    atr_period: i64,
) -> (Series, Series, Series) {
    let mid = ema(&div_scalar(&add(&add(high, low), close), 3.0), period);
    let band = mul_scalar(&atr(close, high, low, atr_period), 2.0);
    (add(&mid, &band), mid.clone(), sub(&mid, &band))
}

/// TRIX: 返回 (TRIX, TRMA)
pub fn trix(close: &[f64], m1: i64, m2: i64) -> (Series, Series) {
    let smoothed = ema(&ema(&ema(close, m1), m1), m1);
    let value = mul_scalar(
        &div(
            &sub(&smoothed, &ref_value(&smoothed, 1)),
            &ref_value(&smoothed, 1),
        ),
        100.0,
    );
    (value.clone(), ma(&value, m2))
}

/// VR
pub fn vr(close: &[f64], volume: &[f64], period: i64) -> Series {
    let rising = greater(close, &ref_value(close, 1));
    let falling = less_equal(close, &ref_value(close, 1));
    mul_scalar(
        &div(
            &sum(&if_select_false_scalar(&rising, volume, 0.0), period),
            &sum(&if_select_false_scalar(&falling, volume, 0.0), period),
        ),
        100.0,
    )
}

/// EMV: 返回 (EMV, MAEMV)
pub fn emv(
    high: &[f64],
    low: &[f64],
    volume: &[f64],
    period: i64,
    average_period: i64,
) -> (Series, Series) {
    let volume_ratio = div(&ma(volume, period), volume);
    let mid = mul_scalar(
        &div(
            &sub(&add(high, low), &ref_value(&add(high, low), 1)),
            &add(high, low),
        ),
        100.0,
    );
    let value = ma(
        &div(
            &mul(&mul(&mid, &volume_ratio), &sub(high, low)),
            &ma(&sub(high, low), period),
        ),
        period,
    );
    (value.clone(), ma(&value, average_period))
}

/// DPO: 返回 (DPO, MADPO)
pub fn dpo(close: &[f64], m1: i64, m2: i64, m3: i64) -> (Series, Series) {
    let value = sub(close, &ref_value(&ma(close, m1), m2));
    (value.clone(), ma(&value, m3))
}

/// BRAR: 返回 (AR, BR)
pub fn brar(
    open: &[f64],
    close: &[f64],
    high: &[f64],
    low: &[f64],
    period: i64,
) -> (Series, Series) {
    let ar = mul_scalar(
        &div(
            &sum(&sub(high, open), period),
            &sum(&sub(open, low), period),
        ),
        100.0,
    );
    let br = mul_scalar(
        &div(
            &sum(
                &maximum_scalar(&sub(high, &ref_value(close, 1)), 0.0),
                period,
            ),
            &sum(
                &maximum_scalar(&sub(&ref_value(close, 1), low), 0.0),
                period,
            ),
        ),
        100.0,
    );
    (ar, br)
}

/// DFMA: 返回 (DIF, DIFMA)
pub fn dfma(close: &[f64], n1: i64, n2: i64, m: i64) -> (Series, Series) {
    let value = sub(&ma(close, n1), &ma(close, n2));
    (value.clone(), ma(&value, m))
}

/// MTM: 返回 (MTM, MTMMA)
pub fn mtm(close: &[f64], period: i64, average_period: i64) -> (Series, Series) {
    let value = sub(close, &ref_value(close, period));
    (value.clone(), ma(&value, average_period))
}

/// MASS: 返回 (MASS, MA_MASS)
pub fn mass(high: &[f64], low: &[f64], n1: i64, n2: i64, m: i64) -> (Series, Series) {
    let range_series = sub(high, low);
    let value = sum(
        &div(&ma(&range_series, n1), &ma(&ma(&range_series, n1), n1)),
        n2,
    );
    (value.clone(), ma(&value, m))
}

/// ROC: 返回 (ROC, MAROC)
pub fn roc(close: &[f64], period: i64, average_period: i64) -> (Series, Series) {
    let lagged = ref_value(close, period);
    let value = mul_scalar(&div(&sub(close, &lagged), &lagged), 100.0);
    (value.clone(), ma(&value, average_period))
}

/// EXPMA: 返回 (EMA(N1), EMA(N2))
pub fn expma(close: &[f64], n1: i64, n2: i64) -> (Series, Series) {
    (ema(close, n1), ema(close, n2))
}

/// OBV
pub fn obv(close: &[f64], volume: &[f64]) -> Series {
    let rising = greater(close, &ref_value(close, 1));
    let falling = less(close, &ref_value(close, 1));
    let signed = if_select(
        &rising,
        volume,
        &if_select_false_scalar(&falling, &mul_scalar(volume, -1.0), 0.0),
    );
    div_scalar(&cumsum(&signed), 10000.0)
}

/// MFI
pub fn mfi(close: &[f64], high: &[f64], low: &[f64], volume: &[f64], period: i64) -> Series {
    let typical = div_scalar(&add(&add(high, low), close), 3.0);
    let up = greater(&typical, &ref_value(&typical, 1));
    let down = less(&typical, &ref_value(&typical, 1));
    let ratio = div(
        &sum(
            &if_select_false_scalar(&up, &mul(&typical, volume), 0.0),
            period,
        ),
        &sum(
            &if_select_false_scalar(&down, &mul(&typical, volume), 0.0),
            period,
        ),
    );
    scalar_sub(100.0, &scalar_div(100.0, &add_scalar(&ratio, 1.0)))
}

/// ASI: 返回 (ASI, ASIT)
pub fn asi(
    open: &[f64],
    close: &[f64],
    high: &[f64],
    low: &[f64],
    m1: i64,
    m2: i64,
) -> (Series, Series) {
    let previous_close = ref_value(close, 1);
    let aa = abs(&sub(high, &previous_close));
    let bb = abs(&sub(low, &previous_close));
    let cc = abs(&sub(high, &ref_value(low, 1)));
    let dd = abs(&sub(&previous_close, &ref_value(open, 1)));
    let first_branch = logic_and(&greater(&aa, &bb), &greater(&aa, &cc));
    let second_branch = logic_and(&greater(&bb, &cc), &greater(&bb, &aa));
    let branch_one = add(&add(&aa, &div_scalar(&bb, 2.0)), &div_scalar(&dd, 4.0));
    let branch_two = add(&add(&bb, &div_scalar(&aa, 2.0)), &div_scalar(&dd, 4.0));
    let branch_three = add(&cc, &div_scalar(&dd, 4.0));
    let r = if_select(
        &first_branch,
        &branch_one,
        &if_select(&second_branch, &branch_two, &branch_three),
    );
    let x = add(
        &add(
            &sub(close, &previous_close),
            &div_scalar(&sub(close, open), 2.0),
        ),
        &sub(&previous_close, &ref_value(open, 1)),
    );
    let si = div(&mul_scalar(&div(&x, &r), 16.0), &maximum(&aa, &bb));
    let value = sum(&si, m1);
    (value.clone(), ma(&value, m2))
}

/// XSII: 返回 (TD1, TD2, TD3, TD4)
pub fn xsii(
    close: &[f64],
    high: &[f64],
    low: &[f64],
    n: i64,
    m: i64,
) -> (Series, Series, Series, Series) {
    let typical = div_scalar(&add(&add(&mul_scalar(close, 2.0), high), low), 4.0);
    let aa = ma(&typical, 5);
    let td1 = mul_scalar(&aa, n as f64 / 100.0);
    let td2 = mul_scalar(&aa, (200 - n) as f64 / 100.0);
    let close_ma = ma(close, 20);
    let cc = div(&abs(&sub(&typical, &close_ma)), &close_ma);
    let dd = dma_series(close, &cc);
    let td3 = mul_scalar(&dd, 1.0 + m as f64 / 100.0);
    let td4 = mul_scalar(&dd, 1.0 - m as f64 / 100.0);
    (td1, td2, td3, td4)
}
