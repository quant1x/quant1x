#include <quant1x/datasets.h>
#include <quant1x/datasets/xdxr.h>
#include <quant1x/datasets/kline.h>
#include <quant1x/datasets/minute.h>
#include <quant1x/datasets/chips.h>
#include <quant1x/datasets/trans.h>
#include <quant1x/datasets/kline_minute.h>
#include <quant1x/factors/f10.h>
#include <quant1x/factors/history.h>
#include <quant1x/pandas/rule.h>

namespace datasets {

    config::MinuteKLineConfig get_minute_kline_config() {
        config::MinuteKLineConfig config{};
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
        const auto d = pandas::ParseTimeRule(key);
        const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(d);
        config.minutes = minutes.count();
        config.frequency = key;
        config.enabled = value;
        return config;
    }

    void init() {
        // 基础数据
        // 除权除息
        data::Register(std::make_unique<DataXdxr>());
        // 日线 - 未除权
        data::Register(std::make_unique<DataKLineRaw>());
        // 日线 - 除权
        data::Register(std::make_unique<DataKLine>());
        // 分时数据
        data::Register(std::make_unique<DataMinute>());
        // 分笔成交
        data::Register(std::make_unique<DataTrans>());
        // 筹码分布
        data::Register(std::make_unique<DataChips>());
        // 分钟级别K线
        auto const &mkc = get_minute_kline_config();
        if (mkc.enabled) {
            data::Register(std::make_unique<DataMinuteKLine>(mkc));
        }

        // 特征数据
        // F10
        data::Register(std::make_unique<F10Feature>());
        // 通用历史数据
        data::Register(std::make_unique<HistoryFeature>());
    }

} // namespace datasets