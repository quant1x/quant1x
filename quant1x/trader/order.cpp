#include <quant1x/trader/order.h>
#include <quant1x/trader/fee.h>

namespace trader {

    namespace detail {
        const std::shared_ptr<config::TraderParameter>& get_trader_config() {
            static const std::shared_ptr<config::TraderParameter> & config = config::TraderConfig();
            return config;
        }
    } // namespace detail

    std::string account_id() {
        return detail::get_trader_config()->AccountId;
    }

    std::string trader_qmt_order_path() {
        static const std::string qmt_order_path = config::get_qmt_cache_path() + "/" + detail::get_trader_config()->AccountId;
        return qmt_order_path;
    }

    std::string order_flag(Direction dir) {
        switch (dir) {
            case Direction::BUY: return "b";
            case Direction::SELL: return "s";
            default: return "unknown";
        }
    }

} // namespace trader