#pragma once
#include <string>
#include <vector>
#include <map>

// Lightweight shared config types used by the example and tests.
struct Rules {
    bool sectors_filter = false;
    int sectors_top_n = 0;
    int stock_top_n_in_sector = 0;
    bool gap_down = false;
    std::vector<int> ignore_rule_group;
    std::string capital;
    std::string price;
    std::string open_turn_z;
    std::string open_quantity_ratio;
    std::string volume_ratio;
    bool check_safety_score = false;
};

struct Strategy {
    int id = 0;
    std::string name;
    bool auto_ = false;
    std::string flag;
    std::string time;
    double weight = 0.0;
    int total = 0;
    double price_cage_ratio = 0.0;
    double minimum_price_fluctuation_unit = 0.0;
    double fee_max = 0.0;
    double fee_min = 0.0;
    std::vector<std::string> sectors;
    bool ignore_margin_trading = false;
    int holding_period = 0;
    int sell_strategy = 0;
    double take_profit_ratio = 0.0;
    double stop_loss_ratio = 0.0;
    Rules rules;
};

// Provide an alias array specialization for Strategy so the YAML key "auto" maps
// to the C++ member `auto_`.
namespace encoding { namespace yaml {
    template <typename T>
    struct yaml_field_aliases;
    template <>
    struct yaml_field_aliases<Strategy> {
        static inline const std::vector<const char*> names = {
            "id", "name", "auto", "flag", "time", "weight", "total",
            "price_cage_ratio", "minimum_price_fluctuation_unit", "fee_max", "fee_min",
            "sectors", "ignore_margin_trading", "holding_period", "sell_strategy",
            "take_profit_ratio", "stop_loss_ratio", "rules"
        };
    };
} }

struct Trader {
    std::string account_id;
    std::string proxy_url;
    std::string order_path;
    double stamp_duty_rate_for_buy = 0.0;
    double stamp_duty_rate_for_sell = 0.0;
    double transfer_rate = 0.0;
    double commission_rate = 0.0;
    double commission_min = 0.0;
    double position_ratio = 0.0;
    double keep_cash = 0.0;
    double buy_amount_max = 0.0;
    double buy_amount_min = 0.0;
    std::vector<Strategy> strategies;
    std::string ask_time;
    std::string tick_time;
};

struct CrontabItem {
    bool enable = false;
    std::string trigger;
};

struct Runtime {
    std::map<std::string, CrontabItem> crontab;
};

struct Cache {
    std::map<std::string, bool> kline;
    struct Chips { bool index_enabled = false; } chips;
};

struct Data {
    std::map<std::string, int> concurrency;
    Cache cache;
    struct Trans { std::string begin_date; } trans;
    struct Feature {
        int tendency = 0;
        struct Wave {
            struct Fields { std::string peak; std::string valley; } fields;
            int periods = 0;
        } wave;
    } feature;
    struct Snapshot { int concurrency = 0; } snapshot;
};

struct Config {
    std::string basedir;
    bool debug = false;
    Trader trader;
    Runtime runtime;
    Data data;
};
