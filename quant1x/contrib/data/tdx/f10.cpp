#include "f10.h"
#include <quant1x/factors/f10.h>
#include <boost/pfr.hpp>
#include <quant1x/encoding/csv.h>

namespace quant1x::contrib::data::tdx {

    quant1x::data::Kind DataF10::Kind() const { return factors::FeatureF10; }
    std::string DataF10::Owner() { return quant1x::data::DefaultDataProvider; }
    std::string DataF10::Key() const { return "f10"; }
    std::string DataF10::Name() const { return "F10因子"; }
    std::string DataF10::Usage() const { return "F10"; }

    void DataF10::Print(const quant1x::data::meta::Instrument& inst, const std::vector<quant1x::data::meta::Timestamp>& dates) {
        (void)inst;
        (void)dates;
    }

    void DataF10::Update(const quant1x::data::meta::Instrument& inst, const quant1x::data::meta::Timestamp& date) {
        // 委托给旧 F10Feature 的实现
        factors::F10Feature oldAdapter;
        oldAdapter.Update(inst, date);
        // 注意: 此处无法直接获取 oldAdapter 的 f10 数据
        // 实际运行时, cache::update_with_adapters 会 clone + update + values
        // 这里仅做兼容占位
    }

    std::unique_ptr<quant1x::data::FeatureAdapter> DataF10::clone() const {
        return std::make_unique<DataF10>(*this);
    }

    std::vector<std::string> DataF10::headers() const {
        std::vector<std::string> header;
        boost::pfr::for_each_field(F10{}, [&](auto& field, auto idx) {
            (void)field;
            constexpr auto field_name = boost::pfr::get_name<idx, F10>();
            header.emplace_back(field_name);
        });
        return header;
    }

    std::vector<std::string> DataF10::values() const {
        std::vector<std::string> row;
        boost::pfr::for_each_field(f10, [&](auto& field, auto /*idx*/) {
            row.emplace_back(encoding::csv::detail::to_csv_string(field));
        });
        return row;
    }

    void DataF10::init(const quant1x::data::meta::Timestamp& timestamp) {
        factors::F10Feature oldAdapter;
        oldAdapter.init(timestamp);
    }

} // namespace quant1x::contrib::data::tdx
