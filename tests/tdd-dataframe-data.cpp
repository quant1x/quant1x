#include <quant1x/test/test.h>
#include <quant1x/runtime/config.h>
#include <quant1x/datasets.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>

#include <boost/pfr.hpp>

std::string join(const std::vector<std::string>& parts, const std::string& sep) {
    std::string result;
    for (size_t i = 0; i < parts.size(); ++i) {
        result += parts[i];
        if (i != parts.size() - 1) {
            result += sep;
        }
    }
    return result;
}

struct Series {
    std::string type;
    std::vector<std::string> data;
};

class DataFrame {
public:
    std::vector<std::string> columns;
    std::vector<Series> seriesList;
    size_t                   nRows_ = 0;
    size_t                   nCols_ = 0;
    std::exception_ptr err = nullptr;

    DataFrame() = default;

    bool hasError() const { return err != nullptr; }

    std::string getError() const {
        if (!err) return "";
        try {
            rethrow_exception(err);
        } catch (const std::exception& e) {
            return e.what();
        }
    }

    size_t               Nrow() const { return nRows_; }
    size_t               Ncol() const { return nCols_; }
    std::tuple<size_t, size_t> Dims() const { return {Nrow(), Ncol()}; }

    std::vector<std::vector<std::string>> Records(bool includeHeader) const {
        std::vector<std::vector<std::string>> result;
        if (includeHeader) {
            result.push_back(columns);
        }
        for (size_t i = 0; i < Nrow(); ++i) {
            std::vector<std::string> row;
            for (const auto& s : seriesList) {
                row.push_back(s.data[i]);
            }
            result.push_back(row);
        }
        return result;
    }

    std::vector<std::string> Types() const {
        std::vector<std::string> types;
        for (const auto& s : seriesList) {
            types.push_back(s.type);
        }
        return types;
    }

    DataFrame Subset(size_t start, size_t end) const {
        DataFrame df;
        df.columns = columns;
        df.nRows_ = end - start;
        for (const auto& s : seriesList) {
            Series newS;
            newS.type = s.type;
            newS.data.assign(s.data.begin() + start, s.data.begin() + end);
            df.seriesList.push_back(newS);
        }
        return df;
    }

    std::string print(
        bool shortRows,
        bool shortCols,
        bool showDims,
        bool showTypes,
        int maxRows,
        int maxCharsTotal,
        const std::string& className
    );
};

