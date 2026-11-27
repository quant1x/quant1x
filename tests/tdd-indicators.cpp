// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

/// indicators 跨语言对拍: 读 `tests/fixtures/formula/indicators/` 的 K 线与期望列,
/// 逐值比较 C++ 与 Python 的输出。期望值只由 Python 生成。

#include <quant1x/data/schema/bar.h>
#include <quant1x/formula/formula.h>
#include <quant1x/indicators/cdtd.h>
#include <quant1x/indicators/f89k.h>
#include <quant1x/indicators/hjk.h>
#include <quant1x/indicators/ma1x.h>
#include <quant1x/indicators/p2fb.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

using quant1x::data::schema::Bar;
using quant1x::formula::Series;

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

/// 读整张表, 首行是表头
bool read_table(const std::filesystem::path &path, std::vector<std::vector<std::string>> &rows) {
    std::ifstream stream(path);
    if (!stream) {
        return false;
    }
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        rows.push_back(split(line));
    }
    return !rows.empty();
}

/// 取列号
std::size_t find_column(const std::vector<std::string> &header, const std::string &name) {
    for (std::size_t index = 0; index < header.size(); ++index) {
        if (header[index] == name) {
            return index;
        }
    }
    return header.size();
}

/// 空串视为 NaN
double parse_double(const std::string &text) {
    if (text.empty()) {
        return quant1x::formula::nan_value();
    }
    return std::strtod(text.c_str(), nullptr);
}

/// 定位样本目录
std::filesystem::path locate() {
    const std::filesystem::path source(__FILE__);
    const auto root = source.parent_path().parent_path();
    const std::vector<std::filesystem::path> candidates = {
        root / "tests" / "fixtures" / "formula" / "indicators",
        "tests/fixtures/formula/indicators",
        "../tests/fixtures/formula/indicators",
    };
    for (const auto &candidate : candidates) {
        if (std::filesystem::exists(candidate)) {
            return candidate;
        }
    }
    return {};
}

/// 读 K 线
std::vector<Bar> load_bars(const std::filesystem::path &path) {
    std::vector<std::vector<std::string>> rows;
    if (!read_table(path, rows)) {
        return {};
    }
    const auto &header = rows.front();
    const std::size_t date = find_column(header, "date");
    const std::size_t open = find_column(header, "open");
    const std::size_t close = find_column(header, "close");
    const std::size_t high = find_column(header, "high");
    const std::size_t low = find_column(header, "low");
    const std::size_t volume = find_column(header, "volume");
    const std::size_t amount = find_column(header, "amount");

    std::vector<Bar> bars;
    bars.reserve(rows.size() - 1);
    for (std::size_t index = 1; index < rows.size(); ++index) {
        Bar bar;
        bar.date = date < rows[index].size() ? rows[index][date] : "";
        bar.open = parse_double(rows[index][open]);
        bar.close = parse_double(rows[index][close]);
        bar.high = parse_double(rows[index][high]);
        bar.low = parse_double(rows[index][low]);
        bar.volume = parse_double(rows[index][volume]);
        bar.amount = parse_double(rows[index][amount]);
        bars.push_back(bar);
    }
    return bars;
}

/// 读期望列
Series load_column(const std::filesystem::path &path, const std::string &name) {
    std::vector<std::vector<std::string>> rows;
    if (!read_table(path, rows)) {
        return {};
    }
    const std::size_t column = find_column(rows.front(), name);
    if (column == rows.front().size()) {
        std::fprintf(stderr, "%s 缺少列 %s\n", path.string().c_str(), name.c_str());
        return {};
    }
    Series out = quant1x::formula::make_series(rows.size() - 1);
    for (std::size_t index = 1; index < rows.size(); ++index) {
        out[index - 1] = parse_double(rows[index][column]);
    }
    return out;
}

/// 比较并打印
int compare(const std::string &label, const Series &actual, const Series &expected) {
    if (actual.size() != expected.size()) {
        std::fprintf(stderr, "%s 长度不一致: %zu != %zu\n", label.c_str(), actual.size(), expected.size());
        return 1;
    }
    int mismatches = 0;
    for (std::size_t index = 0; index < actual.size(); ++index) {
        const double left = actual[index];
        const double right = expected[index];
        const bool equal = (std::isnan(left) && std::isnan(right)) || left == right;
        if (!equal) {
            ++mismatches;
            if (mismatches <= 5) {
                std::fprintf(stderr, "%s[%zu] 实际=%.17g 期望=%.17g\n", label.c_str(), index, left, right);
            }
        }
    }
    return mismatches;
}

}  // namespace

int main() {
    const auto directory = locate();
    if (directory.empty()) {
        std::fprintf(stderr, "找不到 indicators 样本目录\n");
        return 2;
    }
    const auto bars = load_bars(directory / "bars.csv");
    if (bars.empty()) {
        std::fprintf(stderr, "K 线为空\n");
        return 2;
    }

    int failures = 0;
    const auto cdtd_result = quant1x::indicators::cdtd(bars);
    const auto f89k_result = quant1x::indicators::f89k(bars);
    const auto hjk_result = quant1x::indicators::hjk(bars, 0.10);
    const auto ma1x_result = quant1x::indicators::ma1x(bars);
    const auto p2fb_result = quant1x::indicators::p2fb(bars, 0.10);

    const auto check = [&](const char *file, const char *column, const Series &actual) {
        failures += compare(std::string(file) + "." + column, actual,
                            load_column(directory / file, column));
    };
    const auto check_bool = [&](const char *file, const char *column, const quant1x::formula::Bools &actual) {
        check(file, column, quant1x::formula::as_series(actual));
    };

    check("cdtd.csv", "cdtd.wen", cdtd_result.wen);
    check_bool("cdtd.csv", "cdtd.S1", cdtd_result.s1);
    check_bool("cdtd.csv", "cdtd.B", cdtd_result.b);
    check_bool("cdtd.csv", "cdtd.S2", cdtd_result.s2);
    check_bool("cdtd.csv", "cdtd.S3", cdtd_result.s3);

    check("f89k.csv", "f89k.zs", f89k_result.zs);
    check_bool("f89k.csv", "f89k.b", f89k_result.b);
    check("f89k.csv", "f89k.DN", f89k_result.dn);

    check_bool("hjk.csv", "hjk.b", hjk_result.b);

    check("ma1x.csv", "DN", ma1x_result.dn);
    check("ma1x.csv", "ZS", ma1x_result.zs);
    check("ma1x.csv", "YL", ma1x_result.yl);
    check_bool("ma1x.csv", "ma1x.hc", ma1x_result.hc);
    check_bool("ma1x.csv", "ma1x.tp", ma1x_result.tp);

    check("p2fb.csv", "p2fb.hgzx", p2fb_result.hgzx);

    if (failures != 0) {
        std::fprintf(stderr, "indicators 对拍失败: %d 处\n", failures);
        return 1;
    }
    std::printf("indicators 对拍通过\n");
    return 0;
}
