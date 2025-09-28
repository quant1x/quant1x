//! Quant1X Standard Library for Rust
//! 
//! This library provides cross-language compatible utilities for quantitative trading,
//! with implementations in C++, Go, and Rust that maintain API consistency.
//!
//! # 命名空间设计
//! 
//! - **Crate名称**: `quant1x`
//! - **命名空间**: `quant1x::`
//! - **使用方式**: `use quant1x::Timestamp;`
//!
//! # 示例
//! 
//! ```
//! use quant1x::Timestamp;
//! 
//! let ts = Timestamp::now();
//! let parsed = Timestamp::parse("2022-06-15 14:30:45").unwrap();
//! ```

// 直接导出 timestamp 模块的所有公共项 - 扁平化架构
pub use crate::timestamp::*;

// timestamp 模块，现在位于 src/ 根目录
mod timestamp;

// FP-Growth 模块
mod fpgrowth;
pub use crate::fpgrowth::*;

// calendar decoder module
mod decoder;
pub use crate::decoder::*;

// network module (mio-based connection pool)
mod net;
pub use crate::net::*;
mod std;
pub use crate::std::*;

// runtime helpers (rust translation of runtime utilities)
mod runtime;
pub use crate::runtime::*;

// crate-level internal configuration module (used by exchange/calendar and tests)
mod config;

// Level1 protocol bindings (partial, header-only equivalents)
mod level1;
pub use crate::level1::*;
mod exchange;
pub use crate::exchange::*;
// top-level `config` module removed — `level1::config` remains internal to level1.

pub fn add(left: u64, right: u64) -> u64 {
    left + right
}

/// Return the filename used for the calendar cache (convenience wrapper around
/// the internal config helper). This is intentionally a tiny, stable API so
/// external tests and tools can locate the calendar cache without exposing the
/// full `config` module.
pub fn get_calendar_filename() -> String {
    config::get_calendar_filename()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        let result = add(2, 2);
        assert_eq!(result, 4);
    }

    #[test]
    fn test_timestamp_integration() {
        let ts = Timestamp::now();
        assert!(ts.value() > 0);
        
        let formatted = ts.to_string();
        assert!(!formatted.is_empty());
    }
}
