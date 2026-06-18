#include "strategy-no0.h"
#include <quant1x/contrib/data/tdx/kline.h>
#include <quant1x/factors/history.h>
#include <quant1x/factors/base_compat.h>
#include <quant1x/formula.h>
#include <quant1x/pandas/dataframe.h>
#include <quant1x/trader/fee.h>
#include "no0.h"
#include <iostream>

quant1x::error No0Strategy::Filter(const config::StrategyParameter& parameter, const Snapshot::Reader& snapshot) const {
    // 判断价格
    auto price = snapshot.getPrice();
    auto rule_price = parameter.Rules.Price;
    // 确定价格范围
    if(!rule_price.validate(snapshot.getPrice())) {
        return quant1x::make_error_code(0+2, std::format("价格{}不在范围{}内", price, rule_price.to_string()));
    }
    // 判断是否涨停
    double prev_price = snapshot.getLastClose();
    double up_limit = instruments::calc_limit_up_price(snapshot.getSecurityCode(), prev_price);
    if(price == up_limit) {
        return quant1x::make_error_code(0+3, std::format("涨停, 价格{}, 不打板", price));
    }

    return quant1x::make_error_code(0, "no problem");
}

quant1x::error No0Strategy::Filter(const config::StrategyParameter &parameter, const tdx::SecurityQuote &snapshot) const {
    // 判断价格
    auto price = snapshot.price;
    auto rule_price = parameter.Rules.Price;
    // 确定价格范围
    if(!rule_price.validate(snapshot.price)) {
        return quant1x::make_error_code(0+2, std::format("价格{}不在范围{}内", price, rule_price.to_string()));
    }
    // 判断是否涨停
    double prev_price = snapshot.lastClose;
    std::string security_code = data::correct_security_code(static_cast<meta::ExchangeId>(snapshot.market), snapshot.code);
    double up_limit = instruments::calc_limit_up_price(security_code, prev_price);
    if(price == up_limit) {
        return quant1x::make_error_code(0+3, std::format("涨停, 价格{}, 不打板", price));
    }

    return quant1x::make_error_code(0, "no problem");
}

void No0Strategy::Evaluate(const SecurityCode &code, ResultInfo &result) const {
    result.strategy_id = this->Code();
    std::string securityCode = data::correct_security_code(code);
    result.code = securityCode;
    //std::cout << "No0Strategy evaluated for: " << securityCode << std::endl;
    auto timestamp = getTimestamp().pre_market_time();
    auto feature_date = timestamp.only_date();
    result.date = feature_date;
    if(!data::assert_stock_by_security_code(securityCode)) {
        return;
    }
    auto klines = tdx::checkout_klines(securityCode, feature_date);
    // Log klines count for debugging
    spdlog::warn("[No0Strategy::updateIndicators] {} fetched klines: {} (min required {})", securityCode, klines.size(), factors::KLineMin);
    std::cout << "[No0Strategy::updateIndicators] " << securityCode << " fetched klines: " << klines.size() << "\n";
    if (klines.size() < factors::KLineMin) {
        spdlog::warn("[No0Strategy::updateIndicators] {} 日线数据不足: {} < {}", securityCode, klines.size(), factors::KLineMin);
        return;
    }
    klines = std::vector<data::KLine>(klines.begin(), klines.end() -1);

    auto current_price = numeric::decimal(klines[klines.size() -1].close);
    auto prev_close = numeric::decimal(klines[klines.size() - 2].close);
    auto limit_up_price = instruments::calc_limit_up_price(securityCode, prev_close);
    if(limit_up_price == current_price) {
        result.limit_up = true;
        return;
    }
    DataFrame df = DataFrame::from_struct_vector(klines);
    
    (void)df;
}

