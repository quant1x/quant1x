#pragma once
#ifndef QUANT1X_DATA_STATUS_H
#define QUANT1X_DATA_STATUS_H 1

/// data/status — 数据源无关的文件状态检查
/// 与 Python data/status.py 和 Rust data/status.rs 对齐
///
/// Python status.py 调用 cache.get_filename_modified_time() + session.can_initialize()/check_trading_timestamp()
/// C++ 同样调用 data::get_filename_modified_time() + meta::can_initialize()/meta::check_trading_timestamp()

#include <quant1x/data/cache.h>
#include <quant1x/data/meta/session.h>

namespace data {

/// 检查是否应该初始化文件, 基于文件修改时间和交易所交易时段
///
/// 对齐 Python status.should_initialize_file()
/// - 获取文件修改时间失败 → true
/// - 调用 meta::can_initialize() 判断
///
/// @param fname 文件路径
/// @param exchange 交易所, 默认 SSE
/// @return true 需要初始化, false 不需要
inline bool should_initialize_file(const std::string& fname, meta::Exchange exchange = meta::Exchange::SSE) {
    meta::Timestamp mod_time = get_filename_modified_time(fname);
    if (mod_time == meta::Timestamp::zero()) {
        return true;
    }
    return meta::can_initialize(exchange, mod_time);
}

/// 检查文件是否需要更新, 基于文件修改时间和交易所交易时间
///
/// 对齐 Python status.should_update_file()
/// - 获取文件修改时间失败 → true
/// - 调用 meta::check_trading_timestamp() 判断 update_in_real_time
///
/// @param fname 文件路径
/// @param exchange 交易所, 默认 SSE
/// @return true 需要更新, false 不需要
inline bool should_update_file(const std::string& fname, meta::Exchange exchange = meta::Exchange::SSE) {
    meta::Timestamp mod_time = get_filename_modified_time(fname);
    if (mod_time == meta::Timestamp::zero()) {
        return true;
    }
    meta::RuntimeStatus rs = meta::check_trading_timestamp(exchange, mod_time);
    return rs.update_in_real_time;
}

} // namespace data

#endif // QUANT1X_DATA_STATUS_H
