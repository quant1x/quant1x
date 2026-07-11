#pragma once
#ifndef QUANT1X_CONFIG_DETAIL_STRATEGY_PARAMETER_H
#define QUANT1X_CONFIG_DETAIL_STRATEGY_PARAMETER_H 1

#include <quant1x/base/strings.h>
#include <quant1x/encoding/yaml.h>
#include <quant1x/config/rule_parameter.h>
#include <quant1x/config/trading_session.h>
#include <quant1x/config/price_cage.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <unordered_set>
#include <ostream>

namespace quant1x::config {

    // QmtStrategyNameFromId 通过策略ID返回用于在QMT系统中表示的string类型的策略名称
    inline std::string QmtStrategyNameFromId(uint64_t strategyCode) {
        return "S" + std::to_string(strategyCode);
    }

    constexpr const char sectorIgnorePrefix[] = "-";
    constexpr size_t sectorPrefixLength = sizeof(sectorIgnorePrefix) - 1;  // 减去null终止符

    // StrategyParameter 策略参数
    struct StrategyParameter {
        // 策略ID, 默认是1
        uint64_t Id = 1;
        // 是否自动执行
        bool Auto = false;
        // 策略名称
        std::string Name;
        // 订单标识,分早盘,尾盘和盘中
        std::string Flag;
        // 可操作的交易时段
        TradingSession Session;
        // 策略权重, 默认0, 由系统自动分配
        double Weight = 0.0;
        // 订单总数, 默认是3
        int Total = 3;
        // 价格笼子比例, 默认0%, 推荐在配置文件中明确配置, 如何不配置则从交易参数中获取
        double PriceCageRatio = 0.00;
        // 价格最小变动单位, 默认0.00, 推荐在配置文件中明确配置, 如何不配置则从交易参数中获取
        double MinimumPriceFluctuationUnit = 0.00;

        // 卖出滑点比例, 默认0.01
        double FixedSlippageForSell = quant1x::config::FixedSlippageForSell;

        // 可投入资金-最大
        double FeeMax = 20000.00;

        // 可投入资金-最小
        double FeeMin = 10000.00;

        // 板块, 策略适用的板块列表
        std::vector<std::string> Sectors;

        // 剔除两融标的, 默认是剔除
        bool IgnoreMarginTrading = true;

        // 持仓周期, 默认为1天
        int HoldingPeriod = 1;

        // 卖出策略, 默认117
        uint64_t SellStrategy = 117;

        // 固定收益率
        double FixedYield = 0.0;

        // 止盈比例, 默认15%
        double TakeProfitRatio = 15.00;

        // 止损比例, 默认-2%
        double StopLossRatio = -2.00;

        // 阳线, 低开幅度
        double LowOpeningAmplitude = 0.618;

        // 阴线, 高开幅度
        double HighOpeningAmplitude = 0.382;

        // 过滤规则
        RuleParameter Rules;

        // 需要排除的个股
        std::vector<std::string> excludeCodes;

        // QmtStrategyName 获取策略名称
        std::string QmtStrategyName() const {
            return QmtStrategyNameFromId(Id);
        }

        // Enable 策略是否有效
        bool Enable() const {
            return Auto && Id >= 0;
        }

        // BuyEnable 获取可买入状态
        bool BuyEnable() const {
            return Enable() && Total > 0;
        }

        // SellEnable 获取可卖出状态
        bool SellEnable() const {
            return Enable();
        }

        // IsCookieCutterForSell 是否一刀切卖出
        bool IsCookieCutterForSell() const {
            return SellEnable() && Total == 0;
        }

        // NumberOfTargets 获得可买入标的总数
        int NumberOfTargets() const {
            if (!BuyEnable()) {
                return 0;
            }
            return Total;
        }

        // 过滤股票代码
        std::vector<std::string> Filter(const std::vector<std::string>& codes);

