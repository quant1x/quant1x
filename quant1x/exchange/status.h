#pragma once
#ifndef QUANT1X_EXCHANGE_STATUS_H
#define QUANT1X_EXCHANGE_STATUS_H 1

#include <optional>
#include <string>

#include "timestamp.h"

namespace exchange {

    // 获取指定文件的最后修改时间，失败返回 std::nullopt
    std::optional<timestamp> get_filename_modified_time(const std::string &fname);

    // 检查指定文件是否需要更新
    // - 如果获取文件修改时间失败，默认返回 true
    bool should_update_file(const std::string &fname);

} // namespace exchange

#endif // QUANT1X_EXCHANGE_STATUS_H
