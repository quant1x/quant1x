#pragma once
#ifndef QUANT1X_STD_FILESYSTEM_H
#define QUANT1X_STD_FILESYSTEM_H

#include <chrono>
#include <cstdint>
#include <string>
#include <system_error>

#include "quant1x/std/api.h"

namespace filesystem {

    /**
     * @brief Get the current user's home directory
     *
     * Priority:
     * 1. QUANT1X_HOME environment variable
     * 2. GOX_HOME environment variable
     * 3. HOME environment variable
     * 4. USERPROFILE environment variable (Windows only)
     * 5. System temporary directory (fallback)
     *
     * @return std::string The home directory path
     */
    std::string homedir();

    /**
     * @brief Expand user home directory in path (e.g. "~/data" -> "/home/user/data")
     *
     * @param path The path to expand
     * @return std::string The expanded path
     */
    std::string expand_user(const std::string &path);

    std::error_code mkdirs(const std::string &path, bool notExistToCreate = true);

    std::error_code check_filepath(const std::string &path, bool notExistToCreate = false);

    /**
     * @brief 移除文件路径中的扩展名部分
     *
     * @param path_str 输入的文件路径字符串
     * @return std::string 返回移除扩展名后的路径字符串
     *
     * @note 如果路径中没有扩展名, 则返回原始路径
     * @note 扩展名定义为最后一个点号(.)之后的部分
     */
    std::string remove_extension(const std::string &path_str);

    /**
     * @brief 获取当前可执行文件的绝对路径
     *
     * @return std::string 返回当前可执行文件的完整绝对路径字符串
     *
     * @note 路径格式为平台相关, Windows下使用反斜杠分隔符
     * @note 如果获取路径失败, 将返回空字符串
     */
    std::string executable_absolute_path();

    /**
     * @brief 获取当前可执行文件的名称, 不包含扩展名
     *
     * @return std::string 返回当前可执行文件的完整名称字符串
     *
     * @note 该函数通常用于获取程序自身的名称, 用于日志记录或其他需要标识当前进程的场景
     */
    std::string executable_name();

    /**
     * @brief 获取指定文件的最后修改时间
     *
     * @param filename 要检查的文件路径
     * @return int64_t 返回文件的最后修改时间(Unix时间戳格式)
     * @throws std::runtime_error 如果无法获取文件信息时抛出异常
     */
    int64_t last_modified_time(const std::string &filename);
    /**
     * @brief 设置文件的最后修改时间
     *
     * @param filename 要设置修改时间的文件路径
     * @param milliseconds 要设置的修改时间, 以毫秒为单位的Unix时间戳
     * @throws std::runtime_error 如果文件操作失败时抛出异常
     */
    void last_modified_time(const std::string &filename, const int64_t &milliseconds);

    /**
     * @brief 将数据写入指定文件
     *
     * @param filename 目标文件路径
     * @param data 要写入的数据指针, 默认为nullptr
     * @param size 要写入的数据大小, 默认为0
     * @return true 写入成功
     * @return false 写入失败
     * @throws std::runtime_error 当文件操作失败时抛出
     */
    bool write_file(const std::string &filename, const char *data = nullptr, size_t size = 0);

}  // namespace filesystem

#endif  // QUANT1X_STD_FILESYSTEM_H
