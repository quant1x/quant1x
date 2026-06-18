#pragma once
#ifndef QUANT1X_DATA_META_EXCHANGE_H
#define QUANT1X_DATA_META_EXCHANGE_H 1

#include "region.h"
#include <cstdint>
#include <string>
#include <stdexcept>

namespace quant1x::data::meta {

/// 交易所枚举, 与 Python/Go/Rust 的 Exchange 对齐
/// 每个变体持有 (mic, identifier, region, label) 四元组
enum class Exchange : uint8_t {
    // 中国市场
    SSE      = 0,   ///< 上海证券交易所
    XSSC     = 1,   ///< 上海证券交易所 - 沪股通
    SZSE     = 2,   ///< 深圳证券交易所
    XSEC     = 3,   ///< 深证证券交易所 - 深股通
    BSE      = 4,   ///< 北京证券交易所

    // 期货交易所
    SHFE     = 5,   ///< 上海期货交易所
    XINE     = 6,   ///< 上海国际能源交易中心
    CZCE     = 7,   ///< 郑州商品交易所
    DCE      = 8,   ///< 大连商品交易所
    CFFEX    = 9,   ///< 中国金融期货交易所
    GFEX     = 10,  ///< 广州期货交易所
    SGE      = 11,  ///< 上海黄金交易所

    // 香港
    HKEX     = 12,  ///< 香港交易所(现货股票)
    HKSC     = 13,  ///< 香港交易所-港股通
    HKFE     = 14,  ///< 香港期货交易所

    // 指数
    CSI      = 15,  ///< 中证指数
    CNI      = 16,  ///< 国证指数

    // 扩展
    EXTENDED = 17,  ///< 扩展市场

    // 离岸/在岸
    OFFSHORE = 18,  ///< 国际, 其它离岸市场
    ONSHORE  = 19,  ///< 国内, 其它在岸市场
    OTC      = 20,  ///< 国内, 场外
    OFFEX    = 21,  ///< 场外申赎市场

    // 宏观
    MACRO    = 22,  ///< 宏观经济市场

    // 美国
    USA      = 23,  ///< 美国证券市场(泛指)
    NYSE     = 24,  ///< 纽约证券交易所
    NASDAQ   = 25,  ///< 纳斯达克

    // 英国
    LSE      = 26,  ///< 伦敦证券交易所
    GBR      = 27,  ///< 英国证券市场(泛指)

    // 新加坡
    SGX      = 28,  ///< 新加坡交易所

