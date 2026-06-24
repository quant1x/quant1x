use crate::data::cache;
use crate::data::meta::exchange::Exchange;
use crate::data::meta::session;
use crate::data::meta::timestamp::Timestamp;

/// 检查是否应该初始化文件, 对应 Python `status.should_initialize_file`. 
///
/// 基于文件修改时间和交易所交易时段判断. 
pub fn should_initialize_file(fname: &str, exchange: Exchange) -> bool {
    let mod_time = cache::get_filename_modified_time(fname);
    if mod_time == Timestamp::zero() {
        return true;
    }
    session::can_initialize(exchange, Some(mod_time))
}

/// 检查文件是否需要更新, 对应 Python `status.should_update_file`. 
///
/// 基于文件修改时间和交易所交易时间判断. 
pub fn should_update_file(fname: &str, exchange: Exchange) -> bool {
    let mod_time = cache::get_filename_modified_time(fname);
    if mod_time == Timestamp::zero() {
        return true;
    }
    let rs = session::check_trading_timestamp(exchange, Some(mod_time));
    rs.update_in_real_time
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_nonexistent_file_should_initialize() {
        assert!(should_initialize_file("/nonexistent/status_test", Exchange::SSE));
    }

    #[test]
    fn test_nonexistent_file_should_update() {
        assert!(should_update_file("/nonexistent/status_test", Exchange::SSE));
    }
}
