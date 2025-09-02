#include <quant1x/backtest/backtest.h>

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