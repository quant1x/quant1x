use std::sync::Arc;
mod kline;
mod xdxr;

pub use crate::datasets::kline::*;
pub use crate::datasets::xdxr::*;

pub fn init() {
    // register per-dataset adapters
    crate::datasets::xdxr::init();
    crate::datasets::kline::init();
}
