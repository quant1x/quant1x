#pragma once
#ifndef QUANT1X_TRADER_HOLDING_H
#define QUANT1X_TRADER_HOLDING_H 1

#include <cstdint>
#include <string>
#include <vector>
#include "trader.h"

namespace trader {
    struct HoldingPosition {
        int64_t     AccountType;      // 账户类型
        std::string AccountId;        // 资金账户
        std::string StockCode;        // 证券代码
        int64_t     Volume;           // 持仓数量
        int64_t     CanUseVolume;     // 可卖数量
        double      OpenPrice;        // 开仓价
        double      MarketValue;      // 市值
        int64_t     FrozenVolume;     // 冻结数量
        int64_t     OnRoadVolume;     // 在途股份
        int64_t     YesterdayVolume;  // 昨夜拥股
        double      AvgPrice;         // 成本价
        int64_t     HoldingPeriod;    // 持股周期

        HoldingPosition(const PositionDetail detail) {
            AccountType = detail.AccountType;
            AccountId = detail.AccountId;
            StockCode = detail.StockCode;
            Volume = detail.Volume;
            CanUseVolume = detail.CanUseVolume;
            OpenPrice = detail.OpenPrice;
            MarketValue = detail.MarketValue;
            FrozenVolume = detail.FrozenVolume;
            OnRoadVolume = detail.OnRoadVolume;
            YesterdayVolume = detail.YesterdayVolume;
            AvgPrice = detail.AvgPrice;
            HoldingPeriod = 0;
        }

        HoldingPosition operator=(const PositionDetail &detail) {
            AccountType = detail.AccountType;
            AccountId = detail.AccountId;
            StockCode = detail.StockCode;
            Volume = detail.Volume;
            CanUseVolume = detail.CanUseVolume;
            OpenPrice = detail.OpenPrice;
            MarketValue = detail.MarketValue;
            FrozenVolume = detail.FrozenVolume;
            OnRoadVolume = detail.OnRoadVolume;
            YesterdayVolume = detail.YesterdayVolume;
            AvgPrice = detail.AvgPrice;
            HoldingPeriod = 0;
        }
    };

    std::vector<HoldingPosition> GetHoldingPeriodList();
}  // namespace trader

#endif  // QUANT1X_TRADER_HOLDING_H
