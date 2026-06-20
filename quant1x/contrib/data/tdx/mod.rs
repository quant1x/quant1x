pub mod helpers;
pub use helpers::*;
pub mod command;
pub use command::*;
pub mod config;
//pub use config;
pub mod protocol;
pub use protocol::*;
pub mod client;
pub use client::*;
mod market;
pub mod level1;
pub mod datasource;
pub use datasource::*;
pub mod instruments;
pub use instruments::*;
pub mod bar_raw;
pub use bar_raw::*;
pub mod bar;
pub use bar::*;
pub mod trans;
pub use trans::*;
pub mod sector;
pub use sector::*;
pub mod xdxr;
pub use xdxr::*;

pub fn init() {
    // register xdxr, day kline, minute kline, transaction adapters
    // ignore failures — each sub-init handles its own logging
    xdxr::init();
    bar::init();
    //kline_minute::init();
    //trans::init();
}