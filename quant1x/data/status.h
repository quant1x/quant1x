#pragma once
#ifndef QUANT1X_DATA_STATUS_H
#define QUANT1X_DATA_STATUS_H 1

/// data/status — 数据源无关的文件状态检查
/// 与 Python data/status.py 和 Rust data/status.rs 对齐

#include <quant1x/data/meta/timestamp.h>
#include <quant1x/io/file.h>
#include <filesystem>

namespace data {

/// 检查文件是否需要 (重新) 初始化
/// 规则:
///   - 文件不存在或为空 → true
///   - 文件修改时间早于今日盘前时间 → true (文件过期, 需刷新)
///   - 否则 → false (今日已生成)
///
/// 对齐 Python data/status.py should_initialize_file()
/// C++ 简化版: 不依赖完整交易日历 (待后续接入)
inline bool should_initialize_file(const std::string& fname) {
    if (!std::filesystem::exists(fname)) {
        return true;
    }
    if (std::filesystem::file_size(fname) == 0) {
        return true;
    }
    int64_t mod_ms = 0;
    try {
        mod_ms = io::last_modified_time(fname);
    } catch (const std::exception&) {
        return true; // cannot stat → treat as stale
    }
    if (mod_ms <= 0) return true;

    meta::Timestamp mod_time(mod_ms);
    meta::Timestamp pre_market = meta::Timestamp::now().pre_market_time();

    // 如果最后修改时间早于今日盘前, 文件已陈旧
    if (mod_time < pre_market) {
        return true;
    }
    return false;
}

} // namespace data

#endif // QUANT1X_DATA_STATUS_H
