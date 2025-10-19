#include <quant1x/exchange/security.h>
#include <quant1x/std/time.h>
#include <quant1x/runtime/config.h>
#include <quant1x/exchange/blocks.h>
#include <quant1x/level1/client.h>

namespace exchange {

    // 重载<<运算符
    std::ostream& operator<<(std::ostream& os, const SecurityInfo& p) {
        os << "SecurityInfo{code: " << p.code
           << ", name: " << p.name
           << ", lotSize: " << p.lotSize
           << ", pricePrecision: " << static_cast<int>(p.pricePrecision)
           << "}";
        return os;
    }

    static inline auto global_security_once = RollingOnce::create("exchange-security", cron_expr_daily_9am);
    static inline tsl::robin_map<std::string, SecurityInfo> global_security_map = {};
    static inline std::string cache_security_filename = config::get_security_filename();

    void init_securities() {
        spdlog::debug("{}, begin", __FUNCTION__);
        auto cache_security_tp = io::last_modified_time(cache_security_filename);
        auto ec = util::check_filepath(cache_security_filename, true);
        ec.clear();
        std::string cache_security_time = exchange::timestamp(cache_security_tp).toString();
        std::string check_time_point = exchange::timestamp::now().pre_market_time().toString();
        auto now = exchange::timestamp::now().toString();
        bool bUpdate = false;
        if (cache_security_tp == 0 || cache_security_time.empty()) {
            spdlog::debug("文件[{}]不存在", cache_security_filename);
            bUpdate = true;
        } else if (now >= check_time_point && cache_security_time < check_time_point) {
            // 文件时间过了时间检查点, 且缓存文件时间比检查点还要早, 则需要同步缓存文件
            spdlog::debug("文件[{}]过时了", cache_security_filename);
            bUpdate = true;
        }
        //bUpdate = true;
        if (bUpdate) {
            // 同步证券信息文件
            try {
                // 开始业务处理
                auto markets = {exchange::MarketType::ShangHai, exchange::MarketType::ShenZhen};
                // 1. 获取证券列表
                std::vector<level1::Security> allSecurity;
                for(auto const & marketId : markets) {
                    std::string prefix = exchange::GetMarketFlag(marketId);
                    int start = 0;
                    for(;;){
                        level1::SecurityListRequest reqSecurityList((int) marketId, start);
                        level1::SecurityListResponse respSecurityList;
                        auto conn = level1::client();
                        auto err = level1::process(conn->socket(), reqSecurityList, respSecurityList);
                        if (err) {
                            throw std::runtime_error(fmt::format("Process error: {}", err.message()));
                        }
                        if (!respSecurityList.List.empty()) {
                            //allSecurity.insert(allSecurity.end(), respSecurityList.List.begin(), respSecurityList.List.end());
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
                        if (respSecurityList.List.size() < level1::security_list_max) {
                            break;
                        }
                        start += level1::security_list_max;
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
            // 其它情况加载缓存
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
            return it->second; // 返回指针
        } else {
            return std::nullopt; // 未找到返回空指针
        }
    }

    namespace {
        constexpr double HighLimit = 0.20;
        constexpr double NormalLimit = 0.10;
        constexpr double BeijingLimit = 0.30;
    }

    // 工具函数：检查字符串是否以指定前缀开头
    bool starts_with(std::string_view str, std::string_view prefix) {
        return str.size() >= prefix.size() &&
               str.substr(0, prefix.size()) == prefix;
    }

    /**
     * @brief 获取证券的涨停板比率（例如：0.10 表示 10%）
     * @param security_code 证券代码（如 "600000"）
     * @return 涨停幅度（如主板返回 0.10，创业板返回 0.20）
     */
    double get_up_limit_rate(const std::string& security_code) {
        auto [mid, mcode, symbol] = DetectMarket(security_code);

        if (mcode == market_beijing) {
            return BeijingLimit; // 北交所特殊处理
        }

        std::string_view symbol_view = symbol;

        // 判断是否是创业板/科创板
        static constexpr std::array<std::string_view, 2> kHighLimitPrefixes = {"30", "68"};
        for (const auto& prefix : kHighLimitPrefixes) {
            if (starts_with(symbol_view, prefix)) {
                return HighLimit; // 高涨跌幅限制
            }
        }

        return NormalLimit; // 默认涨幅限制
    }

    /**
     * @brief 根据昨日收盘价和证券代码，计算涨停价格
     * @param security_code
     * @param prev_close
     * @return
     */
    double calc_limit_up_price(const std::string& security_code, double prev_close) {
        double rate = get_up_limit_rate(security_code);
        auto price = numerics::decimal(prev_close);
        auto result = price * (1.0 + rate);
        return numerics::decimal(result);
    }

} // namespace exchange