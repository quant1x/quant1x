#include <cpr/cpr.h>
#include <quant1x/runtime/config.h>
#include <quant1x/exchange/code.h>
#include <quant1x/trader/constants.h>
#include <quant1x/trader/fee.h>
#include <quant1x/trader/trader.h>
#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace trader {

    std::ostream &operator<<(std::ostream &os, const AccountDetail &detail) {
        os << "TotalAsset: " << detail.TotalAsset << " Cash: " << detail.Cash << " MarketValue: " << detail.MarketValue
           << " FrozenCash: " << detail.FrozenCash;
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const PositionDetail &detail) {
        os << "AccountType: " << detail.AccountType << " AccountId: " << detail.AccountId << " StockCode: "
           << detail.StockCode << " Volume: " << detail.Volume << " CanUseVolume: " << detail.CanUseVolume
           << " OpenPrice: " << detail.OpenPrice << " MarketValue: " << detail.MarketValue << " FrozenVolume: "
           << detail.FrozenVolume << " OnRoadVolume: " << detail.OnRoadVolume << " YesterdayVolume: "
           << detail.YesterdayVolume << " AvgPrice: " << detail.AvgPrice;
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const OrderDetail &detail) {
        os << "AccountType: " << detail.AccountType << " AccountId: " << detail.AccountId << " OrderTime: "
           << detail.OrderTime << " StockCode: " << detail.StockCode << " OrderType: " << detail.OrderType << " Price: "
           << detail.Price << " PriceType: " << detail.PriceType << " OrderVolume: " << detail.OrderVolume
           << " OrderId: " << detail.OrderId << " OrderSysid: " << detail.OrderSysid << " TradedPrice: "
           << detail.TradedPrice << " TradedVolume: " << detail.TradedVolume << " OrderStatus: " << detail.OrderStatus
           << " StatusMessage: " << detail.StatusMessage << " StrategyName: " << detail.StrategyName << " OrderRemark: "
           << detail.OrderRemark;
        return os;
    }

    std::string to_string(Direction d) {
        switch (d) {
            case Direction::UNKNOWN:
                return "unknown";
            case Direction::BUY:
                return "buy";
            case Direction::SELL:
                return "sell";
            case Direction::JUNK:
                return "junk";
        }
        return "";
    }

    // 结果
    struct ProxyError {
        int64_t     Status = -1;
        std::string Message;
    };

    // 结果
    struct OrderResult {
        ProxyError error;
        int64_t    OrderId = -1;
    };

    void from_json(const json &j, ProxyError &p) {
        j.at("status").get_to(p.Status);
        j.at("message").get_to(p.Message);
    }

    void from_json(const json &j, OrderResult &o) {
        j.at("status").get_to(o.error.Status);
        j.at("message").get_to(o.error.Message);
        j.at("order_id").get_to(o.OrderId);
    }

    void from_json(const json &j, AccountDetail &a) {
        j.at("total_asset").get_to(a.TotalAsset);
        j.at("cash").get_to(a.Cash);
        j.at("market_value").get_to(a.MarketValue);
        j.at("frozen_cash").get_to(a.FrozenCash);
    }

    void from_json(const json &j, PositionDetail &p) {
        j.at("account_type").get_to(p.AccountType);
        j.at("account_id").get_to(p.AccountId);
        j.at("stock_code").get_to(p.StockCode);
        j.at("volume").get_to(p.Volume);
        j.at("can_use_volume").get_to(p.CanUseVolume);
        j.at("open_price").get_to(p.OpenPrice);
        j.at("market_value").get_to(p.MarketValue);
        j.at("frozen_volume").get_to(p.FrozenVolume);
        j.at("on_road_volume").get_to(p.OnRoadVolume);
        j.at("yesterday_volume").get_to(p.YesterdayVolume);
        j.at("avg_price").get_to(p.AvgPrice);
    }

    void from_json(const json &j, OrderDetail &o) {
        j.at("account_type").get_to(o.AccountType);
        j.at("account_id").get_to(o.AccountId);
        j.at("order_time").get_to(o.OrderTime);
        j.at("stock_code").get_to(o.StockCode);
        j.at("order_type").get_to(o.OrderType);
        j.at("price").get_to(o.Price);
        j.at("price_type").get_to(o.PriceType);
        j.at("order_volume").get_to(o.OrderVolume);
        j.at("order_id").get_to(o.OrderId);
        j.at("order_sysid").get_to(o.OrderSysid);
        j.at("traded_price").get_to(o.TradedPrice);
        j.at("traded_volume").get_to(o.TradedVolume);
        j.at("order_status").get_to(o.OrderStatus);
        j.at("status_msg").get_to(o.StatusMessage);
        j.at("strategy_name").get_to(o.StrategyName);
        j.at("order_remark").get_to(o.OrderRemark);
    }

    // 查询账户信息
    std::optional<AccountDetail> QueryAccount() {
        std::string urlPrefixMiniQmtProxy = config::TraderConfig().get()->ProxyUrl;
        std::string urlPrefixForQuery     = urlPrefixMiniQmtProxy + "/query";  // 查询前缀
        std::string urlAccount            = urlPrefixForQuery + "/asset";      // 查询账户信息
        auto r = cpr::Post(cpr::Url{urlAccount});
        if (r.status_code != 200) {
            spdlog::error("trader: 查询账户异常: HTTP {}, error={}", r.status_code, r.error.message);
            return std::nullopt;
        }

        try {
            json          data = json::parse(r.text);
            AccountDetail detail{};
            data.get_to(detail);
            return detail;
        } catch (const std::exception &e) {
            spdlog::error("trader: 解析json异常: {}", e.what());
            return std::nullopt;
        }
    }

    // 查询持仓
    std::vector<PositionDetail> QueryHolding() {
        std::string urlPrefixMiniQmtProxy = config::TraderConfig().get()->ProxyUrl;
        std::string urlPrefixForQuery     = urlPrefixMiniQmtProxy + "/query";  // 查询前缀
        std::string urlHolding = urlPrefixForQuery + "/holding";  // 查询持仓情况
        auto r = cpr::Post(cpr::Url{urlHolding});
        if (r.status_code != 200) {
            spdlog::error("trader: 查询持仓异常: HTTP {}, error={}", r.status_code, r.error.message);
            return {};
        }

        try {
            json data = json::parse(r.text);
            return data.get<std::vector<PositionDetail>>();
        } catch (const std::exception &e) {
            spdlog::error("trader: 解析json异常: {}", e.what());
            return {};
        }
    }

    // 查询当日委托
    std::vector<OrderDetail> QueryOrders(int64_t order_id) {
        cpr::Payload params{
            {"order_id", std::to_string(order_id)},
        };
        std::string urlPrefixMiniQmtProxy = config::TraderConfig().get()->ProxyUrl;
        std::string urlPrefixForQuery     = urlPrefixMiniQmtProxy + "/query";  // 查询前缀
        std::string urlOrders = urlPrefixForQuery + "/order";  // 查询委托
        auto r = cpr::Post(cpr::Url{urlOrders}, params);
        if (r.status_code != 200) {
            spdlog::error("[trader-order-query] : 查询委托异常: HTTP {}, error={}", r.status_code, r.error.message);
            return {};
        } else {
            spdlog::warn("[trader-order-query] response content=[{}]", r.text);
        }

        try {
            json data = json::parse(r.text);
            return data.get<std::vector<OrderDetail>>();
        } catch (const std::exception &e) {
            spdlog::error("[trader-order-query] : 解析json异常: {}", e.what());
            return {};
        }
    }

    // 下委托订单
    int64_t DirectOrder(Direction          direction,
                       const std::string &strategyName,
                       const std::string &orderRemark,
                       const std::string &securityCode,
                       int                priceType,
                       double             price,
                       int                volume) {
        auto [marketId, marketCode, symbol] = exchange::DetectMarket(securityCode);
        marketCode                          = strings::to_upper(marketCode);

        cpr::Payload params{
            {"direction", to_string(direction)},
            {"code", symbol + "." + marketCode},
            {"price_type", std::to_string(priceType)},
            {"price", std::to_string(price)},
            {"volume", std::to_string(volume)},
            {"strategy", strategyName},
            {"remark", orderRemark},
        };

        spdlog::info("trader-order: direction={}, code={}, price_type={}, price={}, volume={}",
                     to_string(direction),
                     symbol + "." + marketCode,
                     priceType,
                     price,
                     volume);
        std::string urlPrefixMiniQmtProxy = config::TraderConfig().get()->ProxyUrl;
        std::string urlPrefixForTrade = urlPrefixMiniQmtProxy + "/trade";  // 交易前缀
        std::string urlPlaceOrder     = urlPrefixForTrade + "/order";      // 委托
        auto r = cpr::Post(cpr::Url{urlPlaceOrder}, params);
        if (r.status_code != 200) {
            spdlog::error("trader-order: 下单操作异常: HTTP {}, error={}", r.status_code, r.error.message);
            return -1;
        } else {
            spdlog::warn("[trader-order] response content=[{}]", r.text);
        }

        try {
            json        data = json::parse(r.text);
            OrderResult result{};
            data.get_to(result);
            spdlog::info("[trade-order] {}, response: order_id={}", to_string(direction), result.OrderId);
            return result.OrderId;
        } catch (const std::exception &e) {
            spdlog::error("[trader-order] 解析json异常: {}", e.what());
            return -1;
        }
    }

    // 下委托订单
    int64_t PlaceOrder(Direction          direction,
                       const std::string &strategyName,
                       const std::string &orderRemark,
                       const std::string &securityCode,
                       int                priceType,
                       double             price,
                       int                volume) {
        return DirectOrder(direction, strategyName, orderRemark, securityCode, priceType, price, volume);
    }

    // 撤单
    int64_t CancelOrder(int64_t orderId) {
        cpr::Payload params{
            {"order_id", std::to_string(orderId)},
        };

        spdlog::info("[trader-cancel] order_id={}", orderId);
        std::string urlPrefixMiniQmtProxy = config::TraderConfig().get()->ProxyUrl;
        std::string urlPrefixForTrade = urlPrefixMiniQmtProxy + "/trade";  // 交易前缀
        std::string urlCancelOrder    = urlPrefixForTrade + "/cancel";     // 撤单

        auto r = cpr::Post(cpr::Url{urlCancelOrder}, params);
        if (r.status_code != 200) {
            spdlog::error("trader-cancel: 撤单操作异常: HTTP {}, error={}", r.status_code, r.error.message);
            return -1;
        } else {
            spdlog::warn("[trader-cancel] response content=[{}]", r.text);
        }

        try {
            json        data = json::parse(r.text);
            OrderResult result{};
            data.get_to(result);
            spdlog::info("[trader-cancel] order_id={}, response: status={}", orderId, result.error.Status);
            return result.OrderId;
        } catch (const std::exception &e) {
            spdlog::error("[trader-cancel] 解析json异常: {}", e.what());
            return -1;
        }
    }

}  // namespace trader