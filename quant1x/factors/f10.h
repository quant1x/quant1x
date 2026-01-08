#pragma once
#ifndef QUANT1X_FACTOR_F10_H
#define QUANT1X_FACTOR_F10_H 1

#include "quant1x/data/adapter.h"
#include "quant1x/exchange/calendar.h"
#include "quant1x/std/numeric.h"

namespace factors {
    namespace risk {
        constexpr const int ReportingRiskPeriod = 3; ///< 预警距离财务报告日期还有多少个交易日, 默认3个交易日
        constexpr const int ReportingWarningDays = ReportingRiskPeriod;
    } // namespace detail
} // namespace factors

struct F10 {
    // 日期
    std::string Date;
    // 代码
    std::string Code;
    // 名称
    std::string SecurityName;
    // 是否次新股
    bool SubNew;
    // 是否两融标的
    bool MarginTradingTarget;
    // 每手单位
    int VolUnit;
    // 小数点
    int DecimalPoint;
    // 上市日期
    std::string IpoDate;
    // 更新日期
    std::string UpdateDate;
    // 总股本
    double TotalCapital;
    // 流通股本
    double Capital;
    // 自由流通股本
    double FreeCapital;
    // 前十大流通股东总股本
    double Top10Capital;
    // 前十大流通股东总股本变化
    double Top10Change;
    // 前十大流通股东持仓变化
    double ChangeCapital;
    // 当期增持比例
    double IncreaseRatio;
    // 当期减持比例
    double ReductionRatio;
    // 当前市场处于哪个季报期, 用于比较个股的季报数据是否存在拖延的情况
    std::string QuarterlyYearQuarter;
    // 最新报告期
    std::string QDate;
    // 年报披露日期
    std::string AnnualReportDate;
    // 最新季报披露日期
    std::string QuarterlyReportDate;
    // 当期营业总收入
    double TotalOperateIncome;
    // 每股净资产
    double BPS;
    // 每股收益
    double BasicEPS;
    // 每股收益(扣除)
    double DeductBasicEPS;
    // 通达信安全分
    int SafetyScore;
    // 公告-增持
    int Increases;
    // 公告-减持
    int Reduces;
    // 公告-风险数
    int Risk;
    // 公告-风险关键词
    std::string RiskKeywords;
    // 更新时间
    std::string UpdateTime;
    // 样本状态
    uint64_t State;

    /// 计算自由换手率
    f64 TurnZ(f64 v) const {
        auto freeCapital = FreeCapital;
        if (freeCapital == 0) {
            freeCapital = Capital;
        }
        if(std::fabs(freeCapital) < std::numeric_limits<double>::epsilon()) {
            return 0.00;
        }
        auto turnoverRateZ = numeric::ChangeRate(freeCapital, v);
        turnoverRateZ *= 10000;
        turnoverRateZ = numeric::decimal(turnoverRateZ);
        return turnoverRateZ;
    }

    /// 是否财报披露前夕
    bool IsReportingRiskPeriod() const {
        if(AnnualReportDate.empty() || QuarterlyReportDate.empty()) {
            // 如果年报和季报日期不确定, 判定为非风险期, 返回false
            // 这种情况有可能是次新股的保护期
            return false;
        }
        auto current_date = exchange::timestamp(Date);
        auto ys = exchange::date_range(current_date, AnnualReportDate);
        auto ly = ys.size();
        auto qs = exchange::date_range(current_date, QuarterlyReportDate);
        auto lq = qs.size();
        if((ly > 0 && ly < factors::risk::ReportingWarningDays) || (lq > 0 && lq < factors::risk::ReportingWarningDays)) {
            return true;
        }
        return false;
    }
};

class F10Feature : public data::FeatureAdapter {
private:
    F10 f10;
public:
    F10Feature() = default;
    F10Feature(const F10Feature&) = default;

    data::Kind Kind() const override;

    std::string Owner() override;

    std::string Key() const override;

    std::string Name() const override;

    std::string Usage() const override;

    std::vector<std::string> headers() const override;
    std::vector<std::string> values() const override;

    std::unique_ptr<data::FeatureAdapter> clone() const override;

    void init(const exchange::timestamp &timestamp) override;

    void Print(const std::string &code, const std::vector<exchange::timestamp> &dates) override;

    void Update(const std::string &code, const exchange::timestamp &date) override;
};


namespace factors {
    /// 获取指定日期的F10数据
    std::optional<F10> get_f10(const std::string& code, const exchange::timestamp& timestamp);
}

#endif //QUANT1X_FACTOR_F10_H
