#pragma once
#ifndef QUANT1X_IO_CSV_WRITER_H
#define QUANT1X_IO_CSV_WRITER_H 1

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "csv-reader.h"

namespace io {
    class CSVWriter {
    public:
        // 打开文件并初始化
        explicit CSVWriter(const std::string &filename, char delimiter = ',')
            : file_(filename, std::ios::binary | std::ios::out | std::ios::trunc), delimiter_(delimiter) {
            if (!file_.is_open()) {
                throw std::runtime_error("Failed to open file: " + filename);
            }
        }

        // 写入单行数据（自动转义）
        template <typename... Args>
        void write_row(const Args &...fields) {
            std::ostringstream line;
            write_fields(line, fields...);
            file_ << line.str() << "\n";
        }

        // 批量写入多行数据（高性能接口）
        void write_rows(const std::vector<std::vector<std::string>> &rows) {
            for (const auto &row : rows) {
                std::ostringstream line;
                for (size_t i = 0; i < row.size(); ++i) {
                    if (i != 0)
                        line << delimiter_;
                    line << escape_field(row[i]);
                }
                file_ << line.str() << "\n";
            }
        }

        /**
         * @brief 强制将缓冲区数据写入文件
         *
         * 如果关联的文件已打开，则调用文件流的flush()方法确保所有缓冲数据被写入磁盘。
         * 如果文件未打开，则不执行任何操作。
         */
        void flush() {
            if (file_.is_open()) {
                file_.flush();
            }
        }

        /**
         * @brief 关闭已打开的文件
         *
         * 如果文件当前处于打开状态，则安全地关闭文件。
         * 如果文件已经关闭，则不执行任何操作。
         */
        void close() {
            if (file_.is_open()) {
                file_.close();
            }
        }

        // 关闭文件（析构时自动调用）
        ~CSVWriter() { close(); }

    private:
        std::ofstream file_;
        char          delimiter_;

        // 递归展开参数包，处理每个字段
        template <typename T>
        void write_fields(std::ostringstream &line, const T &field) {
            line << escape_field(field);
        }

        /**
         * @brief 递归地将多个字段写入输出流，每个字段之间用分隔符分隔
         *
         * @tparam T 第一个字段的类型
         * @tparam Args 剩余字段的类型参数包
         * @param line 输出字符串流，用于写入字段内容
         * @param first 第一个要写入的字段
         * @param rest 剩余要写入的字段（可变参数）
         *
         * @note 每个字段在写入前会经过escape_field处理
         */
        template <typename T, typename... Args>
        void write_fields(std::ostringstream &line, const T &first, const Args &...rest) {
            line << escape_field(first) << delimiter_;
            write_fields(line, rest...);
        }

        /**
         * @brief 对字段进行转义处理，用于CSV格式输出
         *
         * 当字段中包含双引号、逗号或换行符时，将整个字段用双引号包裹，
         * 并对字段内的双引号进行转义（替换为两个双引号）
         *
         * @param field 需要转义的原始字段字符串
         * @return std::string 转义后的字段字符串，如果不需要转义则返回原字段
         */
        std::string escape_field(const std::string &field) {
            if (field.find_first_of("\",\n") != std::string::npos) {
                std::ostringstream escaped;
                escaped << "\"";
                for (char c : field) {
                    if (c == '"')
                        escaped << "\"\"";
                    else
                        escaped << c;
                }
                escaped << "\"";
                return escaped.str();
            }
            return field;
        }

        /**
         * @brief 转义给定的字段字符串
         *
         * 如果输入字段为nullptr，则返回空字符串。否则将字段转换为std::string后转义。
         *
         * @param field 需要转义的C风格字符串指针，可以为nullptr
         * @return std::string 转义后的字符串，如果输入为nullptr则返回空字符串
         */
        std::string escape_field(const char *field) { return field ? escape_field(std::string(field)) : ""; }

        /**
         * @brief 转义字段值，将输入值转换为字符串并进行转义处理
         *
         * @tparam T 输入字段值的类型
         * @param field 需要转义的字段值
         * @return std::string 转义后的字符串结果
         *
         * @note 此函数是模板函数的重载版本，先将输入值转换为字符串，再调用字符串版本的转义函数
         */
        template <typename T>
        std::string escape_field(const T &field) {
            return escape_field(std::to_string(field));
        }
    };
}  // namespace io
#endif  // QUANT1X_IO_CSV_WRITER_H
