#include "trans.h"
#include <spdlog/spdlog.h>

namespace tdx {

    void DataTrans::Print(const meta::Instrument& inst, const std::vector<meta::Timestamp>& dates) {
        (void)inst;
        (void)dates;
    }

    void DataTrans::Update(const meta::Instrument& inst, const meta::Timestamp& date) {
        (void)date;
        auto code = inst.symbol();
        spdlog::info("[DataTrans] update for {} at {}", code, date.only_date());
    }

} // namespace tdx
