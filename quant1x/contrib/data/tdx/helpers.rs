// -*- coding: utf-8 -*-
// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// TDX helpers — 工具函数
// 对应 Python quant1x/contrib/data/tdx/helpers.py
// 对应 C++   level1/helpers.h

use std::sync::atomic::{AtomicU32, Ordering};

// ============================================================
// 序列号
// ============================================================

static SEQ_ID: AtomicU32 = AtomicU32::new(0);

/// 生成并返回一个全局唯一的序列ID。
///
/// 每次调用时, 序列ID会递增1, 并保证在32位无符号整数范围内循环(0xFFFFFFFF)。
pub fn msg_sequence_id() -> u32 {
    SEQ_ID.fetch_add(1, Ordering::SeqCst).wrapping_add(1)
}

// ============================================================
// Varint 编解码 (Python helpers.py 28-85)
// ============================================================

/// 将整数编码为 varint 字节序列。
///
/// 返回值为包含编码后字节的 `Vec<u8>`。
pub fn varint_encode(value: i64) -> Vec<u8> {
    let mut buf = Vec::new();
    let sign = value < 0;
    let mut abs_value = value.unsigned_abs();

    // 第一个 6-bit 块
    let mut first_byte = (abs_value & 0x3F) as u8;
    abs_value >>= 6;

    // 符号位 (0x40) 和 延续位 (0x80)
    if sign {
        first_byte |= 0x40;
    }
    if abs_value != 0 {
        first_byte |= 0x80;
    }

    buf.push(first_byte);

    // 后续 7-bit 块
    while abs_value != 0 {
        let mut byte = (abs_value & 0x7F) as u8;
        abs_value >>= 7;
        if abs_value != 0 {
            byte |= 0x80;
        }
        buf.push(byte);
    }

    buf
}

/// 从 `data` 的位置 `pos` 解码一个 varint。
///
/// 返回 `(value, new_pos)`，其中 `new_pos` 是下一个未读取的索引位置。
pub fn varint_decode(data: &[u8], pos: usize) -> (i64, usize) {
    if pos >= data.len() {
        panic!("varint_decode: index out of range");
    }

    let mut p = pos;
    let byte = data[p];
    p += 1;

    let sign = (byte & 0x40) != 0;
    let mut value = (byte & 0x3F) as i64;
    let mut shift = 6u32;

    while byte & 0x80 != 0 {
        if p >= data.len() {
            panic!("varint_decode: index out of range");
        }
        let byte = data[p];
        p += 1;
        value |= ((byte & 0x7F) as i64) << shift;
        shift += 7;
    }

    if sign {
        value = -value;
    }
    (value, p)
}

// ============================================================
// 日期时间解析
// ============================================================

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

// ============================================================
// 整数转浮点价格 (C++ helpers::integerToFloat64)
// ============================================================

/// 将 32 位无符号整数解释并转换为浮点数（与 level1 协议中使用的转换一致）。
///
/// 该函数把输入分解成四个字节并依照协议的位权与指数规则重建浮点值。
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

// ============================================================
// 默认价格基数 (Python helpers.py default_base_unit)
// ============================================================

/// TDX 市场 ID 常量（本地定义，不依赖外部 exchange 模块）
pub const TDX_MARKET_SHENZHEN: i32 = 0;
pub const TDX_MARKET_SHANGHAI: i32 = 1;
pub const TDX_MARKET_BEIJING: i32 = 2;
pub const TDX_MARKET_HONGKONG: i32 = 31;
pub const TDX_MARKET_HKFE: i32 = 27;
pub const TDX_MARKET_USA: i32 = 74;

/// 获取价格计算所用的默认基数（单位）。
///
/// 参数：
///     market_id: 市场编号（例如 0=深市, 1=沪市）
///     code: 证券代码
///
/// 返回：基数（`100.0` 或 `1000.0`）。
pub fn default_base_unit(market_id: i32, code: &str) -> f64 {
    let s = code.trim();
    if s.is_empty() {
        return 100.0;
    }

    // market_id: 1=ShangHai, 0=ShenZhen
    // 沪市 5 开头（ETF/基金）→ 1000
    if market_id == TDX_MARKET_SHANGHAI && s.as_bytes()[0] == b'5' {
        return 1000.0;
    }
    // 深市 159 开头（ETF）→ 1000
    if market_id == TDX_MARKET_SHENZHEN && s.starts_with("159") {
        return 1000.0;
    }

    100.0
}

// ============================================================
// 交易所 ↔ TDX market_id 映射 (Python helpers.py 211-230)
// ============================================================

use std::collections::HashMap;
use once_cell::sync::Lazy;

