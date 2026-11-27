// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

/// formula 跨语言对拍: 读 `tests/fixtures/formula/cases.csv`, 逐值比较 C++ 与 Python 的输出。
///
/// 期望值只由 Python spec 生成; 本用例只做比对, 不参与生成。
/// 浮点按逐位相等比较, NaN 视为相等。

#include <quant1x/formula/formula.h>

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

using quant1x::formula::Series;
using quant1x::formula::Window;

/// 一行样本
struct Row {
    std::string fields[11];
};

/// 按逗号切分
std::vector<std::string> split(const std::string &line) {
    std::vector<std::string> out;
    std::size_t start = 0;
    while (true) {
        const auto position = line.find(',', start);
        if (position == std::string::npos) {
            out.push_back(line.substr(start));
            break;
        }
        out.push_back(line.substr(start, position - start));
        start = position + 1;
    }
    return out;
}

/// 空串视为 NaN
double parse_double(const std::string &text) {
    if (text.empty()) {
        return quant1x::formula::nan_value();
    }
    return std::strtod(text.c_str(), nullptr);
}

/// 空串视为 0
std::int64_t parse_int(const std::string &text) {
    if (text.empty()) {
        return 0;
    }
    return static_cast<std::int64_t>(std::strtod(text.c_str(), nullptr));
}

/// 取某一列
Series column(const std::vector<Row> &rows, std::size_t field) {
    Series out = quant1x::formula::make_series(rows.size());
    for (std::size_t index = 0; index < rows.size(); ++index) {
        out[index] = parse_double(rows[index].fields[field]);
    }
    return out;
}

/// 取窗口列
Window window_column(const std::vector<Row> &rows, std::size_t field) {
    Window out(rows.size(), -1);
    for (std::size_t index = 0; index < rows.size(); ++index) {
        out[index] = parse_int(rows[index].fields[field]);
    }
    return out;
}

/// 是否为空列
bool empty_column(const std::vector<Row> &rows, std::size_t field) {
    for (const auto &row : rows) {
        if (!row.fields[field].empty()) {
            return false;
        }
    }
    return true;
}

/// 比较两个序列
bool same_series(const Series &actual, const Series &expected) {
    if (actual.size() != expected.size()) {
        return false;
    }
    for (std::size_t index = 0; index < actual.size(); ++index) {
        const double left = actual[index];
        const double right = expected[index];
        if (std::isnan(left) && std::isnan(right)) {
            continue;
        }
        if (left != right) {
            return false;
        }
    }
    return true;
}

/// 定位样本文件
std::filesystem::path locate(int argc, char **argv) {
    if (argc > 1) {
        return argv[1];
    }
    const std::filesystem::path source(__FILE__);
    const auto root = source.parent_path().parent_path();
    const std::vector<std::filesystem::path> candidates = {
        root / "tests" / "fixtures" / "formula" / "cases.csv",
        "tests/fixtures/formula/cases.csv",
        "../tests/fixtures/formula/cases.csv",
    };
    for (const auto &candidate : candidates) {
        if (std::filesystem::exists(candidate)) {
            return candidate;
        }
    }
    return {};
}

