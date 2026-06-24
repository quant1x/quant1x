#pragma once
#ifndef QUANT1X_DATA_SCHEMA_COMPANY_H
#define QUANT1X_DATA_SCHEMA_COMPANY_H 1

#include <string>

namespace quant1x::data::schema {

/// 公司信息文件片段
struct CompanyInfoChunk {
    std::string title;      ///< 标题
    std::string filename;   ///< 文件名
    int         offset = 0; ///< 偏移量
    int         size = 0;   ///< 大小
};

} // namespace quant1x::data::schema

#endif // QUANT1X_DATA_SCHEMA_COMPANY_H
