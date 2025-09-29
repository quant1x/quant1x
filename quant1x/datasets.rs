use std::sync::Arc;
mod kline;
mod xdxr;
mod trans;

pub use crate::datasets::kline::*;
pub use crate::datasets::xdxr::*;

pub fn init() {
    // register per-dataset adapters
    crate::datasets::xdxr::init();
    crate::datasets::kline::init();
    crate::datasets::trans::init();
}
