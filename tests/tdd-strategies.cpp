#include <test/test.h>
#include <q1x/strategies/strategy.h>
#include <capnp/message.h>
#include <q1x/std/numerics.h>
#include <indicators/dynamic_progress.hpp>
#include <indicators/progress_bar.hpp>
#include <users/no1.h>

TEST_CASE("strategy-no0", "[strategies]") {
    try {
        StrategyManager& manager = StrategyManager::Instance();

        StrategyPtr s1 = std::make_shared<HousNo1Strategy>();
        manager.Register(s1);

        std::cout << "已注册策略:\n" << manager.UsageStrategyList() << std::endl;

        auto strategy = manager.GetStrategy(ModelHousNo1);
        std::cout << strategy->DebugString() << std::endl;

        // 示例调用 Evaluate
        ResultInfo info{};
        strategy->Evaluate("SH600000", info);

    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
    }
}

TEST_CASE("strategy-backtest", "[strategies]") {
    runtime::global_init();
    try {
        StrategyManager& manager = StrategyManager::Instance();
        StrategyPtr s1 = std::make_shared<HousNo1Strategy>();
        manager.Register(s1);

        std::cout << "已注册策略:\n" << manager.UsageStrategyList() << std::endl;

        auto strategy = manager.GetStrategy(ModelHousNo1);
        std::cout << strategy->DebugString() << std::endl;
        auto strategyParameter = config::TraderConfig()->GetStrategyParameterByCode(1);

        auto bt_begin = exchange::timestamp(2024,6,1);
        auto bt_end = exchange::timestamp(2025,6,9);
        auto dates = exchange::date_range(bt_begin, bt_end);
        std::cout << "from:" << dates[0].only_date() << ", to:" << dates[dates.size()-1].only_date() << std::endl;
        // 创建多进度条管理器
        indicators::DynamicProgress<indicators::ProgressBar> bars;

        // 主进度条为适配器
        auto date_count = dates.size();
        indicators::ProgressBar barMain{
            indicators::option::ForegroundColor{indicators::Color::cyan},
            indicators::option::MaxProgress{date_count+0}
        };
        bars.push_back(barMain);
        bars[0].set_progress(0);
        auto all_codes = exchange::GetCodeList();
        auto codeCount = all_codes.size();
        indicators::ProgressBar barCodes(
            indicators::option::ForegroundColor{indicators::Color::yellow},
            indicators::option::PrefixText{": fetching..."},
            indicators::option::MaxProgress{codeCount+0});
        bars.push_back(barCodes);
        tsl::robin_map<std::string, double> account; // 账户, 按天缓存
        std::vector<ResultInfo> results; // 总记录
        tsl::robin_map<std::string, ResultInfo> orders; // 缓存订单
        double total_buys = 0; // 总投入
        double total_returns = 0; // 总盈亏
        size_t total_signal = 0; // 信号总数
        size_t total_win_count = 0; // 统计盈利总数
        size_t limit_count = 0;
        for (size_t idx = 0; idx < date_count; ++idx) {
            auto ts = dates[idx];
            std::string module_name = std::format("{}({}/{})", ts.only_date(), (idx+1), date_count);
            bars[0].set_option(indicators::option::PrefixText{module_name + ""});
            bars[1].set_progress(0);
            bars[1].mark_as_started();
            bars[1].set_option(indicators::option::Completed {false});
            // 示例调用 Evaluate
            strategy->setTimestamp(ts);
            std::vector<ResultInfo> result_date;
            size_t win_count_day = 0;
            //double return_rate_day = 0;
            double returns_day = 0;
            std::atomic<size_t> processed_codes = 0;
            for (auto const &code: all_codes) {
                size_t current = ++processed_codes;
                std::string codePrefix = std::format("{}({}/{})", code, current, codeCount);
                bars[1].set_option(indicators::option::PrefixText{codePrefix + ""});
                bars[1].tick();
                ResultInfo info{};
                strategy->Evaluate(code, info);
                if(info.buy) {
                    // 如果是买入
                    // 先看订单是否存在
                    auto it = orders.find(code);
                    if(it != orders.end()) {
                        // 如果订单存在, 则忽略, 不加仓
                    } else {
                        // 如果订单不存在, 则买入
                        info.fee_buy = trader::EvaluateFeeForBuy(code, trader::BacktestAccountTheoreticalFund, info.fee_buy.Price);
                        // 缓存订单
                        orders.emplace(code, info);
                        ++total_signal;
                        total_buys += info.fee_buy.TotalFee;
                    }
                } else if(info.sell) {
                    // 如果是卖出
                    // 先看订单是否存在
                    auto it = orders.find(code);
                    if(it != orders.end()) {
                        // 如果订单存在, 则卖出
                        auto order = it->second;
                        // 计算卖出后的市值
                        order.fee_sell = trader::EvaluateFeeForSell(code, info.fee_sell.Price, order.fee_buy.Volume);
                        auto profit_and_loss_amount = order.fee_sell.MarketValue - order.fee_buy.TotalFee;
                        returns_day += profit_and_loss_amount;
                        total_returns += profit_and_loss_amount;
                        if(profit_and_loss_amount > 0) {
                            ++win_count_day;
                            ++total_win_count;
                        }
                        // 卖出后, 删除订单缓存
                        orders.erase(code);
                    } else {
                        // 不存在订单, 则忽略
                    }
                } else {
                    // 不买也不买，hold持股, 计算当日浮动盈亏
                    // 先看订单是否存在
                    auto it = orders.find(code);
                    if(it != orders.end()) {
                        // 如果订单存在, 则计算
                        auto order = it->second;
                        // 计算假定卖出后的市值
                        auto fee_sell = trader::EvaluateFeeForSell(code, info.fee_sell.Price, order.fee_buy.Volume);
                        auto profit_and_loss_amount = fee_sell.MarketValue - order.fee_buy.TotalFee;
                        returns_day += profit_and_loss_amount;
                        if(profit_and_loss_amount > 0) {
                            ++win_count_day;
                        }
                        // 卖出后, 删除订单缓存
                        orders.erase(code);
                    }
                }

                results.emplace_back(info);
                limit_count += (info.limit_up ? 1 : 0);
            }
            if(idx+1 == date_count) {
                // 如果是最后一天, 持仓计入总亏盈
                total_returns += returns_day;
                total_win_count+= win_count_day;
            }
            // 一天结束后, 缓存盈亏情况
            {
                account.emplace(ts.only_date(), returns_day);
            }
            bars[1].set_option(indicators::option::PrefixText{module_name + ""});
            bars[1].mark_as_completed();
            bars[0].tick();
        }
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "date: " << dates[dates.size()-1].only_date() + ", count signal: " << total_signal << ", win: " << total_win_count << std::endl;
        std::cout << "       Strategy Win Rate(胜率): " << numerics::ChangeRate(f64(total_signal), f64(total_win_count))*100 << std::endl;
        std::cout << "                  return_rate: " << total_returns << ", date_count:"<< total_buys<< std::endl;
        std::cout << "        Return Rate(平均收益率): " << ((total_returns / total_buys) *100)/total_signal << "%" << std::endl;
        std::cout << "Daily Return Amount(净利润比例): " << (total_returns / total_buys) *100 << "%" << std::endl;
        std::cout << "                     其中涨停板: " << limit_count << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
    }
}

