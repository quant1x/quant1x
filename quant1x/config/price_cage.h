#pragma once
#ifndef QUANT1X_CONFIG_DETAIL_PRICE_CAGE_H
#define QUANT1X_CONFIG_DETAIL_PRICE_CAGE_H 1

#include <quant1x/std/api.h>

namespace config {

    // 价格笼子
    //
    //	价格笼子是买卖股票申报价格限制的一种制度
    //	对于主板, 本次新增2%有效申报价格范围要求, 同时增加10个申报价格最小变动单位的安排
    //	A股最小交易变动单位是0.01元, 10个也就是0.1元
    //	买入价取两者高值, 卖出价取两者低值.
    constexpr const double ValidDeclarationPriceRange  = 0.02;  // 价格限制比例, ±2%
    constexpr const double MinimumPriceFluctuationUnit = 0.10;  // 价格浮动最大值, ±0.10

    // 卖出滑点比例, 默认0.01
    constexpr const double FixedSlippageForSell = 0.01;

    struct price_cage {
        double ratio;                // 价格笼子比例
        double minimum_fluctuation_unit;  // 最小价格变动单位

        // 默认构造函数
        explicit price_cage(double _ratio = ValidDeclarationPriceRange, double _unit = MinimumPriceFluctuationUnit)
            : ratio(_ratio), minimum_fluctuation_unit(_unit) {
            validate();
        }

     private:
        void validate() const {
            if (ratio < 0.0) {
                throw std::invalid_argument("Ratio must be non-negative.");
            }
            if (minimum_fluctuation_unit < 0.0) {
                throw std::invalid_argument("Minimum fluctuation unit must be positive.");
            }
        }
    };
}  // namespace config

#endif  // QUANT1X_CONFIG_DETAIL_PRICE_CAGE_H