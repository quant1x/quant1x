//! 数值规格层 —— **与 `quant1x/num` 的 Python 实现同语义的 Rust 对齐实现**
//!
//! 口径见 `quant1x/num/num.md`：完整窗口才计算、NaN 传播、顺序累加、总体方差、
//! 均值中心化回归、半程远离零。容器统一用 `Vec<f64>`，布尔序列用 `Vec<bool>`；
//! 缺失值只有 NaN。
//!
//! 与 Python 的唯一命名差异：`ref` 是 Rust 关键字，改用 [`ref_value`]；`where`
//! 是 Rust 关键字，改用 [`where_select`]。

pub mod align;
pub mod decimals;
pub mod dtype;
pub mod elementwise;
pub mod ewm;
pub mod fillna;
pub mod logic;
pub mod matrix;
pub mod reduce;
pub mod regression;
pub mod rolling;
pub mod shift;
pub mod window;

pub use align::{align, pad_to, repeat};
pub use decimals::{change_rate, decimal, decimal_series, net_change_rate};
pub use dtype::{NAN, default_like, is_nan, to_scalar};
pub use elementwise::{
    abs, add, add_scalar, div, div_scalar, log, maximum, maximum_scalar, minimum, minimum_scalar,
    mul, mul_scalar, pow, sqrt, sub, sub_scalar, where_select,
};
pub use ewm::ewm_mean;
pub use fillna::{fill, fill_na, is_na};
pub use logic::{
    all_of, any_of, count_true, equal, gt, gte, logic_and, logic_not, logic_or, lt, lte, not_equal,
    to_bools,
};
pub use matrix::{arange, concat1d, linspace, ones, shape, transpose2d, zeros};
pub use reduce::{
    argmax, argmin, covariance, max_value, mean, median, min_value, std, sum_value, variance,
};
pub use regression::{forcast, linear_regression, slope};
pub use rolling::{
    rolling, rolling_apply, rolling_avedev, rolling_hhvbars, rolling_llvbars, rolling_max,
    rolling_mean, rolling_min, rolling_std, rolling_sum,
};
pub use shift::{cumsum, diff, ref_value, shift};
pub use window::{Window, as_window, window_at};

/// 一维数组容器（对标 numpy.ndarray）
pub mod ndarray;
pub use ndarray::NdArray;
