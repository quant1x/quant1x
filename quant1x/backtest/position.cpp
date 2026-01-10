#include <quant1x/backtest/position.h>


namespace backtest {

    // 获取所有持仓
    std::unordered_map<std::string, Position>& PositionManager::getPositions() {
        return positions_;
    }

    // 获取特定标的持仓
    const Position* PositionManager::getPosition(const std::string &symbol) const {
        auto it = positions_.find(symbol);
        return it != positions_.end() ? &it->second : nullptr;
    }

    bool PositionManager::hasPosition(const std::string &symbol) const {
        auto it = positions_.find(symbol);
        if(it == positions_.end()) {
            return false;
        }
        return  it->second.quantity > 0;
    }

    // 在PositionManager类中添加
    double PositionManager::calculateFloatingPnL(const Position &pos, double current_price) {
        if (pos.direction == TradeDirection::LONG) {
            // 多头浮动盈亏 = (现价 - 成本价) * 数量
            return (current_price - pos.avg_price) * pos.quantity;
        } else if (pos.direction == TradeDirection::SHORT) {
            // 空头浮动盈亏 = (成本价 - 现价) * 数量
            return (pos.avg_price - current_price) * pos.quantity;
        }
        return 0.0;  // FLAT方向
    }

    double PositionManager::calculateTotalFloatingPnL(double current_price) {
        double total = 0.0;
        for (const auto &[symbol, pos] : positions_) {
            total += calculateFloatingPnL(pos, current_price);
        }
        return total;
    }

    // 获取特定标的的持仓数量
    double PositionManager::getPositionQuantity(const std::string &symbol) const {
        auto it = positions_.find(symbol);
        return it != positions_.end() ? it->second.quantity : 0.0;
    }

    void PositionManager::openNewPosition(Position &pos, const Trade &trade) {
        pos.symbol         = trade.symbol;
        pos.direction      = trade.direction;
        pos.quantity       = trade.quantity;
        pos.avg_price      = trade.price;
        pos.realized_pnl   = 0.0;
        pos.unrealized_pnl = 0.0;
        pos.open_time      = trade.trade_time;
        pos.update_time    = trade.trade_time;
        pos.open_trades.push_back(trade);
    }

    void PositionManager::addToPosition(Position &pos, const Trade &trade) {
        double total_cost = pos.avg_price * pos.quantity + trade.price * trade.quantity;
        pos.quantity += trade.quantity;
        pos.avg_price   = total_cost / pos.quantity;
        pos.update_time = trade.trade_time;
        pos.open_trades.push_back(trade);
    }

    void PositionManager::closePosition(Position &pos, const Trade &trade) {
        double pnl = (trade.price - pos.avg_price) * pos.quantity *
                     (pos.direction == TradeDirection::LONG ? 1.0 : -1.0);

        account_.realized_pnl += pnl;
        account_.current_capital += pnl;

        positions_.erase(trade.symbol);
    }

    void PositionManager::reducePosition(Position &pos, const Trade &trade)  {
        // 仅允许平仓或减仓，禁止反向开仓
        if (trade.quantity >= pos.quantity) {
            closePosition(pos, trade);
        } else {
            double pnl = (trade.price - pos.avg_price) * trade.quantity *
                         (pos.direction == TradeDirection::LONG ? 1.0 : -1.0);

            account_.realized_pnl += pnl;
            pos.quantity -= trade.quantity;
        }
    }

    void PositionManager::reduceOrReversePosition(Position &pos, const Trade &trade) {
        // 平掉部分仓位
        if (trade.quantity < pos.quantity) {
            double pnl = (trade.price - pos.avg_price) * trade.quantity *
                         (pos.direction == TradeDirection::LONG ? 1.0 : -1.0);

            account_.realized_pnl += pnl;
            account_.current_capital += pnl;

            pos.quantity -= trade.quantity;
            pos.update_time = trade.trade_time;
        }
            // 反向开仓(平掉原有仓位并开新仓位)
        else {
            closePosition(pos, trade);
            openNewPosition(pos, trade);
        }
    }

    void PositionManager::updateAccount() {
        double total_position_value = 0.0;
        double total_unrealized_pnl = 0.0;

        for (const auto &[symbol, pos] : positions_) {
            total_position_value += pos.cost();
            total_unrealized_pnl += pos.unrealized_pnl;
        }

        account_.unrealized_pnl    = total_unrealized_pnl;
        account_.total_pnl         = account_.realized_pnl + total_unrealized_pnl;
        account_.current_capital   = account_.initial_capital + account_.total_pnl;
        account_.available_capital = account_.current_capital - total_position_value;
        account_.margin_used       = total_position_value;
    }

    // 更新持仓市值
    void PositionManager::updatePositions(const std::string &code, const data::KLine &market_data) {
        for (auto &[symbol, pos] : positions_) {
            // 找到对应标的的市场数据
            if (symbol != code) {
                continue;
            }
            double current_price = market_data.close;
            pos.unrealized_pnl   = (current_price - pos.avg_price) * pos.quantity *
                                   (pos.direction == TradeDirection::LONG ? 1.0 : -1.0);
            pos.update_time = market_data.datetime;
        }
    }

    // 处理交易对持仓的影响
    void PositionManager::processTrade(const Trade &trade) {
        if(trade.direction == TradeDirection::HOLD) {
            return;
        }
        // 双重保险：再次验证卖空合法性
        if (trade.direction == TradeDirection::SHORT && !hasPosition(trade.symbol)) {
            spdlog::error("非法卖空交易被拦截: {}", trade.symbol);
            return;
        }
        auto it = positions_.find(trade.symbol);


        if(trade.direction == TradeDirection::FLAT || trade.direction == TradeDirection::SHORT) {
            // 如果是平仓指令
            if(it == positions_.end()) {
                // 没找到
                return;
            } else if (it->second.quantity < trade.quantity) {
                // 持仓数量不足卖出
                spdlog::warn("{} code={}, 卖出量[{}]不能超过持仓量[{}]", trade.trade_time, trade.symbol, trade.quantity, it->second.quantity);
                return;
            } else if (it->second.quantity == trade.quantity){
                // 找到了, 平仓
                spdlog::warn("{} code={}, 平仓", trade.trade_time, trade.symbol);
                // 平仓逻辑
                closePosition(it->second, trade);
            } else {
                // 减仓
                spdlog::warn("{} code={}, 允许减仓", trade.trade_time, trade.symbol);
                reducePosition(it->second, trade);
            }
        } else {
            // 买入指令
            if(it == positions_.end() || it->second.quantity == 0) {
                // 没找到 或者持仓为0, 新开仓
                spdlog::warn("{} code={}, 新开仓", trade.trade_time, trade.symbol);
                Position pos{};
                openNewPosition(pos, trade);
                positions_.emplace(trade.symbol, pos);
            } else {
                // 找到了, 加仓
                spdlog::warn("{} code={}, 加仓", trade.trade_time, trade.symbol);
                // 加仓
                addToPosition(it->second, trade);
            }
        }
        // 调试：打印持仓变化
        //spdlog::warn("持仓更新: {} 方向:{} 数量:{} 均价:{}", trade.symbol, (trade.direction == TradeDirection::LONG ? "多头" : "空头"), pos.quantity, pos.avg_price);
        // 更新账户信息
        updateAccount();
    }

} // namespace backtest