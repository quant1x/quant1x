// 执行交易流程
// 1. 同步快照
// 2. 输出有信号可交易的全部标的
// 3. 排序, 优先级越高的优先成交
// 4. 计算单一标的最多可买的数量
// 5. 委托下单
// 6. 记录交易日志

#include <quant1x/trader/tracker.h>
#include <quant1x/runtime/config.h>
#include <spdlog/spdlog.h>
#include <quant1x/exchange.h>
#include <indicators/progress_bar.hpp>
#include <quant1x/realtime/snapshot.h>
#include <quant1x/trader/trader.h>
#include <quant1x/trader/order_state.h>
#include <quant1x/trader/account.h>

namespace trader {

    void tracker(void) {
        uint64_t strategy_id = 1; // 默认1号策略
        auto const & traderParameter = config::TraderConfig();
        auto opt_strategy = traderParameter->GetStrategyParameterByCode(strategy_id);
        if(!opt_strategy.has_value()) {
            spdlog::error("[tracker] {}号策略配置无效, 忽略交易", strategy_id);
            return;
        }
        StrategyManager& manager = StrategyManager::Instance();
        auto strategy = manager.GetStrategy(strategy_id);
        if (strategy == nullptr) {
            spdlog::error("[tracker] {}号策略插件不存在, 忽略交易", strategy_id);
            return;
        }

        auto &strategyParameter = opt_strategy.value();
        if(!strategyParameter.Session.IsTrading()) {
            spdlog::warn("[tracker] {}号策略非交易时段[{}], 不交易", strategy_id, strategyParameter.Session.ToString());
            // 不在交易时段, 退出
            return;
        }

        spdlog::warn("[tracker] {}号策略, 交易流程, 开始", strategy_id);
        // 加载快照
        realtime::load_snapshots();
        auto all_codes = exchange::GetCodeList();
        auto codeCount = all_codes.size();
        {
            indicators::ProgressBar bar{
                indicators::option::BarWidth{50},
                indicators::option::ForegroundColor{indicators::Color::cyan},
                indicators::option::Start{"["},
                indicators::option::Fill{"="},
                indicators::option::Lead{">"},
                indicators::option::Remainder{" "},
                indicators::option::End{"]"},
                indicators::option::ShowElapsedTime{true},
                indicators::option::ShowRemainingTime{true},
                indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}},
                indicators::option::ShowPercentage{true},
                indicators::option::ShowSpeed{true},
                indicators::option::MaxProgress{codeCount + 0},
            };
            int processed_codes = 0;
            auto timestamp = exchange::timestamp::now();
            strategy->setTimestamp(timestamp);
            auto date = timestamp.only_date();
            std::vector<ResultInfo> result_buys;
            std::vector<ResultInfo> result_sells;
            auto tp_start = std::chrono::high_resolution_clock::now();
            for (auto const &code: all_codes) {
                size_t current = ++processed_codes;
                std::string codePrefix = std::format("{}({}/{})", code, current, codeCount);
                bar.set_option(indicators::option::PrefixText{codePrefix + ""});
                // 4. 运行回测
                std::string securityCode = exchange::CorrectSecurityCode(code);
                // result.date = feature_date;
                if (exchange::AssertStockBySecurityCode(securityCode)) {
                    auto snapshot = realtime::get_snapshot(securityCode);
                    if(snapshot.has_value()) {
                        auto stock_state = snapshot->state;
                        if(stock_state == level1::TradeState::SUSPEND) {
                            spdlog::warn("[tracker] code={}, 停牌", securityCode);
                        } else if(stock_state == level1::TradeState::DELISTING) {
                            spdlog::warn("[tracker] code={}, 已退市", securityCode);
                        } else if(stock_state == level1::TradeState::IPO) {
                            spdlog::warn("[tracker] code={}, IPO排队上市, 不能在二级市场交易", securityCode);
                        } else {
                            auto ec = strategy->Filter(strategyParameter, *snapshot);
                            if (!ec) {
                                ResultInfo info{};
                                strategy->Evaluate(code, info, snapshot.value());
                                if (info.buy) {
                                    // 买入
                                    result_buys.emplace_back(info);
                                } else if (info.sell) {
                                    // 卖出
                                    result_sells.emplace_back(info);
                                }
                            } else {
                                spdlog::error("[tracker] security_code={}, rule: code={}, message={}", securityCode, ec.value(),
                                              ec.message());
                            }
                            ec.clear();
                        }
                    }
                }
                bar.tick();
                //std::cout << "code: " << code << std::endl;
            }
            //bar.mark_as_completed();
            // 执行买入
            auto strategy_name = (*strategy).QmtStrategyName();
            auto strategy_remark = (*strategy).OrderFlag();
            // 9.2 判断可交易标的数量
            int quotaForTheNumberOfTargets = std::min(strategyParameter.Total, int(result_buys.size()));
            if (quotaForTheNumberOfTargets < 1) {
                spdlog::error("{}[{}]: 可交易标的数为0, 放弃", strategy_name, strategy_id);
            } else {
                // 5. 统计指定交易日的策略已执行买入的标的数量
                int numberOfStrategy = CountStrategyOrders(date, *strategy, Direction::BUY);
                if (numberOfStrategy >= strategyParameter.Total) {
                    spdlog::error("{} {}: 计划买入={}, 已完成={}. ", date, strategy_name, strategyParameter.Total, numberOfStrategy);
                } else {
                    // 9.3 调用接口计算单只标的可用资金量
                    auto singleFundsAvailable = trader::CalculateAvailableFundsForSingleTarget(
                        quotaForTheNumberOfTargets,
                        strategyParameter.Weight,
                        strategyParameter.FeeMax,
                        strategyParameter.FeeMin);
                    if (singleFundsAvailable <= trader::InvalidFee) {
                        spdlog::error("{}[{}]: 可用资金为0, 放弃", strategy_name, strategy_id);
                    } else {
                        // 假定订单顺序没有变化, 跳过一斤执行买入的次数numberOfStrategy
                        for (int i = 0; i < int(result_buys.size()) && numberOfStrategy < quotaForTheNumberOfTargets; ++i) {
                            auto const &info = result_buys[i];
                            auto direction = Direction::BUY;
                            // 检查买入订单状态
                            auto exists = trader::CheckOrderState(date, *strategy, info.code, direction);
                            if (exists) {
                                continue;
                            }
                            // 策略执行交易数+1
                            numberOfStrategy += 1;
                            // 推送执行买入的状态
                            auto written = trader::PushOrderState(date, *strategy, info.code, direction);
                            if (!written) {
                                // 写入失败, 不交易
                                continue;
                            }
                            // 10.5 启用价格笼子的计算方法
                            auto buy_price = trader::calculate_price_cage(strategy_id, direction, info.fee_buy.Price);
                            // 10.6 计算买入费用
                            auto tradeFee = trader::EvaluateFeeForBuy(info.code, singleFundsAvailable, buy_price);
                            if (tradeFee.Volume <= trader::InvalidVolume) {
                                spdlog::error("{}[{}]: {} 可买数量为0, 放弃", strategy_name, strategy_id, info.code);
                                continue;
                            }
                            // 买入
                            int64_t order_id = trader::PlaceOrder(direction, strategy_name, strategy_remark, info.code,
                                                                  PriceType::FIX_PRICE, tradeFee.Price,
                                                                  tradeFee.Volume);
                            spdlog::info("[tracker] order_id={}", order_id);
                        }
                    }
                }
            }
            // 查询持仓
            auto positions = trader::QueryHolding();
            // 缓存持仓
            std::unordered_map<std::string, trader::PositionDetail> mapHolding;
            for(auto &v : positions) {
                if(v.CanUseVolume < 1) {
                    continue;
                }
                std::string security_code = exchange::CorrectSecurityCode(v.StockCode);
                mapHolding.emplace(std::move(security_code), std::move(v));
            }