std::string DataFrame::print(
    bool shortRows,
    bool shortCols,
    bool showDims,
    bool showTypes,
    int maxRows,
    int maxCharsTotal,
    const std::string& className
) {
    // Helper functions
    auto runeCount = [](const std::string& s) -> int {
        int count = 0;
        const char* p = s.c_str();
        while (*p) {
            unsigned char c = *p++;
            if ((c & 0x80) == 0) count++;         // ASCII
            else if ((c & 0xE0) == 0xC0) p += 1;   // 2-byte
            else if ((c & 0xF0) == 0xE0) p += 2;   // 3-byte
            else if ((c & 0xF8) == 0xF0) p += 3;   // 4-byte
        }
        return count;
    };

    auto addRightPadding = [&](const std::string& s, int nchar) -> std::string {
        int len = runeCount(s);
        if (len < nchar) {
            return s + std::string(nchar - len, ' ');
        }
        return s;
    };

    auto addLeftPadding = [&](const std::string& s, int nchar) -> std::string {
        int len = runeCount(s);
        if (len < nchar) {
            return std::string(nchar - len, ' ') + s;
        }
        return s;
    };

    std::ostringstream oss;
    if (hasError()) {
        oss << className << " error: " << getError();
        return oss.str();
    }

    size_t ncols = seriesList.size();
    size_t nrows = Nrow();

    if (nrows == 0 || ncols == 0) {
        oss << "Empty " << className;
        return oss.str();
    }

    std::vector<std::vector<std::string>> records;
    bool shortening = false;
    size_t                                nMinRows   = maxRows / 2;
    size_t                                nTotal     = 0;

    if (shortRows && nrows > maxRows) {
        shortening = true;
        DataFrame head = Subset(0, nMinRows);
        records = head.Records(true);
        nTotal += head.Nrow();

        std::vector<std::string> dots(ncols, "...");
        records.push_back(dots);
        nTotal += 1;

        DataFrame tail = Subset(nrows - nMinRows, nrows);
        auto tailRecords = tail.Records(true);
        records.insert(records.end(), tailRecords.begin() + 1, tailRecords.end());
        nTotal += tail.Nrow();
    } else {
        records = this->Records(true);
        nTotal += nrows;
    }
    (void)nTotal;

    if (showDims) {
        oss << "[" << nrows << "x" << ncols << "] " << className << "\n\n";
    }

    // Add row numbers
    size_t rowNumbersOffset = 0;
    for (size_t i = 0; i < records.size(); ++i) {
        std::string rowNumber;
        if (i == 0) {
            rowNumber = "";
        } else if (i == nMinRows + 1 && shortening) {
            rowNumbersOffset -= 1;
            rowNumber = "";
        } else {
            size_t idx = (i <= nMinRows || !shortening)
                      ? i - 1 + rowNumbersOffset
                      : nrows - maxRows + i - 1 + rowNumbersOffset;
            rowNumber = std::to_string(idx) + ":";
        }
        records[i].insert(records[i].begin(), rowNumber);
    }

    std::vector<std::string> typesRow(ncols + 1);
    typesRow[0] = "";
    auto types = Types();
    for (int i = 0; i < ncols; ++i) {
        typesRow[i + 1] = "<" + types[i] + ">";
    }

    if (showTypes) {
        records.push_back(typesRow);
    }

    // Compute column widths
    std::vector<int> maxChars(ncols + 1, 0);
    for (const auto& row : records) {
        for (size_t j = 0; j < row.size(); ++j) {
            int len = runeCount(row[j]);
            if (len > maxChars[j]) {
                maxChars[j] = len;
            }
        }
    }

    size_t                   maxCols = ncols + 1;
    std::vector<std::string> notShowing;

    if (shortCols) {
        int colWidth = 0;
        for (int column = 1; column <= ncols; ++column) {
            colWidth += maxChars[column];
            if (colWidth > maxCharsTotal) {
                maxCols = column;
                break;
            }
        }

        if (maxCols <= ncols) {
            notShowing.reserve(ncols - maxCols + 1);
            for (size_t i = maxCols; i <= ncols; ++i) {
                notShowing.push_back(records[0][i] + " " + typesRow[i]);
            }
        }
    }

    // Format and output rows
    for (auto& row : records) {
        if (row.empty()) continue;

        row[0] = addLeftPadding(row[0], maxChars[0] + 1);

        for (size_t j = 1; j < row.size(); ++j) {
            if (int(j) >= maxCols) break;
            row[j] = addRightPadding(row[j], maxChars[j]);
        }

        std::vector<std::string> trimmedRow;
        for (int j = 0; j < maxCols; ++j) {
            if (j < static_cast<int>(row.size())) {
                trimmedRow.push_back(row[j]);
            }
        }

        if (shortCols && !notShowing.empty()) {
            trimmedRow.push_back("...");
        }

        oss << join(trimmedRow, " ") << "\n";
    }

    // Handle not showing info
    if (shortCols && !notShowing.empty()) {
        std::string notShown;
        std::string line;
        int lineLen = 0;

        for (size_t i = 0; i < notShowing.size(); ++i) {
            const std::string& field = notShowing[i];
            int len = runeCount(field);
            if (lineLen + len + 2 > maxCharsTotal && !line.empty()) {
                notShown += line + ",\n";
                line = field;
                lineLen = len;
            } else {
                if (!line.empty()) {
                    line += ", ";
                }
                line += field;
                lineLen += len + 2;
            }
        }

        if (!line.empty()) {
            notShown += line;
        }

        oss << "\nNot Showing: " << notShown << "\n";
    }

    return oss.str();
}

