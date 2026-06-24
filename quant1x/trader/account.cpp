#include <quant1x/trader/account.h>
#include <quant1x/trader/fee.h>
#include <quant1x/config/config.h>
#include <quant1x/std/numeric.h>
#include <spdlog/spdlog.h>
#include <quant1x/runtime/once.h>
#include <quant1x/data/meta/timestamp.h>
#include <quant1x/trader/trader.h>

namespace config = quant1x::config;
namespace meta = quant1x::data::meta;
namespace data = quant1x::data;

namespace trader {

    // 全局交易参数
    static inline double trader_account_theoretical_fund = 0.0;
    static inline double trader_account_remaining_cash   = 0.0;

    static inline auto account_once = RollingOnce::create("trader-account", quant1x::config::GLOBAL_CRON_EXPR_DAILY_INIT);

    void lazy_init_fund_pool() {
        calculateTheoreticalFund(&trader_account_theoretical_fund, &trader_account_remaining_cash);
    }

    // 主要计算函数
    void calculateTheoreticalFund(double* pTheoretical, double* pCash) {
        // 默认初始化为无效值
        double theoretical = InvalidFee;
        double cash = InvalidFee;

        // 查询账户信息
        auto acc = QueryAccount();
        auto positions = QueryHolding();

        if (!acc.has_value()) {
            // 模拟错误：持仓为空
            if (pTheoretical) *pTheoretical = theoretical;
            if (pCash) *pCash = cash;
            return;
        }

        // 计算持仓部分卖出后的增量可用资金
        double canUseAmount = 0.0;
        double vix = 0.10;
        double accValue = 0.0;

        for (const auto& position : positions) {
            accValue += position.MarketValue;
            if (position.CanUseVolume < 1) continue;

            double marketPrice = position.MarketValue / position.Volume;
            double canUseValue = marketPrice * position.CanUseVolume;
            canUseAmount += canUseValue * (1 - vix);
        }

        accValue = numeric::decimal(accValue);
        canUseAmount = numeric::decimal(canUseAmount);

        auto const &traderParameter = config::TraderConfig();
        // 扣除预留现金
        double canUseCash = acc->Cash + canUseAmount - traderParameter->KeepCash;
        // 根据持仓占比, 计算预留多少资金
        double reserveCash = numeric::decimal(acc->TotalAsset * (1 - traderParameter->PositionRatio));
        // 可用金额 = 总资金量 - 预留资金
        double available = canUseCash - reserveCash;

        spdlog::warn("账户资金: 可用={}, 市值={}, 预留={}, 可买={}, 可卖={}",
                     acc->Cash, accValue, reserveCash, available, canUseAmount);

        if (available > acc->Cash) {
            spdlog::warn("!!! 持仓占比[{}%], 已超过可总仓位的[{}%], 必须在收盘前择机降低仓位 !!!",
                         numeric::decimal(100 * (accValue / acc->TotalAsset)),
                         numeric::decimal(100 * (1 - traderParameter->PositionRatio)));
        }
        // 这里重新修订可用金额, 不计算持仓可卖后的可用资金量
        available = (acc->TotalAsset - traderParameter->KeepCash) * traderParameter->PositionRatio;
        if (available > acc->Cash) {
            available = acc->Cash;
        }

        theoretical = available;
        cash = acc->Cash;

        // 最后设置输出参数
        if (pTheoretical) {
            *pTheoretical = theoretical;
        }
        if (pCash) {
            *pCash = cash;
        }
    }

    double CalculateAvailableFundsForSingleTarget(int quantityQuota, double weight, double feeMax, double feeMin) {
        account_once->Do(lazy_init_fund_pool);

        if (quantityQuota < 1) {
            return InvalidFee;
        }

        if (trader_account_theoretical_fund <= InvalidFee + 1e-6) {
            return InvalidFee;
        }

        double strategyFunds = trader_account_theoretical_fund * weight;
        double singleFundsAvailable = numeric::decimal(strategyFunds / quantityQuota);

        if (singleFundsAvailable > feeMax) {
            singleFundsAvailable = feeMax;
        } else if (singleFundsAvailable < feeMin) {
            return InvalidFee;
        }
        auto const &traderParameter = config::TraderConfig();
        if (singleFundsAvailable > traderParameter->BuyAmountMax) {
            singleFundsAvailable = traderParameter->BuyAmountMax;
        } else if (singleFundsAvailable < traderParameter->BuyAmountMin) {
            return InvalidFee;
        }

        return singleFundsAvailable;
    }

    double CalculateAvailableFund(const std::shared_ptr<config::StrategyParameter>& strategyParameter) {
        return CalculateAvailableFundsForSingleTarget(
            strategyParameter->Total,
            strategyParameter->Weight,
            strategyParameter->FeeMax,
            strategyParameter->FeeMin
        );
    }

} // namespace trader