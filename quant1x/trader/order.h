#pragma once
#ifndef QUANT1X_TRADER_ORDER_H
#define QUANT1X_TRADER_ORDER_H 1

#include <quant1x/trader/fee.h>

// ==============================
// 订单管理(OMS)
// ==============================

namespace trader {

    std::string account_id();
    std::string trader_qmt_order_path();
    std::string order_flag(Direction dir);

} // namespace trader

#endif //QUANT1X_TRADER_ORDER_H