// 示例结构体
struct Person {
    int id;
    std::string name;
    double salary;
};

template <typename T>
void print_with_names(const T& value) {
    constexpr auto names = boost::pfr::names_as_array<T>();

    boost::pfr::for_each_field(value, [&names](const auto& field, auto idx) {
        std::cout << names[idx] << ": " << field << std::endl;
    });
}

TEST_CASE("xtensor-load-csv", "[dataframe]") {
    std::string code = "sh603338";
    std::string cache_filename = config::get_kline_filename(code);
    std::cout << "code=" << code << ", cache=" << cache_filename << std::endl;

    DataFrame df;
    df.columns = {"id", "name", "age"};
    df.nRows_ = 5;
    df.seriesList = {
        {"int", {"1", "2", "3", "4", "5"}},
        {"str", {"Alice", "Bob", "Charlie", "David", "Eve"}},
        {"int", {"23", "30", "25", "40", "29"}}
    };

    std::cout << df.print(true, true, true, true, 10, 70, "DataFrame") << std::endl;
    std::cout <<"" <<boost::pfr::tuple_size<Series>::value;
    Person p = {1, "abc",2};
    std::cout << " fields: " << boost::pfr::io(p) << "\n";
    boost::pfr::for_each_field(p, [](const auto& field, std::size_t index) {
        std::cout<< "field=" << field << ":" << index << "\n";
    });
    print_with_names(p);
}

//#include <DataFrame/DataFrame.h>
//#include <DataFrame/DataFrameTypes.h>
#include <csv2/reader.hpp>


TEST_CASE("csv2-load-csv", "[dataframe]") {
    std::string code = "sh603338";
    std::string cache_filename = config::get_kline_filename(code);
    std::cout << "code=" << code << ", cache=" << cache_filename << std::endl;
    csv2::Reader<csv2::delimiter<','>,
    csv2::quote_character<'"'>,
    csv2::first_row_is_header<true>> csv{};

    if (csv.mmap(cache_filename)) {
        // 打印标题
        for (const auto& header : csv.header()) {
            std::string title;
            header.read_value(title);
            std::cout << title << " | ";
        }
        std::cout << "\n-----------------\n";

        // 打印数据
        for (const auto& row : csv) {
            for (const auto& cell : row) {
                std::string value;
                cell.read_value(value);
                std::cout << value << " | ";
            }
            std::cout << "\n";
        }
    } else {
        std::cerr << "无法打开文件\n";
    }
}

