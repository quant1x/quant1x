#pragma once
#ifndef QUANT1X_EXCHANGE_SESSION_H
#define QUANT1X_EXCHANGE_SESSION_H 1

#include "calendar.h"
#include "timestamp.h"
#include <quant1x/std/api.h>
#include <algorithm> // for std::min_element, std::max_element
#include <limits> // for std::numeric_limits
#include <ostream>
#include <magic_enum/magic_enum.hpp>

namespace exchange {

    constexpr static const char * const _layout_session = "{:%H:%M:%S}";

    // ======================================================================
    // 状态掩码标志（bitmask flags）
    // ======================================================================

    constexpr uint8_t MaskClosed      = 0x00; ///< 无任何状态, 收盘, 休市
    constexpr uint8_t MaskActive      = 0x01; ///< 是否活跃（可用于处理订单）
    constexpr uint8_t MaskTrading     = 0x02; ///< 正常连续竞价阶段
    constexpr uint8_t MaskCallAuction = 0x04; ///< 集合竞价阶段
    constexpr uint8_t MaskOrder       = 0x08; ///< 是否可委托
    constexpr uint8_t MaskCancelable  = 0x10; ///< 是否允许撤单
    constexpr uint8_t MaskOpening     = 0x20; ///< 开盘, 集合竞价, 09:15~09:25
    constexpr uint8_t MaskClosing     = 0x40; ///< 收盘, 集合竞价, 14:57~15:00
    constexpr uint8_t MaskHalt        = 0x80; ///< 暂停交易（市场活跃但不能撮合, 熔断或临时停牌）

    // ======================================================================
    // 时间状态枚举（使用掩码组合）
    // ======================================================================

    enum TimeStatus : uint8_t {
        ExchangeClosing               = MaskClosed,                                  ///< 当日收盘（默认状态，不可交易）
        ExchangePreMarket             = MaskActive,                                  ///< 盘前（活跃但未开始交易）
        ExchangeSuspend               = MaskHalt,                                    ///< 休市中（非活跃，不可交易）
        ExchangeContinuousTrading     = MaskActive | MaskOrder | MaskTrading,        ///< 连续竞价（上午/下午，可撤单）
        ExchangeTrading               = ExchangeContinuousTrading,                   ///< 连续竞价, 盘中交易别名
        ExchangeCallAuction           = MaskActive | MaskOrder | MaskCallAuction,    ///< 集合竞价
        ExchangeCallAuctionOpening    = ExchangeCallAuction | MaskOpening,           ///< 早盘集合竞价
        ExchangeCallAuctionOpenPhase1 = ExchangeCallAuctionOpening | MaskCancelable, ///< 9:15~9:20，开盘集合竞价，可撤单
        ExchangeCallAuctionOpenPhase2 = ExchangeCallAuctionOpening,                  ///< 9:20~9:25，开盘集合竞价，不可撤单
        ExchangeCallAuctionClosePhase = ExchangeCallAuction | MaskClosing,           ///< 14:57~15:00，收盘集合竞价，不可撤单
        ExchangeHaltTrading           = MaskActive | MaskHalt,                       ///< 市场活跃但暂停交易（如临时停牌、熔断等）
    };

    // ======================================================================
    // 辅助判断函数
    // ======================================================================

    // 判断当前状态是否为收盘
    inline bool IsMarketClosed(uint8_t status) {
        return status == ExchangeClosing;
    }

    // 判断当前状态是否为休市
    inline bool IsMarketSuspended(uint8_t status) {
        return status == ExchangeSuspend;
    }

    // 判断当前状态是否为暂停交易（如熔断、停牌）
    inline bool IsTradingHalted(uint8_t status) {
        return (status & MaskHalt) != 0;
    }

    // 判断当前状态是否为活跃状态（可以处理订单）
    inline bool IsMarketActive(uint8_t status) {
        return (status & MaskActive) != 0;
    }

