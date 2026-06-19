#include "history.h"
#include <quant1x/factors/history.h>
#include <boost/pfr.hpp>
#include <quant1x/encoding/csv.h>
#include <fmt/format.h>

namespace quant1x::contrib::data::tdx {

    quant1x::data::Kind HistoryFeature::Kind() const { return factors::FeatureHistory; }
    std::string HistoryFeature::Owner() { return quant1x::data::DefaultDataProvider; }
    std::string HistoryFeature::Key() const { return "history"; }
    std::string HistoryFeature::Name() const { return "历史数据"; }
    std::string HistoryFeature::Usage() const { return "历史数据"; }

    void HistoryFeature::Print(const quant1x::data::meta::Instrument& inst, const quant1x::data::meta::Timestamp& date) {
        (void)date;
        auto h = headers();
        auto v = values();
        fmt::print("\n=== {}: {} ===\n", Name(), inst.symbol());
        if (h.empty()) {
            fmt::print("  (no data)\n");
            return;
        }
        size_t max_w = 0;
        for (auto const& s : h) {
            if (s.size() > max_w) max_w = s.size();
        }
        for (size_t i = 0; i < h.size() && i < v.size(); ++i) {
            fmt::print("  {:<{}} : {}\n", h[i], max_w + 2, v[i]);
        }
    }

    void HistoryFeature::Update(const quant1x::data::meta::Instrument& inst, const quant1x::data::meta::Timestamp& date) {
        // 委托给旧 factors::HistoryFeature 的实现
        factors::HistoryFeature oldAdapter;
        oldAdapter.Update(inst, date);
    }

    void HistoryFeature::init(const quant1x::data::meta::Timestamp& timestamp) {
        (void)timestamp;
    }

    std::unique_ptr<quant1x::data::FeatureAdapter> HistoryFeature::clone() const {
        return std::make_unique<HistoryFeature>(*this);
    }

    std::vector<std::string> HistoryFeature::headers() const {
        std::vector<std::string> header;
        boost::pfr::for_each_field(History{}, [&](auto& field, auto idx) {
            (void)field;
            constexpr auto field_name = boost::pfr::get_name<idx, History>();
            header.emplace_back(field_name);
        });
        return header;
    }

    std::vector<std::string> HistoryFeature::values() const {
        std::vector<std::string> row;
        boost::pfr::for_each_field(history, [&](auto& field, auto /*idx*/) {
            row.emplace_back(encoding::csv::detail::to_csv_string(field));
        });
        return row;
    }

} // namespace quant1x::contrib::data::tdx
