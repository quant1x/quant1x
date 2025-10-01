// datasets module - Rust port of C++ datasets adapters
pub mod base;
pub mod kline;
pub mod kline_minute;
pub mod kline_raw;
pub mod trans;
pub mod xdxr;

// Re-export commonly used items
pub use base::*;
pub use kline_raw::*;

/// Initialize datasets and register adapters (Rust port of C++ datasets::init)
pub fn init() {
    // Register Rust-implemented dataset adapters
    // Note: C++ registers various adapters here, but Rust versions may not all be implemented yet
    kline::init();
    kline_minute::init();
    trans::init();
    xdxr::init();
}