//// 正确声明DataFrame：第一个模板参数是索引类型，第二个是列名类型
//using MyDataFrame = hmdf::DataFrame<unsigned, std::string>;  // unsigned作为索引类型
//
//MyDataFrame load_csv_to_dataframe(const std::string &filename) {
//    MyDataFrame df;
//    csv2::Reader<csv2::delimiter<','>,
//        csv2::quote_character<'"'>,
//        csv2::first_row_is_header<true>> csv{};
//
//    if (csv.mmap(filename)) {
//        // 读取列名
//        std::vector<std::string> col_names;
//        for (const auto &header : csv.header()) {
//            std::string name;
//            header.read_value(name);
//            col_names.push_back(name);
//        }
//
//        // 准备数据容器
//        std::vector<std::string> dates;
//        std::vector<double> opens, closes;
//
//        // 填充数据（修改后的迭代器用法）
//        for (const auto &row : csv) {
//            auto it = row.begin();
//            // 读取日期列
//            std::string date;
//            if (it != row.end()) it->read_value(date);
//            dates.push_back(date);
//            if (it != row.end()) ++it;
//
//            // 读取开盘价
//            double open = 0.0;
//            if (it != row.end()) it->read_value(open);
//            opens.push_back(open);
//            if (it != row.end()) ++it;
//
//            // 读取收盘价
//            double close = 0.0;
//            if (it != row.end()) it->read_value(close);
//            closes.push_back(close);
//        }
//
//        // 加载到DataFrame（使用新API）
//        df.load_data(
//            MyDataFrame::gen_icolumn_index(dates.size()),  // 生成索引
//            std::make_pair("date", dates),
//            std::make_pair("open", opens),
//            std::make_pair("close", closes)
//        );
//    }
//
//    return df;
//}
//
//#include <DataFrame/DataFrame.h>  // 主头文件
//#include <DataFrame/Utils/DateTime.h>  // 如果需要处理时间数据
//
//using namespace hmdf;
//
//TEST_CASE("dataframe-load-csv", "[dataframe]") {
//    std::string code = "sh603338";
//    std::string cache_filename = config::get_kline_filename("sh603338");
//    std::cout << "code=" << code << ", cache=" << cache_filename << std::endl;
//
//    // 创建一个DataFrame对象
//    StdDataFrame<unsigned long> df;
//
//    // 从CSV文件读取数据
//    try {
//        // 创建ReadParams对象并设置参数
//        ReadParams params;
//        params.skip_first_line = true;  // 跳过第一行（通常是标题行）
//        params.columns_only = true;    // 读取索引列和数据列
//
//        // 正确方式1：使用文件流读取
//        //std::ifstream ifs(cache_filename);
//        //df.read(ifs, io_format::csv2);
//        df.read(cache_filename.c_str(),
//                io_format::csv2,
//                params);
//
//        auto c1 = df.get_column<double>("close");
//
////        // 或者正确方式2：使用文件路径（需要确保路径正确）
////        // df.read("data.csv", io_format::csv2);
////
////        // 获取列名 - 使用正确的API
////        const auto &col_map = df.get_columns_info<>();
////        for (const auto &[col_name, idx] : col_map) {
////            std::cout << col_name << " ";
////        }
////        std::cout << std::endl;
//
//        // 获取行数
//        std::cout << "行数: " << df.get_index().size() << std::endl;
//
//    } catch (const std::exception &ex) {
//        std::cerr << "读取CSV错误: " << ex.what() << std::endl;
//    }
//}

#include <boost/pfr.hpp>
#include <unordered_map>

// FNV-1a哈希（直接处理大小写）
struct FNV1aCaseInsensitiveHash {
    size_t operator()(const std::string& key) const noexcept {
        constexpr size_t FNV_offset_basis = 2166136261U;
        constexpr size_t FNV_prime = 16777619U;

        size_t hash = FNV_offset_basis;
        for (unsigned char c : key) {  // 注意：必须用unsigned char
            hash ^= std::tolower(c);
            hash *= FNV_prime;
        }
        return hash;
    }
};

// 配套的比较器
struct CaseInsensitiveEqual {
    bool operator()(const std::string& a, const std::string& b) const noexcept {
        if (a.length() != b.length()) return false;
        return std::equal(a.begin(), a.end(), b.begin(),
                          [](char x, char y) {
                              return std::tolower(x) == std::tolower(y);
                          });
    }
};

template<typename ValueType>
using CaseInsensitiveMap = std::unordered_map<
    std::string,
    ValueType,
    FNV1aCaseInsensitiveHash,
    CaseInsensitiveEqual
>;

template<typename Struct>
struct AutoCSVMapper {
    using FieldSetter = std::function<void(Struct&, const std::string&)>;
    using MapperType = CaseInsensitiveMap<FieldSetter>;

    static MapperType create_mapper() {
        MapperType mapper;
        constexpr auto field_count = boost::pfr::tuple_size_v<Struct>;

        auto worker = [&mapper]<size_t... Idx>(std::index_sequence<Idx...>) {
            ( [&]() {
                constexpr auto name = boost::pfr::get_name<Idx, Struct>();
                using Type = std::decay_t<decltype(
                boost::pfr::get<Idx>(std::declval<Struct>())
                )>;

                mapper[std::string(name)] = [](Struct& s, const std::string& val) {
                    auto& target = boost::pfr::get<Idx>(s);
                    if constexpr (std::is_same_v<Type, int>)
                        target = std::stoi(val);
                    else if constexpr (std::is_same_v<Type, double>)
                        target = std::stod(val);
                    else
                        target = val;
                };
            }(), ...);
        };

        worker(std::make_index_sequence<field_count>{});
        return mapper;
    }