    // 判断当前状态是否为连续竞价
    inline bool IsInContinuousTrading(uint8_t status) {
        return (status & (MaskTrading)) != 0;
    }

    // 判断当前状态是否为集合竞价
    inline bool IsInCallAuction(uint8_t status) {
        return (status & MaskCallAuction) != 0;
    }

    // 判断当前是否为早盘集合竞价
    inline bool IsCallAuctionOpenPhase(uint8_t status) {
        return (status & (ExchangeCallAuction | MaskOpening)) == (ExchangeCallAuction | MaskOpening);
    }

    // 判断当前是否为尾盘集合竞价
    inline bool IsCallAuctionClosePhase(uint8_t status) {
        return (status & (ExchangeCallAuction | MaskClosing)) == (ExchangeCallAuction | MaskClosing);
    }

    // 判断当前是否为可撤单状态
    inline bool IsOrderCancelable(uint8_t status) {
        return (status & MaskCancelable) != 0;
    }

    // 判断当前是否为禁止交易状态（收盘、休市、暂停）
    inline bool IsTradingDisabled(uint8_t status) {
        return status == ExchangeClosing || status == ExchangeSuspend || (status & MaskHalt);
    }

    // 交易时段, 左闭右开区间
    class TimeRange {
    private:
        timestamp begin_;   ///< 开始
        timestamp end_;     ///< 结束
        TimeStatus status_; ///< 时段状态
    public:
        TimeRange(timestamp begin, timestamp end, TimeStatus status) : begin_(begin.floor()), end_(end.ceil()), status_(status){}

        // 是否在本交易时段
        std::optional<TimeStatus> in(timestamp ts) const {
            if (begin_ <= ts && ts < end_) {
                return status_;
            }
            return std::nullopt;
        }

        timestamp begin() const { return begin_; }
        timestamp end() const { return end_; }

        std::string toString() const {
            return begin_.toString(_layout_session) + "~" + end_.toString(_layout_session);
        }

        friend std::ostream& operator<<(std::ostream& os, const TimeRange& range) {
            os << range.begin_.toString(_layout_session) << "~" << range.end_.toString(_layout_session);
            return os;
        }

        // 计算分钟数
        int minutes(const exchange::timestamp& timestamp = 0) const;
    };

    // 交易会话
    class TradingSession{
    private:
        std::vector<TimeRange> sessions_;
        timestamp earliest_start_; // 最早开始时间
        timestamp latest_end_;     // 最晚结束时间

        // 更新最早开始时间和最晚结束时间
        void update_time_bounds() {
            if (sessions_.empty()) {
                earliest_start_ = timestamp(std::numeric_limits<int64_t>::max());
                latest_end_ = timestamp(std::numeric_limits<int64_t>::min());
                return;
            }
            earliest_start_ = timestamp(std::numeric_limits<int64_t>::max());
            latest_end_ = timestamp(std::numeric_limits<int64_t>::min());
            for (const auto& session : sessions_) {
                earliest_start_ = std::min(earliest_start_, session.begin());
                latest_end_ = std::max(latest_end_, session.end());
            }
        }
    public:
        // 使用可变参数模板构造函数
        template <typename... Args>
        TradingSession(Args&&... args) {
            (sessions_.emplace_back(std::forward<Args>(args)), ...);
            update_time_bounds();
        }

        // 动态添加交易时段
        void addSession(const TimeRange& range) {
            sessions_.emplace_back(range);
            update_time_bounds();
        }

        // 判断是否在任何交易时段内
        TimeStatus in(timestamp ts) const {
            for (const auto& session : sessions_) {
                auto status = session.in(ts);
                if (status.has_value()) {
                    return status.value();
                }
            }
            // 全天交易开始前
            if (ts < earliest_start_) {
                return ExchangePreMarket;
            }
            // 全天交易结束前, 则会休市
            if (ts < latest_end_) {
                return ExchangeHaltTrading;
            }
            // 不在任何交易时段内, 返回已收盘
            return ExchangeClosing;
        }

