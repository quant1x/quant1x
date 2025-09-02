#pragma once
#ifndef QUANT1X_ORDER_STATE_H
#define QUANT1X_ORDER_STATE_H 1

#include <quant1x/std/api.h>
#include <quant1x/engine/strategy.h>
#include "fee.h"

namespace trader {

    // 获得订单标识文件名
    std::string order_state_filename(const std::string& date,
                                     const StrategyInfo& model,
                                     Direction direction,
                                     const std::string& code);

    // 检查订单执行状态
    bool CheckOrderState(const std::string& date,
                         const StrategyInfo& model,
                         const std::string& code,
                         trader::Direction direction);

    // 推送订单完成状态
    bool PushOrderState(const std::string& date,
                        const StrategyInfo& model,
                        const std::string& code,
                        trader::Direction direction);

    int CountStrategyOrders(const std::string& date,
                            const StrategyInfo& model,
                            trader::Direction direction);

    std::vector<std::string> FetchListForFirstPurchase(const std::string& date,
                                                       const std::string& quantStrategyName,
                                                       Direction direction);
} // namespace trader

#endif //QUANT1X_ORDER_STATE_H
