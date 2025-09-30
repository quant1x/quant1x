use crate::cache::{self, DataAdapter, Kind};
use crate::level1::transaction_data::TickTransaction;
use crate::level1::{self};
use crate::Timestamp;
use std::sync::Arc;

const OFFSET: u16 = 1800; // level1::tick_transaction_max

const HISTORICAL_TRANSACTION_FIRST_TIME: &str = "09:25";
const HISTORICAL_TRANSACTION_START_TIME: &str = "09:30";
const HISTORICAL_TRANSACTION_FINAL_BIDDING_TIME: &str = "14:57";
const HISTORICAL_TRANSACTION_LAST_TIME: &str = "15:00";

#[derive(Debug)]
pub struct DataTrans;

impl cache::Schema for DataTrans {
    fn kind(&self) -> Kind {
        crate::datasets::BaseTransaction
    }
    fn owner(&self) -> String {
        crate::cache::DEFAULT_DATA_PROVIDER.to_string()
    }
    fn key(&self) -> String {
        "trans".to_string()
    }
    fn name(&self) -> String {
        "历史成交".to_string()
    }
    fn usage(&self) -> String {
        "历史成交".to_string()
    }
}

impl DataAdapter for DataTrans {
    fn print(&self, _code: &str, _dates: &[Timestamp]) {}