// 使用时间点别名简化代码
using TimePoint = std::chrono::system_clock::time_point;

// 定义订单类型枚举
enum class OrderType {
    MARKET,   // 市价单
    LIMIT,    // 限价单
    STOP      // 止损单
};

// 定义订单状态枚举
enum class OrderStatus {
    PENDING,   // 待执行
    FILLED,    // 已成交
    CANCELLED,  // 已取消
    REJECTED    // 已拒绝
};

// K线/Bar数据结构
struct BarData {
    std::string symbol;   // 交易标的符号
    TimePoint timestamp;  // 时间戳
    double open;          // 开盘价
    double high;          // 最高价
    double low;           // 最低价
    double close;         // 收盘价
    double volume;        // 成交量
    double turnover;      // 成交额(可选)
    double open_interest; // 持仓量(期货适用)
};

// 订单数据结构
struct Order {
    std::string order_id;     // 订单ID
    std::string symbol;       // 交易标的
    OrderType type;           // 订单类型
    TradeDirection direction; // 交易方向
    double price;             // 订单价格(限价单/止损单)
    double quantity;          // 数量
    TimePoint create_time;    // 创建时间
    TimePoint update_time;    // 更新时间
    OrderStatus status;       // 订单状态
    std::string message;      // 附加信息(如拒绝原因)
};

