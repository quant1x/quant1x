pub fn change_rate(base: f64, current: f64) -> f64 {
    if base == 0.0 {
        0.0
    } else {
        (current - base) / base
    }
}

/// Rounds a floating-point number to a specified number of decimal places.
///
/// # Arguments
/// * `value` - The floating-point number to round
/// * `digits` - The number of decimal places to keep (0-9)
///
/// # Returns
/// The rounded value
pub fn decimal_digits(value: f64, digits: i32) -> f64 {
    let digits = digits.clamp(0, 9) as usize;
    let k_powers_of_10 = [
        1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10
    ];

    if value.is_nan() {
        return 0.0;
    }

    let half = 5.0f64.copysign(value);
    let nj1 = k_powers_of_10[digits + 1];
    let scaled = value * nj1 + half;
    let truncated = (scaled / 10.0).trunc();

    truncated / (nj1 / 10.0)
}

/// Rounds a floating-point number to two decimal places.
///
/// # Arguments
/// * `v` - The floating-point number to round
///
/// # Returns
/// The rounded value with two decimal places
///
/// # Examples
/// ```
/// let result = decimal(3.14159);
/// assert_eq!(result, 3.14);
/// ```
pub fn decimal(v: f64) -> f64 {
    decimal_digits(v, 2)
}
