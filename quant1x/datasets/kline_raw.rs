use crate::level1;

// 日线最小容错回溯(偏移)天数
pub const MAX_KLINE_LOOKBACK_DAYS: usize = 1;
pub const CN_DEFAULT_TOTALFZNUM: i32 = 240; // A股默认全天交易240分钟

mod detail {
    use crate::level1;

    /// 拉取K线数据
    /// 这个函数封装了level1的调用，保持datasets层与level1层的分离
    pub fn fetch_kline(
        code: &str,
        start: u16,
        count: u16,
        kline_type: level1::KLineType,
    ) -> Vec<level1::SecurityBar> {
        match level1::fetch_security_bars(code, kline_type as u16, 1, start as u32, count) {
            Some(response) => response.list,
            None => {
                log::warn!("[datasets::kline_raw] fetch_kline failed for {} start={} count={}", code, start, count);
                Vec::new()
            }
        }
    }
}

// 重新导出detail函数供其他模块使用
pub use detail::fetch_kline;