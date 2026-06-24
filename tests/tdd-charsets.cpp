#include <quant1x/test/test.h>

#include <iconv.h>
#include <quant1x/encoding/charsets.h>
#include <quant1x/std/util.h>

TEST_CASE("raw-gbk-to-utf8", "[charsets]") {
    iconv_t cd = iconv_open("UTF-8", "GBK");
    if (cd == (iconv_t)-1) {
        perror("iconv_open");
    }

    char inbuf[] = "\xC4\xE3\xBA\xC3"; // GBK 编码的数据
    char   outbuf[100]{};
    char *inptr = inbuf, *outptr = outbuf;
    size_t inbytes = sizeof(inbuf);
    size_t outbytes = sizeof(outbuf);

    if (iconv(cd, &inptr, &inbytes, &outptr, &outbytes) == (size_t)(-1)) {
        perror("iconv");
    } else {
        std::cout << "Converted: " << outbuf << std::endl;
    }

    iconv_close(cd);
}

TEST_CASE("raw-utf8-to-gbk", "[charsets]") {
    iconv_t cd = iconv_open("GBK", "UTF-8");
    if (cd == (iconv_t)-1) {
        perror("iconv_open");
    }

    char inbuf[] = "煤炭开采"; // GBK 编码的数据
    char outbuf[100];
    char *inptr = inbuf, *outptr = outbuf;
    size_t inbytes = sizeof(inbuf);
    size_t outbytes = sizeof(outbuf);

    if (iconv(cd, &inptr, &inbytes, &outptr, &outbytes) == (size_t)(-1)) {
        perror("iconv");
    } else {
        std::cout << "Converted: " << outbuf << std::endl;
    }

    iconv_close(cd);
}

// 将 string 的每个字节转为十六进制字符串
std::string to_hex_string(const std::string& input) {
    std::stringstream ss;
    for (unsigned char c : input) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(c)) << " ";
    }
    return ss.str();
}

TEST_CASE("utf8-to-gbk", "[charsets]") {
    std::string in = "煤炭开采";
    std::string out = charsets::utf8_to_gbk(in);
    std::cout << to_hex_string(out) << std::endl;
}

#include <iconv.h>
#include <string>
#include <vector>

std::string gbk_to_utf8_iconv(const std::string& in) {
    // 使用 GB18030 替代 GBK, 兼容性更好
    iconv_t cd = iconv_open("UTF-8", "GB18030");
    if (cd == (iconv_t)-1) {
        throw std::runtime_error("iconv_open failed: unsupported encoding");
    }

    // 创建可写副本(iconv 会修改指针)
    size_t in_bytes = in.size();
    char* in_buf = new char[in_bytes];
    memcpy(in_buf, in.data(), in_bytes);
    char* in_ptr = in_buf;

    // 输出缓冲区(UTF-8 最多是输入长度的 3~4 倍)
    size_t out_bytes = in.size() * 3;
    std::vector<char> out_buf(out_bytes + 1);
    char* out_ptr = out_buf.data();

    // 执行转换
    size_t result = iconv(cd, &in_ptr, &in_bytes, &out_ptr, &out_bytes);

    // 释放资源
    delete[] in_buf;
    iconv_close(cd);

    // 检查错误
    if (result == (size_t)(-1)) {
        std::string msg = "iconv failed: ";
        switch(errno) {
            case EILSEQ: msg += "Invalid byte sequence."; break;
            case EINVAL: msg += "Incomplete character sequence."; break;
            case ENOMEM: msg += "Out of memory."; break;
            default:     msg += "Unknown error."; break;
        }
        throw std::runtime_error(msg);
    }

    // 确保 null 结尾
    *out_ptr = '\0';

    return std::string(out_buf.data());
}

TEST_CASE("gbk-to-utf8", "[charsets]") {
    unsigned char gbk_data[] = {
        0xC3, 0xBA,   // 煤
        0xCC, 0xBF,   // 炭
        0xBF, 0xAA,   // 开
        0xB2, 0xC9,   // 采
        0x00          // null terminator
    };
    std::string in(reinterpret_cast<const char*>(gbk_data), sizeof(gbk_data) - 1);

    std::cout << "Input hex: ";
    for (unsigned char c : in) {
        printf("%02X ", c);
    }
    std::cout << std::endl;

    try {
        std::string out = gbk_to_utf8_iconv(in);
        std::cout << "Output: " << out << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }
}

TEST_CASE("gbk-to-utf8-release", "[charsets]") {
    unsigned char gbk_data[] = {
        0xC3, 0xBA,   // 煤
        0xCC, 0xBF,   // 炭
        0xBF, 0xAA,   // 开
        0xB2, 0xC9,   // 采
        0x00          // null terminator
    };
    std::string in(reinterpret_cast<const char*>(gbk_data), sizeof(gbk_data) - 1);

    std::cout << "Input hex: ";
    for (unsigned char c : in) {
        printf("%02X ", c);
    }
    std::cout << std::endl;

    try {
        std::string out = charsets::gbk_to_utf8(in);
        std::cout << "Output: " << out << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }
}

