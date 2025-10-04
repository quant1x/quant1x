#include "strategy-no0.h"
#include <quant1x/factors/history.h>
#include <quant1x/factors/base.h>
#include <quant1x/formula.h>
#include <quant1x/pandas/dataframe.h>
#include <quant1x/trader/fee.h>
#include "no0.h"

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
    double up_limit = exchange::calc_limit_up_price(snapshot.getSecurityCode(), prev_price);
    if(price == up_limit) {
        return quant1x::make_error_code(0+3, std::format("涨停, 价格{}, 不打板", price));
    }

    return quant1x::make_error_code(0, "no problem");
}

quant1x::error No0Strategy::Filter(const config::StrategyParameter &parameter, const level1::SecurityQuote &snapshot) const {
    // 判断价格
    auto price = snapshot.price;
    auto rule_price = parameter.Rules.Price;
    // 确定价格范围
    if(!rule_price.validate(snapshot.price)) {
        return quant1x::make_error_code(0+2, std::format("价格{}不在范围{}内", price, rule_price.to_string()));
    }
    // 判断是否涨停
    double prev_price = snapshot.lastClose;
    std::string security_code = exchange::GetSecurityCode(static_cast<exchange::MarketType>(snapshot.market), snapshot.code);
    double up_limit = exchange::calc_limit_up_price(security_code, prev_price);
    if(price == up_limit) {
        return quant1x::make_error_code(0+3, std::format("涨停, 价格{}, 不打板", price));
    }

    return quant1x::make_error_code(0, "no problem");
}

void No0Strategy::Evaluate(const SecurityCode &code, ResultInfo &result) const {
    result.strategy_id = this->Code();
    std::string securityCode = exchange::CorrectSecurityCode(code);
    result.code = securityCode;
    //std::cout << "No0Strategy evaluated for: " << securityCode << std::endl;
    auto timestamp = getTimestamp().pre_market_time();
    auto next_trading_day = exchange::next_trading_day(timestamp);
    auto feature_date = timestamp.only_date();
    result.date = feature_date;
    if(!exchange::AssertStockBySecurityCode(securityCode)) {
        return;
    }
    auto klines = factors::checkout_klines(securityCode, next_trading_day.only_date());
    if (klines.size() < factors::KLineMin) {
        return;
    }
    klines = std::vector<datasets::KLine>(klines.begin(), klines.end() -1);

    auto current_price = numerics::decimal(klines[klines.size() -1].Close);
    auto prev_close = numerics::decimal(klines[klines.size() - 2].Close);
    auto limit_up_price = exchange::calc_limit_up_price(securityCode, prev_close);
    if(limit_up_price == current_price) {
        result.limit_up = true;
        return;
    }
    DataFrame df = DataFrame::from_struct_vector(klines);
    
    (void)df;
}

void No0Strategy::updateIndicators(const SecurityCode &code) {
    std::string securityCode = exchange::CorrectSecurityCode(code);
    auto timestamp = getTimestamp().pre_market_time();
    auto next_trading_day = exchange::next_trading_day(timestamp);
    auto feature_date = timestamp.only_date();
    if(!exchange::AssertStockBySecurityCode(securityCode)) {
        return;
    }
    auto klines = factors::checkout_klines(securityCode, next_trading_day.only_date());
    if (klines.size() < factors::KLineMin) {
        return;
    }
    //auto next_close = klines[klines.size() - 1].Close;
    market_data_ = std::vector<datasets::KLine>(klines.begin(), klines.end() -1);
    DataFrame df = DataFrame::from_struct_vector(market_data_);
    (void)df;
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
    std::string securityCode = exchange::CorrectSecurityCode(code);
    result.code = securityCode;
    auto timestamp = exchange::last_trading_day(getTimestamp());
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

void No0Strategy::Evaluate(const SecurityCode &code, ResultInfo &result, const level1::SecurityQuote &snapshot) const {
    result.strategy_id = this->Code();
    std::string securityCode = exchange::CorrectSecurityCode(code);
    result.code = securityCode;
    auto timestamp = exchange::last_trading_day(getTimestamp());
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
