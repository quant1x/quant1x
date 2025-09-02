#include <quant1x/backtest/backtest.h>

namespace backtest {

    BacktestEngine::BacktestEngine(const BacktestConfig &config, StrategyPtr strategy)
        : strategy_(strategy), position_manager(backtest_data.account) {
        backtest_data.config = config;
    }

    // 计算头寸大小
    // 修改资金计算逻辑
    double BacktestEngine::calculatePositionSize(double price) const {
        double position_value = backtest_data.account.available_capital * 0.2;  // 20%仓位
        double raw_shares     = position_value / price;

        // 向下取整到最接近的整手
        return floor(raw_shares / 100) * 100;
    }

    // 计算手续费
    double BacktestEngine::calculateFee(double price, double quantity) const {
        return price * quantity * backtest_data.config.commission_rate;
    }

    // 获取当前持仓数量
    double BacktestEngine::getPositionQuantity(const std::string &symbol) const {
        auto it = position_manager.getPosition(symbol);
        return (it != nullptr) ? it->quantity : 0.0;
    }

    // 创建订单
    Order BacktestEngine::createOrder(const std::string &code, const datasets::KLine &bar, TradeDirection direction) {
        Order order;
        order.order_id = backtest::generateOrderId();
        order.symbol   = code;
        order.type     = OrderType::MARKET;
        // 禁止直接做空开仓
        if (direction == TradeDirection::SHORT && !position_manager.hasPosition(code)) {
            order.status  = OrderStatus::REJECTED;
            order.message = "A股禁止无持仓卖空";
            return order;
        }
        order.direction = direction;
        order.price     = bar.Close * (1.0 + (direction == TradeDirection::LONG ? backtest_data.config.slippage_rate
                                                                                : -backtest_data.config.slippage_rate));
        // 计算理论仓位
        double raw_quantity = calculatePositionSize(bar.Close);
        // A股买入必须为100股整数倍
        int lot_size         = 100;  // A股1手=100股
        int rounded_quantity = static_cast<int>(raw_quantity / lot_size) * lot_size;

        if (direction == TradeDirection::LONG) {
            // 买入必须≥1手
            order.quantity = std::max(rounded_quantity, lot_size);
        } else {
            // 卖出可以零股（但建议整手）
            order.quantity = getPositionQuantity(code);
        }
        //order.quantity    = calculatePositionSize(bar.Close);
        order.create_time = bar.Datetime;
        order.update_time = bar.Datetime;
        order.status      = OrderStatus::PENDING;
        return order;
    }

    // 执行订单
    Trade BacktestEngine::executeOrder(const Order &order) {
        Trade trade{};
        trade.trade_id   = backtest::generateTradeId();
        trade.order_id   = order.order_id;
        trade.symbol     = order.symbol;
        trade.direction  = order.direction;
        trade.price      = order.price;
        trade.quantity   = order.quantity;
        trade.fee        = calculateFee(order.price, order.quantity);
        trade.trade_time = order.create_time;
        return trade;
    }

    // 初始化账户
    void BacktestEngine::initAccount() {
        backtest_data.account.initial_capital   = backtest_data.config.initial_capital;
        backtest_data.account.current_capital   = backtest_data.config.initial_capital;
        backtest_data.account.available_capital = backtest_data.config.initial_capital;
        backtest_data.account.margin_used       = 0.0;
        backtest_data.account.total_pnl         = 0.0;
        backtest_data.account.realized_pnl      = 0.0;
        backtest_data.account.unrealized_pnl    = 0.0;
    }

    // 记录每日状态
    void BacktestEngine::recordDailyStatus(const datasets::KLine &bar) {
        DailyPositionStatus status;
        status.timestamp = bar.Datetime;

        //  包含浮动盈亏的账户快照
        Account snap = backtest_data.account;
        for (const auto &[symbol, pos] : position_manager.getPositions()) {
            snap.current_capital += pos.unrealized_pnl;
        }
        status.account   = snap;
        status.positions = position_manager.getPositions();
        backtest_data.daily_status.push_back(status);
    }

