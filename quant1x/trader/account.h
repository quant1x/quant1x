#pragma once
#ifndef QUANT1X_TRADER_ACCOUNT_H
#define QUANT1X_TRADER_ACCOUNT_H 1

#include "constants.h"
#include <mutex>
#include <functional>
#include <memory>
#include <quant1x/config/strategy_parameter.h>

namespace trader {

    // 计算理论上可用的资金
    void calculateTheoreticalFund(double* pTheoretical, double* pCash);

    double CalculateAvailableFundsForSingleTarget(int quantityQuota, double weight, double feeMax, double feeMin);

    double CalculateAvailableFund(const std::shared_ptr<config::StrategyParameter> &strategyParameter);

} // namespace trader

#endif //QUANT1X_TRADER_ACCOUNT_H