// 成交记录数据结构
struct Trade {
    std::string trade_id;     // 成交ID
    std::string order_id;     // 关联订单ID
    std::string symbol;       // 交易标的
    TradeDirection direction; // 交易方向
    double price;             // 成交价格
    double quantity;          // 成交数量
    double fee;               // 手续费
    TimePoint trade_time;     // 成交时间
};

// 持仓数据结构
struct Position {
    std::string symbol;           // 交易标的
    TradeDirection direction;      // 持仓方向
    double quantity;               // 持仓数量
    double avg_price;              // 平均开仓价格
    double realized_pnl;           // 已实现盈亏
    double unrealized_pnl;         // 未实现盈亏
    TimePoint update_time;         // 最后更新时间
};

// 账户数据结构
struct Account {
    double initial_capital;        // 初始资金
    double current_capital;        // 当前资金
    double available_capital;      // 可用资金
    double margin_used;            // 已用保证金
    double total_pnl;              // 总盈亏
    double realized_pnl;           // 已实现盈亏
    double unrealized_pnl;         // 未实现盈亏
    double sharpe_ratio;           // 夏普比率
    double max_drawdown;           // 最大回撤
    TimePoint update_time;         // 最后更新时间
};

// 回测结果数据结构
struct BacktestResult {
    double  total_return;           // 总收益率
    double  annualized_return;      // 年化收益率
    double  annualized_volatility;  // 年化波动率
    double  sharpe_ratio;           // 夏普比率
    double  sortino_ratio;          // 索提诺比率
    double  max_drawdown;           // 最大回撤
    double  win_rate;               // 胜率
    double  profit_loss_ratio;      // 盈亏比
    int64_t total_trades;           // 总交易次数
    int64_t winning_trades;         // 盈利交易次数
    int64_t losing_trades;          // 亏损交易次数
    double  avg_profit;             // 平均盈利
    double  avg_loss;               // 平均亏损
    std::vector<double> equity_curve; // 资金曲线
};

// 回测配置数据结构
struct BacktestConfig {
    TimePoint start_time;          // 回测开始时间
    TimePoint end_time;            // 回测结束时间
    double initial_capital;        // 初始资金
    std::string data_source;       // 数据源
    std::string strategy_name;     // 策略名称
    double commission_rate;        // 手续费率
    double slippage_rate;          // 滑点率
    bool enable_short_selling;     // 是否允许卖空
    int leverage;                  // 杠杆倍数
    int short_window;             // 短期均线窗口(5日)
    int long_window;              // 长期均线窗口(10日)
};

// 主回测数据结构
struct BacktestData {
    BacktestConfig config;                 // 回测配置
    std::vector<BarData> market_data;      // 市场数据
    std::vector<Order> orders;             // 所有订单
    std::vector<Trade> trades;             // 所有成交
    std::map<std::string, Position> positions; // 持仓记录
    Account account;                       // 账户信息
    BacktestResult result;                 // 回测结果
    std::vector<std::string> logs;         // 日志记录
};

class MovingAverageCrossoverStrategy {
private:
    int short_window;  // 短期均线窗口(5日)
    int long_window;   // 长期均线窗口(10日)
    std::vector<double> short_ma;  // 存储短期均线值
    std::vector<double> long_ma;   // 存储长期均线值
    bool in_position;  // 是否持仓

public:
    MovingAverageCrossoverStrategy(int short_w = 5, int long_w = 10)
        : short_window(short_w), long_window(long_w), in_position(false) {}

