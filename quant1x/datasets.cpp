#include <quant1x/datasets.h>
#include <quant1x/contrib/data/tdx/kline.h>
#include <quant1x/contrib/data/tdx/xdxr.h>
#include <quant1x/contrib/data/tdx/kline_raw.h>
#include <quant1x/contrib/data/tdx/kline.h>
#include <quant1x/contrib/data/tdx/minute.h>
#include <quant1x/contrib/data/tdx/chips.h>
#include <quant1x/contrib/data/tdx/trans.h>
#include <quant1x/contrib/data/tdx/kline_minute.h>
#include <quant1x/contrib/data/tdx/f10.h>
#include <quant1x/contrib/data/tdx/history.h>
#include <quant1x/pandas/rule.h>

namespace data {

    quant1x::config::MinuteKLineConfig get_minute_kline_config() {
        quant1x::config::MinuteKLineConfig config{};
        auto const &local_cfg = config::global_config().data.cache.kline;
        if (local_cfg.size() > 1) {
            throw std::runtime_error("kline config size must be exactly one");
        }
        if (local_cfg.empty()) {
            return config;
        }
        const auto minute_kline_config = local_cfg.begin();
        const auto key = minute_kline_config->first;
        const auto value = minute_kline_config->second;
        const auto d = pandas::parse_time_rule(key);
        const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(d);
        config.minutes = minutes.count();
        config.frequency = key;
        config.enabled = value;
        return config;
    }

    void init() {
        // 基础数据
        // 除权除息
        data::Register(std::make_unique<tdx::DataXdxr>());
        // 日线 - 未除权
        data::Register(std::make_unique<tdx::DataKLineRaw>());
        // 日线 - 除权
        data::Register(std::make_unique<tdx::DataKLine>());
        // 分时数据
        data::Register(std::make_unique<tdx::DataMinute>());
        // 分笔成交
        data::Register(std::make_unique<tdx::DataTrans>());
        // 筹码分布
        data::Register(std::make_unique<tdx::DataChips>());
        // 分钟级别K线
        auto const &mkc = get_minute_kline_config();
        if (mkc.enabled) {
            data::Register(std::make_unique<tdx::DataMinuteKLine>());
        }

        // 特征数据
        // F10
        data::Register(std::make_unique<tdx::DataF10>());
        // 通用历史数据
        data::Register(std::make_unique<tdx::HistoryFeature>());
    }

} // namespace data