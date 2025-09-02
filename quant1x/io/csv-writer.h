#pragma once
#ifndef API_IO_CSV_WRITER_H
#define API_IO_CSV_WRITER_H 1

#include "csv-reader.h"
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>

namespace io {
    class CSVWriter {
    public:
        // 打开文件并初始化
        explicit CSVWriter(const std::string& filename, char delimiter = ',')
                : file_(filename, std::ios::binary|std::ios::out | std::ios::trunc), delimiter_(delimiter) {
            if (!file_.is_open()) {
                throw std::runtime_error("Failed to open file: " + filename);
            }
        }

        // 写入单行数据（自动转义）
        template <typename... Args>
        void write_row(const Args&... fields) {
            std::ostringstream line;
            write_fields(line, fields...);
            file_ << line.str() << "\n";
        }

        // 批量写入多行数据（高性能接口）
        void write_rows(const std::vector<std::vector<std::string>>& rows) {
            for (const auto& row : rows) {
                std::ostringstream line;
                for (size_t i = 0; i < row.size(); ++i) {
                    if (i != 0) line << delimiter_;
                    line << escape_field(row[i]);
                }
                file_ << line.str() << "\n";
            }
        }

        // 关闭文件（析构时自动调用）
        ~CSVWriter() {
            if (file_.is_open()) {
                file_.close();
            }
        }

    private:
        std::ofstream file_;
        char delimiter_;

        // 递归展开参数包，处理每个字段
        template <typename T>
        void write_fields(std::ostringstream& line, const T& field) {
            line << escape_field(field);
        }

        template <typename T, typename... Args>
        void write_fields(std::ostringstream& line, const T& first, const Args&... rest) {
            line << escape_field(first) << delimiter_;
            write_fields(line, rest...);
        }

//        // 转义字段（根据 CSV 规范处理引号、逗号、换行符）
//        std::string escape_field(const std::string& field) {
//            if (field.find_first_of("\",\n") != std::string::npos) {
//                std::ostringstream escaped;
//                escaped << "\"";
//                for (char c : field) {
//                    if (c == '"') escaped << "\"\"";
//                    else escaped << c;
//                }
//                escaped << "\"";
//                return escaped.str();
//            }
//            return field;
//        }

//        // 支持非字符串类型的自动转换（如 int, double）
//        template <typename T>
//        std::string escape_field(const T& field) {
//            return escape_field(std::to_string(field));
//        }
        // 针对 std::string 的转义（直接处理）
        std::string escape_field(const std::string& field) {
            if (field.find_first_of("\",\n") != std::string::npos) {
                std::ostringstream escaped;
                escaped << "\"";
                for (char c : field) {
                    if (c == '"') escaped << "\"\"";
                    else escaped << c;
                }
                escaped << "\"";
                return escaped.str();
            }
            return field;
        }

        // 针对 C 风格字符串的转义（转换为 std::string 后处理）
        std::string escape_field(const char* field) {
            return escape_field(std::string(field));
        }

        // 针对其他类型（如 int, double）的转义（通过 std::to_string）
        template <typename T>
        std::string escape_field(const T& field) {
            return escape_field(std::to_string(field));
        }
    };
}
#endif //API_IO_CSV_WRITER_H
