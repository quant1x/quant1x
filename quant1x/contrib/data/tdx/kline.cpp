#include "kline.h"
#include <spdlog/spdlog.h>

namespace tdx {

    void DataKLine::Print(const meta::Instrument& inst, const std::vector<meta::Timestamp>& dates) {
        (void)inst;
        (void)dates;
    }

    void DataKLine::Update(const meta::Instrument& inst, const meta::Timestamp& date) {
        (void)date;
        auto code = inst.symbol();
        // 前复权K线: 需要先获取 raw kline + xdxr 数据, 然后应用前复权计算
        // 详细实现在 factors/base.cpp 的 klines_forward_adjusted_to_date 中
        spdlog::info("[DataKLine] update for {} at {}", code, date.only_date());
    }

} // namespace tdx
