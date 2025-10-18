use crate::level1;
use crate::level1::protocol::Response;

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

    match level1::client() {
        Ok(mut pooled) => {
            let mut resp = level1::SecurityBarsResponse::new_with(is_index, category);
            match level1::protocol::process(pooled.stream(), &mut req, &mut resp) {
                Ok(()) => {
                    if resp.list.is_empty() {
                        log::warn!(
                            "[datasets::kline_raw] empty response for {} start={} count={} cat={} zip={} unzip={} resp_count={}",
                            code,
                            start,
                            count,
                            category,
                            resp.header().zip_size,
                            resp.header().unzip_size,
                            resp.count
                        );
                    }
                    Some(resp)
                }
                Err(e) => {
                    log::error!(
                        "[datasets::kline_raw] process failed for {} start={} count={}: {}",
                        code,
                        start,
                        count,
                        e
                    );
                    None
                }
            }
        }
        Err(e) => {
            log::error!(
                "[datasets::kline_raw] failed to acquire level1 client for {}: {}",
                code,
                e
            );
            None
        }
    }
}
