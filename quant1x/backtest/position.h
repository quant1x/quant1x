#pragma once
#ifndef QUANT1X_BACKTEST_POSITION_H
#define QUANT1X_BACKTEST_POSITION_H 1

#include <quant1x/backtest/trade.h>
#include <quant1x/data/schema/bar.h>

namespace backtest {

    namespace data = quant1x::data;

    // 持仓数据结构
    struct Position {
        std::string        symbol;          // 交易标的
        TradeDirection     direction;       // 持仓方向
        double             quantity;        // 持仓数量
        double             avg_price;       // 平均开仓价格
        double             realized_pnl;    // 已实现盈亏
        double             unrealized_pnl;  // 未实现盈亏
        std::string        open_time;       // 开仓时间
        std::string        update_time;     // 最后更新时间
        std::vector<Trade> open_trades;     // 构成该持仓的所有交易

        // 计算当前持仓市值
        double market_value(double current_price) const {
            return quantity * current_price;
        }

        // 计算持仓成本
        double cost() const {
            return quantity * avg_price;
        }
    };

    // 账户数据结构
    struct Account {
        double      initial_capital;    // 初始资金
        double      current_capital;    // 当前资金
        double      available_capital;  // 可用资金
        double      margin_used;        // 已用保证金
        double      total_pnl;          // 总盈亏
        double      realized_pnl;       // 已实现盈亏
        double      unrealized_pnl;     // 未实现盈亏
        double      sharpe_ratio;       // 夏普比率
        double      max_drawdown;       // 最大回撤
        std::string update_time;        // 最后更新时间
    };

    // 每日持仓状态
    struct DailyPositionStatus {
        std::string                     timestamp;
        // 为了减少拷贝开销, 日结只保留持仓摘要而非整张持仓表
        struct PositionSummary {
            std::string    symbol;
            TradeDirection direction;
            double         quantity;
            double         avg_price;
            double         unrealized_pnl;
        };

        std::vector<PositionSummary>    positions;
        Account                         account;

        // 序列化方法
        std::string toString() const {
            std::ostringstream oss;
            oss << "Date: " << timestamp << "\n";
            oss << "Account Equity: " << account.current_capital << "\n";
            oss << "Positions:\n";

            for (const auto &pos : positions) {
                oss << "  " << pos.symbol << ": " << (pos.direction == TradeDirection::LONG ? "LONG" : "SHORT") << " "
                    << pos.quantity << " @ " << pos.avg_price << " (Unrealized PnL: " << pos.unrealized_pnl << ")\n";
            }

            return oss.str();
        }
    };

    // ==================== 持仓管理 ====================

    class PositionManager {
    private:
        std::unordered_map<std::string, Position> positions_;  // 按标的代码索引的持仓
        Account                        &account_;    // 关联的账户

    public:
        explicit PositionManager(Account &acc) : account_(acc) {}
        // 获取所有持仓
        std::unordered_map<std::string, Position> &getPositions();
        // 获取特定标的持仓
        const Position *getPosition(const std::string &symbol) const;
        bool hasPosition(const std::string &symbol) const;
        // 在PositionManager类中添加
        double calculateFloatingPnL(const Position &pos, double current_price);
        double calculateTotalFloatingPnL(double current_price);
        // 获取特定标的的持仓数量
        double getPositionQuantity(const std::string &symbol) const;

        void updatePositions(const std::string &code, const data::KLine &market_data);

        void processTrade(const Trade &trade);

    private:
        void openNewPosition(Position &pos, const Trade &trade);
        void addToPosition(Position &pos, const Trade &trade);
        void closePosition(Position &pos, const Trade &trade);
        void reducePosition(Position &pos, const Trade &trade);
        void reduceOrReversePosition(Position &pos, const Trade &trade);
        void updateAccount();
    };

} // namespace backtest

#endif //QUANT1X_BACKTEST_POSITION_H
