#include <quant1x/test/test.h>

#include <csv2/reader.hpp>
#include <csv2/writer.hpp>
#include <boost/pfr.hpp>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>

// 假设这是你的结构体定义
struct Person {
    std::string name;
    int age;
    double height;
    std::string occupation;
};

// 类型转换工具
template <typename T>
void convert_from_string(const std::string& str, T& value) {
    std::string processed = str;
    if (!processed.empty() && processed.front() == '"' && processed.back() == '"') {
        processed = processed.substr(1, processed.size() - 2);
    }

    std::istringstream iss(processed);
    if (!(iss >> value)) value = T{};
}

template <>
void convert_from_string<std::string>(const std::string& str, std::string& value) {
//    if (!str.empty() && str.front() == '"' && str.back() == '"') {
//        value = str.substr(1, str.size() - 2);
//    } else {
//        value = str;
//    }
    value = str;
}

template <typename T>
struct CsvFieldMapping {
    std::vector<size_t> field_indices;

    static std::optional<CsvFieldMapping> create(
        const csv2::Reader<csv2::delimiter<','>, csv2::quote_character<'"'>, csv2::first_row_is_header<true>>& csv)
    {
        CsvFieldMapping mapping;
        constexpr size_t field_count = boost::pfr::tuple_size<T>::value;
        mapping.field_indices.resize(field_count, -1);

        // 构建CSV列名到索引的映射
        std::map<std::string, size_t, std::less<>> csv_col_map;
        size_t col_idx = 0;
        for (const auto& cell : csv.header()) {
            std::string col_name;
            cell.read_value(col_name);
            csv_col_map[col_name] = col_idx++;
        }

        // 使用索引遍历而非for_each_field来避免临时对象问题
        [&]<size_t... I>(std::index_sequence<I...>) {
            ((mapping.field_indices[I] = [&]() -> size_t {
                constexpr auto field_name = boost::pfr::get_name<I, T>();
                if (auto it = csv_col_map.find(field_name); it != csv_col_map.end()) {
                    return it->second;
                }
                return -1;
            }()), ...);
        }(std::make_index_sequence<field_count>{});

        return mapping;
    }
};

template <typename T>
std::vector<T> read_csv_with_name_mapping(const std::string& filename) {
    std::vector<T> result;
    csv2::Reader<csv2::delimiter<','>,
        csv2::quote_character<'"'>,
        csv2::first_row_is_header<true>> csv{};

    if (!csv.mmap(filename)) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return result;
    }

    auto mapping = CsvFieldMapping<T>::create(csv);
    if (!mapping) return result;

    // 处理每行数据
    for (const auto& row : csv) {
        T item;
        std::vector<std::string> row_data;

        for (const auto& cell : row) {
            std::string val{};
            cell.read_value(val);
            std::cout << ">>" << val << std::endl;
            row_data.push_back(val);
        }

        // 使用预计算的映射关系
        [&]<size_t... I>(std::index_sequence<I...>) {
            auto set_field = [&](auto idx, auto& field) {
                const size_t data_col = mapping->field_indices[idx];
                if (data_col != static_cast<size_t>(-1) && data_col < row_data.size()) {
                    convert_from_string(row_data[data_col], field);
                }
            };

            (set_field(I, boost::pfr::get<I>(item)), ...);
        }(std::make_index_sequence<boost::pfr::tuple_size<T>::value>{});

        result.push_back(item);
    }

    return result;
}

TEST_CASE("csv-read-slices", "[encoding]") {
    auto people = read_csv_with_name_mapping<Person>("test.csv");

    // 验证结果
    for (const auto& person : people) {
        std::cout << "Name: " << person.name
                  << ", Age: " << person.age
                  << ", Height: " << person.height
                  << ", Occupation: " << person.occupation
                  << std::endl;
    }
}

// 辅助函数：将任意类型转换为字符串并处理CSV转义
template <typename T>
std::string to_csv_string(const T& value) {
    std::ostringstream oss;
    if constexpr (std::is_arithmetic_v<T>) {
        oss << value; // 数值类型直接输出
    } else {
        // 字符串类型需要处理引号转义
        std::string str_value;
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
    }
    return oss.str();
}

// 主函数：将结构体vector写入CSV文件
template <typename T>
bool write_csv_file(const std::vector<T>& data, const std::string& filename) {
    std::ofstream out_file(filename);
    if (!out_file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    csv2::Writer<csv2::delimiter<','>> writer(out_file);

    // 1. 写入表头（使用结构体字段名）
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
            row.emplace_back(to_csv_string(field));
        });
        writer.write_row(row);
    }

    return true;
}

TEST_CASE("csv-write-slices", "[encoding]") {
    std::vector<Person> people = {
        {"John Doe", 30, 1.75, "Software Engineer"},
        {"Jane Smith", 28, 1.68, "Data Scientist"},
        {"Alice \"The Boss\"", 35, 1.72, "Manager, \"IT\" Department"}
    };

    if (write_csv_file(people, "output.csv")) {
        std::cout << "CSV file written successfully." << std::endl;
    }
}

