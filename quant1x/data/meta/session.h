#pragma once
#ifndef QUANT1X_DATA_META_SESSION_H
#define QUANT1X_DATA_META_SESSION_H 1

#include "timestamp.h"
#include "exchange.h"
#include "region.h"
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace meta {

// =====================================================================
// 1. Permission - 全球统一交易状态位掩码
// =====================================================================
enum Permission : uint16_t {
    PERM_NONE               = 0,           ///< 0b00000000

    // 订单操作权限 (Bit 0-3)
    PERM_CANCEL             = 1 << 0,      ///< 0b00000001 - 允许撤单
    PERM_MODIFY             = 1 << 1,      ///< 0b00000010 - 允许改单
    PERM_MARKET             = 1 << 2,      ///< 0b00000100 - 允许市价单
    PERM_LIMIT              = 1 << 3,      ///< 0b00001000 - 允许限价单

    // 撮合机制 (Bit 4)
    PERM_MATCHING           = 1 << 4,      ///< 0b00010000 - 匹配中

    // 成交机制 (Bit 5)
    PERM_FILL               = 1 << 5,      ///< 0b00100000 - 会产生成交记录

    // 统计标志 (Bit 6)
    PERM_OPEN               = 1 << 6,      ///< 0b01000000 - 计入交易分钟数

    // 状态性质 (Bit 7)
    PERM_IS_TEMPORARY       = 1 << 7,      ///< 0b10000000 - 临时状态

    // === 常用组合 ===
    PERM_MATCHING_TRANSACTION = PERM_MATCHING | PERM_FILL,  ///< 撮合成交

    // 连续交易
    PERM_CONTINUOUS_TRADING  = PERM_MARKET | PERM_LIMIT | PERM_CANCEL | PERM_MODIFY |
                                PERM_OPEN | PERM_MATCHING_TRANSACTION,

    // 初始化阶段
    PERM_INITIALIZING       = PERM_IS_TEMPORARY,

    // 盘前
    PERM_PRE_MARKET         = PERM_IS_TEMPORARY | PERM_CANCEL | PERM_LIMIT,

    // 盘后
    PERM_AFTER_HOURS        = PERM_IS_TEMPORARY | PERM_CANCEL | PERM_LIMIT,

    // 集合竞价
    PERM_CALL_AUCTION       = PERM_LIMIT | PERM_MATCHING | PERM_IS_TEMPORARY,
    PERM_CALL_AUCTION_PRE   = PERM_CALL_AUCTION | PERM_CANCEL,
    PERM_CALL_AUCTION_ORDER = PERM_CALL_AUCTION,
    PERM_CALL_AUCTION_FILL  = PERM_CALL_AUCTION | PERM_FILL,

    // 只挂单不成交
    PERM_ACCEPT_ORDER_ONLY  = PERM_LIMIT,

    // 只读
    PERM_READ_ONLY          = PERM_CANCEL,

    // 完全关闭
    PERM_CLOSED             = PERM_NONE,

    // 紧急停牌
    PERM_EMERGENCY_HALT     = PERM_OPEN,

    // 午间休市
    PERM_LUNCH_BREAK        = PERM_ACCEPT_ORDER_ONLY | PERM_IS_TEMPORARY,
};

/// 权限位操作辅助函数
inline bool perm_has(uint16_t perm, Permission bit) {
    return (perm & static_cast<uint16_t>(bit)) != 0;
}
inline bool perm_can_match(uint16_t p)     { return perm_has(p, PERM_MATCHING); }
inline bool perm_can_cancel(uint16_t p)    { return perm_has(p, PERM_CANCEL); }
inline bool perm_can_modify(uint16_t p)    { return perm_has(p, PERM_MODIFY); }
inline bool perm_can_market_order(uint16_t p)  { return perm_has(p, PERM_MARKET); }
inline bool perm_can_limit_order(uint16_t p)   { return perm_has(p, PERM_LIMIT); }
inline bool perm_is_suspended(uint16_t p)      { return !perm_can_match(p); }
inline bool perm_is_continuous_trading(uint16_t p) { return perm_has(p, PERM_OPEN); }

// =====================================================================
// 2. TimeStatus - 交易时间状态枚举 (使用 Permission 掩码组合)
// =====================================================================
enum TimeStatus : uint16_t {
    TS_OPEN                          = PERM_OPEN,
    TS_CLOSED                        = PERM_CLOSED,
    TS_PRE_MARKET                    = PERM_PRE_MARKET,
    TS_AFTER_HOURS                   = PERM_AFTER_HOURS,
    TS_SUSPEND                       = PERM_LUNCH_BREAK,
    TS_CONTINUOUS_TRADING            = PERM_CONTINUOUS_TRADING,
    TS_TRADING                       = TS_CONTINUOUS_TRADING,  ///< 别名
    TS_CALL_AUCTION                  = PERM_CALL_AUCTION,
    TS_AUCTION_ORDER_INPUT_PERIOD    = PERM_CALL_AUCTION_PRE,  ///< 可撤单
    TS_AUCTION_NO_CANCELLATION_PERIOD = PERM_CALL_AUCTION,      ///< 不可撤
    TS_AUCTION_MATCHING_FILL_PERIOD  = PERM_CALL_AUCTION_FILL,   ///< 随机对盘
    TS_AUCTION_MATCHING_TO_OPENING   = PERM_CALL_AUCTION_FILL,
    TS_AUCTION_MATCHING_TO_CLOSING   = PERM_CALL_AUCTION_FILL,
    TS_EXCHANGE_HALT_TRADING         = PERM_OPEN,               ///< 暂停交易
};

/// TimeStatus 辅助函数
inline bool ts_has_realtime_data(uint16_t ts)     { return perm_has(ts, PERM_MATCHING); }
inline bool ts_is_market_active(uint16_t ts)      { return ts_has_realtime_data(ts); }
inline bool ts_is_open(uint16_t ts)               { return (ts & static_cast<uint16_t>(PERM_OPEN)) == static_cast<uint16_t>(PERM_OPEN); }
inline bool ts_is_continuous_trading(uint16_t ts) { return (ts & static_cast<uint16_t>(PERM_CONTINUOUS_TRADING)) == static_cast<uint16_t>(PERM_CONTINUOUS_TRADING); }
inline bool ts_is_trading_disabled(uint16_t ts)   { return (ts & static_cast<uint16_t>(PERM_MATCHING)) == 0; }

// =====================================================================
// 3. TimeRange - 时间范围 (HH:MM:SS 粒度)
// =====================================================================
class TimeRange {
public:
    Timestamp  begin;
    Timestamp  end;
    TimeStatus status;
    Region     reg;

    TimeRange();
    TimeRange(const std::string& time_range, TimeStatus status = TS_TRADING, Region reg = Region::CN);

    /// 是否在时段内 (左闭右开)
    std::optional<TimeStatus> in_range(const Timestamp& timestamp) const;

    /// 是否连续竞价交易中
    bool is_trading(const Timestamp& timestamp = Timestamp()) const;

    /// 时段是否有效
    bool is_valid() const;

    /// 是否盘前
    bool is_session_pre(const Timestamp& timestamp = Timestamp()) const;

    /// 是否盘中
    bool is_session_reg(const Timestamp& timestamp = Timestamp()) const;

    /// 是否盘后
    bool is_session_post(const Timestamp& timestamp = Timestamp()) const;

    /// 时段总时长 (分钟)
    int get_duration_minutes() const;

    /// 时段已经开始多少分钟
    int get_elapsed_minutes(const Timestamp& current_time) const;
};

// =====================================================================
// 4. TradingSession - 交易时段
// =====================================================================
class TradingSession {
public:
    std::vector<TimeRange> sessions;
    Timestamp earliest_start;
    Timestamp latest_end;
    Timestamp closing_time;

    TradingSession();

    /// 用多个 TimeRange 构造
    explicit TradingSession(const std::vector<TimeRange>& ranges);

    /// 用字符串格式构造: "09:30:00 ~ 11:30:00, 13:00:00 ~ 15:00:00"
    explicit TradingSession(const std::string& time_ranges_str);

    /// 添加时段
    void add_session(const TimeRange& range);

    /// 更新时间边界
    void update_time_bounds();

    /// 判断当前时间状态
    TimeStatus check_status(const Timestamp& timestamp = Timestamp()) const;

    /// 是否交易中
    bool is_trading(const Timestamp& timestamp = Timestamp()) const;

    /// 所有时段是否有效
    bool is_valid() const;

    /// 交易是否尚未开始
    bool is_trading_not_started(const Timestamp& timestamp = Timestamp()) const;

    /// 交易是否已结束
    bool is_trading_ended(const Timestamp& timestamp = Timestamp()) const;

    /// 已交易分钟数
    int minutes(const Timestamp& timestamp = Timestamp()) const;

    /// 当日可交易时段总时长 (分钟)
    int get_trading_minutes() const;
};

// =====================================================================
// 5. 市场交易时段工厂函数
// =====================================================================

/// 初始化 A 股交易时段
TradingSession init_cn_session();

/// 初始化港股交易时段
TradingSession init_hk_session();

/// 初始化美股交易时段
TradingSession init_us_session();

/// 根据交易所获取当日交易时段
TradingSession latest_session_by_exchange(Exchange exchange = Exchange::SSE);

// =====================================================================
// 6. 运行时状态
// =====================================================================
struct RuntimeStatus {
    bool       before_last_trade_day = false;
    bool       is_holiday            = false;
    bool       before_init_time      = false;
    bool       cache_after_init_time = false;
    bool       update_in_real_time   = false;
    TimeStatus status                = TS_CLOSED;
};

// =====================================================================
// 7. 运行时状态检查函数
// =====================================================================

/// 获取今天的盘前初始化时间戳
/// 对齐 Python session.get_today() / Rust session::get_today()
Timestamp get_today();

/// 检查交易时间戳状态
/// 对齐 Python session.check_trading_timestamp() / Rust session::check_trading_timestamp()
///
/// @param exchange 交易所, 默认 SSE
/// @param last_modified 文件修改时间, 为 nullopt 时使用当前时间
/// @return RuntimeStatus 运行时状态
RuntimeStatus check_trading_timestamp(
    Exchange exchange = Exchange::SSE,
    std::optional<Timestamp> last_modified = std::nullopt);

/// 是否可以初始化
/// 对齐 Python session.can_initialize() / Rust session::can_initialize()
///
/// @param exchange 交易所, 默认 SSE
/// @param last_modified 文件修改时间, 为 nullopt 时使用当前时间
/// @return true 可以初始化, false 不需要初始化
bool can_initialize(
    Exchange exchange = Exchange::SSE,
    std::optional<Timestamp> last_modified = std::nullopt);

} // namespace meta

#endif // QUANT1X_DATA_META_SESSION_H