    // 计算简单移动平均
    std::vector<double> calculateSMA(const std::vector<double>& prices, int window) {
        std::vector<double> sma;
        if (prices.size() < size_t(window)) return sma;

        for (size_t i = window - 1; i < prices.size(); ++i) {
            double sum = 0.0;
            for (int j = 0; j < window; ++j) {
                sum += prices[i - j];
            }
            sma.push_back(sum / window);
        }
        return sma;
    }

    // 更新均线数据
    void updateIndicators(const std::vector<BarData>& market_data) {
        std::vector<double> closes;
        for (const auto& bar : market_data) {
            closes.push_back(bar.close);
        }

        short_ma = calculateSMA(closes, short_window);
        long_ma = calculateSMA(closes, long_window);
    }

    // 生成交易信号
    TradeDirection generateSignal(size_t current_index) {
        // 确保有足够的均线数据
        if (current_index < size_t(long_window) - 1 ||
            current_index - size_t(long_window) + 1 >= short_ma.size() ||
            current_index - size_t(long_window) + 1 >= long_ma.size()) {
            return TradeDirection::FLAT;
        }

        size_t ma_index = current_index - long_window + 1;
        double current_short_ma = short_ma[ma_index];
        double current_long_ma = long_ma[ma_index];

        // 金叉: 短期均线上穿长期均线
        if (current_short_ma > current_long_ma && !in_position) {
            in_position = true;
            return TradeDirection::LONG;
        }
            // 死叉: 短期均线下穿长期均线
        else if (current_short_ma < current_long_ma && in_position) {
            in_position = false;
            return TradeDirection::FLAT;
        }

        return TradeDirection::FLAT;
    }

    // 重置策略状态(用于新回测)
    void reset() {
        short_ma.clear();
        long_ma.clear();
        in_position = false;
    }
};

class BacktestEngine {
private:
    BacktestData backtest_data;
    MovingAverageCrossoverStrategy strategy;

public:
    BacktestEngine(const BacktestConfig& config) {
        backtest_data.config = config;
    }

    // 加载市场数据
    void loadMarketData(const std::vector<BarData>& market_data) {
        backtest_data.market_data = market_data;
    }

    // 运行回测
    void run() {
        strategy.reset();

        // 计算技术指标
        strategy.updateIndicators(backtest_data.market_data);

        // 初始化账户
        initAccount();

        // 遍历市场数据
        for (size_t i = backtest_data.config.long_window - 1; i < backtest_data.market_data.size(); ++i) {
            const auto& current_bar = backtest_data.market_data[i];

            // 生成交易信号
            TradeDirection signal = strategy.generateSignal(i);

            // 执行交易
            executeTrade(current_bar, signal);

            // 更新账户和持仓
            updatePositions(current_bar);
            updateAccount(current_bar);

            // 记录资金曲线
            recordEquity();
        }

        // 计算回测结果
        calculateResults();
    }

    // 获取回测结果
    const BacktestResult& getResults() const {
        return backtest_data.result;
    }

private:
    // 初始化账户
    void initAccount() {
        backtest_data.account.initial_capital = backtest_data.config.initial_capital;
        backtest_data.account.current_capital = backtest_data.config.initial_capital;
        backtest_data.account.available_capital = backtest_data.config.initial_capital;
        backtest_data.account.margin_used = 0.0;
        backtest_data.account.total_pnl = 0.0;
        backtest_data.account.realized_pnl = 0.0;
        backtest_data.account.unrealized_pnl = 0.0;
    }

