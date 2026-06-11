#pragma once
#ifndef QUANT1X_EXCHANGE_STATUS_H
#define QUANT1X_EXCHANGE_STATUS_H 1

#include <optional>
#include <string>

#include "timestamp.h"

namespace exchange {

    /**
     * @brief 获取指定文件的最后修改时间
     *
     * @param fname 要查询的文件路径字符串
     * @return std::optional<timestamp> 如果文件存在则返回其最后修改时间，否则返回空值
     * @throws std::filesystem::filesystem_error 如果文件系统操作失败
     */
    std::optional<timestamp> get_filename_modified_time(const std::string &fname);

    /**
     * @brief 检查是否应该初始化指定文件
     *
     * 根据文件名判断该文件是否需要被初始化。通常用于判断配置文件或数据文件
     * 是否需要创建默认版本或执行初始化操作。
     *
     * @param fname 要检查的文件名（包含路径）
     * @return bool 如果文件需要初始化则返回true，否则返回false
     * @throws std::runtime_error 如果文件访问出现错误
     */
    bool should_initialize_file(const std::string &fname);

    /**
     * @brief 检查文件是否需要更新
     *
     * 根据给定的文件名判断该文件是否需要被更新
     *
     * @param fname 要检查的文件名
     * @return bool 如果文件需要更新则返回true，否则返回false
     */
    bool should_update_file(const std::string &fname);

}  // namespace exchange

#endif // QUANT1X_EXCHANGE_STATUS_H
