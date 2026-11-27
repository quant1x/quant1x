//! 窗口参数 —— 常量窗口与逐位置窗口统一成 `Vec<i64>`。
//!
//! 无效窗口（`<= 0`、NaN）由滚动算子统一返回 NaN。

/// 无效窗口标记。
pub const INVALID_WINDOW: i64 = -1;

/// 窗口参数：常量或逐位置序列。
#[derive(Clone, Debug)]
pub enum Window {
    /// 常量窗口。
    Scalar(i64),
    /// 逐位置窗口。
    Series(Vec<i64>),
}

impl From<i64> for Window {
    fn from(value: i64) -> Self {
        Window::Scalar(value)
    }
}

impl From<Vec<i64>> for Window {
    fn from(value: Vec<i64>) -> Self {
        Window::Series(value)
    }
}

impl From<&[i64]> for Window {
    fn from(value: &[i64]) -> Self {
        Window::Series(value.to_vec())
    }
}

/// 把窗口参数归一化成与序列等长的数组。
///
/// 逐位置窗口长度不一致时按位置截断或补 `INVALID_WINDOW`。
pub fn as_window(window: &Window, length: usize) -> Vec<i64> {
    match window {
        Window::Scalar(value) => vec![*value; length],
        Window::Series(values) => {
            let mut out = vec![INVALID_WINDOW; length];
            let count = values.len().min(length);
            out[..count].copy_from_slice(&values[..count]);
            out
        }
    }
}

/// 取某位置的窗口长度。
pub fn window_at(window: &Window, index: usize) -> i64 {
    match window {
        Window::Scalar(value) => *value,
        Window::Series(values) => values.get(index).copied().unwrap_or(INVALID_WINDOW),
    }
}
