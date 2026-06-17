#pragma once
#ifndef QUANT1X_BACKTEST_H
#define QUANT1X_BACKTEST_H 1

#include <quant1x/backtest/order.h>
#include <quant1x/backtest/position.h>
#include <quant1x/backtest/trade.h>
#include <quant1x/data/kline.h>
#include <quant1x/engine/strategy.h>
#include <quant1x/std/api.h>

// 使用时间点别名简化代码
using TimePoint = std::chrono::system_clock::time_point;

namespace backtest {

    // 生成唯一订单ID
    std::string generateOrderId();

    // 生成唯一成交ID
    std::string generateTradeId();

    // 回测结果数据结构
    struct BacktestResult {
        double total_return;           // 总收益率
        double annualized_return;      // 年化收益率
        double annualized_volatility;  // 年化波动率
        double sharpe_ratio;           // 夏普比率
        double sortino_ratio;          // 索提诺比率
        double max_drawdown;           // 最大回撤
        double win_rate;               // 胜率
        double profit_loss_ratio;      // 盈亏比
        // 语义说明: 
        //  - trade_events_count: 原子级别的成交事件数(每笔成交/fill), 等于 backtest_data.trades.size()
        //  - closed_roundtrips: 按完整回合统计的“已平仓回合数”(round-trip), 即只有当一个开仓被完全平掉时计数
        size_t                   trade_events_count;  // 成交事件数(每笔 fill)
        size_t                   closed_roundtrips;   // 已平仓回合数(round-trip)
        size_t                   total_trades;        // 兼容旧名: 等同于 closed_roundtrips
        size_t                   winning_trades;      // 盈利回合数
        size_t                   losing_trades;       // 亏损回合数
        size_t                   closed_trades;  // 兼容字段: 等同于 closed_roundtrips(保留旧名以兼容现有代码/日志)
        size_t                   covered_days;   // 在回测范围内有持仓的天数(按日计)
        double                   coverage_days_rate;   // 日级覆盖率(有仓位的天数 / 总交易日数, 百分比)
        double                   coverage_bars_rate;   // Bar/日内级覆盖率(有仓位的Bar数 / 总Bar数, 百分比)
        double                   coverage_rate;        // 兼容字段: 默认赋值为日级覆盖率(保留旧名以兼容现有代码/日志)
        double                   avg_profit;           // 平均盈利
        double                   avg_loss;             // 平均亏损
        std::vector<double>      equity_curve;         // 资金曲线
        double                   floating_pnl;         // 最终浮动盈亏
        size_t                   unsettled_positions;  // 未平仓头寸数量
        std::vector<std::string> unsettled_symbols;    // 未平仓标的
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
        bool        verbose              = false;  // 是否输出详细日志(回测内部使用)
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
        Order  createOrder(const std::string &code, const data::KLine &bar, TradeDirection direction);
        Trade  executeOrder(const Order &order);

        void recordDailyStatus(const data::KLine &bar);

    public:
        BacktestEngine(const BacktestConfig &config, StrategyPtr strategy);
        // 初始化账户
        void initAccount();
        void finalizeBacktest(const std::string &code, const data::KLine &last_bar);
        // 加载市场数据
        // void loadMarketData(const std::vector<datasets::KLine> &market_data);
        // 运行回测
        void run(const std::string &code);
        /**
         * 计算持仓浮动盈亏(回测结束时调用)
         * @param last_price 最后交易日收盘价
         * @return 总浮动盈亏金额
         */
        double calculateFloatingPnL(double last_price);
        // 获取回测数据
        const BacktestData &getBacktestData() const;
        // 打印回测结果
        void printResults() const;

        void calculateResults();
        // 计算按 round-trip 的交易统计(供测试使用)
        // 该函数会从 backtest_data.trades 中重建 round-trip 并填充 result 中相关字段
        void computeRoundTripStats();
        // 纯函数辅助: 请使用 free function `backtest::computeRoundTripStatsFromTrades`(见 backtest/stats.h)
        // Test helper: allow unit tests to inject trades into the engine for isolated testing
        void setTradesForTest(const std::vector<Trade> &trades);
    };

}  // namespace backtest

#endif  // QUANT1X_BACKTEST_H