/// 计算某函数某分组的输出
Series evaluate(const std::string &label, const std::vector<Row> &rows, bool &ok) {
    using namespace quant1x::formula;
    ok = true;
    const Series a = column(rows, 3);
    const Series b = column(rows, 4);
    const Series c = column(rows, 5);
    const Window w = window_column(rows, 7);
    const double n = parse_double(rows.front().fields[8]);
    const double m = parse_double(rows.front().fields[9]);
    const std::int64_t ni = static_cast<std::int64_t>(n);
    const std::int64_t mi = static_cast<std::int64_t>(m);
    const bool has_window = !empty_column(rows, 7);
    const bool has_b = !empty_column(rows, 4);
    const bool has_n = !rows.front().fields[8].empty();
    const std::string &name = rows.front().fields[1];

    if (label == "abs") return quant1x::formula::abs(a);
    if (label == "sqrt") return quant1x::formula::sqrt(a);
    if (label == "ln") return quant1x::formula::log(a);
    if (label == "pow") return quant1x::formula::pow(a, n);
    if (label == "max") return quant1x::formula::maximum(a, b);
    if (label == "min") return quant1x::formula::minimum(a, b);
    if (label == "if") return if_select(as_bools(a), b, c);
    if (label == "ref") return has_window ? ref(a, w) : ref(a, ni);
    if (label == "diff") return diff(a, ni);
    if (label == "sum") return has_window ? quant1x::formula::sum(a, w) : quant1x::formula::sum(a, ni);
    if (label == "const") return const_series(a);
    if (label == "hhv") return has_window ? hhv(a, w) : hhv(a, ni);
    if (label == "llv") return has_window ? llv(a, w) : llv(a, ni);
    if (label == "hhvbars") return hhvbars(a, ni);
    if (label == "llvbars") return llvbars(a, ni);
    if (label == "ma") return has_window ? ma(a, w) : ma(a, ni);
    if (label == "ema") return ema(a, ni);
    if (label == "sma") return sma(a, ni, mi);
    if (label == "wma") return wma(a, ni);
    if (label == "std") return quant1x::formula::stddev(a, ni);
    if (label == "avedev") return avedev(a, ni);
    if (label == "slope") return slope(a, ni);
    if (label == "forcast") return forcast(a, ni);
    if (label == "dma") return has_b ? dma(a, b) : dma(a, n);
    if (label == "last") return as_series(last(as_bools(a), ni, mi));
    if (label == "count") return count(as_bools(a), ni);
    if (label == "every") return as_series(every(as_bools(a), ni));
    if (label == "exist") return as_series(exist(as_bools(a), ni));
    if (label == "filter") return filter(as_bools(a), ni);
    if (label == "barslast") return barslast(as_bools(a));
    if (label == "barslastcount") return barslastcount(as_bools(a));
    if (label == "barssincen") return barssincen(as_bools(a), ni);
    if (label == "cross") return as_series(cross(a, b));
    if (label == "longcross") return as_series(longcross(a, b, ni));
    if (label == "valuewhen") return valuewhen(as_bools(a), b);
    if (label == "between") return as_series(between(a, n, m));
    if (label == "toprange") return toprange(a);
    if (label == "lowrange") return lowrange(a);
    if (label == "rsi") return rsi(a, has_n ? ni : 24);
    if (label == "ztprice") return ztprice(a, n);
    if (label == "macd.dif") return std::get<0>(macd(a));
    if (label == "macd.dea") return std::get<1>(macd(a));
    if (label == "macd.macd") return std::get<2>(macd(a));
    if (label == "kdj.k") return std::get<0>(kdj(a, b, c));
    if (label == "kdj.d") return std::get<1>(kdj(a, b, c));
    if (label == "kdj.j") return std::get<2>(kdj(a, b, c));
    if (label == "boll.upper") return std::get<0>(boll(a));
    if (label == "boll.mid") return std::get<1>(boll(a));
    if (label == "boll.lower") return std::get<2>(boll(a));

    ok = false;
    std::fprintf(stderr, "未覆盖的函数: %s (%s)\n", label.c_str(), name.c_str());
    return {};
}

}  // namespace

int main(int argc, char **argv) {
    const auto path = locate(argc, argv);
    if (path.empty()) {
        std::fprintf(stderr, "找不到 cases.csv\n");
        return 2;
    }
    std::ifstream stream(path);
    if (!stream) {
        std::fprintf(stderr, "无法打开 %s\n", path.string().c_str());
        return 2;
    }

    std::string line;
    std::getline(stream, line);  // 表头
    std::vector<Row> rows;
    std::string current_label;
    std::string current_case;
    int failures = 0;
    int cases = 0;

    const auto flush = [&]() {
        if (rows.empty()) {
            return;
        }
        ++cases;
        bool ok = true;
        const Series actual = evaluate(current_label, rows, ok);
        if (!ok) {
            ++failures;
            rows.clear();
            return;
        }
        Series expected = quant1x::formula::make_series(rows.size());
        for (std::size_t index = 0; index < rows.size(); ++index) {
            expected[index] = parse_double(rows[index].fields[10]);
        }
        if (!same_series(actual, expected)) {
            ++failures;
            std::fprintf(stderr, "不一致: %s / %s\n", current_label.c_str(), current_case.c_str());
            for (std::size_t index = 0; index < rows.size(); ++index) {
                const double left = actual[index];
                const double right = expected[index];
                const bool equal = (std::isnan(left) && std::isnan(right)) || left == right;
                if (!equal) {
                    std::fprintf(stderr, "  [%zu] 实际=%.17g 期望=%.17g\n", index, left, right);
                }
            }
        }
        rows.clear();
    };

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        const auto fields = split(line);
        if (fields.size() != 11) {
            std::fprintf(stderr, "列数错误: %zu\n", fields.size());
            return 2;
        }
        Row row;
        for (std::size_t index = 0; index < fields.size(); ++index) {
            row.fields[index] = fields[index];
        }
        if (row.fields[0] != current_label || row.fields[1] != current_case) {
            flush();
            current_label = row.fields[0];
            current_case = row.fields[1];
        }
        rows.push_back(row);
    }
    flush();

    if (failures != 0) {
        std::fprintf(stderr, "formula 对拍失败: %d / %d 组\n", failures, cases);
        return 1;
    }
    std::printf("formula 对拍通过: %d 组\n", cases);
    return 0;
}