    // 执行交易
    void executeTrade(const BarData& bar, TradeDirection direction) {
        if (direction == TradeDirection::FLAT && backtest_data.positions.empty()) {
            return;  // 无持仓且信号为平仓，不执行
        }

        // 创建订单
        Order order;
        order.order_id = generateOrderId();
        order.symbol = "TEST";  // 假设我们交易的是TEST标的
        order.type = OrderType::MARKET;
        order.direction = direction;
        order.price = bar.close * (1.0 + (direction == TradeDirection::LONG ?
                                          backtest_data.config.slippage_rate :
                                          -backtest_data.config.slippage_rate));
        order.quantity = calculatePositionSize(bar.close);
        order.create_time = bar.timestamp;
        order.update_time = bar.timestamp;
        order.status = OrderStatus::PENDING;

        // 模拟订单执行
        order.status = OrderStatus::FILLED;
        backtest_data.orders.push_back(order);

        // 创建成交记录
        Trade trade;
        trade.trade_id = generateTradeId();
        trade.order_id = order.order_id;
        trade.symbol = order.symbol;
        trade.direction = order.direction;
        trade.price = order.price;
        trade.quantity = order.quantity;
        trade.fee = calculateFee(order.price, order.quantity);
        trade.trade_time = bar.timestamp;
        backtest_data.trades.push_back(trade);

        // 更新账户资金(扣除交易成本和费用)
        backtest_data.account.available_capital -= (order.price * order.quantity + trade.fee);

        // 处理持仓
        processPosition(trade);
    }

    // 计算头寸大小
    double calculatePositionSize(double price) {
        // 简单起见，使用固定比例(如账户资金的20%)
        double position_value = backtest_data.account.current_capital * 0.2;
        return position_value / price;
    }

    // 计算手续费
    double calculateFee(double price, double quantity) {
        return price * quantity * backtest_data.config.commission_rate;
    }

    // 处理持仓
    void processPosition(const Trade& trade) {
        auto& positions = backtest_data.positions;
        auto it = positions.find(trade.symbol);

        if (it == positions.end()) {
            // 新持仓
            if (trade.direction != TradeDirection::FLAT) {
                Position pos;
                pos.symbol = trade.symbol;
                pos.direction = trade.direction;
                pos.quantity = trade.quantity;
                pos.avg_price = trade.price;
                pos.realized_pnl = 0.0;
                pos.unrealized_pnl = 0.0;
                pos.update_time = trade.trade_time;
                positions[trade.symbol] = pos;
            }
        } else {
            // 已有持仓
            Position& pos = it->second;

            if (trade.direction == TradeDirection::FLAT) {
                // 平仓
                double pnl = (trade.price - pos.avg_price) * pos.quantity *
                             (pos.direction == TradeDirection::LONG ? 1.0 : -1.0);
                pos.realized_pnl += pnl;
                backtest_data.account.realized_pnl += pnl;
                positions.erase(it);
            } else if (trade.direction == pos.direction) {
                // 加仓
                double total_cost = pos.avg_price * pos.quantity + trade.price * trade.quantity;
                pos.quantity += trade.quantity;
                pos.avg_price = total_cost / pos.quantity;
            } else {
                // 反向交易(减仓或反转)
                // 简化处理: 平掉原有仓位，开新仓位
                double pnl = (trade.price - pos.avg_price) * pos.quantity *
                             (pos.direction == TradeDirection::LONG ? 1.0 : -1.0);
                pos.realized_pnl += pnl;
                backtest_data.account.realized_pnl += pnl;

                pos.direction = trade.direction;
                pos.quantity = trade.quantity;
                pos.avg_price = trade.price;
            }

            pos.update_time = trade.trade_time;
        }
    }

    // 更新持仓市值
    void updatePositions(const BarData& bar) {
        for (auto& pair : backtest_data.positions) {
            Position& pos = pair.second;
            if (pos.symbol == bar.symbol) {
                pos.unrealized_pnl = (bar.close - pos.avg_price) * pos.quantity *
                                     (pos.direction == TradeDirection::LONG ? 1.0 : -1.0);
            }
        }
    }

    // 更新账户信息
    void updateAccount(const BarData& bar) {
        double total_position_value = 0.0;
        double total_unrealized_pnl = 0.0;

        for (const auto& pair : backtest_data.positions) {
            const Position& pos = pair.second;
            total_position_value += pos.avg_price * pos.quantity;
            total_unrealized_pnl += pos.unrealized_pnl;
        }

        backtest_data.account.unrealized_pnl = total_unrealized_pnl;
        backtest_data.account.total_pnl = backtest_data.account.realized_pnl + total_unrealized_pnl;
        backtest_data.account.current_capital = backtest_data.account.initial_capital + backtest_data.account.total_pnl;
        backtest_data.account.available_capital = backtest_data.account.current_capital - total_position_value;
        backtest_data.account.margin_used = total_position_value;
        backtest_data.account.update_time = bar.timestamp;
    }

