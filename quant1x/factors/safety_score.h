#pragma once
#ifndef QUANT1X_FACTOR_F10_SAFETY_SCORE_H
#define QUANT1X_FACTOR_F10_SAFETY_SCORE_H 1

#include <quant1x/base/api.h>

namespace risks {

    enum class RiskCategoryType {
        Financial,       // 财务类风险
        Market,          // 市场类风险
        Trading,         // 交易类风险
        STAndDelisting,  // ST风险和退市
        Unknown          // 未知类型
    };

    // 将字符串转换为 RiskCategoryType 枚举
    inline RiskCategoryType toRiskCategoryType(const std::string &categoryName) {
        if (categoryName == "财务类风险")
            return RiskCategoryType::Financial;
        else if (categoryName == "市场类风险")
            return RiskCategoryType::Market;
        else if (categoryName == "交易类风险")
            return RiskCategoryType::Trading;
        else if (categoryName == "ST风险和退市")
            return RiskCategoryType::STAndDelisting;
        else
            return RiskCategoryType::Unknown;
    }

    // 将 RiskCategoryType 枚举转换为对应的中文字符串
    inline const std::string &riskCategoryToString(RiskCategoryType type) {
        static const std::string mapping[] = {"财务类风险", "市场类风险", "交易类风险", "ST风险和退市", "未知类型"};

        return mapping[static_cast<int>(type)];
    }

    // 获取个股安全分
    std::tuple<int, std::string> GetSafetyScore(const std::string &securityCode);
}  // namespace risks

#endif  // QUANT1X_FACTOR_F10_SAFETY_SCORE_H
