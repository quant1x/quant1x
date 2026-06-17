#pragma once
#ifndef QUANT1X_ENCODING_CSV_H
#define QUANT1X_ENCODING_CSV_H 1

#include <quant1x/std/api.h>
#include <quant1x/std/filesystem.h>
#include <csv2/reader.hpp>
#include <csv2/writer.hpp>
#include <boost/pfr.hpp>
#include <spdlog/spdlog.h>

namespace encoding {
    namespace csv {

        namespace detail {

            template<typename T>
            inline void convert_from_string(const std::string &str, T &value) {
                strings::try_parse(str, value);
            }

            template<typename T>
            struct CsvFieldMapping {
                std::vector<size_t> field_indices;

                static std::optional<CsvFieldMapping> create(
                    const csv2::Reader<csv2::delimiter<','>, csv2::quote_character<'"'>, csv2::first_row_is_header<true>> &csv) {
                    CsvFieldMapping mapping;
                    constexpr size_t field_count = boost::pfr::tuple_size<T>::value;
                    mapping.field_indices.resize(field_count, -1);

                    // 构建CSV列名到索引的映射
                    std::map<std::string, size_t, std::less<>> csv_col_map;
                    size_t col_idx = 0;
                    for (const auto &cell: csv.header()) {
                        std::string col_name;
                        cell.read_value(col_name);
                        //col_name = strings::trim(col_name);
                        //std::cout << "field:" << col_name << "," << col_idx << std::endl;
                        csv_col_map[col_name] = col_idx++;
                    }

                    // 使用索引遍历而非for_each_field来避免临时对象问题
                    [&]<size_t... I>(std::index_sequence<I...>) {
                        ((mapping.field_indices[I] = [&]() -> size_t {
                            constexpr auto field_name = boost::pfr::get_name<I, T>();
                            //std::cout << "name:" << field_name << "," << I << std::endl;
                            if (auto it = csv_col_map.find(field_name); it != csv_col_map.end()) {
                                //std::cout << "name:" << field_name << "," << I << "==>" << it->second << std::endl;
                                return it->second;
                            }
                            //std::cout << "name:" << field_name << "," << I << "==>, -1"<< std::endl;
                            return -1;
                        }()), ...);
                    }(std::make_index_sequence<field_count>{});

                    return mapping;
                }
            };
        }

        template <typename T>
        std::vector<T> csv_to_slices(const std::string& filename) {
            std::vector<T> result;
            csv2::Reader<csv2::delimiter<','>,
                         csv2::quote_character<'"'>,
                         csv2::first_row_is_header<true>> csv{};

            if (!csv.mmap(filename)) {
                spdlog::error("Error opening file: {}", filename);
                return result;
            }

            auto mapping = detail::CsvFieldMapping<T>::create(csv);
            if (!mapping) return result;
            //std::cout<<"map:" << mapping->field_indices.size()<< std::endl;

            // 处理每行数据
            for (const auto& row : csv) {
                if (row.length() == 0) {
                    continue;
                }
                T item;
                std::vector<std::string> row_data;

                for (const auto& cell : row) {
                    std::string val{};
                    cell.read_value(val);
                    row_data.push_back(val);
                }

                //std::cout<<"col:>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << row_data.size()<< std::endl;

                // 使用预计算的映射关系
                [&]<size_t... I>(std::index_sequence<I...>) {
                    auto set_field = [&](auto idx, auto& field) {
                        const size_t data_col = mapping->field_indices[idx];
                        //std::cout << "index:" << data_col << std::endl;
                        if (data_col != static_cast<size_t>(-1) && data_col < row_data.size()) {
                            detail::convert_from_string(row_data[data_col], field);
                        }
                    };

                    (set_field(I, boost::pfr::get<I>(item)), ...);
                }(std::make_index_sequence<boost::pfr::tuple_size<T>::value>{});

                result.push_back(item);
            }

            return result;
        }

        namespace detail {
            // 辅助函数: 将任意类型转换为字符串并处理CSV转义
            template<typename T>
            std::string v1_to_csv_string(const T &value) {
                std::ostringstream oss;

                // 字符串类型需要处理引号转义
                std::string str_value = strings::to_string(value);
                if constexpr (std::is_convertible_v<T, std::string>) {
                    str_value = static_cast<std::string>(value);
                } else {
                    oss << value;
                    str_value = oss.str();
                    oss.str("");
                }

                // 检查是否需要加引号
                bool needs_quotes = str_value.find_first_of(",\"\n\r") != std::string::npos;

                // 转义内部引号
                size_t pos = 0;
                while ((pos = str_value.find('"', pos)) != std::string::npos) {
                    str_value.insert(pos, "\"");
                    pos += 2;
                }

                if (needs_quotes) {
                    oss << '"' << str_value << '"';
                } else {
                    oss << str_value;
                }
                return oss.str();
            }

            template<typename T>
            std::string to_csv_string(const T &value) {
                // 字符串类型需要处理引号转义
                std::string str_value = strings::to_string(value);
                if constexpr (std::is_convertible_v<T, std::string>) {
                    // 检查是否需要加引号
                    bool needs_quotes = str_value.find_first_of(",\"\n\r") != std::string::npos;

                    // 转义内部引号
                    size_t pos = 0;
                    while ((pos = str_value.find('"', pos)) != std::string::npos) {
                        str_value.insert(pos, "\"");
                        pos += 2;
                    }

                    if (needs_quotes) {
//                        str_value.insert(0, "\"");
//                        str_value.insert(str_value.end(), "\"");
                        str_value = "\"" +str_value+"\"";
                    }
                }

                return str_value;
            }
        }

        // 将结构体vector写入CSV文件
        template <typename T>
        bool slices_to_csv(const std::vector<T>& data, const std::string& filename) {
            filesystem::check_filepath(filename, true);
            std::ofstream out_file(filename, std::ios::binary|std::ios::out | std::ios::trunc); // 必须以二进制方式写文件
            if (!out_file.is_open()) {
                spdlog::error("Failed to open file: {}", filename);
                return false;
            }

            csv2::Writer<csv2::delimiter<','>> writer(out_file);

            // 1. 写入表头(使用结构体字段名)
            std::vector<std::string> header;
            boost::pfr::for_each_field(T{}, [&](auto& field, auto idx) {
                (void)field;
                constexpr auto field_name = boost::pfr::get_name<idx, T>();
                header.emplace_back(field_name);
            });
            writer.write_row(header);

            // 2. 写入数据行
            for (const auto& item : data) {
                std::vector<std::string> row;
                boost::pfr::for_each_field(item, [&](auto& field, auto /*idx*/) {
                    row.emplace_back(detail::to_csv_string(field));
                });
                writer.write_row(row);
            }

            return true;
        }
    } // namespace csv
} // namespace encoding

#endif //QUANT1X_ENCODING_CSV_H
