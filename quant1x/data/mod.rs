pub mod meta;
pub use meta::*;

pub mod market;
pub use market::*;

//pub mod exchange;
//pub use exchange::*;

pub mod adapter;
pub use adapter::*;

pub mod cache;
pub use cache::*;

mod base;
pub use base::*;

pub mod schema;
pub use schema::*;

pub mod datasource;
pub use datasource::*;


pub mod status;
pub use status::*;

// Reusable macro: apply_forward_adjustment_for_event!
// This macro implements the forward-adjustment algorithm used by both day KLine and minute KLine
// datasets. It operates on a mutable collection (slice or Vec) whose elements expose the same
// fields: `date`, `open`, `close`, `high`, `low`, `volume`, `amount`, `adjustment_count`.
// Usage (examples):
// apply_forward_adjustment_for_event!(klines_slice, start_date, &dividends);
#[macro_export]
macro_rules! apply_forward_adjustment_for_event {
    ($klines:expr, $start_date:expr, $dividends:expr) => {{
        if $klines.is_empty() {
            // nothing to do
        } else {
            let last_day = $klines.last().unwrap().date.clone();
            let ts_last_day = crate::data::meta::Timestamp::parse(&last_day).unwrap_or(crate::data::meta::Timestamp::now());
            let ts_last_day =
                crate::data::meta::Timestamp::pre_market_time_from_current(&ts_last_day).unwrap_or(ts_last_day);
            let last_day_next = crate::data::meta::calendar::next_trading_day(ts_last_day).only_date();
            let start_date_only = $start_date.only_date();

            let xdxr_infos: Vec<crate::contrib::data::tdx::standard::XdxrInfo> = $dividends
                .iter()
                .filter(|x| {
                    if x.category as i32 != 1 {
                        return false;
                    }
                    if let Ok(dts) = crate::data::meta::Timestamp::parse(&x.date) {
                        return last_day_next >= dts.only_date();
                    }
                    false
                })
                .cloned()
                .collect();
            let mut _times = xdxr_infos.len();
            for info in xdxr_infos.iter() {
                if info.date <= start_date_only {
                    // skip
                } else {
                    let (m, a) = info.adjust_factor();
                    let share_ratio = info.share_ratio_factor();
                    let klines_size = $klines.len();
                    for i in 0..klines_size {
                        if $klines[i].date >= info.date {
                            break;
                        }
                        $klines[i].open = $klines[i].open * m + a;
                        $klines[i].close = $klines[i].close * m + a;
                        $klines[i].high = $klines[i].high * m + a;
                        $klines[i].low = $klines[i].low * m + a;

                        let ap = if $klines[i].volume != 0.0 {
                            $klines[i].amount / $klines[i].volume
                        } else {
                            0.0
                        };
                        let ap_adjusted = ap * m + a;
                        $klines[i].volume *= 1.0 + share_ratio;
                        $klines[i].amount = $klines[i].volume * ap_adjusted;
                        $klines[i].adjustment_count += 1;
                    }
                }
                _times -= 1;
            }
        }
    }};
}

// Initialize data adapters that require registration. This replaces the old
// `datasets::init()` entrypoint — call `crate::data::init()` to register
// data adapters implemented in submodules.
pub fn init() {
    // register xdxr, day kline, minute kline, transaction adapters
    // ignore failures — each sub-init handles its own logging
    //xdxr::init();
    crate::contrib::data::tdx::kline::init();
    //kline_minute::init();
    //trans::init();
}