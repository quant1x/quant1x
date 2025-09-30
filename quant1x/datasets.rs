use std::sync::Arc;
mod base;
pub use crate::datasets::base::*;
mod kline;
mod kline_minute;
mod trans;
mod xdxr;

pub use crate::datasets::kline::*;
pub use crate::datasets::xdxr::*;

pub fn init() {
    // register per-dataset adapters
    crate::datasets::xdxr::init();
    // Only register minute-kline adapter when enabled in configuration (mirror C++ behavior)
    let mkc = crate::config::get_minute_kline_config();
    if mkc.enabled {
        crate::datasets::kline_minute::init();
    }
    crate::datasets::kline::init();
    crate::datasets::trans::init();
}
