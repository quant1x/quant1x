#include <quant1x/trader/holding.h>
#include <string>
#include <vector>
#include <mutex>
#include <algorithm>
#include <numeric>
#include <functional>
#include <quant1x/trader/trader.h>
#include <quant1x/market/instruments.h>
#include <quant1x/trader/order_cache.h>

namespace trader {

    namespace {
        // 全局变量
        static inline std::vector<HoldingPosition> holdingOrders;
        static inline auto holdingOnce = RollingOnce::create("trader-holding", exchange::cron_expr_daily_9am);

        template<typename T, typename Pred>
        std::vector<T> Filter(const std::vector<T> &input, Pred predicate) {
            std::vector<T> result;
            for (const auto &item: input) {
                if (predicate(item)) {
                    result.push_back(item);
                }
            }
            return result;
        }

        // 懒加载持仓个股的持股周期
        void lazyLoadHoldingOrder() {
            const std::string methodName = "lazyLoadHoldingOrder";

            // 1. 获取持仓列表
            std::vector<PositionDetail> positions = QueryHolding();
            if (!positions.empty()) {
                // 过滤掉清仓的个股
                positions = Filter(positions, [](const PositionDetail &detail) {
                    return detail.Volume > 0;
                });
            }

            // 清空缓存
            holdingOrders.clear();

            // 2. 用持仓列表遍历历史订单缓存文件, 补全持仓订单
            std::vector<std::string> dates = GetLocalOrderDates();
            if (dates.empty()) {
                return;
            }

            // 3. 重新评估持仓范围, 有可能存在日期没有成交的可能
            std::string firstDate = dates.front();
            std::string lastTradeDate = exchange::last_trading_day().only_date();
            dates = exchange::get_date_range(firstDate, lastTradeDate);

            // 反转日期切片
            std::reverse(dates.begin(), dates.end());

            // 4. 遍历持仓列表
            for (const auto &position: positions) {
                HoldingPosition holding = position;

                std::string code = position.StockCode;
                i64 volume = position.Volume;

                // 矫正证券代码
                std::string securityCode = exchange::CorrectSecurityCode(code);

                // 历史记录合计买数量
                int tmpTradedVolume = 0;

                // 最早的持股日期
                std::string earlierDate = lastTradeDate;

                // 持股周期
                int holdingPeriod = 0;

                // 从当前日期往前回溯订单
                for (const auto &date: dates) {
                    bool isLastTradeDate = (date == lastTradeDate);

                    // 获取 date 的订单列表
                    std::vector<OrderDetail> orders = GetOrderList(date);
                    if (orders.empty() && isLastTradeDate) {
                        // 如果本地缓存订单列表为空, 且是最后一个交易日, 则从券商获取订单列表
                        orders = QueryOrders();
                    }

                    if (orders.empty()) {
                        continue;
                    }

                    // 过滤出当前股票的订单
                    std::vector<OrderDetail> filteredOrders = Filter(orders, [&](const OrderDetail &detail) {
                        return detail.StockCode == code;
                    });

                    if (filteredOrders.empty()) {
                        continue;
                    }

                    int currentTradedVolume = 0;

                    for (const auto &order: filteredOrders) {
                        if (order.OrderType != STOCK_BUY && order.OrderType != STOCK_SELL) {
                            continue; // 忽略非买入/卖出订单
                        }

                        int plus = 1;
                        if (order.OrderType == STOCK_SELL) {
                            plus = -1;
                        }

                        if (order.OrderStatus == ORDER_PART_SUCC || order.OrderStatus == ORDER_SUCCEEDED) {
                            currentTradedVolume += plus * order.TradedVolume;
                        }
                    }

                    earlierDate = date;
                    tmpTradedVolume += currentTradedVolume;

                    if (tmpTradedVolume == volume) {
                        break; // 合计成交量等于持仓量
                    }
                }

                if (tmpTradedVolume != volume) {
                    spdlog::error("[{}]: 加载({})持仓记录异常, 历史委托记录合并持仓量不一致",
                                  methodName.c_str(), securityCode.c_str());
                }

                // 计算持股周期
                std::vector<exchange::timestamp> dateRanges = exchange::date_range(earlierDate, lastTradeDate);
                holdingPeriod = static_cast<int>(dateRanges.size()) - 1;

                holding.HoldingPeriod = holdingPeriod;
                holdingOrders.push_back(holding);
            }
        }
    }

    // GetHoldingPeriodList 获取持仓周期列表
    std::vector<HoldingPosition> GetHoldingPeriodList() {
        holdingOnce->Do(lazyLoadHoldingOrder);
        return holdingOrders;
    }

} // namespace trader