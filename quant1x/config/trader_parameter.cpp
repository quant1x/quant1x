#include <quant1x/config/trader_parameter.h>
#include <quant1x/data/meta/timestamp.h>
#include <quant1x/data/meta/calendar.h>
#include <magic_enum/magic_enum.hpp>

namespace quant1x::config {
namespace meta = quant1x::data::meta;

    // 统计标的总数
    int TraderParameter::TotalNumberOfTargets() const {
        int total = 0;
        for (const auto& v : Strategies) {
            total += v.NumberOfTargets();
        }
        return total;
    }

    // 重置仓位占比
    void TraderParameter::ResetPositionRatio() {
        double remainingRatio = 1.00;
        size_t strategyCount = Strategies.size();
        std::vector<StrategyParameter *> unassignedStrategies;

        for (size_t i = 0; i < strategyCount; i++) {
            StrategyParameter *v = &Strategies[i];
            if (!v->BuyEnable()) {
                continue;
            }
            // 校对个股最大资金
            if (v->FeeMax > BuyAmountMax) {
                v->FeeMax = BuyAmountMax;
            }
            // 校对个股最小资金
            if (v->FeeMin < BuyAmountMin) {
                v->FeeMin = BuyAmountMin;
            }
            if (v->Weight > 1.00) {
                v->Weight = 1.00;
            }
            if (v->Weight > 0) {
                remainingRatio -= v->Weight;
            } else {
                unassignedStrategies.push_back(v);
            }
        }

        size_t remainingCount = unassignedStrategies.size();
        if (remainingRatio > 0 && remainingCount > 0) {
            double averageFundPercentage = remainingRatio / remainingCount;
            for (size_t i = 0; i < remainingCount; i++) {
                StrategyParameter *v = unassignedStrategies[i];
                v->Weight = averageFundPercentage;
                if (i + 1 == remainingCount) {
                    v->Weight = remainingRatio;
                }
                remainingRatio -= v->Weight;
            }
        }
    }

    // 计算每日无风险利率
    double TraderParameter::DailyRiskFreeRate(const std::string& date) const {
        (void)date;
        meta::Timestamp ts = date;
        std::string fixedDate = ts.only_date();
        std::string year = fixedDate.substr(0, 4);
        std::string start = year + "-01-01";
        std::string end = year + "-12-31";
        std::vector<std::string> dates = meta::get_date_range(start, end);
        size_t count = dates.size();
        return AnnualInterestRate / count;
    }

    std::optional<StrategyParameter> TraderParameter::GetStrategyParameterByCode(uint64_t strategyCode) const {
        for (auto const & v : Strategies) {
            if (v.Auto && v.Id == strategyCode) {
                return v;
            }
        }
        return std::nullopt;
    }

    std::ostream &operator<<(std::ostream &os, const TraderParameter &parameter) {
        os << "AccountId: " << parameter.AccountId << "\n"
           << "OrderPath: " << parameter.OrderPath << "\n"
           << "TopN: " << parameter.TopN << "\n"
           << "HaveETF: " << parameter.HaveETF << "\n"
           << "PriceCageRatio: " << parameter.PriceCageRatio << "\n"
           << "MinimumPriceFluctuationUnit: " << parameter.MinimumPriceFluctuationUnit << "\n"
           << "AnnualInterestRate: " << parameter.AnnualInterestRate << "\n"
           << "StampDutyRateForBuy: " << parameter.StampDutyRateForBuy << "\n"
           << "StampDutyRateForSell: " << parameter.StampDutyRateForSell << "\n"
           << "TransferRate: " << parameter.TransferRate << "\n"
           << "CommissionRate: " << parameter.CommissionRate << "\n"
           << "CommissionMin: " << parameter.CommissionMin << "\n"
           << "PositionRatio: " << parameter.PositionRatio << "\n"
           << "KeepCash: " << parameter.KeepCash << "\n"
           << "BuyAmountMax: " << parameter.BuyAmountMax << "\n"
           << "BuyAmountMin: " << parameter.BuyAmountMin << "\n"
           << "Role: " << magic_enum::enum_name(parameter.Role) << "\n"
           << "ProxyUrl: " << parameter.ProxyUrl << "\n"
           << "Strategies:[" << "\n";
        for (auto const & v: parameter.Strategies) {
            os << "  " <<v << "\n";
        }
        os << "]" << "\n";
        //<< " CancelSession: " << parameter.CancelSession
        os << " UndertakeRatio: " << parameter.UndertakeRatio << "\n";
        return os;
    }
} // namespace quant1x::config