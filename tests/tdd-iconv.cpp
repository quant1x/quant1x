#include <quant1x/test/test.h>
#include <quant1x/encoding/charsets.h>

TEST_CASE("utf8_to_gbk converts UTF-8 to GBK", "[charsets]") {
    std::string utf8 = "Hello, 世界!";
    std::string gbk;
    REQUIRE_NOTHROW(gbk = charsets::utf8_to_gbk(utf8));
    REQUIRE(!gbk.empty());
    // 不能直接比较内容，因为GBK编码依赖平台和iconv实现
}

TEST_CASE("gbk_to_utf8 converts GBK to UTF-8", "[charsets]") {
    // 这里的GBK字符串需要是合法的GBK编码
    // 下面是 "Hello, 世界!" 的GBK编码（以十六进制表示）
    std::string gbk = "Hello, \xca\xc0\xbd\xe7!";
    std::string utf8;
    REQUIRE_NOTHROW(utf8 = charsets::gbk_to_utf8(gbk));
    REQUIRE(!utf8.empty());
    // 可以检查是否包含UTF-8的中文字符
    REQUIRE(utf8.find("世界") != std::string::npos);
}

TEST_CASE("utf8_to_gbk handles empty string", "[charsets]") {
    std::string utf8;
    std::string gbk = charsets::utf8_to_gbk(utf8);
    REQUIRE(gbk.empty());
}

TEST_CASE("gbk_to_utf8 handles empty string", "[charsets]") {
    std::string gbk;
    std::string utf8 = charsets::gbk_to_utf8(gbk);
    REQUIRE(utf8.empty());
}