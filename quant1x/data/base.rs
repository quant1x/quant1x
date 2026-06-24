// Auto-port of C++ datasets/base.h kinds for Rust
// Provide UPPER_SNAKE_CASE constants (Rust style) and keep the original
// CamelCase names as backward-compatible aliases to ease review and porting.
#![allow(non_upper_case_globals)]
pub const BASE_KIND: crate::data::Kind = 0;

// Uppercase (Rust style)
pub const BASE_XDXR: crate::data::Kind = crate::data::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 1);
pub const BASE_RAW_DAILY_KLINE: crate::data::Kind = crate::data::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 2);
pub const BASE_KLINE: crate::data::Kind = crate::data::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 3);
pub const BASE_TRANSACTION: crate::data::Kind =
    crate::data::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 4);
pub const BASE_MINUTES: crate::data::Kind = crate::data::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 5);
pub const BASE_QUARTERLY_REPORTS: crate::data::Kind =
    crate::data::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 6);
pub const BASE_SAFETY_SCORE: crate::data::Kind =
    crate::data::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 7);
pub const BASE_WIDE_KLINE: crate::data::Kind =
    crate::data::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 8);
pub const BASE_PERFORMANCE_FORECAST: crate::data::Kind =
    crate::data::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 9);
pub const BASE_CHIP_DISTRIBUTION: crate::data::Kind =
    crate::data::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 10);
pub const BASE_MINUTE_KLINE: crate::data::Kind =
    crate::data::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 11);
    
// Backward-compatible CamelCase aliases (preserve original export names)
pub const BaseXdxr: crate::data::Kind = BASE_XDXR;
pub const BaseRawDailyKLine: crate::data::Kind = BASE_RAW_DAILY_KLINE;
pub const BaseKLine: crate::data::Kind = BASE_KLINE;
pub const BaseTransaction: crate::data::Kind = BASE_TRANSACTION;
pub const BaseMinutes: crate::data::Kind = BASE_MINUTES;
pub const BaseQuarterlyReports: crate::data::Kind = BASE_QUARTERLY_REPORTS;
pub const BaseSafetyScore: crate::data::Kind = BASE_SAFETY_SCORE;
pub const BaseWideKLine: crate::data::Kind = BASE_WIDE_KLINE;
pub const BasePerformanceForecast: crate::data::Kind = BASE_PERFORMANCE_FORECAST;
pub const BaseChipDistribution: crate::data::Kind = BASE_CHIP_DISTRIBUTION;
pub const BaseMinuteKLine: crate::data::Kind = BASE_MINUTE_KLINE;