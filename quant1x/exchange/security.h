#pragma once
#ifndef QUANT1X_EXCHANGE_SECURITY_H
#define QUANT1X_EXCHANGE_SECURITY_H 1

//============================================================
// exchange 证券信息相关                                      //
//============================================================
#include <quant1x/std/api.h>

namespace exchange {

    /// 证券信息
    struct SecurityInfo {
        std::string code; ///< 证券代码
        std::string name; ///< 证券名称
        uint16_t lotSize = 100; ///< 每手股数
        uint8_t pricePrecision = 2; ///< 股价保持小数点后几位
//        double minPriceIncrement;    ///< 最小价格变动单位 (如 0.01)
//        CurrencyType currency;       ///< 货币类型 (枚举类型)
//        // 市场信息
//        MarketType market;           ///< 所属市场 (如 A股、港股)
//        Exchange exchange;           ///< 交易所信息
        // 友元声明，允许访问私有成员（若结构体有私有成员）
        friend std::ostream& operator<<(std::ostream& os, const SecurityInfo& p);
    };

    std::optional<SecurityInfo> get_security_info(const std::string &code);

    /**
     * @brief 获取证券的涨停板比率（例如：0.10 表示 10%）
     * @param security_code 证券代码（如 "600000"）
     * @return 涨停幅度（如主板返回 0.10，创业板返回 0.20）
     */
    double get_up_limit_rate(const std::string& security_code);

    /**
     * @brief 根据昨日收盘价和证券代码，计算涨停价格
     * @param security_code
     * @param prev_close
     * @return
     */
    double calc_limit_up_price(const std::string& security_code, double prev_close);
}

#endif //QUANT1X_EXCHANGE_SECURITY_H
