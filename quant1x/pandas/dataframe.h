// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_PANDAS_DATAFRAME_H
#define QUANT1X_PANDAS_DATAFRAME_H 1

#include <quant1x/pandas/series.h>

#include <boost/pfr.hpp>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace quant1x::pandas {

/// 多列容器 —— 列名到 Series 的有序映射，只做按列名存取。
///
/// 对齐 `quant1x/pandas/dataframe.py`：数值列是 [`Series`]（float64 / bool），
/// 数值语义全部在 `quant1x.num` 与 `Series`，本类不做计算。
/// 字符串列只用于展示与 IO（对照 pandas 的 object dtype），不参与任何算子。
class DataFrame {
  public:
    DataFrame() = default;

    /// 由结构体 vector 构造：算数字段入 Series，字符串字段入展示列。
    ///
    /// 字段名取自 Boost.PFR 反射，列序与字段声明顺序一致。
    template <typename T>
    static DataFrame from_struct_vector(const std::vector<T> &data) {
        DataFrame frame;
        if (data.empty()) {
            return frame;
        }
        constexpr std::size_t field_count = boost::pfr::tuple_size_v<T>;
        append_fields<T>(frame, data, std::make_index_sequence<field_count>{});
        return frame;
    }

    // -- 属性 -------------------------------------------------------------
    /// 行数（首列决定，无列为 0）
    std::size_t row_count() const {
        if (columns_.empty()) {
            return 0;
        }
        return column_size(columns_.front());
    }

    /// 列数
    std::size_t column_count() const { return columns_.size(); }

    /// 形状 (行数, 列数)
    std::pair<std::size_t, std::size_t> shape() const { return {row_count(), column_count()}; }

    /// 列名（插入序）
    std::vector<std::string> columns() const {
        std::vector<std::string> names;
        names.reserve(columns_.size());
        for (const auto &column : columns_) {
            names.push_back(column.name);
        }
        return names;
    }

    /// 是否已有该列
    bool contains(const std::string &name) const { return index_of(name) != npos; }

    // -- 取列 -------------------------------------------------------------
    /// 取数值列；不存在或为字符串列时返回 nullptr
    const Series *get(const std::string &name) const {
        const auto index = index_of(name);
        if (index == npos) {
            return nullptr;
        }
        return std::get_if<Series>(&columns_[index].data);
    }

    /// 取字符串列（展示/IO 用）；不存在或为数值列时返回 nullptr
    const std::vector<std::string> *get_strings(const std::string &name) const {
        const auto index = index_of(name);
        if (index == npos) {
            return nullptr;
        }
        return std::get_if<std::vector<std::string>>(&columns_[index].data);
    }

    /// 按给定列序取子集
    DataFrame select(const std::vector<std::string> &names) const {
        DataFrame frame;
        for (const auto &name : names) {
            const auto index = index_of(name);
            if (index == npos) {
                throw std::out_of_range("DataFrame: 列不存在 " + name);
            }
            frame.columns_.push_back(columns_[index]);
        }
        return frame;
    }

    // -- 写列 -------------------------------------------------------------
    /// 写数值列：已存在原位替换，否则追加；列长必须与现有行数一致
    void set(const std::string &name, Series values) {
        values.set_name(name);
        assign(name, std::move(values));
    }

    /// 写字符串列（展示/IO 用）
    void set_strings(const std::string &name, std::vector<std::string> values) {
        assign(name, std::move(values));
    }

    // -- 复制 -------------------------------------------------------------
    DataFrame copy() const {
        DataFrame frame;
        frame.columns_ = columns_;
        return frame;
    }