void No0Strategy::updateIndicators(const SecurityCode &code) {
    std::string securityCode = data::correct_security_code(code);
    std::cout << "[No0Strategy::updateIndicators] entered for code=" << securityCode << "\n";
    auto timestamp = getTimestamp().pre_market_time();
    auto feature_date = timestamp.only_date();
    if(!data::assert_stock_by_security_code(securityCode)) {
        return;
    }
    auto klines = tdx::checkout_klines(securityCode, feature_date);
    if (klines.size() < factors::KLineMin) {
        return;
    }
    //auto next_close = klines[klines.size() - 1].Close;
    market_data_ = std::vector<data::KLine>(klines.begin(), klines.end() -1);

    // Compute simple moving averages and fill buys_/sells_ for signaling
    size_t n = market_data_.size();
    buys_.assign(n, false);
    sells_.assign(n, false);

    auto close_at = [&](size_t idx)->double { return static_cast<double>(market_data_[idx].close); };

    auto moving_avg = [&](size_t end_idx, size_t period)->double {
        if (end_idx + 1 < period) return 0.0;
        double sum = 0.0;
        for (size_t k = end_idx + 1 - period; k <= end_idx; ++k) sum += close_at(k);
        return sum / static_cast<double>(period);
    };

    // For each bar compute ma5 and ma10 and detect cross (previous ma5 <= ma10 and now ma5 > ma10 -> buy)
    for (size_t i = 0; i < n; ++i) {
        double ma5 = moving_avg(i, 5);
        double ma10 = moving_avg(i, 10);
        double prev_ma5 = (i == 0) ? 0.0 : moving_avg(i - 1, 5);
        double prev_ma10 = (i == 0) ? 0.0 : moving_avg(i - 1, 10);

        // skip invalid (not enough data)
        if (ma5 <= 0.0 || ma10 <= 0.0 || prev_ma5 <= 0.0 || prev_ma10 <= 0.0) continue;

        bool bc1 = prev_ma5 <= prev_ma10;
        bool bc2 = ma5 > ma10;
        if (bc1 && bc2) {
            buys_[i] = true;
        }

        bool sc1 = prev_ma5 >= prev_ma10;
        bool sc2 = ma5 < ma10;
        if (sc1 && sc2) {
            sells_[i] = true;
        }
    }

    DataFrame df = DataFrame::from_struct_vector(market_data_);
    (void)df;
    // Log detected signals for debugging (use warn so it's visible in verbose runs)
    size_t buy_count = std::count(buys_.begin(), buys_.end(), true);
    size_t sell_count = std::count(sells_.begin(), sells_.end(), true);
    // Print first few close prices for inspection
    std::string closes_preview;
    for (size_t i = 0; i < std::min<size_t>(n, 10); ++i) {
        closes_preview += std::to_string(static_cast<double>(market_data_[i].close)) + ", ";
    }
    spdlog::warn("[No0Strategy::updateIndicators] {} bars: {} buys: {} sells: {} closes: {}", securityCode, n, buy_count, sell_count, closes_preview);
    std::cout << "[No0Strategy::updateIndicators] " << securityCode << " bars: " << n << " buys: " << buy_count << " sells: " << sell_count << "\n";
}

TradeDirection No0Strategy::generateSignal(size_t current_index) {
    if(buys_.size()> current_index && buys_[current_index]) {
        return TradeDirection::LONG;
    }
    if(sells_.size() > current_index && sells_[current_index]) {
        return TradeDirection::SHORT;
    }
    return TradeDirection::HOLD;
}

void No0Strategy::reset() {
    buys_.clear();
    sells_.clear();
}

// 增量计算
void No0Strategy::Evaluate(const SecurityCode &code, ResultInfo &result, const Snapshot::Reader &snapshot) const {
    result.strategy_id = this->Code();
    std::string securityCode = data::correct_security_code(code);
    result.code = securityCode;
    auto timestamp = meta::last_trading_day(getTimestamp());
    auto feature_date = timestamp.only_date();
    result.date = feature_date;
    auto history = factors::get_history(code, timestamp);
    if(!history.has_value()) {
        return;
    }
    auto no0 = factors::get_no0(code, timestamp);
    if (!no0.has_value()) {
        return;
    }
    // 增量计算均线
    auto ma5 = (no0->ma4 * 4 + snapshot.getPrice()) / 5;
    auto ma10 = (no0->ma9 * 9 + snapshot.getPrice()) / 10;

    // 买入信号, 5日均线向上突破10日均线
    auto bc1 = no0->ma5 < no0->ma10;
    auto bc2 = ma5 > ma10;
    if (bc1 && bc2) {
        result.buy = true;
    }

    // 卖出信号, 5日均线向下跌破10日均线
    auto sc1 = no0->ma5 > no0->ma10;
    auto sc2 = ma5 < ma10;
    if (sc1 && sc2) {
        result.sell = true;
    }
    result.fee_buy.Price = snapshot.getPrice();
    result.fee_sell.Price = snapshot.getPrice();
}

void No0Strategy::Evaluate(const SecurityCode &code, ResultInfo &result, const tdx::SecurityQuote &snapshot) const {
    result.strategy_id = this->Code();
    std::string securityCode = data::correct_security_code(code);
    result.code = securityCode;
    auto timestamp = meta::last_trading_day(getTimestamp());
    auto feature_date = timestamp.only_date();
    result.date = feature_date;
    auto history = factors::get_history(code, timestamp);
    if(!history.has_value()) {
        return;
    }
    auto no0 = factors::get_no0(code, timestamp);
    if (!no0.has_value()) {
        return;
    }
    // 增量计算均线
    auto ma5 = (no0->ma4 * 4 + snapshot.price) / 5;
    auto ma10 = (no0->ma9 * 9 + snapshot.price) / 10;

    // 买入信号, 5日均线向上突破10日均线
    auto bc1 = no0->ma5 < no0->ma10;
    auto bc2 = ma5 > ma10;
    if (bc1 && bc2) {
        result.buy = true;
    }

    // 卖出信号, 5日均线向下跌破10日均线
    auto sc1 = no0->ma5 > no0->ma10;
    auto sc2 = ma5 < ma10;
    if (sc1 && sc2) {
        result.sell = true;
    }

    result.fee_buy.Price = snapshot.price;
    result.fee_sell.Price = snapshot.price;
}