    // 其它
    MIRROR   = 29,  ///< 镜像市场
    TEMP     = 30,  ///< 临时市场
    UNKNOWN  = 255, ///< 未知交易所
};

/// MIC: Market Identifier Code
inline const char* exchange_mic(Exchange ex) {
    switch (ex) {
        case Exchange::SSE:      return "XSHG";
        case Exchange::XSSC:     return "XSSC";
        case Exchange::SZSE:     return "XSHE";
        case Exchange::XSEC:     return "XSEC";
        case Exchange::BSE:      return "BJSE";
        case Exchange::SHFE:     return "XSGE";
        case Exchange::XINE:     return "XINE";
        case Exchange::CZCE:     return "XZCE";
        case Exchange::DCE:      return "XDCE";
        case Exchange::CFFEX:    return "CCFX";
        case Exchange::GFEX:     return "GFEX";
        case Exchange::SGE:      return "SGEX";
        case Exchange::HKEX:     return "XHKG";
        case Exchange::HKSC:     return "XHKG";
        case Exchange::HKFE:     return "XHKF";
        case Exchange::CSI:      return "CSI";
        case Exchange::CNI:      return "CNI";
        case Exchange::EXTENDED: return "EXTENDED";
        case Exchange::OFFSHORE: return "OFFSHORE";
        case Exchange::ONSHORE:  return "ONSHORE";
        case Exchange::OTC:      return "OTC";
        case Exchange::OFFEX:    return "OFFEX";
        case Exchange::MACRO:    return "MACRO";
        case Exchange::USA:      return "USA";
        case Exchange::NYSE:     return "XNYS";
        case Exchange::NASDAQ:   return "XNAS";
        case Exchange::LSE:      return "XLON";
        case Exchange::GBR:      return "GBR";
        case Exchange::SGX:      return "XSES";
        case Exchange::MIRROR:   return "MIRROR";
        case Exchange::TEMP:     return "TEMP";
        case Exchange::UNKNOWN:  return "UNKNOWN";
        default:                 return "UNKNOWN";
    }
}

/// 标识: 交易所的小写缩写, 如 sh/sz/bj/hk
inline const char* exchange_identifier(Exchange ex) {
    switch (ex) {
        case Exchange::SSE:      return "sh";
        case Exchange::XSSC:     return "sh";
        case Exchange::SZSE:     return "sz";
        case Exchange::XSEC:     return "sz";
        case Exchange::BSE:      return "bj";
        case Exchange::SHFE:     return "shfe";
        case Exchange::XINE:     return "ine";
        case Exchange::CZCE:     return "zce";
        case Exchange::DCE:      return "dce";
        case Exchange::CFFEX:    return "cff";
        case Exchange::GFEX:     return "gfex";
        case Exchange::SGE:      return "sge";
        case Exchange::HKEX:     return "hk";
        case Exchange::HKSC:     return "hksc";
        case Exchange::HKFE:     return "hkf";
        case Exchange::CSI:      return "csi";
        case Exchange::CNI:      return "cni";
        case Exchange::EXTENDED: return "ext";
        case Exchange::OFFSHORE: return "os";
        case Exchange::ONSHORE:  return "on";
        case Exchange::OTC:      return "otc";
        case Exchange::OFFEX:    return "offex";
        case Exchange::MACRO:    return "macro";
        case Exchange::USA:      return "us";
        case Exchange::NYSE:     return "us";
        case Exchange::NASDAQ:   return "us";
        case Exchange::LSE:      return "uk";
        case Exchange::GBR:      return "uk";
        case Exchange::SGX:      return "sg";
        case Exchange::MIRROR:   return "mirror";
        case Exchange::TEMP:     return "temp";
        case Exchange::UNKNOWN:  return "unknown";
        default:                 return "unknown";
    }
}

/// 市场区域
inline Region exchange_region(Exchange ex) {
    switch (ex) {
        case Exchange::SSE: case Exchange::XSSC: case Exchange::SZSE:
        case Exchange::XSEC: case Exchange::BSE: case Exchange::SHFE:
        case Exchange::XINE: case Exchange::CZCE: case Exchange::DCE:
        case Exchange::CFFEX: case Exchange::GFEX: case Exchange::SGE:
        case Exchange::CSI: case Exchange::CNI:
            return Region::CN;
        case Exchange::HKEX: case Exchange::HKSC: case Exchange::HKFE:
            return Region::HK;
        case Exchange::EXTENDED: case Exchange::MACRO:
        case Exchange::MIRROR: case Exchange::TEMP:
            return Region::GLB;
        case Exchange::OFFSHORE:
            return Region::OFFSHORE;
        case Exchange::ONSHORE: case Exchange::OTC: case Exchange::OFFEX:
            return Region::ONSHORE;
        case Exchange::USA: case Exchange::NYSE: case Exchange::NASDAQ:
            return Region::US;
        case Exchange::LSE: case Exchange::GBR:
            return Region::UK;
        case Exchange::SGX:
            return Region::SG;
        case Exchange::UNKNOWN:
        default:
            return Region::UNKNOWN;
    }
}

/// 交易所名称
inline const char* exchange_label(Exchange ex) {
    switch (ex) {
        case Exchange::SSE:      return "上海证券交易所";
        case Exchange::XSSC:     return "上海证券交易所";
        case Exchange::SZSE:     return "深圳证券交易所";
        case Exchange::XSEC:     return "深圳证券交易所";
        case Exchange::BSE:      return "北京证券交易所";
        case Exchange::SHFE:     return "上海期货交易所";
        case Exchange::XINE:     return "上海国际能源交易中心";
        case Exchange::CZCE:     return "郑州商品交易所";
        case Exchange::DCE:      return "大连商品交易所";
        case Exchange::CFFEX:    return "中国金融期货交易所";
        case Exchange::GFEX:     return "广州期货交易所";
        case Exchange::SGE:      return "上海黄金交易所";
        case Exchange::HKEX:     return "香港交易所(现货股票)";
        case Exchange::HKSC:     return "香港交易所-港股通";
        case Exchange::HKFE:     return "香港期货交易所";
        case Exchange::CSI:      return "中证指数有限公司";
        case Exchange::CNI:      return "国证指数";
        case Exchange::EXTENDED: return "扩展市场";
        case Exchange::OFFSHORE: return "国际, 其它离岸市场";
        case Exchange::ONSHORE:  return "国内, 其它在岸市场";
        case Exchange::OTC:      return "国内, 场外";
        case Exchange::OFFEX:    return "场外申赎市场";
        case Exchange::MACRO:    return "宏观经济市场";
        case Exchange::USA:      return "美国证券市场(泛指)";
        case Exchange::NYSE:     return "纽约证券交易所";
        case Exchange::NASDAQ:   return "纳斯达克";
        case Exchange::LSE:      return "伦敦证券交易所";
        case Exchange::GBR:      return "英国证券市场(泛指)";
        case Exchange::SGX:      return "新加坡交易所";
        case Exchange::MIRROR:   return "镜像市场";
        case Exchange::TEMP:     return "临时市场";
        case Exchange::UNKNOWN:  return "未知交易所";
        default:                 return "未知";
    }
}

/// 枚举名
inline const char* exchange_code(Exchange ex) {
    switch (ex) {
        case Exchange::SSE:      return "SSE";
        case Exchange::XSSC:     return "XSSC";
        case Exchange::SZSE:     return "SZSE";
        case Exchange::XSEC:     return "XSEC";
        case Exchange::BSE:      return "BSE";
        case Exchange::SHFE:     return "SHFE";
        case Exchange::XINE:     return "XINE";
        case Exchange::CZCE:     return "CZCE";
        case Exchange::DCE:      return "DCE";
        case Exchange::CFFEX:    return "CFFEX";
        case Exchange::GFEX:     return "GFEX";
        case Exchange::SGE:      return "SGE";
        case Exchange::HKEX:     return "HKEX";
        case Exchange::HKSC:     return "HKSC";
        case Exchange::HKFE:     return "HKFE";
        case Exchange::CSI:      return "CSI";
        case Exchange::CNI:      return "CNI";
        case Exchange::EXTENDED: return "EXTENDED";
        case Exchange::OFFSHORE: return "OFFSHORE";
        case Exchange::ONSHORE:  return "ONSHORE";
        case Exchange::OTC:      return "OTC";
        case Exchange::OFFEX:    return "OFFEX";
        case Exchange::MACRO:    return "MACRO";
        case Exchange::USA:      return "USA";
        case Exchange::NYSE:     return "NYSE";
        case Exchange::NASDAQ:   return "NASDAQ";
        case Exchange::LSE:      return "LSE";
        case Exchange::GBR:      return "GBR";
        case Exchange::SGX:      return "SGX";
        case Exchange::MIRROR:   return "MIRROR";
        case Exchange::TEMP:     return "TEMP";
        case Exchange::UNKNOWN:  return "UNKNOWN";
        default:                 return "UNKNOWN";
    }
}

/// 智能解析字符串为 Exchange 实例
/// 1. 按枚举名匹配 2. 按identifier匹配 3. 按MIC匹配
inline Exchange parse_exchange(const std::string& s) {
    if (s.empty()) {
        throw std::invalid_argument("Empty string cannot be parsed to Exchange");
    }
    // 转大写
    std::string name;
    for (char c : s) { name += static_cast<char>(::toupper(static_cast<unsigned char>(c))); }

    // 1. By code (enum name)
    for (int i = 0; i <= 30; ++i) {
        Exchange ex = static_cast<Exchange>(i);
        if (name == exchange_code(ex)) return ex;
    }
    // Special: UNKNOWN = 255
    if (name == exchange_code(Exchange::UNKNOWN)) return Exchange::UNKNOWN;

    // 2. By identifier
    std::string ident;
    for (char c : s) { ident += static_cast<char>(::tolower(static_cast<unsigned char>(c))); }
    for (int i = 0; i <= 30; ++i) {
        Exchange ex = static_cast<Exchange>(i);
        if (ident == exchange_identifier(ex)) return ex;
    }

    // 3. By MIC
    for (int i = 0; i <= 30; ++i) {
        Exchange ex = static_cast<Exchange>(i);
        if (name == exchange_mic(ex)) return ex;
    }

    throw std::invalid_argument("Cannot parse exchange from: '" + s + "'");
}

/// 根据代码创建 Exchange
inline Exchange exchange_from_code(const std::string& code) {
    std::string name;
    for (char c : code) { name += static_cast<char>(::toupper(static_cast<unsigned char>(c))); }
    for (int i = 0; i <= 30; ++i) {
        Exchange ex = static_cast<Exchange>(i);
        if (name == exchange_code(ex)) return ex;
    }
    throw std::invalid_argument("Unknown exchange code: " + code);
}

/// 根据缩写创建 Exchange
inline Exchange exchange_from_abbr(const std::string& abbr) {
    std::string ident;
    for (char c : abbr) { ident += static_cast<char>(::tolower(static_cast<unsigned char>(c))); }
    for (int i = 0; i <= 30; ++i) {
        Exchange ex = static_cast<Exchange>(i);
        if (ident == exchange_identifier(ex)) return ex;
    }
    throw std::invalid_argument("Unknown exchange abbreviation: " + abbr);
}

/// 根据 MIC 创建 Exchange
inline Exchange exchange_from_mic(const std::string& mic) {
    std::string name;
    for (char c : mic) { name += static_cast<char>(::toupper(static_cast<unsigned char>(c))); }
    for (int i = 0; i <= 30; ++i) {
        Exchange ex = static_cast<Exchange>(i);
        if (name == exchange_mic(ex)) return ex;
    }
    throw std::invalid_argument("Unknown MIC: " + mic);
}

/// 是否国内交易所
inline bool exchange_is_domestic(Exchange ex) {
    Region r = exchange_region(ex);
    return r == Region::CN || r == Region::HK;
}

/// 是否标准行情接口支持的交易所
inline bool exchange_is_std_quote(Exchange ex) {
    return ex == Exchange::SSE || ex == Exchange::SZSE || ex == Exchange::BSE;
}

/// 是否扩展行情接口支持的交易所
inline bool exchange_is_ext_quote(Exchange ex) {
    return !exchange_is_std_quote(ex) && ex != Exchange::UNKNOWN;
}

} // namespace quant1x::data::meta

#endif // QUANT1X_DATA_META_EXCHANGE_H
