#include <quant1x/test/test.h>

#include <quant1x/level1/helpers.h>

// 测试快照时间戳
TEST_CASE("encoding-format_time", "[level1]") {
    int test_cases[] = {
        0,        // 00:00:00.000
        123,      // 00:01:23.000（解析逻辑：h=0, m1=1, tmp2=23）
        5959,     // 00:59:59.000
        6000,     // 01:00:00.000（分钟溢出）
        12345678, // 12:34:34.068
        14986367, // 15:00:59.182
        14986967, // 15:00:59.218
        11026532, // 11:02:39.192
        11295421, // 11:29:32.526
        10100682,
        8836243,  // 09:15:00.051
        150006364, // TODO: 解析成 150:00:38.184, 有时间再修复
        -1            // 结束标记
    };

    for (int i = 0; ; ++i) {
        if (test_cases[i] == -1) break;
        printf("Input: %d, Output: %s\n", test_cases[i], level1::helpers::format_time(test_cases[i]).c_str());
    }
}



// 测试整形编解码
TEST_CASE("Test Varint for quant1x", "[math]") {
    int origin = -123455;
    char buffer[10] = {};
    // ================= 编码部分 =================
    int pos = 0;
    int writen = level1::helpers::varint_encode(origin, reinterpret_cast<uint8_t *>(buffer), &pos);
    std::cout << writen << std::endl;
    std::cout << std::dec;
    for (unsigned char c : buffer) {
        std::cout << static_cast<int>(c) << " "; // 以十六进制格式输出
    }
    // ================= 解码部分 =================
    pos = 0;
    std::cout << std::dec;
    auto decoded_value = level1::helpers::varint_decode(reinterpret_cast<const uint8_t *>(buffer), &pos);
    std::cout << "Decoded value: " << decoded_value << std::endl; // 输出解码后的值
}

