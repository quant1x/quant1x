#include <iconv.h>
#include <quant1x/encoding/charsets.h>
#include <quant1x/base/strings.h>
#include <quant1x/base/util.h>
#include <quant1x/test/test.h>

using namespace charsets;

TEST_CASE("trim-basic", "[strings]") {
    std::string text   = "  hello,  world, c++20,  string_view  ";
    auto        tokens = strings::split(text, ',', true);

    for (const auto &token : tokens) {
        std::cout << "|" << token << "|\n";
    }
}

TEST_CASE("split-string", "[strings]") {
    std::string text  = "hello::world::c++::optimization";
    std::string delim = "::";

    auto tokens = strings::split(text, delim, true);

    for (const auto &token : tokens) {
        std::cout << token << "\n";
    }
}

TEST_CASE("join-basic", "[strings]") {
    std::vector<std::string> tokens = {"hello", "world", "c++", "optimization"};

    auto s1 = strings::join(tokens, ", ");  // 字符串分隔符
    auto s2 = strings::join(tokens, '-');   // 字符分隔符

    std::cout << s1 << "\n";  // 输出: hello, world, c++, optimization
    std::cout << s2 << "\n";  // 输出: hello-world-c++-optimization
}

void print_hex(const std::string &s) {
    for (unsigned char c : s) {
        printf("%02X ", c);
    }
    printf("\n");
}

TEST_CASE("split-v1", "[strings]") {
    std::string origin = "c3baccbf7c3838303330317c327c317c307c54303130310d";
    auto        hexStr = strings::hexToBytes(origin);
    std::string in     = {reinterpret_cast<const char *>(hexStr.data()), hexStr.size()};
    std::string inUtf8 = charsets::gbk_to_utf8(in);
    std::cout << inUtf8 << std::endl;
    print_hex(inUtf8);
    auto arr = strings::split(inUtf8, '|');
    std::cout << arr[0] << std::endl;
}

TEST_CASE("split-v2", "[strings]") {
    std::string inUtf8 = "煤炭|880301|2|1|0|T0101";
    std::cout << inUtf8 << std::endl;
    print_hex(inUtf8);
    auto arr = strings::split(inUtf8, '|');
    std::cout << arr[0] << std::endl;
}

std::vector<std::string> split(const std::string &str, char delimiter) {
    std::vector<std::string> tokens;
    size_t                   start = 0;
    size_t                   end   = 0;

    while ((end = str.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + 1;
    }

    // 添加最后一个 token
    tokens.push_back(str.substr(start));

    return tokens;
}

TEST_CASE("split-v3", "[strings]") {
    char raw_data[] = {(char)0xE7, (char)0x85, (char)0xA4,  // 煤
                       (char)0xE7, (char)0x82, (char)0xAD,  // 炭
                       '|',        '8',        '8',        '0', '3', '0', '1', '|', '2', '|',
                       '1',        '|',        '0',        '|', 'T', '0', '1', '0', '1'};

    std::string inUtf8(raw_data, sizeof(raw_data));

    // 打印原始 hex
    print_hex(inUtf8);  // 输出应为 E7 85 A4 E7 82 AD ...

    // 分割
    auto arr = split(inUtf8, '|');

    // 打印第一个 token 的 hex
    const auto &token = arr[0];
    for (unsigned char c : token) {
        printf("%02X ", c);
    }
    printf("\n");

    // 打印第一个 token 的 size
    std::cout << "Token size: " << token.size() << std::endl;

    // 尝试输出中文 ← 不要依赖这个判断是否正常
    std::cout << token << std::endl;
}

TEST_CASE("to_lower function works correctly", "[to_lower]") {

    SECTION("Empty string") {
        REQUIRE(strings::to_lower("") == "");
    }

    SECTION("All uppercase") {
        REQUIRE(strings::to_lower("HELLO") == "hello");
        REQUIRE(strings::to_lower("WORLD") == "world");
        REQUIRE(strings::to_lower("C++") == "c++");
    }

    SECTION("All lowercase") {
        REQUIRE(strings::to_lower("hello") == "hello");
        REQUIRE(strings::to_lower("world") == "world");
        REQUIRE(strings::to_lower("c++") == "c++");
    }

    SECTION("Mixed case") {
        REQUIRE(strings::to_lower("HeLLo") == "hello");
        REQUIRE(strings::to_lower("wOrLD") == "world");
        REQUIRE(strings::to_lower("Cpp17") == "cpp17");
    }

    SECTION("Non-alphabetic characters") {
        REQUIRE(strings::to_lower("12345") == "12345");
        REQUIRE(strings::to_lower("!@#$%^") == "!@#$%^");
        REQUIRE(strings::to_lower("a1B2c3") == "a1b2c3");
    }

    SECTION("Whitespace and special chars") {
        REQUIRE(strings::to_lower(" Hello World ") == " hello world ");
        REQUIRE(strings::to_lower("\tHello\nWorld\r") == "\thello\nworld\r");
    }

//    SECTION("Unicode UTF-8 safe (ASCII only in this function)") {
//        // 注意: 此 to_lower 只支持 ASCII 字符
//        REQUIRE(strings::to_lower("ÄÖÜ") == "äöü");      // 如果你的系统 locale 是 UTF-8 可能不生效
//        REQUIRE(strings::to_lower("Élève") == "élève");  // 同上, 只转换 ASCII 大写字母
//    }
}