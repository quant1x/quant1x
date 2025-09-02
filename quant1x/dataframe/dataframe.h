#pragma once
#ifndef QUANT1X_DATAFRAME_DATAFRAME_H
#define QUANT1X_DATAFRAME_DATAFRAME_H 1

#include <iostream>
#include <vector>
#include <string>
#include <variant>
#include <type_traits>
#include <algorithm>
#include <boost/pfr.hpp>

// 定义支持的列数据类型
using ColumnVariant = std::variant<
    std::vector<int>,
    std::vector<int64_t>,
    std::vector<float>,
    std::vector<double>,
    std::vector<bool>,
    std::vector<std::string>
    // 可以继续添加其他支持的类型
>;

// 列存储 DataFrame 实现
class DataFrame {
private:
    std::vector<std::string> column_names_;
    std::vector<ColumnVariant> columns_;
    size_t row_count_ = 0;

public:
    ~DataFrame() = default;
    // 添加一列数据
    template <typename T>
    void add_column(const std::string& name, const std::vector<T>& data) {
        if (!columns_.empty() && data.size() != row_count_) {
            throw std::runtime_error("Column size mismatch");
        }
        column_names_.push_back(name);
        columns_.emplace_back(data);
        row_count_ = data.size();
    }

    // 从结构体 vector 自动构建 DataFrame（带字段名）
    template <typename T>
    static DataFrame from_struct_vector(const std::vector<T>& data) {
        DataFrame df;

        if (data.empty()) return df;

        // 使用 Boost.PFR 获取字段数量
        constexpr size_t field_count = boost::pfr::tuple_size_v<T>;

        // 为每个字段创建列（带字段名）
        [&]<size_t... I>(std::index_sequence<I...>) {
            (create_column_for_field<T, I>(df, data), ...);
        }(std::make_index_sequence<field_count>{});

        return df;
    }

private:
    // 为特定字段创建列（带字段名）
    template <typename T, size_t I>
    static void create_column_for_field(DataFrame& df, const std::vector<T>& data) {
        using FieldType = std::decay_t<decltype(boost::pfr::get<I>(data[0]))>;

        std::vector<FieldType> column;
        column.reserve(data.size());

        for (const auto& item : data) {
            column.push_back(boost::pfr::get<I>(item));
        }

        // 获取字段名
        constexpr auto name = boost::pfr::get_name<I, T>();
        df.add_column(std::string(name), column);
    }

public:
    // 获取行数
    size_t row_count() const { return row_count_; }

    // 获取列数
    size_t column_count() const { return column_names_.size(); }

    // 获取列名
    const std::vector<std::string>& column_names() const { return column_names_; }

    // 访问列数据
    template <typename T>
    const std::vector<T>& get_column(size_t index) const {
        return std::get<std::vector<T>>(columns_.at(index));
    }

    // 按列名访问列数据
    template <typename T>
    const std::vector<T>& get_column(const std::string& name) const {
        for (size_t i = 0; i < column_names_.size(); ++i) {
            if (column_names_[i] == name) {
                return std::get<std::vector<T>>(columns_.at(i));
            }
        }
        throw std::out_of_range("Column not found: " + name);
    }

