#pragma once
#ifndef QUANT1X_TRADER_FEE_H
#define QUANT1X_TRADER_FEE_H 1

#include <quant1x/base/api.h>
#include <quant1x/base/numeric.h>

// ==============================
// 交易费用
// ==============================

namespace trader {
    constexpr const double  InvalidFloat64  = numeric::NaN;
    constexpr const double  InvalidFee     = 0.00;  // 无效的费用
    constexpr const int64_t InvalidVolume  = 0;    // 无效的股数
    constexpr const int64_t UnknownVolume  = 1;    // 未知的股数
    constexpr const int64_t InvalidOrderId = -1;   // 无效的订单ID
    constexpr const int64_t MinimumNumberOfOrders = 2;  // 最小订单数为2, 预防一个策略资金都打到一个标的上面
    constexpr const double BacktestAccountTheoreticalFund = 10000.00;  // 测试可用10000.00元

    // 交易方向
    enum class Direction {
        UNKNOWN,  // 未知, 无效的
        BUY,      // 买入
        SELL,     // 卖出
        JUNK      // 废单
    };

    // 交易费用
    struct TradeFee {
        Direction   Direction = Direction::UNKNOWN;  // 交易方向
        std::string SecurityCode;                    // 证券代码
        double      Price         = 0;               // 价格
        int         Volume        = 0;               // 数量
        double      StampDutyFee  = 0;  // 印花税, 按照成交金额计算, 双向, 默认单向, 费率0.1%
        double      TransferFee   = 0;  // 过户费, 按照股票数量, 双向, 默认是0.06%
        double      CommissionFee = 0;  // 券商佣金, 按照成交金额计算, 双向, 0.025%
        double      MarketValue   = 0;  // 股票市值
        double      TotalFee      = 0;  // 支出总费用

        TradeFee() = default;

        double CalculateFundFromSell(double price, int volume);

        int CalculateNumToBuy(double fund, double price);

        std::string toString() const;
    };

    /**
     * @brief 计算价格笼子
     * @param price_cage_ratio 价格笼子比例
     * @param minimum_price_fluctuation_unit 价格最小变动单位
     * @param direction 交易方向
     * @param price 当前价格
     * @return 通过价格笼子计算方法, 返回修正后的委托价格
     */
    double calculate_price_cage(double price_cage_ratio, double minimum_price_fluctuation_unit, Direction direction, double price);

    /**
     * @brief 计算价格笼子, 默认从trader配置中获取参数
     * @param direction 交易方向
     * @param price 当前价格
     * @return 通过价格笼子计算方法, 返回修正后的委托价格
     */
    double calculate_price_cage(Direction direction, double price);

    /**
     * @brief 计算价格笼子, 默认从strategy配置中获取参数
     * @param strategy_id 策略ID
     * @param direction 交易方向
     * @param price 当前价格
     * @return 通过价格笼子计算方法, 返回修正后的委托价格
     */
    double calculate_price_cage(uint64_t strategy_id, Direction direction, double price);

    // 计算合适的买入价格
    double calculate_price_limit_for_buy(double last_price, double price_cage_ratio, double minimum_price_fluctuation_unit);
    // 计算合适的卖出价格
    double calculate_price_limit_for_sell(double last_price, double price_cage_ratio, double minimum_price_fluctuation_unit);
    // 计算合适的卖出价格
    double calculate_price_limit_for_sell(double last_price, double fixed_slippage_for_sell);

    // 评估买入总费用
    TradeFee EvaluateFeeForBuy(const std::string &securityCode, double fund, double price);
    // 评估卖出费用
    TradeFee EvaluateFeeForSell(const std::string &securityCode, double price, int volume);

}  // namespace trader

#endif  // QUANT1X_TRADER_FEE_H
