#include <quant1x/contrib/data/tdx/chips.h>
#include <quant1x/proto/data.h>
#include <quant1x/config/base.h>
#include <quant1x/config/cache.h>
#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>
#include <tsl/robin_map.h>

namespace config = ::config;
namespace io = ::io;

namespace quant1x::contrib::data::tdx {
    namespace fs = std::filesystem;

    void DataChips::Print(const quant1x::data::meta::Instrument& inst, const std::vector<quant1x::data::meta::Timestamp>& dates) {
        (void)inst;
        (void)dates;
    }

    void DataChips::Update(const quant1x::data::meta::Instrument& inst, const quant1x::data::meta::Timestamp& date) {
        auto code = inst.symbol();
        std::string securityCode = quant1x::data::correct_security_code(code);
        std::string factor_date = date.only_date();
        auto cache_filename = config::get_historical_trade_filename(securityCode, factor_date);
        if (!fs::exists(cache_filename)) {
            spdlog::warn("[DataChips] code={},date={}, 历史成交数据不存在", securityCode, factor_date);
            return;
        }
        io::CSVReader<6, io::trim_chars<' ', '\t'>, io::double_quote_escape<',', '"'>> csvReader(cache_filename);
        csvReader.read_header(io::ignore_extra_column, "time", "price", "vol", "num", "amount", "buyOrSell");
        std::string Time;
        f64 Price = 0;
        f64 Vol = 0;
        i64 Num = 0;
        f64 Amount = 0;
        int BuyOrSell = tick_neutral;
        tsl::robin_map<int32_t, PriceLine> chipDistributionMap;
        int32_t front = 0;
        bool is_first = true;
        while (csvReader.read_row(Time, Price, Vol, Num, Amount, BuyOrSell)) {
            auto price = int32_t(Price * 1000);
            PriceLine pl{};
            pl.price = price;
            if (is_first) {
                switch (BuyOrSell) {
                    case tick_buy:  pl.buy = Vol; break;
                    case tick_sell: pl.sell = Vol; break;
                    default: pl.buy = Vol / 2; pl.sell = Vol - pl.buy; break;
                }
                is_first = false;
            } else {
                if (price > front) {
                    BuyOrSell = tick_buy; pl.buy = Vol;
                } else if (price < front) {
                    BuyOrSell = tick_sell; pl.sell = Vol;
                } else {
                    BuyOrSell = tick_neutral; pl.buy = Vol / 2; pl.sell = Vol - pl.buy;
                }
            }
            auto it = chipDistributionMap.find(pl.price);
            if (it != chipDistributionMap.end()) {
                pl.buy += it->second.buy;
                pl.sell += it->second.sell;
            }
            chipDistributionMap[pl.price] = pl;
            front = price;
        }
        std::vector<PriceLine> values;
        for (const auto& [_, v] : chipDistributionMap) {
            values.push_back(v);
        }
        std::sort(values.begin(), values.end(), [](const PriceLine& a, const PriceLine& b) {
            return a.price < b.price;
        });
        datasets::Chips chips{};
        auto ofn = config::get_chip_distribution_filename(securityCode, factor_date);
        std::ofstream out(ofn, std::ios::binary);
        chips.set_date(factor_date);
        for (const auto& v : values) {
            auto l = chips.add_dist();
            l->set_price(v.price);
            l->set_buy(v.buy);
            l->set_sell(v.sell);
        }
        bool result = chips.SerializeToOstream(&out);
        (void)result;
    }

} // namespace quant1x::contrib::data::tdx
