#pragma once
#ifndef QUANT1X_EXCHANGE_CODE_H
#define QUANT1X_EXCHANGE_CODE_H 1

//============================================================
// exchange 证券代码相关                                      //
//============================================================
#include <quant1x/std/api.h>
#include <vector>
#include <cstdint>

namespace exchange {

enum class ExchangeId : std::uint8_t {
    Unknown = 255, // 未知交易所
    ShenZhen = 0, // 深圳交易所
    ShangHai = 1, // 上海交易所
    BeiJing = 2, // 北京交易所
    HongKong = 21, // 香港交易所
    USA = 22, // 美国交易所
};

// SecurityType mirrors the Go `SecurityType` enum in rule.go
enum class SecurityType : std::uint8_t {
    Unknown = 0,    // 未知类型
    Stock = 1,      // 股票
    ETF = 2,        // ETF
    Fund = 3,       // 基金
    Bond = 4,       // 债券
    BStock = 5,     // B股
    IPO = 6,        // 新股申购
    Index = 7,      // 指数
    Block = 8,      // 板块
    Option = 9,     // 期权
    Future = 10,    // 期货
    Warrant = 11,   // 权证
    Forex = 12,     // 外汇
    Commodity = 13, // 商品
    Other = 255,    // 其他类型（与 Go 常量对齐）
};

class ExchangeCode {
public:
    constexpr ExchangeCode(std::string_view sv) : value(sv) {}
    std::string String() const { return std::string(value); }
    ExchangeId Id() const; // throws on unknown
    bool operator==(const ExchangeCode& other) const { return value == other.value; }
    bool operator==(std::string_view sv) const { return value == sv; }

private:
    std::string_view value;
};

static inline constexpr ExchangeCode ExchangeUnknown{"unknown"};
static inline constexpr ExchangeCode ExchangeSSE{"sh"};
static inline constexpr ExchangeCode ExchangeSZSE{"sz"};
static inline constexpr ExchangeCode ExchangeBJSE{"bj"};
static inline constexpr ExchangeCode ExchangeHK{"hk"};
static inline constexpr ExchangeCode ExchangeUS{"us"};

// AllExchangeCodes as strings (align with Go's []string)
static inline const std::vector<std::string> AllExchangeCodes = {
    ExchangeSSE.String(),
    ExchangeSZSE.String(),
    ExchangeBJSE.String(),
    ExchangeHK.String(),
    ExchangeUS.String(),
};

std::string String(ExchangeId m);

struct ExchangeInfo {
    ExchangeId id;
    std::string code;
    std::string name;
    std::string description;
    bool is_active = true;

    std::string ToString() const;
    void Validate() const;
    static ExchangeInfo NewExchange(const std::string& code,
                                    const std::string& name,
                                    const std::string& desc,
                                    ExchangeId id);
};

struct SecurityCode {
    ExchangeId market;
    std::string symbol;
    SecurityType type = SecurityType::Unknown;

    std::string ToString() const;
    void Validate() const;
};

} // namespace exchange

namespace exchange {
    constexpr const char *const stock_delisting          = "DELISTING";  ///< 退市标识
    constexpr const char *const market_cn_first_date     = "19901219";   ///< 上证指数首个交易日
    constexpr const char *const market_cn_first_listtime = "1990-12-19"; ///< 个股上市日期基准

    // Market flag string constants removed — use `exchange::EXCHANGE_*` in exchange/exchange.h

    const std::vector<std::string> marketFlags = {"sh", "sz", "SH", "SZ", "bj", "BJ", "hk", "HK", "us", "US"};
    const std::vector<std::string> marketAShareFlags = {"sh", "sz", "SH", "SZ", "bj", "BJ"};

    /**
     * @brief 根据市场类型和代码生成完整证券代码
     * @param market 市场类型
     * @param symbol 原始代码
     * @return 完整证券代码（格式：市场标识+代码）
     */
    std::string GetSecurityCode(ExchangeId market, const std::string &symbol);

    /**
     * @brief 根据代码判断所属市场
     * @param symbol 证券代码
     * @return 市场标识（sh/sz/bj等）
     */
    std::string GetMarket(const std::string &symbol);

    /**
     * @brief 获取市场ID
     * @param symbol 证券代码
     * @return 市场类型枚举值
     */
    ExchangeId GetMarketId(const std::string &symbol);

    /**
     * @brief 根据市场ID获取市场标识
     * @param marketId 市场类型枚举
     * @return 市场标识字符串
     */
    std::string GetMarketFlag(ExchangeId marketId);

    /**
     * @brief 综合解析证券代码
     * @param symbol 原始证券代码
     * @return 元组（市场ID，市场标识，纯代码）
     */
    std::tuple<ExchangeId, std::string, std::string> DetectMarket(const std::string &symbol);

    /**
     * @brief 判断是否为指数代码（通过市场ID和纯代码）
     * @param marketId 市场ID
     * @param symbol 纯代码
     * @return 是否为指数
     */
    bool AssertIndexByMarketAndCode(ExchangeId marketId, const std::string &symbol);

    /**
     * @brief 判断是否为指数代码（通过完整证券代码）
     * @param securityCode 完整证券代码
     * @return 是否为指数
     */
    bool AssertIndexBySecurityCode(const std::string &securityCode);

    /**
     * @brief 判断并修正板块代码
     * @param securityCode 完整证券代码（会被修改）
     * @return 是否为板块代码
     */
    bool AssertBlockBySecurityCode(std::string *securityCode);

    /**
     * @brief 判断是否为ETF（通过市场ID和纯代码）
     * @param marketId 市场ID
     * @param symbol 纯代码
     * @return 是否为ETF
     */
    bool AssertETFByMarketAndCode(ExchangeId marketId, const std::string &symbol);

    /**
     * @brief 判断是否为个股（通过市场ID和纯代码）
     * @param marketId 市场ID
     * @param symbol 纯代码
     * @return 是否为个股
     */
    bool AssertStockByMarketAndCode(ExchangeId marketId, const std::string &symbol);

    /**
     * @brief 判断是否为个股（通过完整证券代码）
     * @param securityCode 完整证券代码
     * @return 是否为个股
     */
    bool AssertStockBySecurityCode(const std::string &securityCode);

    /**
     * @brief 修正证券代码格式
     * @param symbol 原始代码
     * @return 标准化后的证券代码
     */
    std::string CorrectSecurityCode(const std::string &symbol);

    enum class TargetKind {
        STOCK, ///< 普通股票
        INDEX, ///< 指数
        BLOCK, ///< 板块
        ETF    ///< ETF基金
    };

    /**
     * @brief 判断证券代码类型
     * @param securityCode 完整证券代码
     * @return 证券类型枚举
     */
    TargetKind AssertCode(const std::string &securityCode);

    // 检查指数和个股
    bool checkIndexAndStock(const std::string &securityCode);
}

#endif //QUANT1X_EXCHANGE_CODE_H
