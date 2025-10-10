// Auto-port of C++ datasets/base.h kinds for Rust
// Provide UPPER_SNAKE_CASE constants (Rust style) and keep the original
// CamelCase names as backward-compatible aliases to ease review and porting.
#![allow(non_upper_case_globals)]
pub const BASE_KIND: crate::cache::Kind = 0;

// Uppercase (Rust style)
pub const BASE_XDXR: crate::cache::Kind = crate::cache::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 1);
pub const RAW_KLINE: crate::cache::Kind = crate::cache::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 2);
pub const BASE_KLINE: crate::cache::Kind = crate::cache::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 3);
pub const BASE_TRANSACTION: crate::cache::Kind =
    crate::cache::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 4);
pub const BASE_MINUTES: crate::cache::Kind = crate::cache::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 5);
pub const BASE_QUARTERLY_REPORTS: crate::cache::Kind =
    crate::cache::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 6);
pub const BASE_SAFETY_SCORE: crate::cache::Kind =
    crate::cache::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 7);
pub const BASE_WIDE_KLINE: crate::cache::Kind =
    crate::cache::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 8);
pub const BASE_PERFORMANCE_FORECAST: crate::cache::Kind =
    crate::cache::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 9);
pub const BASE_CHIP_DISTRIBUTION: crate::cache::Kind =
    crate::cache::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 10);
pub const BASE_MINUTE_KLINE: crate::cache::Kind =
    crate::cache::PLUGIN_MASK_BASE_DATA | (BASE_KIND + 11);

// Backward-compatible CamelCase aliases (preserve original export names)
pub const BaseXdxr: crate::cache::Kind = BASE_XDXR;
pub const RawKLine: crate::cache::Kind = RAW_KLINE;
pub const BaseKLine: crate::cache::Kind = BASE_KLINE;
pub const BaseTransaction: crate::cache::Kind = BASE_TRANSACTION;
pub const BaseMinutes: crate::cache::Kind = BASE_MINUTES;
pub const BaseQuarterlyReports: crate::cache::Kind = BASE_QUARTERLY_REPORTS;
pub const BaseSafetyScore: crate::cache::Kind = BASE_SAFETY_SCORE;
pub const BaseWideKLine: crate::cache::Kind = BASE_WIDE_KLINE;
pub const BasePerformanceForecast: crate::cache::Kind = BASE_PERFORMANCE_FORECAST;
pub const BaseChipDistribution: crate::cache::Kind = BASE_CHIP_DISTRIBUTION;
pub const BaseMinuteKLine: crate::cache::Kind = BASE_MINUTE_KLINE;
