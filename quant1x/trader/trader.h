#pragma once
#ifndef QUANT1X_TRADER_H
#define QUANT1X_TRADER_H 1

#include <quant1x/trader/constants.h>
#include <quant1x/trader/fee.h>
#include <ostream>

namespace trader {

    // 账户信息
    struct AccountDetail {
        double TotalAsset  = 0.00;  // 总金额
        double Cash        = 0.00;  // 可用
        double MarketValue = 0.00;  // 市值
        double FrozenCash  = 0.00;  // 冻结

        friend std::ostream &operator<<(std::ostream &os, const AccountDetail &detail);
    };

    // 查询账户信息
    std::optional<AccountDetail> QueryAccount();

    // 持仓信息
    struct PositionDetail {
        int64_t     AccountType = AccountType::SECURITY_ACCOUNT;  // 账户类型
        std::string AccountId;                                    // 资金账户
        std::string StockCode;                                    // 证券代码, 例如"600000.SH"
        int64_t     Volume          = 0;  // 持仓数量,股票以'股'为单位, 债券以'张'为单位
        int64_t     CanUseVolume    = 0;  // 可卖数量
        double      OpenPrice       = 0;  // 开仓价
        double      MarketValue     = 0;  // 市值
        int64_t     FrozenVolume    = 0;  // 冻结数量
        int64_t     OnRoadVolume    = 0;  // 在途股份
        int64_t     YesterdayVolume = 0;  // 昨夜拥股
        double      AvgPrice        = 0;  // 成本价

        friend std::ostream &operator<<(std::ostream &os, const PositionDetail &detail);
    };

    // 查询持仓
    std::vector<PositionDetail> QueryHolding();

    // 委托订单
    struct OrderDetail {
        int         AccountType = 0;  // 账户类型
        std::string AccountId;        // 资金账号
        std::string OrderTime;        // 委托时间
        std::string StockCode;        // 证券代码, 例如"600000.SH"
        int         OrderType = 0;    // 委托类型, 23:买, 24:卖
        double      Price = 0;  // 委托价格, 如果price_type为指定价, 那price为指定的价格, 否则填0
        int         PriceType   = 0;   // 报价类型, 详见帮助手册
        int         OrderVolume = 0;   // 委托数量, 股票以'股'为单位, 债券以'张'为单位
        int         OrderId     = 0;   // 委托编号
        std::string OrderSysid;        // 柜台编号
        double      TradedPrice  = 0;  // 成交均价
        int         TradedVolume = 0;  // 成交数量, 股票以'股'为单位, 债券以'张'为单位
        int         OrderStatus  = 0;  // 委托状态
        std::string StatusMessage;     // 委托状态描述, 如废单原因
        std::string StrategyName;      // 策略名称
        std::string OrderRemark;       // 委托备注
        friend std::ostream &operator<<(std::ostream &os, const OrderDetail &detail);
    };

    // 查询当日委托
    std::vector<OrderDetail> QueryOrders(int64_t order_id = 0);

    // 撤单
    int64_t CancelOrder(int64_t orderId);

    // 下委托订单
    int64_t PlaceOrder(Direction          direction,
                       const std::string &strategyName,
                       const std::string &orderRemark,
                       const std::string &securityCode,
                       int                priceType,
                       double             price,
                       int                volume);
}  // namespace trader

#endif  // QUANT1X_TRADER_H
