#pragma once
#ifndef QUANT1X_ENCODING_ICONV_H
#define QUANT1X_ENCODING_ICONV_H 1

#include <string>

/// 编码
namespace charsets {
    std::string utf8_to_gbk(const std::string& utf8_str);
    std::string gbk_to_utf8(const std::string& gbk_str);
} // namespace charsets



#endif //QUANT1X_ENCODING_ICONV_H