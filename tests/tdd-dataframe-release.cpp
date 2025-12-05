#include <quant1x/test/test.h>
#include <quant1x/config/base.h>
#include <quant1x/config/cache.h>

#include <vector>
#include <string>
#include <variant>
#include <type_traits>
#include <boost/pfr.hpp>

// 定义支持的列数据类型
using ColumnVariant = std::variant<
    std::vector<int>,
    std::vector<int64_t >,
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

        std::ostringstream oss;
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
private:

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

// 定义任意结构体
struct Person {
    int         id;
    std::string name;
    double      score;
};

TEST_CASE("struct-to-dataframe", "[dataframe]") {
    // 创建测试数据
    std::vector<Person> people = {
        {1, "Alice", 95.5},
        {2, "Bob", 88.0},
        {3, "Charlie", 91.2}
    };

    // 自动转换为 DataFrame
    DataFrame df = DataFrame::from_struct_vector(people);

    // 打印基本信息
    std::cout << "DataFrame with " << df.row_count() << " rows and "
              << df.column_count() << " columns\n";

    std::cout << "Column names: ";
    for (const auto& name : df.column_names()) {
        std::cout << name << " ";
    }
    std::cout << "\n";

    // 访问数据示例
    if (df.column_count() > 0) {
        const auto& ids = df.get_column<int>(0);
        std::cout << "First column values: ";
        for (auto id : ids) {
            std::cout << id << " ";
        }
        std::cout << "\n";
    }

    std::cout << df.to_string() << std::endl;
}

struct StockData {
    std::string date;
    double      open;
    double      high;
    double      low;
    double      close;
    int         volume;
};

TEST_CASE("v0-struct-to-dataframe-print", "[dataframe]") {
    // 创建15行数据
    std::vector<StockData> stocks(15);
    for (int i = 0; i < 15; ++i) {
        stocks[i] = {
            "2023-01-" + std::to_string(i+1).insert(0, 2 - std::to_string(i+1).length(), '0'),
            145.0 + i*0.5,
            148.0 + i*0.5,
            144.0 + i*0.5,
            147.0 + i*0.5,
            90000000 + i*1000000
        };
    }

    DataFrame df = DataFrame::from_struct_vector(stocks);
    std::cout << df.to_string() << std::endl;;
}

TEST_CASE("v1_struct-to-dataframe-print", "[dataframe]") {
    // 创建15行数据
    std::vector<StockData> stocks(10);
    for (int i = 0; i < 10; ++i) {
        stocks[i] = {
            "2023-01-" + std::to_string(i+1).insert(0, 2 - std::to_string(i+1).length(), '0'),
            145.0 + i*0.5,
            148.0 + i*0.5,
            144.0 + i*0.5,
            147.0 + i*0.5,
            90000000 + i*1000000
        };
    }

    DataFrame df = DataFrame::from_struct_vector(stocks);
    std::cout << df.to_string() << std::endl;;
}

#include <quant1x/datasets/kline.h>

TEST_CASE("v2_struct-to-dataframe-print", "[dataframe]") {
    std::string code = "sh603338";
    std::string cache_filename = config::get_kline_filename(code);
    auto klines = datasets::read_kline_from_csv(cache_filename);

    DataFrame df = DataFrame::from_struct_vector(klines);
    std::cout << df.to_string() << std::endl;
}

TEST_CASE("v3_struct-to-dataframe-print", "[dataframe]") {
    std::string code = "sh603338";
    std::string cache_filename = config::get_kline_filename(code);
    auto klines = datasets::read_kline_from_csv(cache_filename);

    // 1. 验证原始数据
    REQUIRE_FALSE(klines.empty());
    std::cout << "原始数据验证 - 第一条记录的close值: " << klines[0].Close << "\n";

    // 2. 转换为DataFrame
    DataFrame df = DataFrame::from_struct_vector(klines);

    // 3. 打印调试信息
    df.debug_print_columns();

//    // 4. 测试close列访问
//    try {
//        std::cout << "尝试访问close列...\n";
//        const auto& close_prices = static_cast<const std::vector<double>&>(df["Close"]);
//
//        std::cout << "close列大小: " << close_prices.size() << "\n";
//        if (!close_prices.empty()) {
//            std::cout << "前5个close值: ";
//            for (size_t i = 0; i < std::min(size_t(5), close_prices.size()); ++i) {
//                std::cout << close_prices[i] << " ";
//            }
//            std::cout << "\n";
//        }
//    } catch (const std::exception& e) {
//        FAIL("访问close列失败: " << e.what());
//    }
    // 直接获取列
    const auto& close_col = df["close"]; // 注意大小写匹配

    // 使用std::get获取具体vector
    try {
        const auto& close_prices = std::get<std::vector<double>>(close_col);
        for (const auto& price : close_prices) {
            std::cout << price << " ";
        }
    } catch (const std::bad_variant_access&) {
        std::cerr << "类型不匹配或列不存在";
    }
}

TEST_CASE("print-v1", "[dataframe]") {
    std::ostringstream oss1, oss2;

    const int width = 6;

    oss1 << std::setw(width) << std::right << "No." << " ";
    oss2 << std::setw(width) << std::left << "No.";

    std::cout << ">" << oss1.str() << "<" << std::endl; // > No.<
    std::cout << ">" << oss2.str() << "<" << std::endl; // >No. <
}