static EXCHANGE_TO_MARKET: Lazy<HashMap<&'static str, i32>> = Lazy::new(|| {
    let mut m = HashMap::new();
    // 标准行情
    m.insert("SSE", TDX_MARKET_SHANGHAI);   // 上海证券交易所
    m.insert("SZSE", TDX_MARKET_SHENZHEN);  // 深圳证券交易所
    m.insert("BSE", TDX_MARKET_BEIJING);    // 北京证券交易所
    // 扩展行情
    m.insert("HKEX", TDX_MARKET_HONGKONG);  // 香港交易所
    m.insert("HKFE", TDX_MARKET_HKFE);      // 香港期货交易所
    m.insert("USA", TDX_MARKET_USA);        // 美国市场
    m
});

static MARKET_TO_EXCHANGE: Lazy<HashMap<i32, &'static str>> = Lazy::new(|| {
    let mut m = HashMap::new();
    m.insert(TDX_MARKET_SHANGHAI, "SSE");
    m.insert(TDX_MARKET_SHENZHEN, "SZSE");
    m.insert(TDX_MARKET_BEIJING, "BSE");
    m.insert(TDX_MARKET_HONGKONG, "HKEX");
    m.insert(TDX_MARKET_HKFE, "HKFE");
    m.insert(TDX_MARKET_USA, "USA");
    m
});

/// 将交易所标识符转换为 TDX 市场编号
pub fn exchange_to_market(exchange: &str) -> Option<i32> {
    EXCHANGE_TO_MARKET.get(exchange).copied()
}

/// 将 TDX 市场编号转换为交易所标识符
pub fn market_to_exchange(market_id: i32) -> Option<&'static str> {
    MARKET_TO_EXCHANGE.get(&market_id).copied()
}

// ============================================================
// 测试
// ============================================================

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_msg_sequence_id() {
        let a = msg_sequence_id();
        let b = msg_sequence_id();
        assert!(a > 0);
        assert!(b > a);
    }

    #[test]
    fn test_varint_roundtrip() {
        let test_values = vec![0i64, 1, -1, 127, -127, 128, 16383, -16383, 16384, 100000, -100000, i32::MAX as i64, i32::MIN as i64];
        for v in test_values {
            let encoded = varint_encode(v);
            let (decoded, pos) = varint_decode(&encoded, 0);
            assert_eq!(decoded, v, "varint roundtrip failed for {}", v);
            assert_eq!(pos, encoded.len());
        }
    }

    #[test]
    fn test_get_datetime_from_u32() {
        // category < 4: zipday = 0x2A5F (year=2004 + 5=2009, month=2, day=31), tminutes=570 (9:30)
        let (y, m, d, h, min) = get_datetime_from_u32(0, 0x2A5F, 570);
        assert_eq!(y, 2009);
        assert_eq!(m, 2);
        assert_eq!(d, 31);
        assert_eq!(h, 9);
        assert_eq!(min, 30);

        // category >= 4: zipday = 20250610
        let (y, m, d, h, min) = get_datetime_from_u32(4, 20250610, 0);
        assert_eq!(y, 2025);
        assert_eq!(m, 6);
        assert_eq!(d, 10);
        assert_eq!(h, 15);
        assert_eq!(min, 0);
    }

    #[test]
    fn test_int_to_float64() {
        assert_eq!(int_to_float64(0), 0.0);
        // 一个简单已知值的验证
        let v = int_to_float64(0x41A00000); // just a smoke test
        assert!(v > 0.0);
    }

    #[test]
    fn test_default_base_unit() {
        // 沪市 5 开头 → 1000
        assert_eq!(default_base_unit(TDX_MARKET_SHANGHAI, "510050"), 1000.0);
        // 深市 159 开头 → 1000
        assert_eq!(default_base_unit(TDX_MARKET_SHENZHEN, "159915"), 1000.0);
        // 普通股票 → 100
        assert_eq!(default_base_unit(TDX_MARKET_SHANGHAI, "600000"), 100.0);
        assert_eq!(default_base_unit(TDX_MARKET_SHENZHEN, "000001"), 100.0);
        // 空字符串
        assert_eq!(default_base_unit(TDX_MARKET_SHENZHEN, ""), 100.0);
    }

    #[test]
    fn test_exchange_market_mapping() {
        assert_eq!(exchange_to_market("SSE"), Some(TDX_MARKET_SHANGHAI));
        assert_eq!(exchange_to_market("SZSE"), Some(TDX_MARKET_SHENZHEN));
        assert_eq!(exchange_to_market("HKEX"), Some(TDX_MARKET_HONGKONG));
        assert_eq!(exchange_to_market("UNKNOWN"), None);

        assert_eq!(market_to_exchange(TDX_MARKET_SHANGHAI), Some("SSE"));
        assert_eq!(market_to_exchange(TDX_MARKET_HONGKONG), Some("HKEX"));
        assert_eq!(market_to_exchange(99), None);
    }
}
