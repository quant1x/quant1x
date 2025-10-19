use crate::level1;
use crate::level1::protocol::Response;
use std::thread;
use std::time::Duration;

// 日线最小容错回溯(偏移)天数
pub const MAX_KLINE_LOOKBACK_DAYS: usize = 1;
pub const CN_DEFAULT_TOTALFZNUM: i32 = 240; // A股默认全天交易240分钟

pub fn fetch_kline(
    code: &str,
    start: u32,
    count: u16,
    kline_type: level1::KLineType,
) -> Option<level1::SecurityBarsResponse> {
    let category = kline_type as u16;
    let start_u16 = (start.min(u16::MAX as u32)) as u16;
    let frequency = 1u16;
    let mut req =
        level1::SecurityBarsRequest::with_frequency(code, category, start_u16, count, frequency);
    let is_index = req.is_index();

    const MAX_RETRIES: usize = 3;
    const RETRY_DELAY_MS: u64 = 1000;

    for attempt in 0..=MAX_RETRIES {
        match level1::client() {
            Ok(mut pooled) => {
                let endpoint = pooled.addr();
                let mut resp = level1::SecurityBarsResponse::new_with(is_index, category);
                match level1::protocol::process(pooled.stream(), &mut req, &mut resp)
                    .map_err(|s| std::io::Error::new(std::io::ErrorKind::Other, s))
                {
                    Ok(()) => {
                            if resp.list.is_empty() {
                                log::warn!(
                                    "[datasets::kline_raw] empty response from {} for {} start={} count={} cat={} zip={} unzip={} resp_count={}",
                                    endpoint,
                                    code,
                                    start,
                                    count,
                                    category,
                                    resp.header().zip_size,
                                    resp.header().unzip_size,
                                    resp.count
                                );
                            }
                        return Some(resp);
                    }
                    Err(e) => {
                        // 检查是否是连接相关错误，如果是则重试
                        let is_connection_error = e.kind() == std::io::ErrorKind::TimedOut
                            || e.kind() == std::io::ErrorKind::ConnectionRefused
                            || e.kind() == std::io::ErrorKind::ConnectionReset
                            || e.kind() == std::io::ErrorKind::ConnectionAborted
                            || e.raw_os_error() == Some(10060)  // WSAETIMEDOUT
                            || e.raw_os_error() == Some(10061)  // WSAECONNREFUSED
                            || e.raw_os_error() == Some(10054); // WSAECONNRESET

                        if is_connection_error && attempt < MAX_RETRIES {
                            log::warn!(
                                "[datasets::kline_raw] connection error to {} for {} start={} count={} (attempt {}/{}): {}",
                                endpoint,
                                code,
                                start,
                                count,
                                attempt + 1,
                                MAX_RETRIES + 1,
                                e
                            );
                            thread::sleep(Duration::from_millis(RETRY_DELAY_MS));
                            continue;
                        } else {
                            log::error!(
                                "[datasets::kline_raw] process failed for {} from {} start={} count={}: {}",
                                code,
                                endpoint,
                                start,
                                count,
                                e
                            );
                            return None;
                        }
                    }
                }
            }
            Err(e) => {
                if attempt < MAX_RETRIES {
                    log::warn!(
                        "[datasets::kline_raw] failed to acquire level1 client for {} (attempt {}/{}): {}",
                        code,
                        attempt + 1,
                        MAX_RETRIES + 1,
                        e
                    );
                    thread::sleep(Duration::from_millis(RETRY_DELAY_MS));
                    continue;
                } else {
                    log::error!(
                        "[datasets::kline_raw] failed to acquire level1 client for {}: {}",
                        code,
                        e
                    );
                    return None;
                }
            }
        }
    }

    None
}
