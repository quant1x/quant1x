//! Quant1X Standard Library for Rust
//! 
//! This library provides cross-language compatible utilities for quantitative trading,
//! with implementations in C++, Go, and Rust that maintain API consistency.
//!
//! # 命名空间设计
//! 
//! - **Crate名称**: `quant1x-std`
//! - **命名空间**: `quant1x_std::`
//! - **使用方式**: `use quant1x_std::Timestamp;`
//!
//! # 示例
//! 
//! ```
//! use quant1x_std::Timestamp;
//! 
//! let ts = Timestamp::now();
//! let parsed = Timestamp::parse("2022-06-15 14:30:45").unwrap();
//! ```

// 直接导出 timestamp 模块的所有公共项 - 扁平化架构
pub use crate::timestamp::*;

// timestamp 模块，现在位于 src/ 根目录
mod timestamp;

pub fn add(left: u64, right: u64) -> u64 {
    left + right
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
