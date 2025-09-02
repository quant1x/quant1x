#pragma once
#ifndef QUANT1X_TRADER_POSITION_H
#define QUANT1X_TRADER_POSITION_H 1

#include <string>

#include "trader.h"

namespace trader {

    struct Position {
        int64_t     AccountType;      // 账户类型
        std::string AccountId;        // 资金账号
        int64_t     StrategyCode;     // 策略编码
        std::string OrderFlag;        // 订单标识
        std::string SecurityCode;     // 证券代码
        int64_t     Volume;           // 持仓数量
        int64_t     CanUseVolume;     // 可卖数量
        double      OpenPrice;        // 开仓价
        double      MarketValue;      // 市值
        int64_t     FrozenVolume;     // 冻结数量
        int64_t     OnRoadVolume;     // 在途股份
        int64_t     YesterdayVolume;  // 昨夜拥股
        double      AvgPrice;         // 成本价
        std::string CreateTime;       // 创建时间
        std::string LastOrderId;      // 前订单ID
        std::string BuyTime;          // 买入时间
        double      BuyPrice;         // 买入价格
        int64_t     BuyVolume;        // 买入数量
        std::string SellTime;         // 卖出时间
        double      SellPrice;        // 卖出价格
        int64_t     SellVolume;       // 卖出数量
        std::string CancelTime;       // 撤单时间
        std::string UpdateTime;       // 更新时间

        std::string Key() const;
        bool        Sync(const PositionDetail &other);
        bool        MergeFromOrder(const OrderDetail &order, double price);
    };

    // 同步持仓
    void SyncPositions();

    // 更新持仓
    void UpdatePositions();

    // 持仓缓存同步到文件
    void CacheSync();

}  // namespace trader

#endif  // QUANT1X_TRADER_POSITION_H
