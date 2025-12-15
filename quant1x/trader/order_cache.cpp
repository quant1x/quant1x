#include <quant1x/trader/order_cache.h>
#include <quant1x/trader/order.h>

#include <quant1x/instruments/markets.h>
#include <quant1x/encoding/csv.h>

namespace trader {

    // 获取订单文件名
    std::string GetOrderFilename(const std::string& date /*= ""*/) {
        std::string tradeDate;
        if (!date.empty()) {
            tradeDate = exchange::timestamp(date).only_date();
        } else {
            tradeDate = exchange::last_trading_day().only_date();
        }

        auto path = std::filesystem::path(trader_qmt_order_path()) / ("orders." + tradeDate);
        return path.lexically_normal().generic_string();
    }

    // 获取指定日期的订单列表
    std::vector<OrderDetail> GetOrderList(const std::string& date) {
        std::string filename = GetOrderFilename(date);
        std::vector<OrderDetail> list = encoding::csv::csv_to_slices<OrderDetail>(filename);

        return list;
    }

    // 获取本地订单日期列表
    std::vector<std::string> GetLocalOrderDates() {
        std::vector<std::string> list;
        std::string pattern_prefix = "orders.";

        for (const auto& entry : std::filesystem::directory_iterator(trader_qmt_order_path())) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.rfind(pattern_prefix, 0) == 0) {
                    std::string datePart = filename.substr(pattern_prefix.size());
                    list.push_back(datePart);
                }
            }
        }

        return list;
    }

} // namespace trader