    static Struct map_row(
        const std::vector<std::string>& row,
        const std::unordered_map<std::string, size_t>& col_index)
    {
        Struct result;
        static const auto mapper = create_mapper();

        for (const auto& [field_name, setter] : mapper) {
            if (col_index.count(field_name)) {
                size_t col_pos = col_index.at(field_name);
                if (col_pos < row.size()) {
                    setter(result, row[col_pos]);
                }
            }
        }
        return result;
    }
};

// CSV行转换工具
std::vector<std::string> csv_row_to_vector(
    const typename csv2::Reader<csv2::delimiter<','>>::Row& row)
{
    std::vector<std::string> values;
    for (auto it = row.begin(); it != row.end(); ++it) {
        std::string val;
        (*it).read_value(val);  // 修正迭代器解引用方式
        values.push_back(val);
    }
    return values;
}

#include <q1x/datasets/kline.h>

void process_csv(const std::string& filename) {
    csv2::Reader<csv2::delimiter<','>> reader;
    if (!reader.mmap(filename)) return;

    // 建立列名索引
    std::unordered_map<std::string, size_t> col_index;
    size_t idx = 0;
    for (const auto& cell : reader.header()) {
        std::string name;
        cell.read_value(name);
        col_index[name] = idx++;
    }

    // 处理数据行
    for (auto it = reader.begin(); it != reader.end(); ++it) {
        auto row_data = csv_row_to_vector(*it);
        datasets::KLine data = AutoCSVMapper<datasets::KLine>::map_row(row_data, col_index);
        // 使用data...
    }
}


TEST_CASE("struct-load-csv", "[dataframe]") {
    std::string code = "sh603338";
    std::string cache_filename = config::get_kline_filename(code);
    std::cout << "code=" << code << ", cache=" << cache_filename << std::endl;
    // b. 初始化mapper（全局唯一）
    using Mapper = AutoCSVMapper<datasets::KLine>;
    std::unordered_map<std::string, size_t> global_col_index;
    bool is_first = true;

    csv2::Reader<csv2::delimiter<','>> reader;
    reader.mmap(cache_filename);
    // c. 建立列名索引（只需首次）
    if (is_first) {
        size_t idx = 0;
        for (const auto& cell : reader.header()) {
            std::string name;
            cell.read_value(name);
            global_col_index[name] = idx++;
        }
        is_first = false;
    }

    // d. 处理所有行
    for (auto && it : reader) {
        auto row_data = csv_row_to_vector(it);
        datasets::KLine data = Mapper::map_row(row_data, global_col_index);
        std::cout << data << std::endl;
        // 使用data...
    }
}

#include <variant>
#include <vector>
#include <string>
#include <unordered_map>

// 类型转换工具（保持不变）
template<typename T>
T convert_from_string(const std::string& str) {
    if constexpr (std::is_same_v<T, int>) return std::stoi(str);
    else if constexpr (std::is_same_v<T, double>) return std::stod(str);
    else if constexpr (std::is_same_v<T, bool>) return (str == "1" || str == "true");
    else return str;
}

// 列处理器元数据
template<typename Struct>
struct csvToStructMapperWithLine {
    using FieldHandler = std::function<void(Struct&, const std::string&)>;
    std::unordered_map<size_t, FieldHandler> field_handlers;
    std::vector<size_t> required_columns;

