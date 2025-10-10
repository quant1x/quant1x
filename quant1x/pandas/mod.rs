use std::time::Duration;

/// Parse frequency string similar to C++ pandas::ParseTimeRule + parse_frequency
/// Returns (minutes, normalized_freq_string) or Err(message)
pub fn parse_frequency(freq: &str) -> Result<(i64, String), String> {
    let frequency = freq.trim();
    if frequency.is_empty() {
        return Err("empty freq string".to_string());
    }
    // find numeric prefix
    let mut i = 0usize;
    for c in frequency.chars() {
        if c.is_ascii_digit() {
            i += 1;
        } else {
            break;
        }
    }
    let n: i64 = if i == 0 {
        1
    } else {
        frequency[..i]
            .parse::<i64>()
            .map_err(|e| format!("invalid number in freq: {}", e))?
    };
    let unit = frequency[i..].to_string();
    if unit.is_empty() {
        return Err("missing unit in freq".to_string());
    }
    // map unit to minutes or hours
    // Accept common variations: N, ns, U, us, L, ms, S/s, T/min, H/h, D/d
    let minutes: i64 = match unit.as_str() {
        "N" | "ns" => return Err("nanoseconds not supported for minute kline".to_string()),
        "U" | "us" | "μs" => return Err("microseconds not supported for minute kline".to_string()),
        "L" | "ms" => return Err("milliseconds not supported for minute kline".to_string()),
        "S" | "s" => return Err("seconds not supported for minute kline".to_string()),
        "T" | "min" => n,
        "H" | "h" => n * 60,
        "D" | "d" => n * 60 * 24,
        _ => return Err(format!("unsupported freq unit: {}", unit)),
    };
    Ok((minutes, frequency.to_string()))
}