    // -- 打印 -------------------------------------------------------------
    /// pandas 风格的表格文本：索引列 + 右对齐数值 + 字符串左对齐；
    /// 行数超过 max_rows 时显示首尾各 head_rows 行并用 `..` 省略。
    std::string to_string() const {
        if (columns_.empty()) {
            return "Empty DataFrame\nColumns: []\nIndex: []";
        }

        const std::size_t rows = row_count();
        const std::size_t cols = columns_.size();
        if (rows == 0) {
            std::string text = "Empty DataFrame\nColumns: [";
            for (std::size_t col = 0; col < cols; ++col) {
                text += (col == 0 ? "" : ", ") + columns_[col].name;
            }
            return text + "]\nIndex: []";
        }

        // 每列的显示单元（pandas 默认 justify='right'，所有列右对齐）
        std::vector<std::vector<std::string>> cells(cols);
        for (std::size_t col = 0; col < cols; ++col) {
            if (const auto *series = std::get_if<Series>(&columns_[col].data)) {
                cells[col] = format_numeric(*series);
            } else {
                cells[col] = std::get<std::vector<std::string>>(columns_[col].data);
            }
        }

        // 列宽 = max(表头宽, 值宽)
        std::vector<std::size_t> widths(cols, 0);
        for (std::size_t col = 0; col < cols; ++col) {
            widths[col] = display_width(columns_[col].name);
            for (const auto &cell : cells[col]) {
                widths[col] = std::max(widths[col], display_width(cell));
            }
        }
        const std::size_t index_width = std::to_string(rows - 1).size();

        const bool truncated = rows > max_rows;
        const std::size_t head = truncated ? head_rows : rows;
        const std::size_t tail = truncated ? head_rows : 0;

        std::string text;
        // 表头：索引列留空，其余列右对齐
        text.append(index_width, ' ');
        for (std::size_t col = 0; col < cols; ++col) {
            text += "  ";
            text += pad(columns_[col].name, widths[col]);
        }
        text += '\n';

        auto append_row = [&](std::size_t row) {
            const auto label = std::to_string(row);
            text.append(index_width - label.size(), ' ');
            text += label;
            for (std::size_t col = 0; col < cols; ++col) {
                text += "  ";
                text += pad(cells[col][row], widths[col]);
            }
            text += '\n';
        };

        for (std::size_t row = 0; row < head; ++row) {
            append_row(row);
        }
        if (truncated) {
            text.append(index_width - 2, ' ');
            text += "..";
            for (std::size_t col = 0; col < cols; ++col) {
                text += "  ";
                text += pad("...", widths[col]);
            }
            text += '\n';
            for (std::size_t row = rows - tail; row < rows; ++row) {
                append_row(row);
            }
            text += "\n[" + std::to_string(rows) + " rows x " + std::to_string(cols) +
                    " columns]";
        }
        return text;
    }

  private:
    struct Column {
        std::string name;
        std::variant<Series, std::vector<std::string>> data;
    };

    static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();
    static constexpr std::size_t max_rows = 60;
    static constexpr std::size_t head_rows = 5;

    std::vector<Column> columns_;

    std::size_t index_of(const std::string &name) const {
        for (std::size_t index = 0; index < columns_.size(); ++index) {
            if (columns_[index].name == name) {
                return index;
            }
        }
        return npos;
    }

    static std::size_t column_size(const Column &column) {
        if (const auto *series = std::get_if<Series>(&column.data)) {
            return series->size();
        }
        return std::get<std::vector<std::string>>(column.data).size();
    }

    template <typename Data>
    void assign(const std::string &name, Data values) {
        const std::size_t rows = row_count();
        const std::size_t size = values.size();
        if (!columns_.empty() && size != rows) {
            throw std::invalid_argument("DataFrame: 列长不一致");
        }
        const auto index = index_of(name);
        if (index != npos) {
            columns_[index].data = std::move(values);
            return;
        }
        columns_.push_back(Column{name, std::move(values)});
    }

    // -- 结构体字段展开 ---------------------------------------------------
    template <typename T, std::size_t... I>
    static void append_fields(DataFrame &frame, const std::vector<T> &data,
                              std::index_sequence<I...>) {
        (append_field<T, I>(frame, data), ...);
    }

