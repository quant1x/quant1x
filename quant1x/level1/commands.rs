// Centralized Level1 standard command constants ported from C++ `StdCommand`.
// Keep names identical to C++ for easy cross-reference.
#![allow(non_upper_case_globals)]
pub const HEARTBEAT: u16 = 0x0004;
pub const LOGIN1: u16 = 0x000d;
pub const LOGIN2: u16 = 0x0fdb;
pub const XDXR_INFO: u16 = 0x000f;
pub const FINANCE_INFO: u16 = 0x0010;
pub const PING: u16 = 0x0015;
pub const COMPANY_CATEGORY: u16 = 0x02cf;
pub const COMPANY_CONTENT: u16 = 0x02d0;
pub const SECURITY_COUNT: u16 = 0x044e;
pub const SECURITY_LIST: u16 = 0x0450;
pub const INDEX_BARS: u16 = 0x052d;
pub const SECURITY_BARS: u16 = 0x052d; // same numeric value as INDEX_BARS in C++
pub const SECURITY_QUOTES_OLD: u16 = 0x053e;
pub const SECURITY_QUOTES_NEW: u16 = 0x054c;
pub const MINUTE_TIME_DATA: u16 = 0x051d;
pub const BLOCK_META: u16 = 0x02c5;
pub const BLOCK_DATA: u16 = 0x06b9;
pub const TRANSACTION_DATA: u16 = 0x0fc5;
pub const HISTORY_MINUTE_DATA: u16 = 0x0fb4;
pub const HISTORY_TRANSACTION_DATA: u16 = 0x0fb5;

// Convenience re-exports using Rust-style UPPER_SNAKE_CASE already applied above.
// Consumers should `use crate::level1::commands::*` or import individual constants.
