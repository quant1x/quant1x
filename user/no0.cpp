#include <quant1x/encoding/csv.h>
#include <quant1x/formula.h>
#include <quant1x/pandas/dataframe.h>

#include "no0.h"

void DataNo0::Print(const std::string &code, const std::vector<exchange::timestamp> &dates) {
    (void)code;
    (void)dates;
}

void DataNo0::Update(const std::string &code, const exchange::timestamp &date) {
    std::string         feature_date = date.only_date();
    exchange::timestamp ts_cache     = exchange::next_trading_day(date);
    feature.Date                     = ts_cache.only_date();
    feature.Code                     = code;
    auto klines                      = factors::klines_forward_adjusted_to_date(code, feature_date);
    if (klines.size() < factors::KLineMin) {
        spdlog::warn("[DataNo0] code={},date={}, 日线数据不足", code, feature_date);
        return;
    }
    DataFrame df = DataFrame::from_struct_vector(klines);
    // 直接获取列
    auto const            &col_close = df.get<f64>("close");
    const xt::xarray<f64> &CLOSE     = xt::adapt(col_close);

    // 0号策略补充
    auto ma4     = formula::ma(CLOSE, 4);
    feature.ma4  = formula::at(ma4, -1);
    auto ma5     = formula::ma(CLOSE, 5);
    feature.ma5  = formula::at(ma5, -1);
    auto ma9     = formula::ma(CLOSE, 9);
    feature.ma9  = formula::at(ma9, -1);
    auto ma10    = formula::ma(CLOSE, 10);
    feature.ma10 = formula::at(ma10, -1);

    feature.UpdateTime = api::get_timestamp();
    feature.State |= factors::FeatureNo0;
}

void DataNo0::init(const exchange::timestamp &timestamp) {
    (void)timestamp;
}

std::unique_ptr<cache::FeatureAdapter> DataNo0::clone() const {
    return std::make_unique<DataNo0>(*this);
}

std::vector<std::string> DataNo0::headers() const {
    std::vector<std::string> header;
    boost::pfr::for_each_field(No0{}, [&](auto &field, auto idx) {
        (void)field;
        constexpr auto field_name = boost::pfr::get_name<idx, No0>();
        header.emplace_back(field_name);
    });
    return header;
}

std::vector<std::string> DataNo0::values() const {
    std::vector<std::string> row;
    boost::pfr::for_each_field(
        feature, [&](auto &field, auto /*idx*/) { row.emplace_back(encoding::csv::detail::to_csv_string(field)); });
    return row;
}
namespace factors {

    namespace {
        inline std::mutex                       g_factor_no0_mutex{};
        inline tsl::robin_map<std::string, No0> g_factor_no0_map{};
        inline exchange::timestamp              g_factor_no0_date{};

        void check_and_update(const exchange::timestamp &timestamp) {
            std::lock_guard<std::mutex> lock{g_factor_no0_mutex};
            exchange::timestamp         algin_date = timestamp.pre_market_time();
            if (g_factor_no0_map.empty() || g_factor_no0_date != algin_date) {
                g_factor_no0_date   = algin_date;
                auto adapter        = DataNo0();
                auto cache_filename = adapter.Filename(g_factor_no0_date);
                if (!std::filesystem::exists(cache_filename)) {
                    spdlog::error("[no0] cache file[{}], not found", cache_filename);
                    return;
                }
                std::vector<No0> list = encoding::csv::csv_to_slices<No0>(cache_filename);
                for (auto const &v : list) {
                    g_factor_no0_map.insert_or_assign(v.Code, v);
                }
            }
        }
    }  // namespace

    /// 获取指定日期的No0数据
    std::optional<No0> get_no0(const std::string &code, const exchange::timestamp &timestamp) {
        check_and_update(timestamp);
        auto it = g_factor_no0_map.find(code);
        if (it != g_factor_no0_map.end()) {
            return it->second;
        }
        return std::nullopt;
    }

}  // namespace factors