    std::string to_string() const {
        if (row_count_ == 0 || column_count() == 0) {
            return "Empty DataFrame\n";
        }

        std::ostringstream oss{};
        const bool should_truncate = row_count_ > 10;
        const size_t display_head_rows = should_truncate ? 5 : row_count_;
        const size_t display_tail_rows = should_truncate ? 5 : 0;
        const size_t index_col_width = std::max(
            static_cast<size_t>(4), // 最小宽度为3（"No."和单数字）
            std::to_string(row_count_ - 1).size() + 1 // 最大索引数字的位数+1（例如2453→宽度5）
        );

        // 1. 计算各列最大宽度
        std::vector<size_t> col_widths(column_count());
        for (size_t col = 0; col < column_count(); ++col) {
            col_widths[col] = column_names_[col].size();

            auto update_width = [&](const auto& vec, size_t row) {
                std::ostringstream tmp;
                tmp << vec[row];
                col_widths[col] = std::max(col_widths[col], tmp.str().size());
            };

            std::visit([&](const auto& vec) {
                // 前N行
                for (size_t row = 0; row < display_head_rows; ++row) {
                    update_width(vec, row);
                }
                // 后N行（如果需要截断）
                if (should_truncate) {
                    for (size_t row = row_count_ - display_tail_rows; row < row_count_; ++row) {
                        update_width(vec, row);
                    }
                }
            }, columns_[col]);
        }

        // 2. 打印表头
        oss << ">" << std::setw(index_col_width - 1) << std::right << "No." << " ";
        for (size_t col = 0; col < column_count(); ++col) {
            oss << std::setw(col_widths[col]) << std::left << column_names_[col] << " ";
        }
        oss << "\n";

        // 3. 打印分隔线
        oss << std::string(index_col_width, '-') << " ";
        for (size_t col = 0; col < column_count(); ++col) {
            oss << std::string(col_widths[col], '-') << " ";
        }
        oss << "\n";

        // 4. 打印数据行
        auto print_row = [&](size_t row) {
            oss << std::setw(index_col_width) << std::right << row << " ";
            for (size_t col = 0; col < column_count(); ++col) {
                std::visit([&](const auto& vec) {
                    oss << std::setw(col_widths[col]) << std::left << vec[row] << " ";
                }, columns_[col]);
            }
            oss << "\n";
        };

        // 打印前N行
        for (size_t row = 0; row < display_head_rows; ++row) {
            print_row(row);
        }

        // 打印省略行（如果需要）
        if (should_truncate && row_count_ > display_head_rows + display_tail_rows) {
            oss << std::setw(index_col_width) << std::right << "..." << " ";
            for (size_t col = 0; col < column_count(); ++col) {
                oss << std::setw(col_widths[col]) << std::left << "..." << " ";
            }
            oss << "\n";
        }

        // 打印后N行（如果需要截断）
        if (should_truncate) {
            for (size_t row = row_count_ - display_tail_rows; row < row_count_; ++row) {
                print_row(row);
            }
        }

        // 5. 打印总结信息
        oss << "\n[" << row_count_ << " rows x " << column_count() << " columns]\n";

        return oss.str();
    }

    // 添加调试方法
    void print_debug_info() const {
        std::cout << "DataFrame 列信息:\n";
        for (size_t i = 0; i < column_names_.size(); ++i) {
            std::cout << "列 " << i << ": " << column_names_[i]
                      << " (大小: " << std::visit([](auto&& v) { return v.size(); }, columns_[i])
                      << ")\n";
        }
    }

    void debug_print_columns() const {
        std::cout << "DataFrame调试信息 (" << row_count_ << "行, "
                  << column_names_.size() << "列):\n";

        for (size_t i = 0; i < column_names_.size(); ++i) {
            std::cout << "列[" << i << "] " << column_names_[i] << ": ";

            // 打印每列的类型和大小
            std::visit([&](auto&& vec) {
                using VecType = std::decay_t<decltype(vec)>;
                using ElemType = typename VecType::value_type;

                std::cout << "类型=" << typeid(ElemType).name()
                          << ", 大小=" << vec.size();

                if (!vec.empty()) {
                    std::cout << ", 首值=" << vec[0];
                }
                std::cout << "\n";
            }, columns_[i]);
        }
    }

    // 检查列是否存在
    bool contains(const std::string& column_name) const {
        return std::find(column_names_.begin(), column_names_.end(), column_name) != column_names_.end();
    }

    // 获取所有列名
    const std::vector<std::string>& columns() const {
        return column_names_;
    }

    // 直接返回variant引用
    const ColumnVariant& operator[](const std::string& name) const {
        for (size_t i = 0; i < column_names_.size(); ++i) {
            if (iequals(column_names_[i], name)) {
                return columns_[i];
            }
        }
        static const ColumnVariant empty;
        return empty; // 返回空variant
    }

    const ColumnVariant& operator[](const char* name) const {
        return operator[](std::string(name));
    }

    template<typename T>
    std::vector<T>& get(const std::string& name) {
        return std::get<std::vector<T>>(get_column(name));
    }

private:

    // 获取列（避免重复查找）
    ColumnVariant& get_column(const std::string& name) {
        for (size_t i = 0; i < column_names_.size(); ++i) {
            if (iequals(column_names_[i], name)) {
                return columns_[i];
            }
        }
        static ColumnVariant empty;
        return empty; // 返回空variant
        //throw std::runtime_error("Type mismatch for column '" + name + "'. ");
    }

    // 辅助函数：不区分大小写的字符串比较
    static bool iequals(const std::string& a, const std::string& b) {
        return std::equal(a.begin(), a.end(), b.begin(), b.end(),
                          [](char a, char b) {
                              return std::tolower(a) == std::tolower(b);
                          });
    }

    // 辅助函数：更新列宽
    template <typename T>
    static void update_col_width(size_t& current_width, const T& value) {
        std::ostringstream tmp;
        tmp << value;
        current_width = std::max(current_width, tmp.str().size());
    }
};

#endif //QUANT1X_DATAFRAME_DATAFRAME_H