    template <typename T, std::size_t I>
    static void append_field(DataFrame &frame, const std::vector<T> &data) {
        using Field = std::decay_t<decltype(boost::pfr::get<I>(data.front()))>;
        const std::string name(boost::pfr::get_name<I, T>());
        if constexpr (std::is_same_v<Field, std::string>) {
            std::vector<std::string> column;
            column.reserve(data.size());
            for (const auto &item : data) {
                column.push_back(boost::pfr::get<I>(item));
            }
            frame.set_strings(name, std::move(column));
        } else if constexpr (std::is_same_v<Field, bool>) {
            std::vector<bool> column;
            column.reserve(data.size());
            for (const auto &item : data) {
                column.push_back(boost::pfr::get<I>(item));
            }
            frame.set(name, Series(std::move(column)));
        } else if constexpr (std::is_arithmetic_v<Field>) {
            std::vector<double> column;
            column.reserve(data.size());
            for (const auto &item : data) {
                column.push_back(static_cast<double>(boost::pfr::get<I>(item)));
            }
            frame.set(name, Series(std::move(column)));
        }
        // 其它类型（嵌套结构、自定义类）不入表
    }

    // -- 打印辅助 ---------------------------------------------------------
    /// 按列统一小数位（浮点列至少 1 位），与 pandas 的 per-column 精度一致
    static std::vector<std::string> format_numeric(const Series &series) {
        const std::size_t rows = series.size();
        std::vector<std::string> texts(rows);
        std::vector<std::size_t> decimals(rows, 0);
        bool uniform = true;
        for (std::size_t row = 0; row < rows; ++row) {
            const double value = series.is_bool()
                                     ? (series.get_bool(row) ? 1.0 : 0.0)
                                     : series.get_f64(row);
            if (std::isnan(value) || std::isinf(value)) {
                texts[row] = format_special(value);
                uniform = false;
                continue;
            }
            if (!format_fixed(value, texts[row], decimals[row])) {
                texts[row] = format_shortest(value);
                uniform = false;
            }
        }
        if (!uniform) {
            return texts;
        }
        std::size_t width = 1;
        for (const auto digits : decimals) {
            width = std::max(width, digits);
        }
        for (std::size_t row = 0; row < rows; ++row) {
            texts[row] = pad_decimals(texts[row], width);
        }
        return texts;
    }

    /// 定点表示，取能往返的最小小数位；量级落在科学计数法区间时返回 false
    static bool format_fixed(double value, std::string &text, std::size_t &decimals) {
        const double magnitude = std::fabs(value);
        if (magnitude != 0.0 && (magnitude < 1e-4 || magnitude >= 1e16)) {
            return false;
        }
        char buffer[64];
        for (int precision = 0; precision <= 17; ++precision) {
            const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value,
                                              std::chars_format::fixed, precision);
            if (result.ec != std::errc{}) {
                return false;
            }
            std::string candidate(buffer, result.ptr);
            if (std::strtod(candidate.c_str(), nullptr) == value) {
                text = std::move(candidate);
                decimals = static_cast<std::size_t>(precision);
                return true;
            }
        }
        return false;
    }

    /// 最短往返表示（科学计数法区间用它，与 pandas 的 repr 一致）
    static std::string format_shortest(double value) {
        char buffer[64];
        const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
        return std::string(buffer, result.ptr);
    }

    static std::string format_special(double value) {
        if (std::isnan(value)) {
            return "NaN";
        }
        return value > 0 ? "inf" : "-inf";
    }

    static std::string pad_decimals(const std::string &text, std::size_t decimals) {
        const auto point = text.find('.');
        if (point == std::string::npos) {
            return text + "." + std::string(decimals, '0');
        }
        const std::size_t current = text.size() - point - 1;
        if (current >= decimals) {
            return text;
        }
        return text + std::string(decimals - current, '0');
    }

    /// 显示宽度：UTF-8 多字节字符按 1 列计
    static std::size_t display_width(const std::string &text) {
        std::size_t width = 0;
        for (unsigned char byte : text) {
            if ((byte & 0xC0) != 0x80) {
                ++width;
            }
        }
        return width;
    }

    static std::string pad(const std::string &text, std::size_t width) {
        const std::size_t current = display_width(text);
        if (current >= width) {
            return text;
        }
        return std::string(width - current, ' ') + text;
    }
};

} // namespace quant1x::pandas

#endif // QUANT1X_PANDAS_DATAFRAME_H
