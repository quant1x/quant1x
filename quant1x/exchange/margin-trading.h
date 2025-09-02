#pragma once
#ifndef QUANT1X_EXCHANGE_MARGIN_TRADING_H
#define QUANT1X_EXCHANGE_MARGIN_TRADING_H 1

#include <vector>
#include <string>

namespace exchange {

    // 获取两融标的列表
    std::vector<std::string> MarginTradingList();

    // 判断是否两融标的
    bool IsMarginTradingTarget(const std::string& code);

} // namespace exchange

#endif //QUANT1X_EXCHANGE_MARGIN_TRADING_H