    // 使用index_sequence解决捕获问题
    template<size_t... Idx>
    static void register_handlers(csvToStructMapperWithLine& mapper,
                                  const std::unordered_map<std::string_view , size_t>& col_index_map,
                                  std::index_sequence<Idx...>) {
        ( [&] {
            constexpr auto field_name = boost::pfr::get_name<Idx, Struct>();
            if (auto it = col_index_map.find(field_name); it != col_index_map.end()) {
                size_t col_idx = it->second;
                mapper.required_columns.push_back(col_idx);
                mapper.field_handlers[col_idx] = [](Struct& obj, const std::string& value) {
                    auto& field = boost::pfr::get<Idx>(obj);
                    field = convert_from_string<std::decay_t<decltype(field)>>(value);
                };
            }
        }(), ...);
    }

    static csvToStructMapperWithLine build(const std::vector<std::string>& csv_headers) {
        csvToStructMapperWithLine mapper;
        std::unordered_map<std::string_view , size_t> col_index_map;

        // 建立列名到索引的映射
        for (size_t i = 0; i < csv_headers.size(); ++i) {
            col_index_map[csv_headers[i]] = i;
        }

        // 注册处理器
        register_handlers(
            mapper,
            col_index_map,
            std::make_index_sequence<boost::pfr::tuple_size_v<Struct>>{}
        );

        return mapper;
    }

    void load_row(Struct& obj, const std::vector<std::string>& row_cells) const {
        for (size_t col_idx : required_columns) {
            if (col_idx < row_cells.size()) {
                if (auto it = field_handlers.find(col_idx); it != field_handlers.end()) {
                    it->second(obj, row_cells[col_idx]);
                }
            }
        }
    }
};

// CSV行转换（保持不变）
std::vector<std::string> csv_row_to_vector(const auto& row) {
    std::vector<std::string> cells;
    for (auto it = row.begin(); it != row.end(); ++it) {
        std::string val;
        it->read_value(val);
        cells.push_back(val);
    }
    return cells;
}


TEST_CASE("rows-load-csv", "[dataframe]") {
    std::string code = "sh603338";
    std::string cache_filename = config::get_kline_filename(code);
    std::cout << "code=" << code << ", cache=" << cache_filename << std::endl;

    // 1. 初始化CSV读取器
    csv2::Reader<csv2::delimiter<','>> reader;
    if (!reader.mmap(cache_filename)) return;

    // 2. 读取列名
    std::vector<std::string> headers;
    for (const auto& cell : reader.header()) {
        std::string name;
        cell.read_value(name);
        headers.push_back(name);
    }

    // 3. 构建映射关系（只需一次）
    auto mapper = csvToStructMapperWithLine<datasets::KLine>::build(headers);

    // 4. 处理数据行
    std::vector<datasets::KLine> dataset;
    for (auto it = reader.begin(); it != reader.end(); ++it) {
        datasets::KLine row;
        mapper.load_row(row, csv_row_to_vector(*it));
        dataset.push_back(row);
    }

    // 5. 使用数据（示例）
    for (const auto& data : dataset) {
        printf("date: %s, Price: %.2f, Volume: %f, Active: %d\n",
               data.Date.c_str(), data.Close, data.Volume, data.AdjustmentCount);
    }

}

// 类型转换工具
template<typename T>
T convert_from_string_v2(const std::string& str) {
    if constexpr (std::is_same_v<T, int>) return std::stoi(str);
    else if constexpr (std::is_same_v<T, double>) return std::stod(str);
    else if constexpr (std::is_same_v<T, bool>) return (str == "1" || str == "true");
    else return str;
}

// 列数据类型定义
using ColumnData = std::variant<
    std::vector<int>,
    std::vector<double>,
    std::vector<bool>,
    std::vector<std::string>
>;

using ColumnMap = std::unordered_map<std::string, ColumnData>;

// CSV行转换
std::vector<std::string> csv_row_to_vector_v2(const auto& row) {
    std::vector<std::string> cells;
    for (auto it = row.begin(); it != row.end(); ++it) {
        std::string val;
        it->read_value(val);
        cells.push_back(val);
    }
    return cells;
}

