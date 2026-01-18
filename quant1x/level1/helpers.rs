use std::sync::atomic::{AtomicU32, Ordering};

static SEQ_ID: AtomicU32 = AtomicU32::new(0);

/// 对应 C++ 中的 `SequenceId()`，保持前置自增行为。
pub fn sequence_id() -> u32 {
    SEQ_ID.fetch_add(1, Ordering::SeqCst).wrapping_add(1)
}

/// 对应 C++ helpers::getDatetimeFromUint32，用于从压缩的日期/分钟编码中恢复年月日时分
pub fn get_datetime_from_u32(
    category: i32,
    zipday: u32,
    tminutes: u16,
) -> (i32, i32, i32, i32, i32) {
    if category < 4 || category == 7 || category == 8 {
        let year = ((zipday >> 11) as i32) + 2004;
        let rem = (zipday % 2048) as i32;
        let month = rem / 100;
        let day = rem % 100;
        let hour = (tminutes / 60) as i32;
        let minute = (tminutes % 60) as i32;
        (year, month, day, hour, minute)
    } else {
        let year = (zipday / 10000) as i32;
        let month = ((zipday % 10000) / 100) as i32;
        let day = (zipday % 100) as i32;
        (year, month, day, 15, 0)
    }
}

/// 对应 C++ helpers::integerToFloat64，将整数编码解码为浮点价格值
pub fn int_to_float64(v: u32) -> f64 {
    if v == 0 {
        return 0.0;
    }
    let log_point = ((v >> 24) & 0xFF) as i32;
    let hleax = ((v >> 16) & 0xFF) as i32;
    let lheax = ((v >> 8) & 0xFF) as i32;
    let lleax = (v & 0xFF) as i32;

    let dw_ecx = log_point * 2 - 0x7F;
    let dw_edx = log_point * 2 - 0x86;
    let dw_esi = log_point * 2 - 0x8E;
    let dw_eax = log_point * 2 - 0x96;

    let dbl_xmm6 = {
        let tmp_eax = if dw_ecx < 0 { -dw_ecx } else { dw_ecx };
        let mut value = 2f64.powi(tmp_eax);
        if dw_ecx < 0 {
            value = 1.0 / value;
        }
        value
    };

    let dbl_xmm4 = if hleax > 0x80 {
        let dwtmpeax = dw_edx + 1;
        let tmpdbl_xmm3 = 2f64.powi(dwtmpeax);
        let mut dbl_xmm0 = 2f64.powi(dw_edx) * 128.0;
        dbl_xmm0 += (hleax & 0x7F) as f64 * tmpdbl_xmm3;
        dbl_xmm0
    } else if dw_edx >= 0 {
        2f64.powi(dw_edx) * (hleax as f64)
    } else {
        (1.0 / 2f64.powi(-dw_edx)) * (hleax as f64)
    };

    let mut dbl_xmm3 = 2f64.powi(dw_esi) * (lheax as f64);
    let mut dbl_xmm1 = 2f64.powi(dw_eax) * (lleax as f64);

    if (hleax & 0x80) != 0 {
        dbl_xmm3 *= 2.0;
        dbl_xmm1 *= 2.0;
    }

    dbl_xmm6 + dbl_xmm4 + dbl_xmm3 + dbl_xmm1
}

/// defaultBaseUnit 等价实现（来自 C++ security_quote.h）
pub fn default_base_unit(_market_id: i32, code: &str) -> f64 {
    // Align with C++ helpers::defaultBaseUnit in `level1/helpers.h`:
    // - If market is ShangHai and code starts with '5' => 1000.0
    // - If market is ShenZhen and code starts with "159" => 1000.0
    // - Otherwise => 100.0
    use crate::exchange::{MARKET_SHANGHAI, MARKET_SHENZHEN};

    // Normalize input and guard for empty strings
    let s = code.trim();
    if s.is_empty() {
        return 100.0;
    }

    if _market_id == (MARKET_SHANGHAI as i32) && s.as_bytes()[0] == b'5' {
        return 1000.0;
    }

    if _market_id == (MARKET_SHENZHEN as i32) && s.starts_with("159") {
        return 1000.0;
    }

    100.0
}
