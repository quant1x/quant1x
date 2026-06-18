#pragma once
#ifndef QUANT1X_DATA_META_INSTRUMENT_H
#define QUANT1X_DATA_META_INSTRUMENT_H 1

#include "exchange.h"
#include <cstdint>
#include <string>
#include <format>

namespace quant1x::data::meta {

/// 资产子类型(高4位), 语义由主类型(InstrumentType)决定
enum Subtype : uint8_t {
    SubtypeDefault        = 0x00, ///< 默认/无特殊子类(如A股, 普通指数)
    SubtypeChinext        = 0x10, ///< 深交所, 创业板
    SubtypeStar           = 0x20, ///< 上交所, 科创板
    SubtypeB              = 0x30, ///< B股
    SubtypeH              = 0x40, ///< H股
    SubtypeGem            = 0x50, ///< 港交所创业板
    SubtypeExchangeTraded = 0x60, ///< 交易型开放式
    SubtypeListed         = 0x70, ///< 上市型开放式
    SubtypeOpenEnded      = 0x80, ///< 开放式
    SubtypeMutual         = 0xB0, ///< 公募市场
    SubtypePrivate        = 0xC0, ///< 私募市场
    SubtypeMoney          = 0xD0, ///< 货币
    SubtypeSpecial        = 0xE0, ///< 特殊变体
    SubtypeTemporary      = 0xF0, ///< 临时市场
};

/// 合约类型(低4位=资产大类, 高4位=子类型扩展)
enum class InstrumentType : uint16_t {
    Unknown   = 0x00, ///< 未知类型
    Index     = 0x01, ///< 指数
    Stock     = 0x02, ///< 股票(默认A股)
    Fund      = 0x03, ///< 基金
    Bond      = 0x04, ///< 债券
    Forex     = 0x05, ///< 外汇
    Commodity = 0x06, ///< 商品现货
    Future    = 0x07, ///< 期货
    Option    = 0x08, ///< 期权
    Warrant   = 0x09, ///< 权证
    Macro     = 0x0F, ///< 宏观指标

    // === 组合类型 ===
    // 股票子类
    BStock    = SubtypeB | Stock,        ///< B股
    HStock    = SubtypeH | Stock,        ///< H股
    IPO       = SubtypeSpecial | Stock,  ///< IPO

    ChinextMarket  = SubtypeChinext | Stock,   ///< 深交所, 创业板
    StarMarket     = SubtypeStar | Stock,      ///< 上交所, 科创板
    GemMarket      = SubtypeGem | Stock,       ///< 港交所, 创业板
    TemporaryStock = SubtypeTemporary | Stock,  ///< 港交所, 临时柜台

    // 基金子类
    ETF           = SubtypeExchangeTraded | Fund, ///< ETF基金
    LOF           = SubtypeListed | Fund,         ///< LOF基金
    OpenEndedFund = SubtypeOpenEnded | Fund,      ///< 开放式基金
    MoneyFund     = SubtypeMoney | Fund,          ///< 货币基金

    MaxroIndicator = Macro, ///< 宏观指标别名

    // 指数子类
    Sector = SubtypeSpecial | Index, ///< 板块

