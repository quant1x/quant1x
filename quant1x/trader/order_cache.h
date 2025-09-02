#pragma once
#ifndef QUANT1X_TRADER_ORDER_CACHE_H
#define QUANT1X_TRADER_ORDER_CACHE_H 1

#include <string>
#include <vector>
#include "trader.h"

namespace trader {

    //extern std::string traderQmtOrderPath;

    // 获取订单文件名（支持传入日期）
    std::string GetOrderFilename(const std::string& date = "");

    // 获取指定日期的订单列表
    std::vector<OrderDetail> GetOrderList(const std::string& date);

    // 获取本地订单日期列表
    std::vector<std::string> GetLocalOrderDates();

} // namespace trader

#endif //QUANT1X_TRADER_ORDER_CACHE_H
