use crate::level1::protocol;
use crate::level1::transaction_data::{TickTransaction, TransactionRequest, TransactionResponse};
use crate::level1::{self};
use crate::Timestamp;
use std::sync::Arc;


const HISTORICAL_TRANSACTION_FIRST_TIME: &str = "09:25";
const HISTORICAL_TRANSACTION_START_TIME: &str = "09:30";
const HISTORICAL_TRANSACTION_FINAL_BIDDING_TIME: &str = "14:57";
const HISTORICAL_TRANSACTION_LAST_TIME: &str = "15:00";

#[derive(Debug)]
pub struct DataTrans;

impl crate::data::Schema for DataTrans {
    fn kind(&self) -> crate::Kind {
        crate::data::BaseTransaction
    }
    fn owner(&self) -> String {
        crate::data::DEFAULT_DATA_PROVIDER.to_string()
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

impl crate::data::DataAdapter for DataTrans {
    fn print(&self, _code: &str, _dates: &[Timestamp]) {}

    fn update(&self, code: &str, date: Timestamp) {
        // 遵循 C++ CheckoutTransactionData 的行为：读取缓存并分页增量拉取
        let corrected = crate::data::market::correct_security_code(code);
        let mut path = std::path::PathBuf::from(crate::config::default_cache_path());
        path.push("trans");
        // 使用按年/日期目录的组织方式：trans/YYYY/YYYY-MM-DD/<code>.csv
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

        // 如果存在则尝试读取已有缓存(容错处理, 读取失败视为空)
        let mut list: Vec<TickTransaction> = Vec::new();
        if std::path::Path::new(&filename).exists() {
            // 尝试读取CSV；容错，读取失败视为空
            if let Ok(mut rdr) = csv::ReaderBuilder::new()
                .has_headers(true)
                .from_path(&filename)
            {
                for result in rdr.records() {
                    if let Ok(rec) = result {
                // 字段: time,price,volume,number,amount,buy_or_sell
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

        // 从缓存中计算起始时间（如果存在缓存）
        let mut start_time = HISTORICAL_TRANSACTION_FIRST_TIME.to_string();
        if !list.is_empty() {
            if let Some(last) = list.last() {
                if last.time == HISTORICAL_TRANSACTION_LAST_TIME {
                    return; // 已更新
                }
            }

            // 查找需要追加的最早时间（镜像C++逻辑）
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

        fn fetch_transaction_page(
            security_code: &str,
            start: u16,
            count: u16,
        ) -> Option<TransactionResponse> {
            match crate::level1::get_std_conn() {
                Ok(mut pooled) => {
                    let mut request = TransactionRequest::new(security_code, start, count);
                    let mut response = TransactionResponse::new_from_request(&request);
                    match protocol::process(pooled.stream(), &mut request, &mut response) {
                        Ok(_) => Some(response),
                        Err(e) => {
                            log::error!(
                                "level1 protocol::process error for transaction_data {}: {}",
                                security_code,
                                e
                            );
                            None
                        }
                    }
                }
                Err(e) => {
                    log::error!(
                        "failed to acquire level1 client for transaction_data {}: {}",
                        security_code,
                        e
                    );
                    None
                }
            }
        }

        if today_is_last {
            // 拉取当日实时成交分页数据
            loop {
                match fetch_transaction_page(&corrected, start, level1::transaction_data::TICK_TRANSACTION_PER_REQUEST_MAX) {
                    Some(mut resp) => {
                        if resp.count == 0 || resp.list.is_empty() {
                            break;
                        }
                        // C++反转每页并按start_time过滤
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
                        if (size as u16) < level1::transaction_data::TICK_TRANSACTION_PER_REQUEST_MAX {
                            break;
                        }
                        start = start.wrapping_add(level1::transaction_data::TICK_TRANSACTION_PER_REQUEST_MAX);
                    }
                    None => {
                        break;
                    }
                }
            }
        } else {
            // 获取历史成交数据页
            // 拉取历史成交分页数据
            loop {
                match crate::level1::transaction_history::fetch_history_transactions(
                    &corrected,
                    date.yyyymmdd(),
                    start,
                    level1::transaction_data::TICK_TRANSACTION_PER_REQUEST_MAX,
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
                        if (size as u16) < level1::transaction_data::TICK_TRANSACTION_PER_REQUEST_MAX {
                            break;
                        }
                        start = start.wrapping_add(level1::transaction_data::TICK_TRANSACTION_PER_REQUEST_MAX);
                    }
                    None => {
                        break;
                    }
                }
            }
        }

        // 反转页面并展平
        hs.reverse();
        for v in hs.into_iter() {
            history.extend(v.into_iter());
        }

        if history.is_empty() {
            return;
        }

        list.extend(history.into_iter());

        // 写回CSV
        let tmp = format!("{}.tmp", filename);
        match std::fs::File::create(&tmp) {
            Ok(f) => {
                let mut w = csv::WriterBuilder::new().has_headers(true).from_writer(f);
                // 表头
                let header = vec!["time", "price", "volume", "number", "amount", "buy_or_sell"];
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

/// Return the full filename for a transaction cache file for `code` and `date`.
/// The date should be in "YYYYMMDD" or "YYYY-MM-DD" format.
pub fn get_trans_filepath(code: &str, date: &str) -> String {
    let corrected = crate::data::market::correct_security_code(code);
    let mut path = std::path::PathBuf::from(crate::config::default_cache_path());
    path.push("trans");
    let date_str = date.replace("-", "");
    let year = if date_str.len() >= 4 {
        &date_str[..4]
    } else {
        "0000"
    };
    path.push(year);
    path.push(&date_str);
    path.push(format!("{}.csv", corrected));
    path.to_string_lossy().to_string()
}

pub fn init() {
    let plugin = Arc::new(DataTrans) as Arc<dyn crate::data::DataAdapter>;
    crate::data::register(plugin);
}
