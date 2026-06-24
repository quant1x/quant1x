#pragma once
#ifndef QUANT1X_BACKTEST_STATS_H
#define QUANT1X_BACKTEST_STATS_H

#include <quant1x/backtest/backtest.h>

namespace backtest {

// 从 trades 中计算 round-trip 统计并写入 result(FIFO 开仓匹配, 处理部分成交)
void computeRoundTripStatsFromTrades(const std::vector<Trade> &trades, BacktestResult &result);

} // namespace backtest

#endif // QUANT1X_BACKTEST_STATS_H
