#pragma once
#ifndef QUANT1X_TDX_KLINE_RAW_ADAPTER_H
#define QUANT1X_TDX_KLINE_RAW_ADAPTER_H 1

#include <quant1x/data/adapter.h>
#include <quant1x/data/base.h>
#include <quant1x/data/meta/instrument.h>
#include <quant1x/data/schema/bar.h>

namespace quant1x::contrib::data::tdx {

    using namespace quant1x::data;

    /// 从TDX服务器拉取原始K线数据 (对应 Python fetch_kline_raw)
    /// 根据交易所类型自动分发到标准行情 (SecurityBarsContext) 或扩展行情 (InstrumentBars)
    /// @param inst 证券信息
    /// @param start 起始偏移
    /// @param count 请求数量
    /// @param category K线类型 (如 KLineType::DAILY)
    /// @return Bar列表 (domain schema Bar), 失败时返回空
    std::vector<schema::Bar> fetch_kline_raw(const meta::Instrument& inst, int start, int count, u16 category);

    /// 未复权K线RAW数据适配器 (对应 Python DataKLineRaw)
    class DataKLineRaw : public quant1x::data::DataAdapter {
    public:
        quant1x::data::Kind Kind() const override { return quant1x::data::BaseRawDailyKLine; }
        std::string Owner() override { return quant1x::data::DefaultDataProvider; }
        std::string Key() const override { return "day_raw"; }
        std::string Name() const override { return "日K线RAW"; }
        std::string Usage() const override { return "日K线RAW数据适配器"; }

        void Print(const meta::Instrument& inst, const std::vector<meta::Timestamp>& dates = {}) override;
        void Update(const meta::Instrument& inst, const meta::Timestamp& date = meta::Timestamp()) override;
    };

} // namespace quant1x::contrib::data::tdx

#endif // QUANT1X_TDX_KLINE_RAW_ADAPTER_H
