#pragma once
#ifndef API_IO_FILE_H
#define API_IO_FILE_H 1

#include "quant1x/std/api.h"
#include <chrono>
#include <string>

namespace io {

    // 去掉扩展名
    std::string remove_extension(const std::string& path_str);

    // 获取当前执行程序的完整路径
    std::string executable_absolute_path();
    // 执行程序名, 去掉扩展名
    std::string executable_name();
    // 生成一个文件
    bool write_file(const std::string& filename, const char *data = nullptr, size_t size = 0);
    int64_t last_modified_time(const std::string& filename);
    void last_modified_time(const std::string& filename, const int64_t& milliseconds);
}

#endif //API_IO_FILE_H
