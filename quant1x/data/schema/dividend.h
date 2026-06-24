#pragma once
#ifndef QUANT1X_DATA_SCHEMA_DIVIDEND_H
#define QUANT1X_DATA_SCHEMA_DIVIDEND_H 1

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <cstdint>
#include <cmath>

namespace quant1x::data::schema {

// ================= 枚举定义 =================

/// 市场类型
enum class MarketType : uint8_t {
    A_SHARE  = 0,  ///< A 股
    HK_SHARE = 1,  ///< 港股
    US_SHARE = 2,  ///< 美股
    UK_SHARE = 3,  ///< 英股
    SG_SHARE = 4,  ///< 新加坡
    FUND     = 5,  ///< 基金
    REITS    = 6,  ///< REITs
    OTHER    = 7,  ///< 其他
};

/// 分红类型
enum class DividendType : uint8_t {
    CASH     = 0,  ///< 现金分红
    SPECIAL  = 1,  ///< 特别分红
    PROPERTY = 2,  ///< 实物分红
    NONE     = 3,  ///< 无分红
};

/// 红股类型
enum class BonusType : uint8_t {
    BONUS_ISSUE    = 0, ///< 红股发行
    STOCK_DIVIDEND = 1, ///< 股票分红
    CAPITALIZATION = 2, ///< 资本化发行
    NONE           = 3, ///< 无红股
};

/// 公司行为类型
enum class ActionType : uint8_t {
    DIVIDEND       = 0,  ///< 分红
    BONUS          = 1,  ///< 送红股
    SPLIT          = 2,  ///< 拆股
    REVERSE_SPLIT  = 3,  ///< 缩股/合股
    CONSOLIDATION  = 4,  ///< 股份合并
    RIGHTS_ISSUE   = 5,  ///< 供股/配股
    MIXED          = 6,  ///< 混合方案
    SPIN_OFF       = 7,  ///< 分拆上市
};

/// MarketType → 字符串
inline const char* market_type_to_string(MarketType mt) {
    switch (mt) {
        case MarketType::A_SHARE:  return "A 股";
        case MarketType::HK_SHARE: return "港股";
        case MarketType::US_SHARE: return "美股";
        case MarketType::UK_SHARE: return "英股";
        case MarketType::SG_SHARE: return "新加坡";
        case MarketType::FUND:     return "基金";
        case MarketType::REITS:    return "REITs";
        default:                   return "其他";
    }
}

/// DividendType → 字符串
inline const char* dividend_type_to_string(DividendType dt) {
    switch (dt) {
        case DividendType::CASH:     return "现金分红";
        case DividendType::SPECIAL:  return "特别分红";
        case DividendType::PROPERTY: return "实物分红";
        default:                     return "无分红";
    }
}

/// BonusType → 字符串
inline const char* bonus_type_to_string(BonusType bt) {
    switch (bt) {
        case BonusType::BONUS_ISSUE:    return "红股发行";
        case BonusType::STOCK_DIVIDEND: return "股票分红";
        case BonusType::CAPITALIZATION: return "资本化发行";
        default:                        return "无红股";
    }
}

/// ActionType → 字符串
inline const char* action_type_to_string(ActionType at) {
    switch (at) {
        case ActionType::DIVIDEND:       return "分红";
        case ActionType::BONUS:          return "送红股";
        case ActionType::SPLIT:          return "拆股";
        case ActionType::REVERSE_SPLIT:  return "缩股/合股";
        case ActionType::CONSOLIDATION:  return "股份合并";
        case ActionType::RIGHTS_ISSUE:   return "供股/配股";
        case ActionType::MIXED:          return "混合方案";
        case ActionType::SPIN_OFF:       return "分拆上市";
        default:                         return "未知";
    }
}

// ================= 除权除息记录 =================

/// 除权除息因子
struct AdjustmentFactor {
    double price_factor  = 1.0;  ///< 价格调整因子
    double share_factor  = 1.0;  ///< 股本调整因子
    double cash_dividend = 0.0;  ///< 现金分红金额
};

/// 除权除息记录
struct DividendAdjustmentRecord {
    std::string               symbol;              ///< 股票代码
    MarketType                market = MarketType::A_SHARE;
    ActionType                action_type = ActionType::DIVIDEND;

    // 日期字段
    std::optional<std::string> announcement_date;  ///< 公告日期
    std::optional<std::string> record_date;        ///< 股权登记日
    std::optional<std::string> ex_date;            ///< 除权除息日
    std::optional<std::string> payment_date;       ///< 派息日