    NEEQ  = 0xFE, ///< 新三板/股转系统
    Other = 0xFF, ///< 其他未分类
};

/// 提取基础资产类型(低4位)
inline InstrumentType instype_base_type(InstrumentType t) {
    return static_cast<InstrumentType>(static_cast<uint16_t>(t) & 0x0F);
}

/// 提取子类型扩展位(高4位)
inline uint8_t instype_subtype(InstrumentType t) {
    return static_cast<uint8_t>(static_cast<uint16_t>(t) & 0xF0);
}

/// 判断是否为股票类型
inline bool instype_is_stock(InstrumentType t) {
    return instype_base_type(t) == InstrumentType::Stock;
}

/// 判断是否为指数类
inline bool instype_is_index(InstrumentType t) {
    return instype_base_type(t) == InstrumentType::Index;
}

/// 转换为字符串
inline std::string instype_to_string(InstrumentType t) {
    switch (t) {
        case InstrumentType::Unknown:   return "unknown";
        case InstrumentType::Index:     return "index";
        case InstrumentType::Stock:     return "stock";
        case InstrumentType::Fund:      return "fund";
        case InstrumentType::Bond:      return "bond";
        case InstrumentType::Forex:     return "forex";
        case InstrumentType::Commodity: return "commodity";
        case InstrumentType::Future:    return "future";
        case InstrumentType::Option:    return "option";
        case InstrumentType::Warrant:   return "warrant";
        case InstrumentType::Macro:     return "macro";
        case InstrumentType::BStock:    return "bstock";
        case InstrumentType::HStock:    return "hstock";
        case InstrumentType::IPO:       return "ipo";
        case InstrumentType::ChinextMarket:  return "chinext";
        case InstrumentType::StarMarket:     return "star";
        case InstrumentType::GemMarket:      return "gem";
        case InstrumentType::TemporaryStock: return "temp_stock";
        case InstrumentType::ETF:            return "etf";
        case InstrumentType::LOF:            return "lof";
        case InstrumentType::OpenEndedFund:  return "open_ended";
        case InstrumentType::MoneyFund:      return "money_fund";
        case InstrumentType::Sector:         return "sector";
        case InstrumentType::NEEQ:           return "neeq";
        case InstrumentType::Other:          return "other";
        default: return "unknown";
    }
}

/// 从字符串解析
inline InstrumentType instype_from_string(const std::string& s) {
    if (s == "index")      return InstrumentType::Index;
    if (s == "stock")      return InstrumentType::Stock;
    if (s == "fund")       return InstrumentType::Fund;
    if (s == "bond")       return InstrumentType::Bond;
    if (s == "forex")      return InstrumentType::Forex;
    if (s == "commodity")  return InstrumentType::Commodity;
    if (s == "future")     return InstrumentType::Future;
    if (s == "option")     return InstrumentType::Option;
    if (s == "warrant")    return InstrumentType::Warrant;
    if (s == "macro")      return InstrumentType::Macro;
    if (s == "bstock")     return InstrumentType::BStock;
    if (s == "hstock")     return InstrumentType::HStock;
    if (s == "ipo")        return InstrumentType::IPO;
    if (s == "chinext")    return InstrumentType::ChinextMarket;
    if (s == "star")       return InstrumentType::StarMarket;
    if (s == "gem")        return InstrumentType::GemMarket;
    if (s == "etf")        return InstrumentType::ETF;
    if (s == "lof")        return InstrumentType::LOF;
    if (s == "open_ended") return InstrumentType::OpenEndedFund;
    if (s == "money_fund") return InstrumentType::MoneyFund;
    if (s == "sector")     return InstrumentType::Sector;
    if (s == "neeq")       return InstrumentType::NEEQ;
    return InstrumentType::Unknown;
}

/// 证券信息结构体, 与 Python/Go/Rust 的 Instrument 对齐
struct Instrument {
    Exchange       exchange = Exchange::UNKNOWN;  ///< 交易所
    InstrumentType type = InstrumentType::Unknown; ///< 证券类型
    std::string    ticker;                        ///< 交易所原始分配代码
    std::string    name;                          ///< 证券名称
    int            lot_size = 100;                ///< 每手股数
    int            price_precision = 2;           ///< 价格小数位数
    int            ext_market = 0;                ///< 扩展市场代码
    int            ext_category = 0;              ///< 扩展类别代码
    std::string    alias_ticker;                  ///< 市场惯例别名代码(可覆盖 ticker)
    std::string    desc;                          ///< 证券描述

    /// 构建交易符号字符串
    /// CN 市场: {identifier}{ticker}, 如 sh600000
    /// 非 CN 市场: {ticker}.{identifier}, 如 aapl.us
    std::string symbol() const {
        std::string ident = exchange_identifier(exchange);
        if (exchange_region(exchange) == Region::CN) {
            return ident + ticker;
        }
        std::string t;
        for (char c : ticker) { t += static_cast<char>(::tolower(static_cast<unsigned char>(c))); }
        return t + "." + ident;
    }

    /// 检查是否可以构造有效的交易符号
    bool can_construct_symbol() const {
        return exchange != Exchange::UNKNOWN && type != InstrumentType::Unknown;
    }

    /// 检查证券是否有效
    bool is_valid() const {
        return exchange != Exchange::UNKNOWN && type != InstrumentType::Unknown &&
               lot_size > 0 && price_precision > 0;
    }

    /// 获取市场惯例代码(优先返回 alias_ticker, 否则返回交易所原始 ticker)
    /// 语义: alias_ticker 表示市场惯例写法/别名映射, 而非交易所原始代码
    std::string marker_ticker() const {
        return alias_ticker.empty() ? ticker : alias_ticker;
    }

    /// 获取缓存目录路径
    std::string cache_dir() const {
        std::string code = exchange_code(exchange);
        for (auto& c : code) { c = static_cast<char>(::tolower(static_cast<unsigned char>(c))); }
        return code;
    }
};

} // namespace quant1x::data::meta

#endif // QUANT1X_DATA_META_INSTRUMENT_H
