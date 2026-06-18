#pragma once
#ifndef QUANT1X_DATA_SCHEMA_ADJUSTMENT_H
#define QUANT1X_DATA_SCHEMA_ADJUSTMENT_H 1

#include <quant1x/data/meta/timestamp.h>
#include <quant1x/data/meta/exchange.h>
#include <cstdint>
#include <string>
#include <vector>

namespace quant1x::data::meta::schema {

/// 除权除息类别
enum class XdxrCategory : uint8_t {
    EX_DIVIDEND                    = 1,   ///< 除权除息
    BONUS_SHARES_LISTING           = 2,   ///< 送股上市 (无偿)
    RESTRICTED_SHARES_LISTING      = 3,   ///< 非流通股上市
    UNSPECIFIED_CAPITAL_ADJUSTMENT = 4,   ///< 未知股本变动
    GENERAL_CAPITAL_ADJUSTMENT     = 5,   ///< 股本变化
    NEW_SHARE_ISSUANCE             = 6,   ///< 增发新股
    SHARE_REPURCHASE               = 7,   ///< 股份回购
    NEW_SHARES_LISTING             = 8,   ///< 增发新股上市
    TRANSFERRED_RIGHTS_SHARES      = 9,   ///< 转配股上市
    CONVERTIBLE_BOND_LISTING       = 10,  ///< 可转债上市
    STOCK_SPLIT_OR_REVERSE_SPLIT   = 11,  ///< 拆股或合股
    RESTRICTED_SHARES_CONSOLIDATION = 12, ///< 非流通股缩股
    ISSUE_CALL_WARRANTS            = 13,  ///< 送认购权证
    ISSUE_PUT_WARRANTS             = 14,  ///< 送认沽权证
};

inline const char* xdxr_category_to_string(int category) {
    switch (category) {
        case 1:  return "除权除息";
        case 2:  return "送配股上市";
        case 3:  return "非流通股上市";
        case 4:  return "未知股本变动";
        case 5:  return "股本变化";
        case 6:  return "增发新股";
        case 7:  return "股份回购";
        case 8:  return "增发新股上市";
        case 9:  return "转配股上市";
        case 10: return "可转债上市";
        case 11: return "扩缩股";
        case 12: return "非流通股缩股";
        case 13: return "送认购权证";
        case 14: return "送认沽权证";
        default: return "Unknown";
    }
}

/// 除权除息信息
struct XdxrInfo {
    std::string date;                   ///< 日期 YYYY-MM-DD
    int         category = 0;           ///< 类型编号
    std::string name;                   ///< 类型名称
    double      fen_hong = 0.0;         ///< 分红 (元)
    std::string dividend_currency;      ///< 分红币种
    double      pei_gu_jia = 0.0;       ///< 配股价 (元)
    std::string rights_currency;        ///< 配股价币种
    double      song_zhuan_gu = 0.0;    ///< 送转股 (股)
    double      pei_gu = 0.0;           ///< 配股 (股)
    double      suo_gu = 0.0;           ///< 缩股 (股)
    double      qian_liu_tong = 0.0;    ///< 除权前流通股 (万股)
    double      hou_liu_tong = 0.0;     ///< 除权后流通股 (万股)
    double      qian_zong_gu_ben = 0.0; ///< 除权前总股本 (万股)
    double      hou_zong_gu_ben = 0.0;  ///< 除权后总股本 (万股)
    double      fen_shu = 0.0;          ///< 权证份数
    double      xing_quan_jia = 0.0;    ///< 行权价格 (元)

    bool is_adjust() const {
        return (fen_hong + pei_gu + song_zhuan_gu + suo_gu + fen_shu) > 0.0;
    }

    /// 返回 (m, a) 复权因子对
    std::pair<double, double> adjust_factor() const {
        double a = compute_monetary_adjustment();
        double b = compute_share_adjustment_ratio();
        double m = 1.0, a_val = 0.0;
        if (std::abs(1.0 + b) > 1e-10) {
            m = 1.0 / (1.0 + b);
            a_val = a * m;
        }
        return {m, a_val};
    }

    double compute_monetary_adjustment() const {
        return (pei_gu * pei_gu_jia - fen_hong + fen_shu * xing_quan_jia) / 10.0;
    }

    double compute_share_adjustment_ratio() const {
        return (song_zhuan_gu + pei_gu - suo_gu + fen_shu) / 10.0;
    }

    bool is_capital_change() const {
        if (category == 1 || category == 11 || category == 12 ||
            category == 13 || category == 14) {
            return false;
        }
        return hou_liu_tong > 0 && hou_zong_gu_ben > 0;
    }
};

/// 除权除息条目
struct XdxrEntry {
    Exchange              exchange;
    std::string           ticker;
    int                   count = 0;
    std::vector<XdxrInfo> list;
};

/// 累积复权结构
struct CumulativeAdjustment {
    Timestamp timestamp;                 ///< 复权日期
    double    m = 0.0;                   ///< 乘性因子
    double    a = 0.0;                   ///< 加性因子
    double    monetary_adjustment = 0.0; ///< 货币调整
    double    share_adjustment_ratio = 0.0; ///< 股本调整比率
    int       no = 0;                    ///< 序号

    /// 复权
    double apply(double price) const {
        return price * m + a;
    }

    /// 还权
    double inverse(double adjusted_price) const {
        return (adjusted_price - a) / m;
    }
};

} // namespace quant1x::data::meta::schema

#endif // QUANT1X_DATA_SCHEMA_ADJUSTMENT_H
