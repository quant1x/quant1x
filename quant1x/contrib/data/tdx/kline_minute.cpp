#include "kline_minute.h"
#include <spdlog/spdlog.h>

namespace tdx {

    void DataMinuteKLine::Print(const meta::Instrument& inst, const std::vector<meta::Timestamp>& dates) {
        (void)inst;
        (void)dates;
    }

    void DataMinuteKLine::Update(const meta::Instrument& inst, const meta::Timestamp& date) {
        (void)date;
        auto code = inst.symbol();
        spdlog::info("[DataMinuteKLine] update for {} at {}", code, date.only_date());
    }

} // namespace tdx
