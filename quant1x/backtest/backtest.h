#pragma once
#ifndef QUANT1X_BACKTEST_H
#define QUANT1X_BACKTEST_H 1

#include <quant1x/std/api.h>
#include <quant1x/backtest/order.h>
#include <quant1x/backtest/position.h>
#include <quant1x/backtest/trade.h>
#include <quant1x/datasets/kline.h>
#include <quant1x/engine/strategy.h>

// 使用时间点别名简化代码
using TimePoint = std::chrono::system_clock::time_point;

namespace backtest {

    // 生成唯一订单ID
    std::string generateOrderId();

    // 生成唯一成交ID
    std::string generateTradeId();

    // 回测结果数据结构
    struct BacktestResult {
        double                   total_return;           // 总收益率
        double                   annualized_return;      // 年化收益率
        double                   annualized_volatility;  // 年化波动率
        double                   sharpe_ratio;           // 夏普比率
        double                   sortino_ratio;          // 索提诺比率
        double                   max_drawdown;           // 最大回撤
        double                   win_rate;               // 胜率
        double                   profit_loss_ratio;      // 盈亏比
        size_t                   total_trades;           // 总交易次数
        size_t                   winning_trades;         // 盈利交易次数
        size_t                   losing_trades;          // 亏损交易次数
        double                   avg_profit;             // 平均盈利
        double                   avg_loss;               // 平均亏损
        std::vector<double>      equity_curve;           // 资金曲线
        double                   floating_pnl;           // 最终浮动盈亏
        size_t                   unsettled_positions;    // 未平仓头寸数量
        std::vector<std::string> unsettled_symbols;      // 未平仓标的
    };

    // 回测配置数据结构
    struct BacktestConfig {
        TimePoint   start_time;                    // 回测开始时间
        TimePoint   end_time;                      // 回测结束时间
        double      initial_capital = 0;           // 初始资金
        std::string data_source;                   // 数据源
        std::string strategy_name;                 // 策略名称
        double      commission_rate      = 0;      // 手续费率
        double      slippage_rate        = 0;      // 滑点率
        bool        enable_short_selling = false;  // 是否允许卖空
        int         leverage             = 0;      // 杠杆倍数
    };

    // 主回测数据结构
    struct BacktestData {
        BacktestConfig                   config;        // 回测配置
        Account                          account;       // 账户信息
        BacktestResult                   result;        // 回测结果
        std::vector<Order>               orders;        // 所有订单
        std::vector<Trade>               trades;        // 所有成交
        std::vector<DailyPositionStatus> daily_status;  // 每日持仓状态
        // std::vector<std::string>         logs;          // 日志记录
    };
    // ==================== 回测引擎 ====================

    class BacktestEngine {
    private:
        BacktestData    backtest_data{};
        StrategyPtr     strategy_;  // 多态指针
        PositionManager position_manager;

    private:
        double calculatePositionSize(double price) const;
        double calculateFee(double price, double quantity) const;
        double getPositionQuantity(const std::string &symbol) const;
        Order  createOrder(const std::string &code, const datasets::KLine &bar, TradeDirection direction);
        Trade  executeOrder(const Order &order);

        void recordDailyStatus(const datasets::KLine &bar);

    public:
        BacktestEngine(const BacktestConfig &config, StrategyPtr strategy);
        // 初始化账户
        void initAccount();
        void finalizeBacktest(const std::string &code, const datasets::KLine &last_bar);
        // 加载市场数据
        // void loadMarketData(const std::vector<datasets::KLine> &market_data);
        // 运行回测
        void run(const std::string &code);
        /**
         * 计算持仓浮动盈亏（回测结束时调用）
         * @param last_price 最后交易日收盘价
         * @return 总浮动盈亏金额
         */
        double calculateFloatingPnL(double last_price);
        // 获取回测数据
        const BacktestData &getBacktestData() const;
        // 打印回测结果
        void printResults() const;

        void calculateResults();
    };

}  // namespace backtest

#endif  // QUANT1X_BACKTEST_H
