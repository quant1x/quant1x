//! 指数加权移动平均 —— NaN 跳过，首个有效值作种子。
//!
//! 递推式固定为 `y_t = (1 - alpha) * y_{t-1} + alpha * x_t`，`y_0 = x_0`。

use crate::num::dtype::NAN;

/// 指数加权均值。
pub fn ewm_mean(values: &[f64], alpha: f64) -> Vec<f64> {
    let mut out = vec![NAN; values.len()];
    let decay = 1.0 - alpha;
    let mut previous = NAN;
    let mut seeded = false;
    for (index, value) in values.iter().enumerate() {
        let current = *value;
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
