#include <quant1x/datasets.h>
#include <quant1x/datasets/xdxr.h>
#include <quant1x/datasets/kline.h>
#include <quant1x/datasets/minute.h>
#include <quant1x/datasets/chips.h>
#include <quant1x/factors/f10.h>
#include <quant1x/factors/history.h>
#include <quant1x/datasets/trans.h>

namespace datasets {

    void init() {
        // 基础数据
        // 除权除息
        cache::Register(std::make_unique<DataXdxr>());
        // 日线 - 未除权
        cache::Register(std::make_unique<DataKLineRaw>());
        // 日线 - 除权
        cache::Register(std::make_unique<DataKLine>());
        // 分时数据
        cache::Register(std::make_unique<DataMinute>());
        // 分笔成交
        cache::Register(std::make_unique<DataTrans>());
        // 筹码分布
        cache::Register(std::make_unique<DataChips>());

        // 特征数据
        // F10
        cache::Register(std::make_unique<F10Feature>());
        // 通用历史数据
        cache::Register(std::make_unique<HistoryFeature>());
    }

} // namespace datasets