// 列处理器元数据
template<typename Struct>
struct CsvStructMapper {
    using FieldHandler = std::function<void(Struct&, const std::string&)>;
    using ColumnAppender = std::function<void(ColumnMap&, const Struct&, size_t)>;

    std::unordered_map<size_t, FieldHandler> field_handlers;
    std::unordered_map<size_t, ColumnAppender> column_appenders;
    std::vector<size_t> required_columns;

    // 加载行数据
    void load_row(Struct& obj, const std::vector<std::string>& row_cells) const {
        for (size_t col_idx : required_columns) {
            if (col_idx < row_cells.size()) {
                if (auto it = field_handlers.find(col_idx); it != field_handlers.end()) {
                    it->second(obj, row_cells[col_idx]);
                }
            }
        }
    }

    // 初始化列存储
    static void initialize_columns(ColumnMap& columns, const std::vector<std::string>& headers) {
        for (const auto& header : headers) {
            // 默认初始化为空的string列，后续会根据实际类型调整
            columns[header] = std::vector<std::string>{};
        }
    }

    // 注册字段处理器
    template<size_t Idx>
    static void register_handler(
        CsvStructMapper& mapper,
        const std::unordered_map<std::string, size_t>& col_index_map
    ) {
        constexpr auto field_name = boost::pfr::get_name<Idx, Struct>();
        std::string field_name_str(field_name);

        if (auto it = col_index_map.find(field_name_str); it != col_index_map.end()) {
            size_t col_idx = it->second;
            mapper.required_columns.push_back(col_idx);

            // 字段处理器（行式解析）
            mapper.field_handlers[col_idx] = [](Struct& obj, const std::string& value) {
                auto& field = boost::pfr::get<Idx>(obj);
                field = convert_from_string<std::decay_t<decltype(field)>>(value);
            };

            // 列追加器（列式存储）
            mapper.column_appenders[col_idx] = [field_name_str](
                ColumnMap& columns,
                const Struct& obj,
                [[maybe_unused]] size_t row_idx ) {
                auto& field = boost::pfr::get<Idx>(obj);
                auto& column = columns[field_name_str];

                // 根据字段类型处理
                using FieldType = std::decay_t<decltype(field)>;

                if constexpr (std::is_same_v<FieldType, int>) {
                    if (!std::holds_alternative<std::vector<int>>(column)) {
                        // 如果列类型不匹配，重新初始化
                        column = std::vector<int>{};
                    }
                    std::get<std::vector<int>>(column).push_back(field);
                }
                else if constexpr (std::is_same_v<FieldType, double>) {
                    if (!std::holds_alternative<std::vector<double>>(column)) {
                        column = std::vector<double>{};
                    }
                    std::get<std::vector<double>>(column).push_back(field);
                }
                else if constexpr (std::is_same_v<FieldType, bool>) {
                    if (!std::holds_alternative<std::vector<bool>>(column)) {
                        column = std::vector<bool>{};
                    }
                    std::get<std::vector<bool>>(column).push_back(field);
                }
                else if constexpr (std::is_same_v<FieldType, std::string>) {
                    if (!std::holds_alternative<std::vector<std::string>>(column)) {
                        column = std::vector<std::string>{};
                    }
                    std::get<std::vector<std::string>>(column).push_back(field);
                }
            };
        }
    }

    // 构建映射关系
    static CsvStructMapper build(const std::vector<std::string>& csv_headers) {
        CsvStructMapper mapper;
        std::unordered_map<std::string, size_t> col_index_map;

        for (size_t i = 0; i < csv_headers.size(); ++i) {
            col_index_map[csv_headers[i]] = i;
        }

        // 注册所有字段
        auto register_handlers = [&]<size_t... Idx>(std::index_sequence<Idx...>) {
            (register_handler<Idx>(mapper, col_index_map), ...);
        };
        register_handlers(std::make_index_sequence<boost::pfr::tuple_size_v<Struct>>{});

        return mapper;
    }
};

