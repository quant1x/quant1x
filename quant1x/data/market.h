#pragma once
#ifndef QUANT1X_DATA_MARKET_H
#define QUANT1X_DATA_MARKET_H 1

/// market — 市场/证券代码识别与纠正，与 Python data/market.py 对齐

#include "meta/exchange.h"
#include "meta/instrument.h"
#include "meta/ticker_rules/rule.h"
#include <string>
#include <vector>
#include <unordered_set>
#include <optional>

namespace data {

/// 根据交易所和代码，使用对应规则检测证券类型
/// 对应 Python 的 detect_instrument_type_by_rule
meta::InstrumentType detect_instrument_type_by_rule(meta::Exchange exchange, const std::string& code);

/// 检测并解析证券代码的市场类型及证券类型
/// 对应 Python 的 detect_symbol
///
/// 支持多种格式:
///   - 前缀形式: sh600000
///   - 后缀形式: 600000.sh 或 AAPL.us
///   - 纯数字形式: 600000 (自动推断交易所)
///   - 4字母全大写: AAPL (自动识别为美股)
///   - 5位数字: 00700 (自动识别为港股)
meta::Instrument detect_symbol(const std::string& input_str);

/// 根据证券代码验证是否为股票
/// 对应 Python 的 assert_stock_by_security_code
inline bool assert_stock_by_security_code(const std::string& security_code) {
    auto inst = detect_symbol(security_code);
    return instype_is_stock(inst.type);
}

/// 根据证券代码验证是否为指数
/// 对应 Python 的 assert_index_by_security_code
inline bool assert_index_by_security_code(const std::string& security_code) {
    auto inst = detect_symbol(security_code);
    return instype_is_index(inst.type);
}

/// 纠正证券代码格式，补全前缀或后缀
/// 对应 Python 的 correct_security_code
///
/// 支持多种格式:
///   - 前缀形式: sh600000
///   - 后缀形式: 600000.sh 或 AAPL.us
///   - 纯数字形式: 600000 (自动推断交易所)
///   - 4字母全大写: AAPL (自动识别为美股)
///   - 6位数字: 600000 (自动推断交易所)
std::string correct_security_code(const std::string& code);

/// 板块信息 (对齐 Python contrib/data/tdx/sector.py)
struct SectorInfo {
    std::vector<std::string> ConstituentStocks;
};

/// 获取板块成份股 (TODO: 从 Python 移植)
inline std::optional<SectorInfo> get_sector_info(const std::string& sectorCode) {
    (void)sectorCode;
    return std::nullopt;
}

/// 获取两融标的列表 (TODO: 从 Python 移植)
inline std::vector<std::string> margin_trading_list() {
    return {};
}

/// 判断是否为两融标的 (TODO: 从 Python 移植)
inline bool is_margin_trading_target(const std::string& code) {
    (void)code;
    return false;
}

/// 下载板块原始数据 (TODO: 从 Python 移植)
inline void download_block_raw_data(const std::string& filename) {
    (void)filename;
}

} // namespace data

#endif // QUANT1X_DATA_MARKET_H