            // 执行卖出
            for(auto const & info : result_sells) {
                auto direction = Direction::SELL;
//                // 检查买入订单状态
//                auto exists = trader::CheckOrderState(date, *strategy, info.code, direction);
//                if (exists) {
//                    continue;
//                }
                // 如果有持仓且可以卖出
                auto it = mapHolding.find(info.code);
                if(it == mapHolding.end()) {
                    continue;
                }

                int volume = int(it->second.CanUseVolume);
                // 修正卖出价格
                auto sell_price = trader::calculate_price_limit_for_sell(info.fee_sell.Price, strategyParameter.FixedSlippageForSell);
                // 买入
                int64_t order_id = trader::PlaceOrder(direction, strategy_name, strategy_remark, info.code, PriceType::LATEST_PRICE, sell_price, volume);
                spdlog::info("[tracker] order_id={}", order_id);

            }
            auto tp_end = std::chrono::high_resolution_clock::now();
            auto diff = tp_end - tp_start;
            spdlog::info("[tracker] strategy id={}, cross time:{}", strategy_id, util::format_duration_auto(diff));
            spdlog::info("[tracker] buy signal total: {}", result_buys.size());
            for(auto const &v : result_buys) {
                spdlog::warn("[tracker] buy signal: code={}, price={}", v.code, v.fee_buy.Price);
            }
            spdlog::info("[tracker] sell signal total: {}", result_sells.size());
            for(auto const &v : result_sells) {
                spdlog::warn("[tracker] sell signal: code={}, price={}", v.code, v.fee_buy.Price);
            }
        }
        spdlog::warn("[tracker] {}号策略, 交易流程, 结束", strategy_id);
    }

} // namespace trader