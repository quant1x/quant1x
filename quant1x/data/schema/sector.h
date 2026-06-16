#pragma once
#ifndef QUANT1X_DATA_SCHEMA_SECTOR_H
#define QUANT1X_DATA_SCHEMA_SECTOR_H 1

#include <string>
#include <vector>

namespace meta::schema {

/// 板块信息结构体
struct Sector {
    std::string              name;                ///< 板块名称
    std::string              code;                ///< 板块代码
    int                      type = 0;            ///< 板块类型
    int                      count = 0;           ///< 成分股数量
    std::string              block;               ///< 板块分组
    std::vector<std::string> constituent_stocks;  ///< 成分股列表
};

} // namespace meta::schema

#endif // QUANT1X_DATA_SCHEMA_SECTOR_H
