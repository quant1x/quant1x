#include "security.h"
#include "blocks.h"
#include <quant1x/std/time.h>
#include <quant1x/config/cache.h>
#include <quant1x/level1/client.h>
#include <quant1x/std/filepath.h>

namespace instruments {

    std::ostream& operator<<(std::ostream& os, const SecurityInfo& p) {
        os << "SecurityInfo{code: " << p.code
           << ", name: " << p.name
           << ", lotSize: " << p.lotSize
           << ", pricePrecision: " << static_cast<int>(p.pricePrecision)
           << "}";
        return os;
    }

    static inline auto global_security_once = RollingOnce::create("instruments-security", exchange::cron_expr_daily_9am);
    static inline tsl::robin_map<std::string, SecurityInfo> global_security_map = {};
    static inline std::string cache_security_filename = config::get_security_filename();

    void init_securities() {
        spdlog::debug("{}, begin", __FUNCTION__);
        auto cache_security_tp = io::last_modified_time(cache_security_filename);
        auto ec = filepath::check_filepath(cache_security_filename, true);
        ec.clear();
        std::string cache_security_time = exchange::timestamp(cache_security_tp).toString();
        std::string check_time_point = exchange::timestamp::now().pre_market_time().toString();
        auto now = exchange::timestamp::now().toString();
        bool bUpdate = false;
        if (cache_security_tp == 0 || cache_security_time.empty()) {
            spdlog::debug("文件[{}]不存在", cache_security_filename);
            bUpdate = true;
        } else if (now >= check_time_point && cache_security_time < check_time_point) {
            spdlog::debug("文件[{}]过时了", cache_security_filename);
            bUpdate = true;
        }
        if (bUpdate) {
            try {
                auto markets = {exchange::ExchangeId::ShangHai, exchange::ExchangeId::ShenZhen, exchange::ExchangeId::BeiJing};
                std::vector<level1::Security> allSecurity;
                for(auto const & marketId : markets) {
                    std::string prefix = exchange::GetMarketFlag(static_cast<exchange::ExchangeId>(marketId));
                    int start = 0;
                    for(;;){
                        level1::SecurityListRequest reqSecurityList(static_cast<int>(marketId), start, level1::security_list_pre_request_max);
                        level1::SecurityListResponse respSecurityList;
                        auto conn = level1::get_std_conn();
                        auto err = level1::process(conn->socket(), reqSecurityList, respSecurityList);
                        if (err) {
                            throw std::runtime_error(fmt::format("Process error: {}", err.message()));
                        }
                        if (!respSecurityList.List.empty()) {
                            for(int i = 0; i < respSecurityList.Count; ++i) {
                                auto v = &respSecurityList.List[i];
                                v->Code = prefix + v->Code;
                                if(!exchange::checkIndexAndStock(v->Code)) {
                                    continue;
                                }
                                if(exchange::AssertBlockBySecurityCode(&(v->Code))) {
                                    auto blk  = exchange::get_sector_info(v->Code);
                                    if(blk.has_value()) {
                                        v->Name = (*blk).name;
                                    }
                                }
                                allSecurity.emplace_back(*v);
                            }
                        }
                        if (respSecurityList.List.size() < level1::security_list_pre_request_max) {
                            break;
                        }
                        start += level1::security_list_pre_request_max;
                    }
                }
                if (!allSecurity.empty()){
                    io::CSVWriter writer(cache_security_filename);
                    writer.write_row("Code", "VolUnit", "DecimalPoint", "Name", "PreClose");
                    for (auto const &v: allSecurity) {
                        writer.write_row(v.Code, v.VolUnit, v.DecimalPoint, v.Name, v.PreClose);
                    }
                }
            }
            catch (const std::exception& e) {
                spdlog::error("Error: {}", e.what());
            }

        }
        {
            io::CSVReader<4> in(cache_security_filename);
            in.read_header(io::ignore_extra_column, "Code", "VolUnit", "DecimalPoint", "Name");
            std::string code;
            uint16_t lotSize = 0;
            uint8_t pricePrecision = 0;
            std::string name;
            global_security_map.clear();
            while (in.read_row(code, lotSize, pricePrecision, name)) {
                auto v = SecurityInfo{code, name, lotSize, pricePrecision};
                global_security_map.insert_or_assign(code, v);
            }
        }
        spdlog::debug("{}, end", __FUNCTION__);
    }

    std::optional<SecurityInfo> get_security_info(const std::string &code) {
        global_security_once->Do(init_securities);
        auto securityCode = exchange::CorrectSecurityCode(code);
        auto it = global_security_map.find(securityCode);
        if (it != global_security_map.end()) {
            return it->second;
        } else {
            return std::nullopt;
        }
    }

    namespace {
        constexpr double HighLimit = 0.20;
        constexpr double NormalLimit = 0.10;
        constexpr double BeijingLimit = 0.30;
    }

    bool starts_with(std::string_view str, std::string_view prefix) {
        return str.size() >= prefix.size() &&
               str.substr(0, prefix.size()) == prefix;
    }

    double get_up_limit_rate(const std::string& security_code) {
        auto [mid, mcode, symbol] = exchange::DetectMarket(security_code);

        if (mcode == exchange::ExchangeBJSE.String()) {
            return BeijingLimit;
        }

        std::string_view symbol_view = symbol;

        static constexpr std::array<std::string_view, 2> kHighLimitPrefixes = {"30", "68"};
        for (const auto& prefix : kHighLimitPrefixes) {
            if (starts_with(symbol_view, prefix)) {
                return HighLimit;
            }
        }

        return NormalLimit;
    }

    double calc_limit_up_price(const std::string& security_code, double prev_close) {
        double rate = get_up_limit_rate(security_code);
        auto price = numeric::decimal(prev_close);
        auto result = price * (1.0 + rate);
        return numeric::decimal(result);
    }

} // namespace instruments
