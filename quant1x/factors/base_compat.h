#pragma once
#ifndef QUANT1X_FACTOR_BASE_COMPAT_H
#define QUANT1X_FACTOR_BASE_COMPAT_H 1

// Minimal constants from factors/base.h — used during API migration
// TODO: remove after full migration

#include <quant1x/data/adapter.h>

namespace factors {
    constexpr int  KLineMin    = 120;
    constexpr auto baseFeature = data::PluginMaskFeature;

    constexpr auto FeatureF10                       = baseFeature + 1;
    constexpr auto FeatureHistory                   = baseFeature + 2;
    constexpr auto FeatureNo0                       = baseFeature + 3;
    constexpr auto FeatureMisc                      = baseFeature + 4;
    constexpr auto FeatureBreaksThroughBox          = baseFeature + 5;
    constexpr auto FeatureKLineShap                 = baseFeature + 6;
    constexpr auto FeatureInvestmentSentimentMaster = baseFeature + 7;
    constexpr auto FeatureSecuritiesMarginTrading   = baseFeature + 8;
}

#endif // QUANT1X_FACTOR_BASE_COMPAT_H
