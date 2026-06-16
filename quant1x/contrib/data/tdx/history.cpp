#include "history.h"
#include <quant1x/factors/history.h>
#include <boost/pfr.hpp>
#include <quant1x/encoding/csv.h>

namespace tdx {

    data::Kind HistoryFeature::Kind() const { return factors::FeatureHistory; }
    std::string HistoryFeature::Owner() { return data::DefaultDataProvider; }
    std::string HistoryFeature::Key() const { return "history"; }
    std::string HistoryFeature::Name() const { return "历史数据"; }
    std::string HistoryFeature::Usage() const { return "历史数据"; }

    void HistoryFeature::Print(const meta::Instrument& inst, const std::vector<meta::Timestamp>& dates) {
        (void)inst;
        (void)dates;
    }

    void HistoryFeature::Update(const meta::Instrument& inst, const meta::Timestamp& date) {
        // 委托给旧 factors::HistoryFeature 的实现
        factors::HistoryFeature oldAdapter;
        oldAdapter.Update(inst, date);
    }

    void HistoryFeature::init(const meta::Timestamp& timestamp) {
        (void)timestamp;
    }

    std::unique_ptr<data::FeatureAdapter> HistoryFeature::clone() const {
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

} // namespace tdx
