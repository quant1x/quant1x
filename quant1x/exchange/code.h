#pragma once
#ifndef QUANT1X_EXCHANGE_CODE_H
#define QUANT1X_EXCHANGE_CODE_H 1

//============================================================
// exchange 证券代码相关                                      //
//============================================================
#include <quant1x/std/api.h>
#include <vector>

namespace exchange {
    enum MarketType : uint8_t {
        ShenZhen =  0, ///< 深圳证券交易所
        ShangHai =  1, ///< 上海证券交易所
        BeiJing  =  2, ///< 北京证券交易所
        HongKong = 21, ///< 香港交易所
        USA      = 22  ///< 美国交易所
    };

    constexpr const char *const stock_delisting          = "DELISTING";  ///< 退市标识
    constexpr const char *const market_cn_first_date     = "19901219";   ///< 上证指数首个交易日
    constexpr const char *const market_cn_first_listtime = "1990-12-19"; ///< 个股上市日期基准

    constexpr const char *const market_shangHai = "sh"; ///< 上海市场标识
    constexpr const char *const market_shenzhen = "sz"; ///< 深圳市场标识
    constexpr const char *const market_beijing  = "bj"; ///< 北京市场标识
    constexpr const char *const market_hongkong = "hk"; ///< 香港市场标识
    constexpr const char *const market_usa      = "us"; ///< 美国市场标识

    const std::vector<std::string> marketFlags = {"sh", "sz", "SH", "SZ", "bj", "BJ", "hk", "HK", "us", "US"};
    const std::vector<std::string> marketAShareFlags = {"sh", "sz", "SH", "SZ", "bj", "BJ"};

    /**
     * @brief 根据市场类型和代码生成完整证券代码
     * @param market 市场类型
     * @param symbol 原始代码
     * @return 完整证券代码（格式：市场标识+代码）
     */
    std::string GetSecurityCode(MarketType market, const std::string &symbol);

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
    MarketType GetMarketId(const std::string &symbol);

    /**
     * @brief 根据市场ID获取市场标识
     * @param marketId 市场类型枚举
     * @return 市场标识字符串
     */
    std::string GetMarketFlag(MarketType marketId);

    /**
     * @brief 综合解析证券代码
     * @param symbol 原始证券代码
     * @return 元组（市场ID，市场标识，纯代码）
     */
    std::tuple<MarketType, std::string, std::string> DetectMarket(const std::string &symbol);

    /**
     * @brief 判断是否为指数代码（通过市场ID和纯代码）
     * @param marketId 市场ID
     * @param symbol 纯代码
     * @return 是否为指数
     */
    bool AssertIndexByMarketAndCode(MarketType marketId, const std::string &symbol);

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
    bool AssertETFByMarketAndCode(MarketType marketId, const std::string &symbol);

    /**
     * @brief 判断是否为个股（通过市场ID和纯代码）
     * @param marketId 市场ID
     * @param symbol 纯代码
     * @return 是否为个股
     */
    bool AssertStockByMarketAndCode(MarketType marketId, const std::string &symbol);

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
