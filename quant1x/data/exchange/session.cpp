#include <quant1x/data/exchange/session.h>

namespace exchange {

    int TimeRange::minutes(const exchange::timestamp& timestamp) const {
        i64 seconds = 0;
        bool in_status = status_ == TimeStatus::ExchangeTrading || status_ == TimeStatus::ExchangeCallAuctionClosePhase;
        auto ts_close_phase = end_;
        if(in_status) {
            if (status_ == TimeStatus::ExchangeCallAuctionClosePhase) {
                ts_close_phase = ts_close_phase.floor();
            }
            if (timestamp.value() == 0) {
                seconds = (ts_close_phase - begin_) / exchange::milliseconds_per_second;
            } else if(this->in(timestamp)) {
                seconds = (std::min(ts_close_phase, timestamp) - begin_) / exchange::milliseconds_per_second;
            }
        }
        int minutes = (int(seconds) + 59) / 60;
        spdlog::debug("time range={}, minutes={}", this->toString(), minutes);
        return minutes;
    }

    int TradingSession::minutes(const exchange::timestamp& timestamp) const {
        int minutes = 0;
        for (const auto& session : sessions_) {
            minutes += session.minutes(timestamp);
        }
        return minutes;
    }

    /**
     * @brief 检查运行时交易状态
     * @param lastModified
     * @return
     */
    RuntimeStatus check_trading_timestamp(std::optional<timestamp> lastModified) {
        RuntimeStatus rs = {};

        auto now = timestamp::now();
        auto ts = lastModified.value_or(now);

        timestamp lastDay = exchange::last_trading_day(ts_today_init); // 从日历出来的时间戳, 时分秒必须以9点整划分
        spdlog::debug("ts = {}", ts.toString());
        spdlog::debug("lastDay = {}", lastDay.toString());
        // 1. 时间戳在当日初始化之前, 资源加载前一个交易日数据
        if (ts < lastDay) {
            rs.beforeLastTradeDay = true;
            return rs;
        }

        // 2. 缓存日期和最后一个交易日相同
        //std::string today = timeToString(timestamp, TradingDayDateFormat);
        const timestamp &today = now;
        if (!today.is_same_date(lastDay)) {
            // 节假日
            rs.isHoliday = true;
            return rs;
        }

        // 3. 交易日初始化前检查
        if (ts < ts_today_init.get()) {
            rs.beforeInitTime = true;
            return rs;
        }
        rs.status = TimeStatus::ExchangePreMarket;

        // 4. 缓存是否在初始化后
        rs.cacheAfterInitTime = true;

        // 5. 实时数据前检查
        if (ts_today_session.get().is_trading_not_started(ts)) {
            return rs;
        }

        // 实时更新状态
        rs.updateInRealTime = true;

        // 6. 交易时间段判断
        rs.status = ts_today_session.get().in(ts);
        if (IsTradingDisabled(rs.status)) {
            // 如果是禁止交易, 暂停更新
            rs.updateInRealTime = false;
        }
        return rs;
    }

    /**
     * @brief 是否能实时更新
     * @param lastModified 检测的时间点
     * @return
     */
    std::tuple<bool, TimeStatus>can_update_in_realtime(std::optional<timestamp> lastModified) {
        auto [beforeLastTradeDay, isHoliday, beforeInitTime, cacheAfterInitTime, updateInRealTime, status] = check_trading_timestamp(lastModified);
        return {updateInRealTime, status};
    }

    /**
     * @brief 是否可以初始化数据
     * @param lastModified 检测的时间点
     * @return
     */
    bool can_initialize(std::optional<timestamp> lastModified) {
        auto [beforeLastTradeDay, isHoliday, beforeInitTime, cacheAfterInitTime, x1, x2] = check_trading_timestamp(
            lastModified);
        if (beforeLastTradeDay) {
            return true;
        }
        if (isHoliday) {
            return false;
        }
        if (beforeInitTime) {
            return false;
        }
        return !cacheAfterInitTime;
    }


} // namespace exchange