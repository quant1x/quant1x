#pragma once
#ifndef QUANT1X_BACKTEST_ORDER_H
#define QUANT1X_BACKTEST_ORDER_H 1

#include <quant1x/engine/strategy.h>

namespace backtest {

    // 订单类型枚举
    enum class OrderType {
        MARKET,  ///< 市价单
        LIMIT,   ///< 限价单
        STOP     ///< 止损单
    };

    // 订单状态枚举
    enum class OrderStatus {
        PENDING,    ///< 待执行
        FILLED,     ///< 已成交
        CANCELLED,  ///< 已取消
        REJECTED    ///< 已拒绝
    };

    // ==================== 数据结构定义 ====================

    // 订单数据结构
    struct Order {
        std::string    order_id;     // 订单ID
        std::string    symbol;       // 交易标的
        OrderType      type;         // 订单类型
        TradeDirection direction;    // 交易方向
        double         price;        // 订单价格(限价单/止损单)
        double         quantity;     // 数量
        std::string    create_time;  // 创建时间
        std::string    update_time;  // 更新时间
        OrderStatus    status;       // 订单状态
        std::string    message;      // 附加信息(如拒绝原因)
    };

} // namespace backtest

#endif //QUANT1X_BACKTEST_ORDER_H