    // 计算回测结果
    void BacktestEngine::calculateResults() {
        if (backtest_data.daily_status.empty()) {
            return;
        }
        const auto &equity_curve = backtest_data.result.equity_curve;

        // 计算总收益率
        double initial                    = backtest_data.account.initial_capital;
        double final                      = equity_curve.back();
        backtest_data.result.total_return = (final - initial) / initial * 100.0;

        // 计算年化收益率(简化计算)
        size_t num_days = equity_curve.size();
        double years    = double(num_days) / 252.0;
        backtest_data.result.annualized_return = (pow(1.0 + backtest_data.result.total_return / 100.0, 1.0 / years) - 1.0) * 100.0;

        // 计算夏普比率
        std::vector<double> daily_returns;
        for (size_t i = 1; i < equity_curve.size(); ++i) {
            double ret = (equity_curve[i] - equity_curve[i - 1]) / equity_curve[i - 1];
            daily_returns.push_back(ret);
        }

        double mean_return = std::accumulate(daily_returns.begin(), daily_returns.end(), 0.0) / daily_returns.size();
        double sq_sum      = std::inner_product(daily_returns.begin(), daily_returns.end(), daily_returns.begin(), 0.0);
        double stdev       = std::sqrt(sq_sum / daily_returns.size() - mean_return * mean_return);

        backtest_data.result.annualized_volatility = stdev * sqrt(252) * 100.0;
        backtest_data.result.sharpe_ratio          = mean_return / stdev * sqrt(252);

        // 计算最大回撤
        double peak         = initial;
        double max_drawdown = 0.0;
        for (double equity : equity_curve) {
            if (equity > peak) {
                peak = equity;
            }
            double drawdown = (peak - equity) / peak * 100.0;
            if (drawdown > max_drawdown) {
                max_drawdown = drawdown;
            }
        }
        backtest_data.result.max_drawdown = max_drawdown;

        // 计算交易统计
        backtest_data.result.total_trades = backtest_data.trades.size();

        // 计算胜率和盈亏比
        int    winning_trades = 0;
        double total_profit   = 0.0;
        double total_loss     = 0.0;

        for (const auto &trade : backtest_data.trades) {
            // 假设只有平仓交易才产生盈亏
            if (trade.direction == TradeDirection::FLAT) {
                // 查找该笔成交关联的订单
                auto it = std::find_if(backtest_data.orders.begin(),
                                       backtest_data.orders.end(),
                                       [&trade](const Order &ord) { return ord.order_id == trade.order_id; });
                if (it != backtest_data.orders.end()) {
                    // 计算盈亏
                    double entry_price = it->price;
                    double exit_price  = trade.price;
                    double quantity    = trade.quantity;

                    // 如果是做多单
                    if (it->direction == TradeDirection::LONG) {
                        double pnl = (exit_price - entry_price) * quantity;
                        if (pnl > 0) {
                            winning_trades++;
                            total_profit += pnl;
                        } else {
                            total_loss += std::abs(pnl);
                        }
                    } else if (it->direction == TradeDirection::SHORT) {
                        // 如果是做空单（可选）
                        double pnl = (entry_price - exit_price) * quantity;
                        if (pnl > 0) {
                            winning_trades++;
                            total_profit += pnl;
                        } else {
                            total_loss += std::abs(pnl);
                        }
                    }
                }
            }
        }

        // 统计结果
        backtest_data.result.winning_trades = winning_trades;
        backtest_data.result.losing_trades  = backtest_data.trades.size() - winning_trades;
        backtest_data.result.win_rate       = winning_trades * 100.0 /
                                              (backtest_data.trades.empty() ? 1.00 : double(backtest_data.trades.size()));

        // 计算平均盈利/亏损
        backtest_data.result.avg_profit = winning_trades > 0 ? total_profit / winning_trades : 0.0;
        backtest_data.result.avg_loss   = (backtest_data.result.losing_trades > 0)
                                          ? total_loss / double(backtest_data.result.losing_trades)
                                          : 0.0;

        // 盈亏比 = 平均盈利 / 平均亏损
        backtest_data.result.profit_loss_ratio = (backtest_data.result.avg_loss != 0)
                                                 ? backtest_data.result.avg_profit / backtest_data.result.avg_loss
                                                 : 0.0;

        // 统计未平仓头寸
        backtest_data.result.unsettled_positions = position_manager.getPositions().size();
        backtest_data.result.floating_pnl        = 0.0;

        for (const auto &[symbol, pos] : position_manager.getPositions()) {
            backtest_data.result.unsettled_symbols.push_back(symbol);
            backtest_data.result.floating_pnl += pos.unrealized_pnl;
        }

        // 在计算总收益率前添加
        if (!position_manager.getPositions().empty()) {
            spdlog::debug("警告: 回测结束仍有未平仓头寸");
        }
    }

    void BacktestEngine::finalizeBacktest(const std::string &code, const datasets::KLine &last_bar) {
        // 调试：打印开始结算信息
        spdlog::debug("开始结算未平仓头寸...");
        // 检查剩余持仓
        auto &positions = position_manager.getPositions();
        if (!positions.empty()) {
            // 获取最后交易日收盘价
            double last_price = last_bar.Close;

            // 记录未平仓信息
            for (auto &[symbol, position] : positions) {
                if (symbol != code) {
                    continue;
                }
                // 计算浮动盈亏
                position.unrealized_pnl = position_manager.calculateFloatingPnL(position, last_price);

                // 更新回测结果中的未平仓信息
                backtest_data.result.unsettled_positions++;
                backtest_data.result.floating_pnl += position.unrealized_pnl;
                backtest_data.result.unsettled_symbols.push_back(symbol);

                // 打印日志
                spdlog::debug("[未平仓] {} 方向:{} 数量:{} 成本价:{} 结算价:{} 盈亏:{}", symbol, (position.direction == TradeDirection::LONG ? "多头" : "空头"),
                          position.quantity, position.avg_price, last_price, position.unrealized_pnl);
            }
        } else {
            spdlog::debug("没有未平仓头寸需要结算");
        }
    }

