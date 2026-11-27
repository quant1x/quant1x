// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

/// num 跨语言对拍: 读 `tests/fixtures/num/cases.csv`, 逐值比较 C++ 与 Python 的输出。
///
/// 期望值只由 Python spec 生成; 本用例只做比对, 不参与生成。
/// 浮点按逐位相等比较, NaN 视为相等。

#include <quant1x/num/num.h>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

namespace {

namespace num = quant1x::num;

/// 一行样本: function,case,a,b,c,w,n,expected
struct Row {
    std::string fields[8];
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

/// 解析浮点: nan / inf / 空串
double parse_double(const std::string &text) {
    if (text.empty() || text == "nan") {
        return num::nan_value();
    }
    if (text == "inf") {
        return std::numeric_limits<double>::infinity();
    }
    if (text == "-inf") {
        return -std::numeric_limits<double>::infinity();
    }
    return std::strtod(text.c_str(), nullptr);
}

/// 按 | 切分序列, 空串为空序列
num::Series parse_series(const std::string &text) {
    num::Series out;
    if (text.empty()) {
        return out;
    }
    std::size_t start = 0;
    while (true) {
        const auto position = text.find('|', start);
        const auto token = text.substr(start, position == std::string::npos
                                                  ? std::string::npos
                                                  : position - start);
        out.push_back(parse_double(token));
        if (position == std::string::npos) {
            break;
        }
        start = position + 1;
    }
    return out;
}

/// 按 | 切分布尔序列
num::Bools parse_bools(const std::string &text) {
    num::Bools out;
    if (text.empty()) {
        return out;
    }
    std::size_t start = 0;
    while (true) {
        const auto position = text.find('|', start);
        const auto token = text.substr(start, position == std::string::npos
                                                  ? std::string::npos
                                                  : position - start);
        out.push_back(!token.empty() && parse_double(token) != 0.0);
        if (position == std::string::npos) {
            break;
        }
        start = position + 1;
    }
    return out;
}

/// 按 | 切分整型序列
std::vector<std::int64_t> parse_ints(const std::string &text) {
    std::vector<std::int64_t> out;
    if (text.empty()) {
        return out;
    }
    std::size_t start = 0;
    while (true) {
        const auto position = text.find('|', start);
        const auto token = text.substr(start, position == std::string::npos
                                                  ? std::string::npos
                                                  : position - start);
        out.push_back(static_cast<std::int64_t>(std::strtoll(token.c_str(), nullptr, 10)));
        if (position == std::string::npos) {
            break;
        }
        start = position + 1;
    }
    return out;
}

/// 逐位相等: NaN 视为相等
bool bitwise_equal(double left, double right) {
    if (std::isnan(left) && std::isnan(right)) {
        return true;
    }
    return left == right;
}

/// 比较浮点序列
bool compare_series(const num::Series &actual, const num::Series &expected, std::string &message) {
    if (actual.size() != expected.size()) {
        message = "长度不一致: " + std::to_string(actual.size()) + " vs " +
                  std::to_string(expected.size());
        return false;
    }
    for (std::size_t index = 0; index < actual.size(); ++index) {
        if (!bitwise_equal(actual[index], expected[index])) {
            message = "位置 " + std::to_string(index) + ": " +
                      std::to_string(actual[index]) + " vs " +
                      std::to_string(expected[index]);
            return false;
        }
    }
    return true;
}

/// 比较布尔序列
bool compare_bools(const num::Bools &actual, const num::Bools &expected, std::string &message) {
    if (actual.size() != expected.size()) {
        message = "长度不一致: " + std::to_string(actual.size()) + " vs " +
                  std::to_string(expected.size());
        return false;
    }
    for (std::size_t index = 0; index < actual.size(); ++index) {
        if (actual[index] != expected[index]) {
            message = "位置 " + std::to_string(index);
            return false;
        }
    }
    return true;
}

/// 取窗口: w 列优先, 否则用 n 列作常量窗口
num::Window window_of(const Row &row) {
    const auto values = parse_ints(row.fields[5]);
    if (!values.empty()) {
        return num::Window(values);
    }
    if (!row.fields[6].empty()) {
        return num::Window(static_cast<std::int64_t>(parse_double(row.fields[6])));
    }
    return num::Window(std::int64_t{0});
}

/// 执行一个用例; bool_series 表示期望是布尔序列
bool run_case(const Row &row, bool bool_series, std::string &message) {
    const std::string &function = row.fields[0];
    const auto a = parse_series(row.fields[2]);
    const auto b = parse_series(row.fields[3]);
    const auto c = parse_series(row.fields[4]);
    const double n = row.fields[6].empty() ? 0.0 : parse_double(row.fields[6]);
    const auto expected = bool_series ? num::Series{} : parse_series(row.fields[7]);
    const auto expected_bools = bool_series ? parse_bools(row.fields[7]) : num::Bools{};

    // dtype
    if (function == "to_scalar") {
        return compare_series({num::to_scalar(a)}, expected, message);
    }
    if (function == "default_like") {
        return compare_series(num::default_like(a), expected, message);
    }

    // elementwise: 一元
    if (function == "abs") {
        return compare_series(num::abs(a), expected, message);
    }
    if (function == "sqrt") {
        return compare_series(num::sqrt(a), expected, message);
    }
    if (function == "log") {
        return compare_series(num::log(a), expected, message);
    }
    if (function == "pow") {
        return compare_series(num::pow(a, n), expected, message);
    }

    // elementwise: 二元
    if (function == "add") {
        return compare_series(num::add(a, b), expected, message);
    }
    if (function == "sub") {
        return compare_series(num::sub(a, b), expected, message);
    }
    if (function == "mul") {
        return compare_series(num::mul(a, b), expected, message);
    }
    if (function == "div") {
        return compare_series(num::div(a, b), expected, message);
    }
    if (function == "maximum") {
        return compare_series(num::maximum(a, b), expected, message);
    }
    if (function == "minimum") {
        return compare_series(num::minimum(a, b), expected, message);
    }

    // elementwise: 标量
    if (function == "add_scalar") {
        return compare_series(num::add_scalar(a, n), expected, message);
    }
    if (function == "sub_scalar") {
        return compare_series(num::sub_scalar(a, n), expected, message);
    }
    if (function == "mul_scalar") {
        return compare_series(num::mul_scalar(a, n), expected, message);
    }
    if (function == "div_scalar") {
        return compare_series(num::div_scalar(a, n), expected, message);
    }
    if (function == "maximum_scalar") {
        return compare_series(num::maximum_scalar(a, n), expected, message);
    }
    if (function == "minimum_scalar") {
        return compare_series(num::minimum_scalar(a, n), expected, message);
    }

    // elementwise: 条件选择 (a 列为 0/1 条件)
    if (function == "where") {
        return compare_series(num::where(parse_bools(row.fields[2]), b, c), expected, message);
    }

    // logic: 比较
    if (function == "gt") {
        return compare_bools(num::gt(a, b), expected_bools, message);
    }
    if (function == "gte") {
        return compare_bools(num::gte(a, b), expected_bools, message);
    }
    if (function == "lt") {
        return compare_bools(num::lt(a, b), expected_bools, message);
    }
    if (function == "lte") {
        return compare_bools(num::lte(a, b), expected_bools, message);
    }
    if (function == "equal") {
        return compare_bools(num::equal(a, b), expected_bools, message);
    }
    if (function == "not_equal") {
        return compare_bools(num::not_equal(a, b), expected_bools, message);
    }

    // logic: 布尔
    if (function == "to_bools") {
        return compare_bools(num::to_bools(a), expected_bools, message);
    }
    if (function == "logic_and") {
        return compare_bools(num::logic_and(parse_bools(row.fields[2]), parse_bools(row.fields[3])),
                             expected_bools, message);
    }
    if (function == "logic_or") {
        return compare_bools(num::logic_or(parse_bools(row.fields[2]), parse_bools(row.fields[3])),
                             expected_bools, message);
    }
    if (function == "logic_not") {
        return compare_bools(num::logic_not(parse_bools(row.fields[2])), expected_bools, message);
    }
    if (function == "all_of") {
        return compare_series({num::all_of(parse_bools(row.fields[2])) ? 1.0 : 0.0}, expected,
                              message);
    }
    if (function == "any_of") {
        return compare_series({num::any_of(parse_bools(row.fields[2])) ? 1.0 : 0.0}, expected,
                              message);
    }
    if (function == "count_true") {
        return compare_series({static_cast<double>(num::count_true(parse_bools(row.fields[2])))},
                              expected, message);
    }

    // reduce
    if (function == "sum_value") {
        return compare_series({num::sum_value(a)}, expected, message);
    }
    if (function == "mean") {
        return compare_series({num::mean(a)}, expected, message);
    }
    if (function == "median") {
        return compare_series({num::median(a)}, expected, message);
    }
    if (function == "min_value") {
        return compare_series({num::min_value(a)}, expected, message);
    }
    if (function == "max_value") {
        return compare_series({num::max_value(a)}, expected, message);
    }
    if (function == "argmin") {
        return compare_series({static_cast<double>(num::argmin(a))}, expected, message);
    }
    if (function == "argmax") {
        return compare_series({static_cast<double>(num::argmax(a))}, expected, message);
    }
    if (function == "variance") {
        return compare_series({num::variance(a)}, expected, message);
    }
    if (function == "std") {
        return compare_series({num::stddev(a)}, expected, message);
    }
    if (function == "covariance") {
        return compare_series({num::covariance(a, b)}, expected, message);
    }

    // rolling
    if (function == "rolling_sum") {
        return compare_series(num::rolling_sum(a, window_of(row)), expected, message);
    }
    if (function == "rolling_mean") {
        return compare_series(num::rolling_mean(a, window_of(row)), expected, message);
    }
    if (function == "rolling_std") {
        return compare_series(num::rolling_std(a, window_of(row)), expected, message);
    }
    if (function == "rolling_min") {
        return compare_series(num::rolling_min(a, window_of(row)), expected, message);
    }
    if (function == "rolling_max") {
        return compare_series(num::rolling_max(a, window_of(row)), expected, message);
    }
    if (function == "rolling_avedev") {
        return compare_series(num::rolling_avedev(a, window_of(row)), expected, message);
    }
    if (function == "rolling_hhvbars") {
        return compare_series(num::rolling_hhvbars(a, window_of(row)), expected, message);
    }
    if (function == "rolling_llvbars") {
        return compare_series(num::rolling_llvbars(a, window_of(row)), expected, message);
    }

    // ewm
    if (function == "ewm_mean") {
        return compare_series(num::ewm_mean(a, n), expected, message);
    }

    // shift
    if (function == "shift") {
        return compare_series(num::shift(a, static_cast<std::int64_t>(n)), expected, message);
    }
    if (function == "ref") {
        return compare_series(num::ref(a, static_cast<std::int64_t>(n)), expected, message);
    }
    if (function == "diff") {
        return compare_series(num::diff(a, static_cast<std::int64_t>(n)), expected, message);
    }
    if (function == "cumsum") {
        return compare_series(num::cumsum(a), expected, message);
    }

    // fillna
    if (function == "fill") {
        return compare_series(num::fill(a, n), expected, message);
    }
    if (function == "fill_na") {
        return compare_series(num::fill_na(a, n), expected, message);
    }
    if (function == "is_na") {
        return compare_bools(num::is_na(a), expected_bools, message);
    }

    // align / repeat
    if (function == "align") {
        return compare_series(num::align(a, static_cast<std::size_t>(n)), expected, message);
    }
    if (function == "pad_to") {
        return compare_series(num::pad_to(a, static_cast<std::size_t>(n)), expected, message);
    }
    if (function == "repeat") {
        return compare_series(num::repeat(a, static_cast<std::size_t>(n)), expected, message);
    }

    // regression
    if (function == "linear_regression") {
        const auto result = num::linear_regression(a);
        return compare_series({result.first, result.second}, expected, message);
    }
    if (function == "slope") {
        return compare_series({num::slope(a)}, expected, message);
    }
    if (function == "forcast") {
        return compare_series({num::forcast(a)}, expected, message);
    }

    // decimals
    if (function == "decimal") {
        return compare_series({num::decimal(a.empty() ? num::nan_value() : a[0],
                                             static_cast<int>(n))},
                              expected, message);
    }
    if (function == "decimal_series") {
        return compare_series(num::decimal_series(a, static_cast<int>(n)), expected, message);
    }
    if (function == "change_rate") {
        return compare_series({num::change_rate(a[0], b[0])}, expected, message);
    }
    if (function == "net_change_rate") {
        return compare_series({num::net_change_rate(a[0], b[0])}, expected, message);
    }

    // matrix
    if (function == "shape") {
        return compare_series({static_cast<double>(num::shape(a)[0])}, expected, message);
    }
    if (function == "concat1d") {
        return compare_series(num::concat1d({a, b}), expected, message);
    }
    if (function == "transpose2d") {
        return compare_series(a, expected, message);
    }
    if (function == "zeros") {
        return compare_series(num::zeros(static_cast<std::size_t>(n)), expected, message);
    }
    if (function == "ones") {
        return compare_series(num::ones(static_cast<std::size_t>(n)), expected, message);
    }
    if (function == "arange") {
        if (a.empty()) {
            return compare_series(num::arange(0.0, n, 1.0), expected, message);
        }
        return compare_series(num::arange(a[0], b[0], c.empty() ? 1.0 : c[0]), expected, message);
    }
    if (function == "linspace") {
        return compare_series(num::linspace(0.0, 1.0, static_cast<std::size_t>(n)), expected,
                              message);
    }

    message = "未知算子: " + function;
    return false;
}

}  // namespace

/// 定位样本文件
std::filesystem::path locate(int argc, char **argv) {
    if (argc > 1) {
        return argv[1];
    }
    const std::filesystem::path source(__FILE__);
    const auto root = source.parent_path().parent_path();
    const std::vector<std::filesystem::path> candidates = {
        root / "tests" / "fixtures" / "num" / "cases.csv",
        "tests/fixtures/num/cases.csv",
        "../tests/fixtures/num/cases.csv",
    };
    for (const auto &candidate : candidates) {
        if (std::filesystem::exists(candidate)) {
            return candidate;
        }
    }
    return {};
}

int main(int argc, char **argv) {
    const auto fixture = locate(argc, argv);
    if (fixture.empty()) {
        std::fprintf(stderr, "找不到 cases.csv\n");
        return 2;
    }
    std::ifstream stream(fixture);

    std::string line;
    std::getline(stream, line);  // 表头
    std::size_t total = 0;
    std::size_t passed = 0;
    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }
        const auto fields = split(line);
        if (fields.size() != 8) {
            std::fprintf(stderr, "行字段数错误: %s\n", line.c_str());
            return 2;
        }
        Row row;
        for (std::size_t index = 0; index < 8; ++index) {
            row.fields[index] = fields[index];
        }
        const std::string &function = row.fields[0];
        const bool bool_series = function == "gt" || function == "gte" || function == "lt" ||
                                 function == "lte" || function == "equal" ||
                                 function == "not_equal" || function == "to_bools" ||
                                 function == "logic_and" || function == "logic_or" ||
                                 function == "logic_not" || function == "is_na";
        std::string message;
        ++total;
        if (run_case(row, bool_series, message)) {
            ++passed;
        } else {
            std::fprintf(stderr, "FAIL %s/%s: %s\n", function.c_str(), row.fields[1].c_str(),
                         message.c_str());
        }
    }

    std::printf("num parity: %zu/%zu passed\n", passed, total);
    return passed == total ? 0 : 1;
}