    // 记录资金曲线
    void recordEquity() {
        backtest_data.result.equity_curve.push_back(backtest_data.account.current_capital);
    }

    // 计算回测结果
    void calculateResults() {
        if (backtest_data.result.equity_curve.empty()) return;

        // 计算总收益率
        double initial = backtest_data.account.initial_capital;
        double final = backtest_data.result.equity_curve.back();
        backtest_data.result.total_return = (final - initial) / initial * 100.0;

        // 计算年化收益率(简化计算)
        // 假设数据是日线，每年252个交易日
        size_t num_days = backtest_data.result.equity_curve.size();
        double years = num_days / 252.0;
        backtest_data.result.annualized_return =
            (pow(1.0 + backtest_data.result.total_return / 100.0, 1.0 / years) - 1.0) * 100.0;

        // 计算夏普比率(简化)
        std::vector<double> daily_returns;
        for (size_t i = 1; i < backtest_data.result.equity_curve.size(); ++i) {
            double ret = (backtest_data.result.equity_curve[i] - backtest_data.result.equity_curve[i-1]) /
                         backtest_data.result.equity_curve[i-1];
            daily_returns.push_back(ret);
        }

        double mean_return = std::accumulate(daily_returns.begin(), daily_returns.end(), 0.0) / daily_returns.size();
        double sq_sum = std::inner_product(daily_returns.begin(), daily_returns.end(),
                                           daily_returns.begin(), 0.0);
        double stdev = std::sqrt(sq_sum / daily_returns.size() - mean_return * mean_return);

        backtest_data.result.annualized_volatility = stdev * sqrt(252) * 100.0;
        backtest_data.result.sharpe_ratio = mean_return / stdev * sqrt(252);

        // 计算最大回撤
        double peak = initial;
        double max_drawdown = 0.0;
        for (double equity : backtest_data.result.equity_curve) {
            if (equity > peak) {
                peak = equity;
            }
            double drawdown = (peak - equity) / peak * 100.0;
            if (drawdown > max_drawdown) {
                max_drawdown = drawdown;
            }
        }
        backtest_data.result.max_drawdown = max_drawdown;

        // 计算交易统计(简化)
        backtest_data.result.total_trades = backtest_data.trades.size();
        // ... 其他统计指标可以类似计算
    }

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
};

void printResults(const BacktestResult& result) {
    std::cout << "========== 回测结果 ==========\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "总收益率: " << result.total_return << "%\n";
    std::cout << "年化收益率: " << result.annualized_return << "%\n";
    std::cout << "年化波动率: " << result.annualized_volatility << "%\n";
    std::cout << "夏普比率: " << result.sharpe_ratio << "\n";
    std::cout << "最大回撤: " << result.max_drawdown << "%\n";
    std::cout << "总交易次数: " << result.total_trades << "\n";
}

TEST_CASE("back-test-v0", "[strategy]") {
    // 1. 准备回测配置
    BacktestConfig config;
    config.initial_capital = 100000.0;  // 初始资金10万
    config.commission_rate = 0.0005;    // 0.05%手续费
    config.slippage_rate = 0.0002;      // 0.02%滑点
    config.short_window = 5;    // 5日均线
    config.long_window = 10;    // 10日均线

    // 设置回测时间范围(示例)
    using namespace std::chrono;
    config.start_time = system_clock::now() - hours(24 * 365);  // 1年前
    config.end_time = system_clock::now();                      // 现在

    // 2. 创建回测引擎
    BacktestEngine engine(config);

    // 3. 加载市场数据(这里应该是从文件或数据库读取，示例中简化为手动创建)
    std::vector<BarData> market_data;
    // 填充market_data...
    // 实际应用中应该从CSV或数据库加载真实数据

    // 4. 运行回测
    engine.loadMarketData(market_data);
    engine.run();

    // 5. 输出结果
    printResults(engine.getResults());
}