// 加载CSV到列式存储
template<typename Struct>
ColumnMap load_csv_to_columns(
    csv2::Reader<csv2::delimiter<','>>& reader,
    const std::vector<std::string>& headers
) {
    ColumnMap columns;
    CsvStructMapper<Struct>::initialize_columns(columns, headers);
    auto mapper = CsvStructMapper<Struct>::build(headers);

    size_t row_idx = 0;
    for (auto it = reader.begin(); it != reader.end(); ++it) {
        Struct row_data;
        auto cells = csv_row_to_vector(*it);
        mapper.load_row(row_data, cells);

        // 将数据追加到列
        for (size_t col_idx : mapper.required_columns) {
            if (auto it2 = mapper.column_appenders.find(col_idx); it2 != mapper.column_appenders.end()) {
                it2->second(columns, row_data, row_idx);
            }
        }
        row_idx++;
    }

    return columns;
}

TEST_CASE("load-csv-to-columns") {
    std::string code = "sh603338";
    std::string cache_filename = config::get_kline_filename(code);
    std::cout << "code=" << code << ", cache=" << cache_filename << std::endl;
    csv2::Reader<csv2::delimiter<','>> reader;
    if (!reader.mmap(cache_filename)) return;

    // 读取列名
    std::vector<std::string> headers;
    for (const auto& cell : reader.header()) {
        std::string name;
        cell.read_value(name);
        headers.push_back(name);
    }

    // 加载为列式存储
    ColumnMap columns = load_csv_to_columns<datasets::KLine>(reader, headers);
    std::cout << "column num=" << columns.size() << std::endl;

    // 访问某一列
    if (auto it = columns.find("Date"); it != columns.end()) {
        if (auto* close_prices = std::get_if<std::vector<std::string>>(&it->second)) {
            for (auto const & v : *close_prices) {
                std::cout << v << std::endl;
            }
        }
    }
}

struct CaseInsensitiveCompare {
    bool operator()(const std::string& a, const std::string& b) const {
        return std::equal(a.begin(), a.end(), b.begin(), b.end(),
                          [](char a, char b) { return std::tolower(a) == std::tolower(b); });
    }
};

struct CaseInsensitiveHash {
    size_t operator()(const std::string& key) const {
        std::string lower_key;
        std::transform(key.begin(), key.end(), std::back_inserter(lower_key),
                       [](char c) { return std::tolower(c); });
        return std::hash<std::string>{}(lower_key);
    }
};

using CaseInsensitiveMap2 = std::unordered_map<
    std::string,
    size_t,
    CaseInsensitiveHash,
    CaseInsensitiveCompare>;

TEST_CASE("Case-insensitive CSV loading") {
    std::string code = "sh603338";
    std::string cache_filename = config::get_kline_filename(code);
    std::cout << "code=" << code << ", cache=" << cache_filename << std::endl;
    // 1. 准备测试数据
    std::vector<std::string> headers = {"Date", "Open", "Close"};
    std::vector<std::vector<std::string>> test_data = {
        {"1", "Alice", "100.5"},
        {"2", "Bob", "200.3"}
    };

    // 2. 构建大小写不敏感的映射
    CaseInsensitiveMap2 col_map;
    for (size_t i = 0; i < headers.size(); ++i) {
        col_map[headers[i]] = i;
    }

    // 3. 测试大小写不敏感查找
    REQUIRE(col_map.find("date") != col_map.end());    // "id" 匹配 "ID"
    REQUIRE(col_map.find("open") != col_map.end());  // "NAME" 匹配 "Name"
    REQUIRE(col_map.find("cLosE") != col_map.end()); // "ValuE" 匹配 "Value"

    // 4. 实际CSV加载测试
    csv2::Reader<csv2::delimiter<','>> reader;
    REQUIRE(reader.mmap(cache_filename)); // 确保文件存在

    // 使用修改后的CsvStructMapper
    auto mapper = CsvStructMapper<datasets::KLine>::build(headers);

    // 验证字段匹配
    REQUIRE(mapper.required_columns.size() == headers.size());
}