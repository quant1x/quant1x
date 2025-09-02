#include <quant1x/datasets/chips.h>
#include <quant1x/datasets/trans.h>
#include <quant1x/proto/data.h>

namespace datasets {
    namespace fs = std::filesystem;

    void DataChips::Print(const std::string &code, const std::vector<exchange::timestamp> &dates) {
        (void)code;
        (void)dates;
    }

    void DataChips::Update(const std::string &code, const exchange::timestamp &date) {
        //auto list = CheckoutTransactionData(code, date, true);
        std::string securityCode = exchange::CorrectSecurityCode(code);
        std::string factor_date = date.only_date();
        auto cache_filename = config::get_historical_trade_filename(securityCode, factor_date);
        if (!fs::exists(cache_filename)) {
            spdlog::warn("[DataChips] code={},date={}, 历史成交数据存不存在", securityCode, factor_date);
            return;
        }
        io::CSVReader<6,io::trim_chars<' ', '\t'>,io::double_quote_escape<',','"'>> csvReader(cache_filename);
        csvReader.read_header(io::ignore_extra_column, "time", "price", "vol", "num", "amount", "buyOrSell");
        std::string Time;  // 时间 hh:mm
        f64 Price = 0;     // 价格
        f64 Vol = 0;       // 成交量, 股数
        i64 Num = 0;       // 历史成交数据中无该字段，但仍保留
        f64 Amount = 0;    // 金额
        int BuyOrSell = level1::tick_neutral;    // 交易方向
        tsl::robin_map<int32_t, PriceLine> chipDistributionMap;
        int32_t front = 0;
        bool is_first = true;
        while (csvReader.read_row(Time, Price, Vol, Num, Amount, BuyOrSell)) {
            auto price = int32_t(Price * 1000);
            PriceLine pl{};
            pl.price = price;
            if (is_first) {
                switch (BuyOrSell) {
                    case level1::tick_buy:
                        pl.buy = Vol;
                        break;
                    case level1::tick_sell:
                        pl.sell = Vol;
                        break;
                    default:
                        pl.buy = Vol/2;
                        pl.sell = Vol - pl.buy;
                        break;
                }
                is_first = false;
            } else {
                if ( price > front) {
                    BuyOrSell = level1::tick_buy;
                    pl.buy = Vol;
                } else if (price < front) {
                    BuyOrSell = level1::tick_sell;
                    pl.sell = Vol;
                } else {
                    BuyOrSell = level1::tick_neutral;
                    pl.buy = Vol/2;
                    pl.sell = Vol - pl.buy;
                }
            }
            auto it = chipDistributionMap.find(pl.price);
            if(it != chipDistributionMap.end()) {
                pl.buy += it->second.buy;
                pl.sell += it->second.sell;
            }
            chipDistributionMap[pl.price] = pl;
            front = price;
        }
//        // 提取所有 key 到 vector
//        std::vector<int32_t> keys;
//        keys.reserve(chipDistributionMap.size());  // 预分配空间优化性能
//        for (const auto& [key, value] : chipDistributionMap) {
//            keys.push_back(key);
//        }
//        std::sort(keys.begin(), keys.end());
        std::vector<PriceLine> values;
        for (const auto& [_,v] : chipDistributionMap) {
            values.push_back(v);
        }
        std::sort(values.begin(), values.end(), [](const PriceLine& a, const PriceLine &b){
            return a.price < b.price;
        });

        datasets::Chips chips{};
        auto ofn = config::get_chip_distribution_filename(securityCode, factor_date);
        std::ofstream out(ofn, std::ios::binary);
        chips.set_date(factor_date);
        for(const auto & v : values) {
            auto l = chips.add_dist();
            l->set_price(v.price);
            l->set_buy(v.buy);
            l->set_sell(v.sell);
        }
        bool result = chips.SerializeToOstream(&out);
        (void)result;
    }

} // namespace datasets