    // Dividend 专用字段
    std::optional<double>      dividend_amount;    ///< 每股现金分红
    std::optional<std::string> dividend_currency;   ///< 分红币种
    DividendType               dividend_type = DividendType::NONE;

    // Bonus 专用字段
    std::optional<double>      bonus_ratio;        ///< 红股比例
    BonusType                  bonus_type = BonusType::NONE;

    // Split 专用字段
    std::optional<double>      split_ratio;        ///< 拆股比例

    // Rights Issue 专用字段
    std::optional<double>      rights_ratio;       ///< 配股比例
    std::optional<double>      rights_price;       ///< 配股价
    std::optional<std::string> rights_currency;    ///< 配股价币种

    // Consolidation 专用字段
    std::optional<double>      consolidation_ratio;  ///< 缩股比例
    std::optional<int>         consolidation_base;   ///< 合并基数
    std::optional<int>         consolidation_target; ///< 合并目标

    std::string raw_description;                    ///< 原始方案描述

    // === 便捷方法 ===

    bool has_cash_dividend() const {
        return dividend_amount.has_value() && dividend_amount.value() > 0;
    }

    bool has_bonus() const {
        return bonus_ratio.has_value() && bonus_ratio.value() > 0;
    }

    bool has_split() const {
        return split_ratio.has_value() && split_ratio.value() > 1.0;
    }

    bool has_consolidation() const {
        if (consolidation_ratio.has_value() && consolidation_ratio.value() < 1.0) return true;
        if (consolidation_base.has_value() && consolidation_target.has_value()) return true;
        return false;
    }

    bool has_rights_issue() const {
        return rights_ratio.has_value() && rights_ratio.value() > 0;
    }

    std::optional<double> get_consolidation_factor() const {
        if (consolidation_ratio.has_value()) return consolidation_ratio;
        if (consolidation_base.has_value() && consolidation_target.has_value()) {
            return static_cast<double>(consolidation_target.value()) /
                   static_cast<double>(consolidation_base.value());
        }
        return std::nullopt;
    }

    double get_bonus_factor() const {
        if (has_bonus()) return 1.0 + bonus_ratio.value();
        return 1.0;
    }

    double get_split_factor() const {
        if (has_split()) return split_ratio.value();
        return 1.0;
    }

    AdjustmentFactor get_adjustment_factor() const {
        AdjustmentFactor factor;

        // 1. 现金分红
        if (has_cash_dividend()) {
            factor.cash_dividend = dividend_amount.value();
        }

        // 2. Bonus 红股
        if (has_bonus()) {
            double bf = get_bonus_factor();
            factor.price_factor /= bf;
            factor.share_factor *= bf;
        }

        // 3. Split 拆股
        if (has_split()) {
            double sf = get_split_factor();
            factor.price_factor /= sf;
            factor.share_factor *= sf;
        }

        // 4. Consolidation 缩股
        if (has_consolidation()) {
            auto cf = get_consolidation_factor();
            if (cf.has_value() && cf.value() > 0) {
                factor.price_factor /= cf.value();
                factor.share_factor *= cf.value();
            }
        }

        return factor;
    }
};

// ================= 分红处理中心 =================

/// 分红除权除息数据处理中心
class DividendAdjustment {
public:
    void add_record(const DividendAdjustmentRecord& record) {
        records_.push_back(record);
    }

    void add_records(const std::vector<DividendAdjustmentRecord>& recs) {
        records_.insert(records_.end(), recs.begin(), recs.end());
    }

    std::vector<DividendAdjustmentRecord> get_dividend_records(const std::string& symbol) const {
        std::vector<DividendAdjustmentRecord> result;
        for (const auto& r : records_) {
            if (r.symbol == symbol && r.has_cash_dividend()) result.push_back(r);
        }
        return result;
    }

    std::vector<DividendAdjustmentRecord> get_bonus_records(const std::string& symbol) const {
        std::vector<DividendAdjustmentRecord> result;
        for (const auto& r : records_) {
            if (r.symbol == symbol && r.has_bonus()) result.push_back(r);
        }
        return result;
    }

    std::vector<DividendAdjustmentRecord> get_all_records(const std::string& symbol) const {
        std::vector<DividendAdjustmentRecord> result;
        for (const auto& r : records_) {
            if (r.symbol == symbol) result.push_back(r);
        }
        return result;
    }

    const std::vector<DividendAdjustmentRecord>& records() const { return records_; }
    size_t size() const { return records_.size(); }

private:
    std::vector<DividendAdjustmentRecord> records_;
};

} // namespace quant1x::data::schema

#endif // QUANT1X_DATA_SCHEMA_DIVIDEND_H
