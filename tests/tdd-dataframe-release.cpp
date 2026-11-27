// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#include <quant1x/test/test.h>

#include <quant1x/config/cache.h>
#include <quant1x/contrib/data/tdx/bar.h>
#include <quant1x/pandas/dataframe.h>

#include <algorithm>
#include <string>
#include <vector>

namespace pandas = quant1x::pandas;
namespace config = quant1x::config;
namespace tdx = quant1x::contrib::data::tdx;

// Boost.PFR 需要外部链接的类型, 测试结构体放在全局作用域
struct Person {
    int         id;
    std::string name;
    double      score;
};

struct StockData {
    std::string date;
    double      open;
    double      high;
    double      low;
    double      close;
    int         volume;
};

std::string make_date(int day) {
    std::string text = std::to_string(day);
    return "2023-01-" + std::string(2 - std::min<std::size_t>(2, text.size()), '0') + text;
}

TEST_CASE("dataframe-struct-fields", "[dataframe]") {
    std::vector<Person> people = {
        {1, "Alice", 95.5},
        {2, "Bob", 88.0},
        {3, "Charlie", 91.2},
    };

    auto df = pandas::DataFrame::from_struct_vector(people);

    REQUIRE(df.row_count() == 3);
    REQUIRE(df.column_count() == 3);
    REQUIRE(df.columns() == std::vector<std::string>{"id", "name", "score"});
    REQUIRE(df.contains("name"));
    REQUIRE_FALSE(df.contains("missing"));

    // 算数字段进 Series（int 收敛为 double）
    const auto *ids = df.get("id");
    REQUIRE(ids != nullptr);
    REQUIRE(ids->size() == 3);
    REQUIRE(ids->get_f64(0) == 1.0);
    REQUIRE(ids->get_f64(2) == 3.0);
    REQUIRE(df.get_strings("id") == nullptr);

    // 字符串字段是展示列
    const auto *names = df.get_strings("name");
    REQUIRE(names != nullptr);
    REQUIRE((*names)[1] == "Bob");
    REQUIRE(df.get("name") == nullptr);

    const auto text = df.to_string();
    REQUIRE(text.find("name") != std::string::npos);
    REQUIRE(text.find("Charlie") != std::string::npos);
    REQUIRE(text.find("Alice") != std::string::npos);
}

TEST_CASE("dataframe-print-columns", "[dataframe]") {
    std::vector<StockData> stocks(15);
    for (int i = 0; i < 15; ++i) {
        stocks[i] = {
            make_date(i + 1),
            145.0 + i * 0.5,
            148.0 + i * 0.5,
            144.0 + i * 0.5,
            147.0 + i * 0.5,
            90000000 + i * 1000000,
        };
    }

    auto df = pandas::DataFrame::from_struct_vector(stocks);
    REQUIRE(df.row_count() == 15);
    REQUIRE(df.column_count() == 6);

    // 逐列统一小数位：close 列的值都有 1 位小数
    const auto close_cells = df.to_string();
    REQUIRE(close_cells.find("147.0") != std::string::npos);

    // 表头按插入序排列：date 在 open 之前
    REQUIRE(close_cells.find("date") < close_cells.find("open"));

    // 未截断时没有页脚
    REQUIRE(close_cells.find("rows x") == std::string::npos);
}

TEST_CASE("dataframe-print-truncate", "[dataframe]") {
    std::vector<Person> people(70);
    for (int i = 0; i < 70; ++i) {
        people[i] = {i, "P" + std::to_string(i), static_cast<double>(i)};
    }

    auto df = pandas::DataFrame::from_struct_vector(people);
    const auto text = df.to_string();

    // 首尾各 5 行 + `..` 省略行 + 页脚
    REQUIRE(text.find("P0") != std::string::npos);
    REQUIRE(text.find("P4") != std::string::npos);
    REQUIRE(text.find("P5") == std::string::npos);
    REQUIRE(text.find("P65") != std::string::npos);
    REQUIRE(text.find("..") != std::string::npos);
    REQUIRE(text.find("[70 rows x 3 columns]") != std::string::npos);
}

TEST_CASE("dataframe-column-ops", "[dataframe]") {
    pandas::DataFrame df;
    df.set("open", pandas::Series(std::vector<double>{1.0, 2.0, 3.0}));
    df.set("close", pandas::Series(std::vector<double>{1.5, 2.5, 3.5}));

    REQUIRE(df.columns() == std::vector<std::string>{"open", "close"});
    const auto [rows, columns] = df.shape();
    REQUIRE(rows == 3);
    REQUIRE(columns == 2);

    // 原位替换不改变列序
    df.set("open", pandas::Series(std::vector<double>{10.0, 20.0, 30.0}));
    REQUIRE(df.columns() == std::vector<std::string>{"open", "close"});
    REQUIRE(df.get("open")->get_f64(2) == 30.0);

    // 列长不一致拒绝写入
    REQUIRE_THROWS(df.set("volume", pandas::Series(std::vector<double>{1.0})));

    // 子集与深拷贝
    auto subset = df.select({"close"});
    REQUIRE(subset.column_count() == 1);
    REQUIRE(subset.row_count() == 3);

    auto copied = df.copy();
    copied.set("open", pandas::Series(std::vector<double>{0.0, 0.0, 0.0}));
    REQUIRE(df.get("open")->get_f64(0) == 10.0);
    REQUIRE(copied.get("open")->get_f64(0) == 0.0);
}

TEST_CASE("dataframe-empty", "[dataframe]") {
    pandas::DataFrame df;
    REQUIRE(df.row_count() == 0);
    REQUIRE(df.column_count() == 0);
    REQUIRE(df.to_string() == "Empty DataFrame\nColumns: []\nIndex: []");
}

TEST_CASE("dataframe-from-bars", "[dataframe]") {
    std::string code = "sh603338";
    std::string cache_filename = config::get_bar_filename(code);
    auto bars = tdx::read_bar_from_csv(cache_filename);
    if (bars.empty()) {
        SKIP("本地无 " + code + " 的缓存, 跳过");
    }

    auto df = pandas::DataFrame::from_struct_vector(bars);
    REQUIRE(df.row_count() == bars.size());
    REQUIRE(df.contains("close"));
    REQUIRE(df.contains("date"));

    const auto *close = df.get("close");
    REQUIRE(close != nullptr);
    REQUIRE(close->get_f64(0) == bars[0].close);
    REQUIRE(close->get_f64(bars.size() - 1) == bars.back().close);

    const auto *dates = df.get_strings("date");
    REQUIRE(dates != nullptr);
    REQUIRE((*dates)[0] == bars[0].date);

    // 打印保留：字符串列左对齐、数值列右对齐
    const auto text = df.to_string();
    REQUIRE(text.find("date") != std::string::npos);
    REQUIRE(text.find("close") != std::string::npos);
}