        // 取得可以交易的证券代码列表
        std::vector<std::string> StockList();

        friend std::ostream &operator<<(std::ostream &os, const StrategyParameter &parameter);

    private:
        void initExclude();
    };

} // namespace quant1x::config

namespace YAML {
    // StrategyParameter转换
    template<>
    struct convert<quant1x::config::StrategyParameter> {
        static bool decode(const Node& node, quant1x::config::StrategyParameter& param) {
            if (!node.IsMap()) return false;

            // 基本参数
            encoding::safe_yaml::try_parse_field(node, "id", param.Id);
            encoding::safe_yaml::try_parse_field(node, "auto", param.Auto);
            encoding::safe_yaml::try_parse_field(node, "name", param.Name);
            encoding::safe_yaml::try_parse_field(node, "flag", param.Flag);

            // 交易时段
            if (node["time"]) {
                quant1x::config::TradingSession session;
                if (convert<quant1x::config::TradingSession>::decode(node["time"], session)) {
                    param.Session = session;
                }
            }

            // 权重和数量
            encoding::safe_yaml::try_parse_field(node, "weight", param.Weight);
            encoding::safe_yaml::try_parse_field(node, "total", param.Total);

            // 价格参数
            encoding::safe_yaml::try_parse_field(node, "price_cage_ratio", param.PriceCageRatio);
            encoding::safe_yaml::try_parse_field(node, "minimum_price_fluctuation_unit", param.MinimumPriceFluctuationUnit);
            encoding::safe_yaml::parse_field(node, "fixed_slippage_for_sell", param.FixedSlippageForSell, quant1x::config::FixedSlippageForSell);

            // 资金参数
            encoding::safe_yaml::try_parse_field(node, "fee_max", param.FeeMax);
            encoding::safe_yaml::try_parse_field(node, "fee_min", param.FeeMin);

            // 板块配置
            if (node["sectors"] && node["sectors"].IsSequence()) {
                param.Sectors.clear();
                for (const auto& item : node["sectors"]) {
                    std::string sector;
                    if (item.IsScalar()) {
                        sector = item.as<std::string>();
                        param.Sectors.push_back(std::move(sector));
                    }
                }
            }

            // 交易控制
            encoding::safe_yaml::try_parse_field(node, "ignore_margin_trading", param.IgnoreMarginTrading);
            encoding::safe_yaml::try_parse_field(node, "holding_period", param.HoldingPeriod);
            encoding::safe_yaml::try_parse_field(node, "sell_strategy", param.SellStrategy);

            // 收益率参数
            encoding::safe_yaml::try_parse_field(node, "fixed_yield", param.FixedYield);
            encoding::safe_yaml::try_parse_field(node, "take_profit_ratio", param.TakeProfitRatio);
            encoding::safe_yaml::try_parse_field(node, "stop_loss_ratio", param.StopLossRatio);

            // 开盘幅度
            encoding::safe_yaml::try_parse_field(node, "low_opening_amplitude", param.LowOpeningAmplitude);
            encoding::safe_yaml::try_parse_field(node, "high_opening_amplitude", param.HighOpeningAmplitude);

            // 规则配置
            if (node["rules"] && node["rules"].IsMap()) {
                quant1x::config::RuleParameter rules;
                if (convert<quant1x::config::RuleParameter>::decode(node["rules"], rules)) {
                    param.Rules = rules;
                }
            }

            // 排除代码
            if (node["exclude_codes"] && node["exclude_codes"].IsSequence()) {
                param.excludeCodes.clear();
                for (const auto& item : node["exclude_codes"]) {
                    std::string code;
                    if (item.IsScalar()) {
                        code = item.as<std::string>();
                        param.excludeCodes.push_back(std::move(code));
                    }
                }
            }

            return true;
        }
    };
} // namespace YAML

#endif //QUANT1X_CONFIG_DETAIL_STRATEGY_PARAMETER_H