    fn update(&self, code: &str, date: Timestamp) {
        // Follow C++ CheckoutTransactionData behavior: read cache, fetch incremental pages
        let corrected = crate::exchange::correct_security_code(code);
        let mut path = std::path::PathBuf::from(crate::config::default_cache_path());
        path.push("trans");
        // use directory per year and date: trans/YYYY/YYYY-MM-DD/<code>.csv
        let date_str = date.only_date();
        let year = if date_str.len() >= 4 {
            &date_str[..4]
        } else {
            "0000"
        };
        path.push(year);
        path.push(format!("{}", date_str));
        if let Err(e) = std::fs::create_dir_all(&path) {
            log::error!("[DataTrans] create_dir_all failed: {}", e);
            return;
        }
        path.push(format!("{}.csv", corrected));
        let filename = path.to_string_lossy().to_string();

        // read existing cache if present
        let mut list: Vec<TickTransaction> = Vec::new();
        if std::path::Path::new(&filename).exists() {
            // try to read CSV; tolerate errors and treat as empty
            if let Ok(mut rdr) = csv::ReaderBuilder::new()
                .has_headers(true)
                .from_path(&filename)
            {
                for result in rdr.records() {
                    if let Ok(rec) = result {
                        // fields: time,price,vol,num,amount,buyOrSell
                        let time = rec.get(0).unwrap_or("").to_string();
                        let price = rec
                            .get(1)
                            .and_then(|s| s.parse::<f64>().ok())
                            .unwrap_or(0.0);
                        let vol = rec.get(2).and_then(|s| s.parse::<i64>().ok()).unwrap_or(0);
                        let num = rec.get(3).and_then(|s| s.parse::<i64>().ok()).unwrap_or(0);
                        let amount = rec
                            .get(4)
                            .and_then(|s| s.parse::<f64>().ok())
                            .unwrap_or(0.0);
                        let buy_or_sell =
                            rec.get(5).and_then(|s| s.parse::<i64>().ok()).unwrap_or(0);
                        list.push(TickTransaction {
                            time,
                            price,
                            vol,
                            num,
                            amount,
                            buy_or_sell,
                        });
                    }
                }
            }
        }

        // compute startTime from cache (if any)
        let mut start_time = HISTORICAL_TRANSACTION_FIRST_TIME.to_string();
        if !list.is_empty() {
            if let Some(last) = list.last() {
                if last.time == HISTORICAL_TRANSACTION_LAST_TIME {
                    return; // up-to-date
                }
            }

            // find earliest time after which we need to append (mirror C++ logic)
            let mut first_time = String::new();
            let mut skip_count: usize = 0;
            let cache_len = list.len();
            for i in 1..=cache_len {
                let tm = &list[cache_len - i].time;
                if first_time.is_empty() {
                    first_time = tm.clone();
                    start_time = first_time.clone();
                    skip_count += 1;
                    continue;
                }
                if tm < &first_time {
                    start_time = first_time.clone();
                    break;
                } else {
                    skip_count += 1;
                }
            }
            list.truncate(cache_len - skip_count);
        }

        let today_is_last =
            date.is_same_date(&crate::exchange::last_trading_day(crate::Timestamp::now()));

        let mut start: u16 = 0;
        let mut history: Vec<TickTransaction> = Vec::new();
        let mut hs: Vec<Vec<TickTransaction>> = Vec::new();

        if today_is_last {
            // fetch TRANSACTION_DATA pages
            loop {
                match level1::transaction_data::fetch_transaction_data(&corrected, start, OFFSET) {
                    Some(mut resp) => {
                        if resp.count == 0 || resp.list.is_empty() {
                            break;
                        }
                        // C++ reverses each page and filters by start_time
                        let mut tmp: Vec<TickTransaction> = Vec::new();
                        resp.list.reverse();
                        for td in resp.list.into_iter() {
                            if td.time >= start_time {
                                tmp.push(td);
                            }
                        }
                        tmp.reverse();
                        let size = tmp.len();
                        if size > 0 {
                            hs.push(tmp);
                        }
                        if (size as u16) < OFFSET {
                            break;
                        }
                        start = start.wrapping_add(OFFSET);
                    }
                    None => {
                        break;
                    }
                }
            }
        } else {
            // fetch HISTORY_TRANSACTION_DATA pages
            loop {
                match crate::level1::transaction_history::fetch_history_transactions(
                    &corrected,
                    date.yyyymmdd(),
                    start,
                    OFFSET,
                ) {
                    Some(mut resp) => {
                        if resp.count == 0 || resp.list.is_empty() {
                            break;
                        }
                        let mut tmp: Vec<TickTransaction> = Vec::new();
                        resp.list.reverse();
                        for td in resp.list.into_iter() {
                            if td.time >= start_time {
                                tmp.push(td);
                            }
                        }
                        tmp.reverse();
                        let size = tmp.len();
                        if size > 0 {
                            hs.push(tmp);
                        }
                        if (size as u16) < OFFSET {
                            break;
                        }
                        start = start.wrapping_add(OFFSET);
                    }
                    None => {
                        break;
                    }
                }
            }
        }

        // reverse pages and flatten
        hs.reverse();
        for v in hs.into_iter() {
            history.extend(v.into_iter());
        }

        if history.is_empty() {
            return;
        }

        list.extend(history.into_iter());

        // write CSV back
        let tmp = format!("{}.tmp", filename);
        match std::fs::File::create(&tmp) {
            Ok(f) => {
                let mut w = csv::WriterBuilder::new().has_headers(true).from_writer(f);
                // header
                let header = vec!["time", "price", "vol", "num", "amount", "buyOrSell"];
                let _ = w.write_record(&header);
                for rec in &list {
                    let _ = w.write_record(&[
                        &rec.time,
                        &rec.price.to_string(),
                        &rec.vol.to_string(),
                        &rec.num.to_string(),
                        &rec.amount.to_string(),
                        &rec.buy_or_sell.to_string(),
                    ]);
                }
                let _ = w.flush();
                if let Err(e) = std::fs::rename(&tmp, &filename) {
                    log::error!("[DataTrans] rename failed {} -> {}: {}", tmp, filename, e);
                }
            }
            Err(e) => {
                log::error!("[DataTrans] create tmp {} failed: {}", tmp, e);
            }
        }
    }
}

pub fn init() {
    let plugin = Arc::new(DataTrans) as Arc<dyn DataAdapter>;
    crate::cache::register(plugin);
}
