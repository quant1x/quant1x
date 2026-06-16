#include <quant1x/backtest/backtest.h>
#include <quant1x/backtest/stats.h>

#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace backtest {

    // 生成唯一订单ID
    std::string generateOrderId() {
        static int counter = 0;
        return "ORD_" + std::to_string(++counter);
    }

    // 生成唯一成交ID
    std::string generateTradeId() {
        static int counter = 0;
        return "TRD_" + std::to_string(++counter);
    }


} // namespace backtest


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
    Order BacktestEngine::createOrder(const std::string &code, const data::KLine &bar, TradeDirection direction) {
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
        order.price     = bar.close * (1.0 + (direction == TradeDirection::LONG ? backtest_data.config.slippage_rate
                                                                                : -backtest_data.config.slippage_rate));
        // 计算理论仓位
        double raw_quantity = calculatePositionSize(bar.close);
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
        // order.quantity    = calculatePositionSize(bar.Close);
        order.create_time = bar.datetime;
        order.update_time = bar.datetime;
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
    void BacktestEngine::recordDailyStatus(const data::KLine &bar) {
        DailyPositionStatus status;
        status.timestamp = bar.datetime;

        //  包含浮动盈亏的账户快照（避免频繁拷贝positions map）
        Account     snap      = backtest_data.account;
        const auto &positions = position_manager.getPositions();
        status.positions.reserve(positions.size());
        for (const auto &p : positions) {
            const auto &pos = p.second;
            snap.current_capital += pos.unrealized_pnl;
            DailyPositionStatus::PositionSummary ps;
            ps.symbol         = p.first;
            ps.direction      = pos.direction;
            ps.quantity       = pos.quantity;
            ps.avg_price      = pos.avg_price;
            ps.unrealized_pnl = pos.unrealized_pnl;
            status.positions.push_back(std::move(ps));
        }
        status.account = snap;
        backtest_data.daily_status.emplace_back(std::move(status));
    }

    // 计算回测结果
    void BacktestEngine::calculateResults() {
        if (backtest_data.daily_status.empty()) {
            return;
        }
        const auto &equity_curve = backtest_data.result.equity_curve;

        // 计算总收益率
        double initial = backtest_data.account.initial_capital;
        double final   = equity_curve.empty() ? initial : equity_curve.back();
        if (initial == 0.0) {
            backtest_data.result.total_return = 0.0;
        } else {
            backtest_data.result.total_return = (final - initial) / initial * 100.0;
        }

        // 计算年化收益率：使用实际记录的交易日数（daily_status）来估算年化，避免用 equity_curve.size() 导致的异常放大
        double annualized           = 0.0;
        double total_return_decimal = 0.0;
        if (initial > 0.0) {
            total_return_decimal = (final / initial) - 1.0;
        }

        // 交易日数：以每日结算记录为准
        size_t trading_days = backtest_data.daily_status.size();

        // 尝试基于实际的日历天数来计算年化（更稳健），fallback 到交易日/252 计法
        double years        = 0.0;
        double elapsed_days = 0.0;

        // 调试日志：打印用于年化计算的中间值，便于排查异常年化收益
        spdlog::debug("calculateResults: initial={}, final={}, trading_days={}, equity_curve_size={}",
                      initial,
                      final,
                      trading_days,
                      equity_curve.size());

        if (trading_days >= 2) {
            // daily_status.timestamp 格式为 "YYYY-MM-DD HH:MM:SS"，取前10位日期部分
            try {
                std::string        first_date_str = backtest_data.daily_status.front().timestamp.substr(0, 10);
                std::string        last_date_str  = backtest_data.daily_status.back().timestamp.substr(0, 10);
                std::tm            tm1{};
                std::tm            tm2{};
                std::istringstream ss1(first_date_str);
                std::istringstream ss2(last_date_str);
                ss1 >> std::get_time(&tm1, "%Y-%m-%d");
                ss2 >> std::get_time(&tm2, "%Y-%m-%d");
                tm1.tm_hour = tm1.tm_min = tm1.tm_sec = 0;
                tm2.tm_hour = tm2.tm_min = tm2.tm_sec = 0;
                std::time_t t1                        = std::mktime(&tm1);
                std::time_t t2                        = std::mktime(&tm2);
                if (t1 != -1 && t2 != -1 && t2 >= t1) {
                    elapsed_days = std::difftime(t2, t1) / 86400.0;
                    years        = elapsed_days / 365.25;
                }
                spdlog::debug("calculateResults: first_date={}, last_date={}, elapsed_days={}, years={}",
                              first_date_str,
                              last_date_str,
                              elapsed_days,
                              years);
            } catch (...) {
                spdlog::warn("calculateResults: 无法解析日期，回退到交易日计数作为年化基准");
                years = (trading_days > 0) ? (static_cast<double>(trading_days) / 252.0) : 0.0;
            }
        } else {
            years = (trading_days > 0) ? (static_cast<double>(trading_days) / 252.0) : 0.0;
        }

        spdlog::debug("calculateResults: total_return_decimal={}, years={} ", total_return_decimal, years);

        if (trading_days <= 1 || initial <= 0.0) {
            // 无法年化（只有1日或更少），将年化收益设为总收益（不年化）以避免夸大
            spdlog::warn("calculateResults: 交易日过少（{}），不进行年化，直接返回总收益率作为年化近似", trading_days);
            annualized = total_return_decimal;
        } else if (years <= 0.0) {
            // 无法计算有效年份（例如日期解析失败），回退到交易日/252的方法
            double fallback_years = static_cast<double>(trading_days) / 252.0;
            spdlog::warn("calculateResults: 无法基于日历天数计算年份，使用交易日退化年数={}进行年化", fallback_years);
            if (fallback_years > 0.0 && std::isfinite(total_return_decimal) && (1.0 + total_return_decimal) > 0.0) {
                annualized = std::pow(1.0 + total_return_decimal, 1.0 / fallback_years) - 1.0;
            } else {
                annualized = 0.0;
            }
        } else if (years < 0.0833333) {  // 小于约1个月 (1/12年)
            // 时间窗口过短，年化会极度放大。为避免误导，保持不年化并输出告警。
            spdlog::warn("calculateResults: 回测时间窗口过短（{:.1f}天），跳过年化以避免夸大结果", elapsed_days);
            annualized = total_return_decimal;
        } else {
            // 正常计算 CAGR
            if (std::isfinite(total_return_decimal) && (1.0 + total_return_decimal) > 0.0) {
                annualized = std::pow(1.0 + total_return_decimal, 1.0 / years) - 1.0;
            } else {
                annualized = 0.0;
            }
        }
        // 转为百分比
        backtest_data.result.annualized_return = annualized * 100.0;

        // 计算夏普比率
        std::vector<double> daily_returns;
        daily_returns.reserve((equity_curve.size() > 0) ? (equity_curve.size() - 1) : 0);
        for (size_t i = 1; i < equity_curve.size(); ++i) {
            double prev = equity_curve[i - 1];
            if (prev == 0.0) {
                daily_returns.push_back(0.0);
            } else {
                double ret = (equity_curve[i] - prev) / prev;
                daily_returns.push_back(ret);
            }
        }

        if (!daily_returns.empty()) {
            double sum         = std::accumulate(daily_returns.begin(), daily_returns.end(), 0.0);
            double mean_return = sum / static_cast<double>(daily_returns.size());
            double sq_sum = std::inner_product(daily_returns.begin(), daily_returns.end(), daily_returns.begin(), 0.0);
            double variance = sq_sum / static_cast<double>(daily_returns.size()) - mean_return * mean_return;
            double stdev    = (variance > 0.0) ? std::sqrt(variance) : 0.0;

            backtest_data.result.annualized_volatility = stdev * std::sqrt(252.0) * 100.0;
            backtest_data.result.sharpe_ratio          = (stdev > 0.0) ? (mean_return / stdev * std::sqrt(252.0)) : 0.0;
        } else {
            backtest_data.result.annualized_volatility = 0.0;
            backtest_data.result.sharpe_ratio          = 0.0;
        }

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

        // // 计算交易统计：按完整回合（round-trip）统计已平仓回合数，支持部分成交（multi-fill）
        // // 思路：对每个品种维护 FIFO 的开仓队列（每次买入产生一个 OpenLot），卖出（平仓）时按 FIFO 抵消开仓，
        // //      仅当一个 OpenLot 被完全抵消（其剩余数量变为0）时，计为一个已完成的 round-trip。
        // size_t closed_roundtrips = 0;
        // size_t winning_roundtrips = 0;
        // double total_profit = 0.0;
        // double total_loss = 0.0;

        // 使用独立函数计算 round-trip 统计
        computeRoundTripStats();

        // 计算基于 daily_status 的日级覆盖率
        size_t covered_days = 0;
        for (const auto &d : backtest_data.daily_status) {
            if (!d.positions.empty())
                ++covered_days;
        }
        backtest_data.result.covered_days = covered_days;
        backtest_data.result.coverage_days_rate =
            backtest_data.daily_status.empty()
                ? 0.0
                : (static_cast<double>(covered_days) / static_cast<double>(backtest_data.daily_status.size()) * 100.0);
        // 兼容字段，保持旧名返回日级覆盖率
        backtest_data.result.coverage_rate = backtest_data.result.coverage_days_rate;

        // 统计未平仓头寸
        backtest_data.result.unsettled_positions = position_manager.getPositions().size();
        backtest_data.result.floating_pnl        = 0.0;

        for (const auto &p : position_manager.getPositions()) {
            backtest_data.result.unsettled_symbols.push_back(p.first);
            backtest_data.result.floating_pnl += p.second.unrealized_pnl;
        }

        // 在计算总收益率前添加
        if (!position_manager.getPositions().empty()) {
            spdlog::debug("警告: 回测结束仍有未平仓头寸");
        }
    }

    void BacktestEngine::finalizeBacktest(const std::string &code, const data::KLine &last_bar) {
        // 调试：打印开始结算信息
        spdlog::debug("开始结算未平仓头寸...");
        // 检查剩余持仓
        auto &positions = position_manager.getPositions();
        if (!positions.empty()) {
            // 获取最后交易日收盘价
            double last_price = last_bar.close;

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
                spdlog::debug("[未平仓] {} 方向:{} 数量:{} 成本价:{} 结算价:{} 盈亏:{}",
                              symbol,
                              (position.direction == TradeDirection::LONG ? "多头" : "空头"),
                              position.quantity,
                              position.avg_price,
                              last_price,
                              position.unrealized_pnl);
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
            spdlog::debug("[结算] {} 持仓量:{} 成本价:{} 结算价:{} 盈亏:{}",
                          symbol,
                          position.quantity,
                          position.avg_price,
                          last_price,
                          pnl);
        }

        return total_pnl;
    }

    // 运行回测
    void BacktestEngine::run(const std::string &code) {
        strategy_->reset();
        std::cout << "[BacktestEngine::run] calling updateIndicators for " << code << "\n";
        strategy_->updateIndicators(code);
        std::cout << "[BacktestEngine::run] market_data size=" << strategy_->market_data().size() << " for " << code
                  << "\n";
        auto const &market_data = strategy_->market_data();

        // If user provided a start/end date in BacktestConfig, filter the market_data
        // so the engine only iterates bars within [start_date, end_date].
        std::vector<data::KLine> filtered_market_data;
        if (backtest_data.config.start_time.time_since_epoch().count() != 0 &&
            backtest_data.config.end_time.time_since_epoch().count() != 0) {
            // Convert TimePoint -> meta::Timestamp for date-only comparisons
            meta::Timestamp ts_start(backtest_data.config.start_time);
            meta::Timestamp ts_end(backtest_data.config.end_time);
            std::string         start_date = ts_start.only_date();
            std::string         end_date   = ts_end.only_date();
            for (const auto &b : market_data) {
                if (b.date >= start_date && b.date <= end_date) {
                    filtered_market_data.push_back(b);
                }
            }
        } else {
            filtered_market_data.assign(market_data.begin(), market_data.end());
        }

        // 预分配，避免频繁扩容
        backtest_data.result.equity_curve.clear();
        backtest_data.result.equity_curve.reserve(filtered_market_data.size());
        backtest_data.daily_status.clear();
        backtest_data.daily_status.reserve(filtered_market_data.size());
        size_t bars_with_positions = 0;

        // Process the available (and possibly filtered) market data
        for (size_t i = 0; i < filtered_market_data.size(); ++i) {
            const auto &bar = filtered_market_data[i];

            // 调试：打印当前处理日期
            // spdlog::debug("处理日期: {}", bar.Datetime);
            // 更新持仓市值
            position_manager.updatePositions(code, bar);
            // 生成信号
            TradeDirection signal = strategy_->generateSignal(i);
            if (backtest_data.config.verbose) {
                spdlog::warn("{} {}, signal:{}", bar.datetime, code, magic_enum::enum_name(signal));
                std::cout << "[BacktestEngine::run] " << code << " idx=" << i << " signal=" << static_cast<int>(signal)
                          << "\n";
            }
            if (signal == TradeDirection::HOLD) {
                continue;
            }
            if (signal != TradeDirection::FLAT) {
                // 只有当有持仓时才允许卖出（平仓）
                if (signal == TradeDirection::SHORT) {
                    if (!position_manager.hasPosition(code)) {
                        // 没有持仓，禁止卖出
                        spdlog::warn(
                            "{} {}, signal:{}, 没有持仓，禁止卖出", bar.datetime, code, magic_enum::enum_name(signal));
                        continue;
                    }
                }
                // 执行交易
                Order order = createOrder(code, bar, signal);
                if (backtest_data.config.verbose)
                    spdlog::warn("{} order: {}, message={}", code, order.quantity, order.message);
                if (order.status == OrderStatus::REJECTED || order.quantity == 0) {
                    if (backtest_data.config.verbose)
                        spdlog::warn("{} {}, signal:{}, 订单被拒绝", bar.datetime, code, magic_enum::enum_name(signal));
                    continue;
                }
                backtest_data.orders.push_back(order);

                Trade trade = executeOrder(order);
                backtest_data.trades.push_back(trade);

                // 调试：打印交易详情
                if (backtest_data.config.verbose)
                    spdlog::warn("执行交易: {} {}股 @{}  当前持仓量: {}",
                                 (trade.direction == TradeDirection::LONG ? "买入" : "卖出"),
                                 trade.quantity,
                                 trade.price,
                                 position_manager.getPositionQuantity(code));

                // 处理持仓变化
                position_manager.processTrade(trade);
            }

            // 调试：打印每日持仓状态
            // spdlog::debug("日期结束持仓: {}股", position_manager.getPositionQuantity(code));
            // 记录每日状态
            backtest_data.result.equity_curve.push_back(backtest_data.account.current_capital +
                                                        position_manager.calculateTotalFloatingPnL(bar.close));
            recordDailyStatus(bar);
            // 记录本Bar是否存在任何持仓（用于覆盖率计算）
            if (!position_manager.getPositions().empty()) {
                ++bars_with_positions;
            }
        }
        // spdlog::warn("trade number:{}", backtest_data.trades.size());

        // 添加回测结束处理
        if (!market_data.empty()) {
            finalizeBacktest(code, market_data.back());
        }
        // 日内/Bar级覆盖率（保留供调试/扩展使用）
        if (!filtered_market_data.empty()) {
            // 存储为 bar 级覆盖率（百分比）
            backtest_data.result.coverage_bars_rate = static_cast<double>(bars_with_positions) /
                                                      static_cast<double>(filtered_market_data.size()) * 100.0;
        } else {
            backtest_data.result.coverage_bars_rate = 0.0;
        }

        calculateResults();  // 重新计算绩效
    }

    // 获取回测数据
    const BacktestData &BacktestEngine::getBacktestData() const {
        return backtest_data;
    }

    // Test helper: assign trades directly (used by unit tests)
    void BacktestEngine::setTradesForTest(const std::vector<Trade> &trades) {
        backtest_data.trades = trades;
    }

    // 从 trades 中重建 round-trip 统计（FIFO 开仓匹配），处理部分成交
    void BacktestEngine::computeRoundTripStats() {
    // Delegate to the pure helper to allow reuse in tests
    backtest::computeRoundTripStatsFromTrades(backtest_data.trades, backtest_data.result);
    }

    

    // 打印回测结果
    void BacktestEngine::printResults() const {
        const auto   &result = backtest_data.result;
        io::CSVWriter out("acc.csv");
        out.write_row("amount");
        for (auto const &v : result.equity_curve) {
            out.write_row(v);
        }

        std::cout << "========== 回测结果 ==========\n";
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "总收益率: " << result.total_return << "%\n";
        std::cout << "年化收益率: " << result.annualized_return << "%\n";
        std::cout << "年化波动率: " << result.annualized_volatility << "%\n";
        std::cout << "夏普比率: " << result.sharpe_ratio << "\n";
        std::cout << "最大回撤: " << result.max_drawdown << "%\n";
    // 区分两个概念：
    //  - 总成交事件数: backtest_data.trades.size()（每一笔成交/fill）
    //  - 总回合数(已平仓): result.total_trades（按完整 round-trip 统计的已闭合回合数）
    std::cout << "总成交事件数: " << backtest_data.trades.size() << "\n";
    std::cout << "总回合数(已平仓): " << result.total_trades << "\n";
        std::cout << "胜率: " << result.win_rate << "%\n";
        std::cout << "盈亏比: " << result.profit_loss_ratio << "\n";
        std::cout << "覆盖天数(有持仓的日数): " << result.covered_days << "\n";
        std::cout << "日级覆盖率: " << result.coverage_days_rate << "%\n";
        std::cout << "Bar级覆盖率: " << result.coverage_bars_rate << "%\n";
    }

}  // namespace backtest