        // 判断全天交易是否未开始
        bool is_trading_not_started(const timestamp& ts) const {
            return ts < earliest_start_;
        }

        // 判断全天交易是否已结束
        bool is_trading_ended(const timestamp& ts) const {
            return ts > latest_end_;
        }

        friend std::ostream &operator<<(std::ostream &os, const TradingSession &session) {
            os << "{sessions:[";
            std::vector<std::string> oss;
            oss.reserve(session.sessions_.size());
            for(const auto & v: session.sessions_) {
                oss.emplace_back(v.toString());
            }
            os << strings::join(oss, ",");
            os << "]";
            os << ", earliest:" << session.earliest_start_.toString(_layout_session)
                << ", latest:" << session.latest_end_.toString(_layout_session)
                << "}";
            return os;
        }

        // 计算分钟数
        int minutes(const exchange::timestamp& timestamp = 0) const;
    };

    /**
     * @brief 初始化当日的交易会话时段
     * @return
     */
    inline TradingSession init_session() {
        auto now = timestamp::midnight();
        TimeRange tr1(now.offset(9,15,0,0), now.offset(9,20,0,0), TimeStatus::ExchangeCallAuctionOpenPhase1);
        TimeRange tr2(now.offset(9,20,0,0), now.offset(9,25,0,0), TimeStatus::ExchangeCallAuctionOpenPhase2);
        TimeRange tr3(now.offset(9,25,0,0), now.offset(9,29,0,0), TimeStatus::ExchangeSuspend);
        TimeRange tr4(now.offset(9,30,0,0), now.offset(11,29,0,0), TimeStatus::ExchangeTrading);
        TimeRange tr5(now.offset(13,0,0,0), now.offset(14,56,0,0), TimeStatus::ExchangeTrading);
        TimeRange tr6(now.offset(14,57,0,0), now.offset(15,0,0,0), TimeStatus::ExchangeCallAuctionClosePhase);
        return TradingSession(tr1, tr2, tr3, tr4, tr5, tr6);
    }

    inline auto ts_today_session = runtime::cache1d<TradingSession>("ts_today_session", init_session);

    /**
     * @brief 运行时状态机
     */
    struct RuntimeStatus {
        bool beforeLastTradeDay = false; // 最后交易日前
        bool isHoliday          = false; // 是否节假日休市
        bool beforeInitTime     = false; // 初始化时间前
        bool cacheAfterInitTime = false; // 缓存在初始化时间之后
        bool updateInRealTime   = false; // 是否可以实时更新
        TimeStatus status = TimeStatus::ExchangeClosing;

        friend std::ostream &operator<<(std::ostream &os, const RuntimeStatus &obj) {
            os << "beforeLastTradeDay: " << obj.beforeLastTradeDay << " isHoliday: " << obj.isHoliday
               << " beforeInitTime: " << obj.beforeInitTime << " cacheAfterInitTime: " << obj.cacheAfterInitTime
               << " updateInRealTime: " << obj.updateInRealTime << " status: " << magic_enum::enum_name(obj.status);
            return os;
        }
    };

    /**
     * @brief 检查运行时交易状态
     * @param lastModified
     * @return
     */
    RuntimeStatus check_trading_timestamp(std::optional<timestamp> lastModified = std::nullopt);

    /**
     * @brief 是否能实时更新
     * @param lastModified 检测的时间点
     * @return
     */
    std::tuple<bool, TimeStatus>can_update_in_realtime(std::optional<timestamp> lastModified = std::nullopt);

    /**
     * @brief 是否可以初始化数据
     * @param lastModified 检测的时间点
     * @return
     */
    bool can_initialize(std::optional<timestamp> lastModified = std::nullopt);

} // namespace exchange

#endif //QUANT1X_EXCHANGE_SESSION_H
