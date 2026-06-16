#include <quant1x/data/meta/session.h>
#include <quant1x/data/meta/layout.h>
#include <sstream>
#include <algorithm>
#include <ctime>

namespace meta {

// =====================================================================
// 辅助: Region → UTC 偏移小时数
// =====================================================================
static int region_utc_offset_hours(Region r) {
    switch (r) {
        case Region::CN: return 8;
        case Region::HK: return 8;
        case Region::US: return -5;
        case Region::UK: return 0;
        case Region::EU: return 1;
        case Region::SG: return 8;
        case Region::JP: return 9;
        default:         return 8;
    }
}

// =====================================================================
// TimeRange
// =====================================================================

TimeRange::TimeRange() : status(TS_CLOSED), reg(Region::CN) {}

TimeRange::TimeRange(const std::string& time_range, TimeStatus status, Region reg)
    : status(status), reg(reg)
{
    int zone_offset_hours = region_utc_offset_hours(reg) * -1;

    std::string s = time_range;
    // trim
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();

    // 查找分隔符 ~ 或 -
    size_t sep = s.find('~');
    if (sep == std::string::npos) sep = s.find('-');
    if (sep == std::string::npos) {
        throw std::runtime_error("invalid time range format: " + time_range);
    }

    std::string begin_str = s.substr(0, sep);
    std::string end_str = s.substr(sep + 1);
    // trim
    while (!begin_str.empty() && std::isspace(static_cast<unsigned char>(begin_str.front()))) begin_str.erase(begin_str.begin());
    while (!begin_str.empty() && std::isspace(static_cast<unsigned char>(begin_str.back()))) begin_str.pop_back();
    while (!end_str.empty() && std::isspace(static_cast<unsigned char>(end_str.front()))) end_str.erase(end_str.begin());
    while (!end_str.empty() && std::isspace(static_cast<unsigned char>(end_str.back()))) end_str.pop_back();

    begin = Timestamp::parse_time(begin_str).offset(zone_offset_hours, 0, 0, 0);
    end   = Timestamp::parse_time(end_str).offset(zone_offset_hours, 0, 0, 0);

    if (begin > end) std::swap(begin, end);
}

std::optional<TimeStatus> TimeRange::in_range(const Timestamp& timestamp) const {
    if (begin <= timestamp && timestamp < end) {
        return status;
    }
    return std::nullopt;
}

bool TimeRange::is_trading(const Timestamp& timestamp) const {
    Timestamp ts = timestamp.empty() ? Timestamp::now() : timestamp;
    auto s = in_range(ts);
    if (s.has_value()) {
        return (static_cast<uint16_t>(s.value()) & static_cast<uint16_t>(TS_TRADING)) == static_cast<uint16_t>(TS_TRADING);
    }
    return false;
}

bool TimeRange::is_valid() const {
    return !begin.empty() && !end.empty();
}

bool TimeRange::is_session_pre(const Timestamp& timestamp) const {
    Timestamp ts = timestamp.empty() ? Timestamp::now() : timestamp;
    return ts < begin;
}

bool TimeRange::is_session_reg(const Timestamp& timestamp) const {
    return is_trading(timestamp);
}

bool TimeRange::is_session_post(const Timestamp& timestamp) const {
    Timestamp ts = timestamp.empty() ? Timestamp::now() : timestamp;
    return ts >= end;
}

int TimeRange::get_duration_minutes() const {
    int64_t start_mins = begin.value() / 60000;
    int64_t end_mins = end.value() / 60000;
    if (end_mins > start_mins) {
        return static_cast<int>(end_mins - start_mins);
    }
    return static_cast<int>((24 * 60 - start_mins) + end_mins);
}

int TimeRange::get_elapsed_minutes(const Timestamp& current_time) const {
    Timestamp current = current_time < end ? current_time : end;
    Timestamp start = begin < current ? begin : current;
    int64_t current_mins = current.value() / 60000;
    int64_t start_mins = start.value() / 60000;
    if (current_mins >= start_mins) {
        return static_cast<int>(current_mins - start_mins);
    }
    return 0;
}

// =====================================================================
// TradingSession
// =====================================================================

TradingSession::TradingSession()
    : earliest_start(Timestamp::parse_time("23:59:59"))
    , latest_end(Timestamp::parse_time("00:00:00"))
    , closing_time(Timestamp::parse_time("00:00:00"))
{}

TradingSession::TradingSession(const std::vector<TimeRange>& ranges)
    : TradingSession()
{
    sessions = ranges;
    update_time_bounds();
}

TradingSession::TradingSession(const std::string& time_ranges_str)
    : TradingSession()
{
    std::string s = time_ranges_str;
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();

    std::istringstream stream(s);
    std::string segment;
    while (std::getline(stream, segment, ',')) {
        while (!segment.empty() && std::isspace(static_cast<unsigned char>(segment.front()))) segment.erase(segment.begin());
        while (!segment.empty() && std::isspace(static_cast<unsigned char>(segment.back()))) segment.pop_back();
        if (!segment.empty()) {
            sessions.emplace_back(segment);
        }
    }
    update_time_bounds();
}

void TradingSession::add_session(const TimeRange& range) {
    sessions.push_back(range);
    update_time_bounds();
}

void TradingSession::update_time_bounds() {
    if (sessions.empty()) {
        earliest_start = Timestamp::parse_time("23:59:59");
        latest_end = Timestamp::parse_time("00:00:00");
        closing_time = Timestamp::parse_time("00:00:00");
        return;
    }

    earliest_start = Timestamp::parse_time("23:59:59");
    latest_end = Timestamp::parse_time("00:00:00");
    closing_time = Timestamp::parse_time("00:00:00");

    for (const auto& session : sessions) {
        if (session.begin < earliest_start) earliest_start = session.begin;
        if (session.end > latest_end) {
            latest_end = session.end;
            if (ts_is_open(session.status)) {
                closing_time = session.end;
            }
        }
    }
}

TimeStatus TradingSession::check_status(const Timestamp& timestamp) const {
    Timestamp ts = timestamp.empty() ? Timestamp::now() : timestamp;

    for (const auto& session : sessions) {
        auto status = session.in_range(ts);
        if (status.has_value()) {
            return status.value();
        }
    }

    if (ts < earliest_start) return TS_PRE_MARKET;
    if (ts < latest_end)     return TS_EXCHANGE_HALT_TRADING;
    return TS_CLOSED;
}

bool TradingSession::is_trading(const Timestamp& timestamp) const {
    TimeStatus s = check_status(timestamp);
    return (static_cast<uint16_t>(s) & static_cast<uint16_t>(TS_TRADING)) == static_cast<uint16_t>(TS_TRADING);
}

bool TradingSession::is_valid() const {
    for (const auto& session : sessions) {
        if (!session.is_valid()) return false;
    }
    return true;
}

bool TradingSession::is_trading_not_started(const Timestamp& timestamp) const {
    Timestamp ts = timestamp.empty() ? Timestamp::now() : timestamp;
    return ts < earliest_start;
}

bool TradingSession::is_trading_ended(const Timestamp& timestamp) const {
    Timestamp ts = timestamp.empty() ? Timestamp::now() : timestamp;
    return ts > latest_end;
}

int TradingSession::minutes(const Timestamp& timestamp) const {
    Timestamp ts = timestamp.empty() ? Timestamp::now() : timestamp;
    int total = 0;
    for (const auto& tr : sessions) {
        if (ts_is_open(tr.status)) {
            total += tr.get_elapsed_minutes(ts);
        }
    }
    return total;
}

int TradingSession::get_trading_minutes() const {
    int total = 0;
    for (const auto& tr : sessions) {
        if (ts_is_open(tr.status)) {
            total += tr.get_duration_minutes();
        }
    }
    return total;
}

// =====================================================================
// 市场交易时段工厂函数
// =====================================================================

TradingSession init_cn_session() {
    // 9:15~9:20, 开盘集合竞价, 可撤单
    TimeRange tr1("09:15:00 ~ 09:20:00", TS_AUCTION_ORDER_INPUT_PERIOD);
    // 9:20~9:25, 开盘集合竞价, 不可撤单
    TimeRange tr2("09:20:00 ~ 09:25:00", TS_AUCTION_MATCHING_TO_OPENING);
    // 9:25~9:30, 休市
    TimeRange tr3("09:25:00 ~ 09:30:00", TS_SUSPEND);
    // 9:30~11:30, 连续竞价
    TimeRange tr4("09:30:00 ~ 11:30:00", TS_TRADING);
    // 13:00~14:57, 连续竞价
    TimeRange tr5("13:00:00 ~ 14:57:00", TS_TRADING);
    // 14:57~15:00, 收盘集合竞价
    TimeRange tr6("14:57:00 ~ 15:00:00", static_cast<TimeStatus>(
        static_cast<uint16_t>(TS_AUCTION_MATCHING_TO_CLOSING) |
        static_cast<uint16_t>(PERM_OPEN)));

    return TradingSession(std::vector<TimeRange>{tr1, tr2, tr3, tr4, tr5, tr6});
}

TradingSession init_hk_session() {
    // 9:00~9:15, 输入买卖盘, 可撤单
    TimeRange tr1("09:00:00 ~ 09:15:00", TS_AUCTION_ORDER_INPUT_PERIOD, Region::HK);
    // 9:15~9:20, 不可取消
    TimeRange tr2("09:15:00 ~ 09:20:00", TS_AUCTION_NO_CANCELLATION_PERIOD, Region::HK);
    // 9:20~9:22, 随机对盘
    TimeRange tr3("09:20:00 ~ 09:22:00", TS_AUCTION_MATCHING_TO_OPENING, Region::HK);
    // 9:22~9:30, 暂停
    TimeRange tr4("09:22:00 ~ 09:30:00", TS_SUSPEND, Region::HK);
    // 9:30~12:00, 连续交易
    TimeRange tr5("09:30:00 ~ 12:00:00", TS_CONTINUOUS_TRADING, Region::HK);
    // 12:00~13:00, 午休
    TimeRange tr6("12:00:00 ~ 13:00:00", TS_SUSPEND, Region::HK);
    // 13:00~16:00, 连续交易
    TimeRange tr7("13:00:00 ~ 16:00:00", TS_CONTINUOUS_TRADING, Region::HK);

    // 收盘竞价
    TimeRange tr8("16:00:00 ~ 16:01:00", TS_AUCTION_ORDER_INPUT_PERIOD, Region::HK);
    TimeRange tr9("16:01:00 ~ 16:06:00", TS_AUCTION_ORDER_INPUT_PERIOD, Region::HK);
    TimeRange tr10("16:06:00 ~ 16:08:00", TS_AUCTION_NO_CANCELLATION_PERIOD, Region::HK);
    TimeRange tr11("16:06:00 ~ 16:10:00", TS_AUCTION_MATCHING_TO_CLOSING, Region::HK);

    return TradingSession(std::vector<TimeRange>{tr1, tr2, tr3, tr4, tr5, tr6, tr7, tr8, tr9, tr10, tr11});
}

TradingSession init_us_session() {
    TimeRange tr1("04:00:00 ~ 09:30:00", TS_PRE_MARKET, Region::US);
    TimeRange tr2("09:30:00 ~ 16:00:00", TS_TRADING, Region::US);
    TimeRange tr3("16:00:00 ~ 20:00:00", TS_AFTER_HOURS, Region::US);

    return TradingSession(std::vector<TimeRange>{tr1, tr2, tr3});
}

TradingSession latest_session_by_exchange(Exchange exchange) {
    switch (exchange_region(exchange)) {
        case Region::HK: return init_hk_session();
        case Region::US: return init_us_session();
        default:         return init_cn_session();
    }
}

} // namespace meta
