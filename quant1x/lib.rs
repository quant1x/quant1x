#![allow(
    hidden_glob_reexports,
    ambiguous_glob_reexports,
    dead_code,
    unused_imports
)]
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

// Use mimalloc as the global allocator for improved allocation performance
use mimalloc::MiMalloc;

#[global_allocator]
static GLOBAL: MiMalloc = MiMalloc;

// Machine Learning module
pub mod learn;

// // calendar decoder module
// mod decoder;
// pub use crate::decoder::*;

// network module (mio-based connection pool)
mod io;
pub use crate::io::*;
mod base;
pub use crate::base::*;

// core module
pub mod core;

// runtime helpers (rust translation of runtime utilities)
pub mod runtime;

// pandas utilities (frequency parsing)
mod pandas;
pub use crate::pandas::*;

// encoding helpers (GBK/UTF-8 detection and decode)
mod encoding;
pub use crate::encoding::*;

// ringbuffer module (MPMC ring buffer)
//pub mod ringbuffer;
//pub use crate::ringbuffer::*;

// crate-level internal configuration module (used by exchange/calendar and tests)
pub mod config;
// small application-level shims for main.rs to call; these are no-op fallbacks
pub mod app;
pub use crate::app::*;

pub mod factors;
pub use crate::factors::*;
// cache adapter module (port of C++ engine::adapter)
pub mod data;
pub use crate::data::*;
// datasets adapters (Rust ports)
// datasets 已并入 data 模块, 移除旧的 datasets 顶级导出
pub mod contrib;
pub use crate::contrib::*;
// distributed ID generator (port of Go quant1x/distributed/id)
pub mod distributed;
pub use crate::distributed::*;

/// Return the filename used for the calendar cache (convenience wrapper around
/// the internal config helper). This is intentionally a tiny, stable API so
/// external tests and tools can locate the calendar cache without exposing the
/// full `config` module.
pub fn get_calendar_filename() -> String {
    config::get_calendar_filename()
}

/// Public wrapper to return the meta directory path (eg. ~/.q1x-rs/meta)
pub fn get_meta_path() -> String {
    config::get_meta_path()
}

/// Public wrapper to return the securities CSV filename under meta
pub fn get_security_filename() -> String {
    config::get_security_filename()
}

/// Public wrapper to return the xdxr cache directory
pub fn get_xdxr_path() -> String {
    config::get_xdxr_path()
}

/// Public wrapper for default home path
pub fn default_home_path() -> String {
    config::default_home_path()
}

/// Public wrapper for default cache path
pub fn default_cache_path() -> String {
    config::default_cache_path()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_timestamp_integration() {
        let ts = crate::data::meta::Timestamp::now();
        assert!(ts.value() > 0);

        let formatted = ts.to_string();
        assert!(!formatted.is_empty());
    }
}
