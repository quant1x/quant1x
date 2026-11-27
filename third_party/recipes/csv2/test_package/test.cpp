// csv2 引号解析回归：外层引号要剥离、内部 "" 要折叠、单引号原样保留。
#include <csv2/reader.hpp>

#include <cstdio>
#include <string>
#include <vector>

int main() {
    std::string text = "\"a,b\",\"a\"\"b\",a'b,\"line1\nline2\",\"\"\n";
    csv2::Reader<csv2::delimiter<','>, csv2::quote_character<'"'>, csv2::first_row_is_header<false>> reader;
    if (!reader.parse(text)) {
        std::fprintf(stderr, "parse failed\n");
        return 1;
    }

    std::vector<std::string> values;
    for (const auto &row : reader) {
        for (const auto &cell : row) {
            std::string value;
            cell.read_value(value);
            values.push_back(value);
        }
    }

    if (values.size() != 5) {
        std::fprintf(stderr, "expected 5 cells, got %zu\n", values.size());
        return 1;
    }
    if (values[0] != "a,b") {
        std::fprintf(stderr, "cell 0 = [%s]\n", values[0].c_str());
        return 1;
    }
    if (values[1] != "a\"b") {
        std::fprintf(stderr, "cell 1 = [%s]\n", values[1].c_str());
        return 1;
    }
    if (values[2] != "a'b") {
        std::fprintf(stderr, "cell 2 = [%s]\n", values[2].c_str());
        return 1;
    }
    if (values[3] != "line1\nline2") {
        std::fprintf(stderr, "cell 3 = [%s]\n", values[3].c_str());
        return 1;
    }
    if (!values[4].empty()) {
        std::fprintf(stderr, "cell 4 = [%s]\n", values[4].c_str());
        return 1;
    }

    std::printf("csv2 quote test passed\n");
    return 0;
}