    /**
     * 计算持仓浮动盈亏（回测结束时调用）
     * @param last_price 最后交易日收盘价
     * @return 总浮动盈亏金额
     */
    double BacktestEngine::calculateFloatingPnL(double last_price) {
        double total_pnl = 0.0;

        for (auto &[symbol, position] : position_manager.getPositions()) {
            // 多头：(现价 - 成本价)*数量
            // 空头：(成本价 - 现价)*数量
            double pnl = (position.direction == TradeDirection::LONG)
                         ? (last_price - position.avg_price) * position.quantity
                         : (position.avg_price - last_price) * position.quantity;

            position.unrealized_pnl = pnl;
            total_pnl += pnl;

            // 记录日志（生产环境可降低为DEBUG级别）
            spdlog::debug("[结算] {} 持仓量:{} 成本价:{} 结算价:{} 盈亏:{}", symbol, position.quantity, position.avg_price, last_price, pnl);
        }

        return total_pnl;
    }

    // 运行回测
    void BacktestEngine::run(const std::string &code) {
        strategy_->reset();
        strategy_->updateIndicators(code);
        int max_periods = 10;

        auto const & market_data = strategy_->market_data();

        for (size_t i = market_data.size() - max_periods; i < market_data.size(); ++i) {
            const auto &bar = market_data[i];

            // 调试：打印当前处理日期
            //spdlog::debug("处理日期: {}", bar.Datetime);
            // 更新持仓市值
            position_manager.updatePositions(code, bar);
            // 生成信号
            TradeDirection signal = strategy_->generateSignal(i);
            spdlog::warn("{} {}, signal:{}", bar.Datetime, code, magic_enum::enum_name(signal));
            if (signal == TradeDirection::HOLD) {
                continue;
            }
            if (signal != TradeDirection::FLAT) {
                // 只有当有持仓时才允许卖出（平仓）
                if (signal == TradeDirection::SHORT) {
                    if (!position_manager.hasPosition(code)) {
                        // 没有持仓，禁止卖出
                        spdlog::warn("{} {}, signal:{}, 没有持仓，禁止卖出", bar.Datetime, code, magic_enum::enum_name(signal));
                        continue;
                    }
                }
                // 执行交易
                Order order = createOrder(code, bar, signal);
                spdlog::warn("{} order: {}, message={}", code, order.quantity, order.message);
                if(order.status == OrderStatus::REJECTED || order.quantity == 0) {
                    spdlog::warn("{} {}, signal:{}, 订单被拒绝", bar.Datetime, code, magic_enum::enum_name(signal));
                    continue;
                }
                backtest_data.orders.push_back(order);

                Trade trade = executeOrder(order);
                backtest_data.trades.push_back(trade);

                // 调试：打印交易详情
                spdlog::warn("执行交易: {} {}股 @{}  当前持仓量: {}", (trade.direction == TradeDirection::LONG ? "买入" : "卖出"),
                             trade.quantity, trade.price, position_manager.getPositionQuantity(code));

                // 处理持仓变化
                position_manager.processTrade(trade);
            }

            // 调试：打印每日持仓状态
            //spdlog::debug("日期结束持仓: {}股", position_manager.getPositionQuantity(code));
            // 记录每日状态
            backtest_data.result.equity_curve.push_back(backtest_data.account.current_capital + position_manager.calculateTotalFloatingPnL(bar.Close));
            recordDailyStatus(bar);
        }
        //spdlog::warn("trade number:{}", backtest_data.trades.size());

        // 添加回测结束处理
        if (!market_data.empty()) {
            finalizeBacktest(code, market_data.back());
        }
        calculateResults();  // 重新计算绩效
    }

    // 获取回测数据
    const BacktestData& BacktestEngine::getBacktestData() const {
        return backtest_data;
    }

    // 打印回测结果
    void BacktestEngine::printResults() const {
        const auto &result = backtest_data.result;
        io::CSVWriter out("acc.csv");
        out.write_row("amount");
        for(auto const &v : result.equity_curve) {
            out.write_row(v);
        }

        std::cout << "========== 回测结果 ==========\n";
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "总收益率: " << result.total_return << "%\n";
        std::cout << "年化收益率: " << result.annualized_return << "%\n";
        std::cout << "年化波动率: " << result.annualized_volatility << "%\n";
        std::cout << "夏普比率: " << result.sharpe_ratio << "\n";
        std::cout << "最大回撤: " << result.max_drawdown << "%\n";
        std::cout << "总交易次数: " << result.total_trades << "\n";
        std::cout << "胜率: " << result.win_rate << "%\n";
        std::cout << "盈亏比: " << result.profit_loss_ratio << "\n";
    }

} // namespace backtest