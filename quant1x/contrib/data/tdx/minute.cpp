#include "minute.h"
#include <spdlog/spdlog.h>

namespace tdx {

    void DataMinute::Print(const meta::Instrument& inst, const std::vector<meta::Timestamp>& dates) {
        (void)inst;
        (void)dates;
    }

    void DataMinute::Update(const meta::Instrument& inst, const meta::Timestamp& date) {
        (void)date;
        auto code = inst.symbol();
        spdlog::info("[DataMinute] update for {} at {}", code, date.only_date());
    }

} // namespace tdx
