#pragma once
#ifndef QUANT1X_DATA_META_REGION_H
#define QUANT1X_DATA_META_REGION_H 1

#include <cstdint>
#include <string>

namespace meta {

/// 市场区域, 用于收敛货币和时区
enum class Region : uint8_t {
    CN       = 0,   ///< 中国
    HK       = 1,   ///< 香港
    US       = 2,   ///< 美国
    UK       = 3,   ///< 英国
    EU       = 4,   ///< 欧元区
    SG       = 5,   ///< 新加坡
    JP       = 6,   ///< 日本
    OFFSHORE = 7,   ///< 离岸市场
    ONSHORE  = 8,   ///< 内地市场
    GLB      = 9,   ///< 全球市场
    UNKNOWN  = 255, ///< 未知区域
};

/// 获取区域的主要货币
inline const char* region_currency(Region r) {
    switch (r) {
        case Region::CN:       return "CNY";
        case Region::HK:       return "HKD";
        case Region::US:       return "USD";
        case Region::UK:       return "GBP";
        case Region::EU:       return "EUR";
        case Region::SG:       return "SGD";
        case Region::JP:       return "JPY";
        case Region::OFFSHORE: return "USD";
        case Region::ONSHORE:  return "CNY";
        default:               return "USD";
    }
}

/// 获取区域的主要时区
inline const char* region_timezone(Region r) {
    switch (r) {
        case Region::CN:       return "Asia/Shanghai";
        case Region::HK:       return "Asia/Hong_Kong";
        case Region::US:       return "America/New_York";
        case Region::UK:       return "Europe/London";
        case Region::EU:       return "Europe/Berlin";
        case Region::SG:       return "Asia/Singapore";
        case Region::JP:       return "Asia/Tokyo";
        case Region::OFFSHORE: return "America/New_York";
        case Region::ONSHORE:  return "Asia/Shanghai";
        default:               return "UTC";
    }
}

/// 从字符串解析 Region
inline Region parse_region(const std::string& s) {
    if (s == "CN")       return Region::CN;
    if (s == "HK")       return Region::HK;
    if (s == "US")       return Region::US;
    if (s == "UK")       return Region::UK;
    if (s == "EU")       return Region::EU;
    if (s == "SG")       return Region::SG;
    if (s == "JP")       return Region::JP;
    if (s == "OS")       return Region::OFFSHORE;
    if (s == "ON")       return Region::ONSHORE;
    if (s == "GLB")      return Region::GLB;
    return Region::UNKNOWN;
}

/// Region 转字符串
inline std::string region_to_string(Region r) {
    switch (r) {
        case Region::CN:       return "CN";
        case Region::HK:       return "HK";
        case Region::US:       return "US";
        case Region::UK:       return "UK";
        case Region::EU:       return "EU";
        case Region::SG:       return "SG";
        case Region::JP:       return "JP";
        case Region::OFFSHORE: return "OS";
        case Region::ONSHORE:  return "ON";
        case Region::GLB:      return "GLB";
        default:               return "UNKNOWN";
    }
}

} // namespace meta

#endif // QUANT1X_DATA_META_REGION_H
