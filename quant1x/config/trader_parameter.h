#pragma once
#ifndef QUANT1X_CONFIG_DETAIL_TRADER_PARAMETER_H
#define QUANT1X_CONFIG_DETAIL_TRADER_PARAMETER_H 1

#include <quant1x/encoding/yaml.h>
#include <string>
#include <vector>
#include <algorithm>
#include <ostream>
#include "trading_session.h"
#include "strategy_parameter.h"
#include "price_cage.h"

namespace quant1x::config {

    // TraderRole 交易员角色
    enum class TraderRole {
        RoleDisable = 0, // 禁止自动化交易
        RolePython = 1,  // python脚本自动化交易
        RoleProxy = 2,   // 代理交易模式
        RoleManual = 3   // 人工干预
    };

    // TraderParameter 预览交易通道参数
    struct TraderParameter {
        // 账号ID
        std::string AccountId = "888xxxxxxx";

        // 订单路径
        std::string OrderPath;

        // 最多输出前多少名个股
        int TopN = 3;

        // 是否包含ETF
        bool HaveETF = false;

        // 价格笼子比例, 默认0%
        double PriceCageRatio = quant1x::config::ValidDeclarationPriceRange;

        // 价格最小变动单位, 默认0.00
        double MinimumPriceFluctuationUnit = quant1x::config::MinimumPriceFluctuationUnit;

        // 卖出滑点比例, 默认0.01
        double FixedSlippageForSell = quant1x::config::FixedSlippageForSell;

        // 2024年2月18日建设银行1年期存款利率1.65%
        double AnnualInterestRate = 1.65;

        // 印花税-买入, 没有
        double StampDutyRateForBuy = 0.0000;

        // 印花税-卖出, 默认是千分之1
        double StampDutyRateForSell = 0.0010;

        // 过户费, 双向, 默认是万分之6
        double TransferRate = 0.0006;

        // 券商佣金, 双向, 默认万分之2.5
        double CommissionRate = 0.00025;

        // 券商佣金最低, 双向, 默认5.00
        double CommissionMin = 5.0000;

        // 当日持仓占比, 默认50%
        double PositionRatio = 0.5000;

        // 保留现金, 默认10000.00
        double KeepCash = 10000.00;

        // 买入最大金额, 默认250000.00
        double BuyAmountMax = 250000.00;

        // 买入最小金额, 默认1000.00
        double BuyAmountMin = 1000.00;

        // 交易员角色, 默认是需要人工干预, 系统不做自动交易处理
        TraderRole Role = TraderRole::RoleManual;

        // 代理URL, 禁止使用公网地址
        std::string ProxyUrl = "http://127.0.0.1:18168/qmt";

        // 策略集合
        std::vector<StrategyParameter> Strategies;

        // 撤单时段配置
        TradingSession CancelSession;

        // 竞价承接强度
        double UndertakeRatio = 0.8000;

        friend std::ostream &operator<<(std::ostream &os, const TraderParameter &parameter);

    public:
        TraderParameter() = default;

        TraderParameter& operator=(const TraderParameter& other) {
            if (this != &other) {
                // 字符串成员
                AccountId = other.AccountId;
                OrderPath = other.OrderPath;
                ProxyUrl = other.ProxyUrl;

                // 基本数值类型
                TopN = other.TopN;
                HaveETF = other.HaveETF;
                PriceCageRatio = other.PriceCageRatio;
                MinimumPriceFluctuationUnit = other.MinimumPriceFluctuationUnit;
                AnnualInterestRate = other.AnnualInterestRate;
                StampDutyRateForBuy = other.StampDutyRateForBuy;
                StampDutyRateForSell = other.StampDutyRateForSell;
                TransferRate = other.TransferRate;
                CommissionRate = other.CommissionRate;
                CommissionMin = other.CommissionMin;
                PositionRatio = other.PositionRatio;
                KeepCash = other.KeepCash;
                BuyAmountMax = other.BuyAmountMax;
                BuyAmountMin = other.BuyAmountMin;
                UndertakeRatio = other.UndertakeRatio;

                // 枚举类型
                Role = other.Role;

                // 容器成员
                Strategies = other.Strategies;

                // 对象成员
                CancelSession = other.CancelSession;
            }
            return *this;
        }

        int TotalNumberOfTargets() const;
        void ResetPositionRatio();

        double DailyRiskFreeRate(const std::string &date) const;

        // 通过策略编码查找规则
        std::optional<StrategyParameter> GetStrategyParameterByCode(uint64_t strategyCode) const;
    };

} // namespace quant1x::config

namespace YAML {
    // TraderRole枚举转换
    template<>
    struct convert<quant1x::config::TraderRole> {
        static Node encode(const quant1x::config::TraderRole& role) {
            static const std::map<quant1x::config::TraderRole, std::string> names = {
                {quant1x::config::TraderRole::RoleDisable, "disable"},
                {quant1x::config::TraderRole::RolePython, "python"},
                {quant1x::config::TraderRole::RoleProxy, "proxy"},
                {quant1x::config::TraderRole::RoleManual, "manual"}
            };
            return Node(names.at(role));
        }

        static bool decode(const Node& node, quant1x::config::TraderRole& role) {
            static const std::map<std::string, quant1x::config::TraderRole> roles = {
                {"disable", quant1x::config::TraderRole::RoleDisable},
                {"python", quant1x::config::TraderRole::RolePython},
                {"proxy", quant1x::config::TraderRole::RoleProxy},
                {"manual", quant1x::config::TraderRole::RoleManual}
            };

            std::string roleStr = node.as<std::string>();
            std::transform(roleStr.begin(), roleStr.end(), roleStr.begin(), ::tolower);

            auto it = roles.find(roleStr);
            if (it != roles.end()) {
                role = it->second;
                return true;
            }
            return false;
        }
    };

    // 主配置转换
    template<>
    struct convert<quant1x::config::TraderParameter> {
        static bool decode(const Node& node, quant1x::config::TraderParameter& param) {
            // 基本参数
            encoding::safe_yaml::try_parse_field(node, "account_id", param.AccountId);
            encoding::safe_yaml::try_parse_field(node, "order_path", param.OrderPath);
            encoding::safe_yaml::try_parse_field(node, "top_n", param.TopN);
            encoding::safe_yaml::try_parse_field(node, "have_etf", param.HaveETF);

            // 价格相关
            encoding::safe_yaml::try_parse_field(node, "price_cage_ratio", param.PriceCageRatio);
            encoding::safe_yaml::try_parse_field(node, "minimum_price_fluctuation_unit", param.MinimumPriceFluctuationUnit);
            encoding::safe_yaml::parse_field(node, "fixed_slippage_for_sell", param.FixedSlippageForSell, quant1x::config::FixedSlippageForSell);

            // 费率相关
            encoding::safe_yaml::try_parse_field(node, "annual_interest_rate", param.AnnualInterestRate);
            encoding::safe_yaml::try_parse_field(node, "stamp_duty_rate_for_buy", param.StampDutyRateForBuy);
            encoding::safe_yaml::try_parse_field(node, "stamp_duty_rate_for_sell", param.StampDutyRateForSell);
            encoding::safe_yaml::try_parse_field(node, "transfer_rate", param.TransferRate);
            encoding::safe_yaml::try_parse_field(node, "commission_rate", param.CommissionRate);
            encoding::safe_yaml::try_parse_field(node, "commission_min", param.CommissionMin);

            // 仓位控制
            encoding::safe_yaml::try_parse_field(node, "position_ratio", param.PositionRatio);
            encoding::safe_yaml::try_parse_field(node, "keep_cash", param.KeepCash);
            encoding::safe_yaml::try_parse_field(node, "buy_amount_max", param.BuyAmountMax);
            encoding::safe_yaml::try_parse_field(node, "buy_amount_min", param.BuyAmountMin);

            // 交易模式
            if (node["role"]) {
                quant1x::config::TraderRole role = param.Role; // 保持原默认值
                if (convert<quant1x::config::TraderRole>::decode(node["role"], role)) {
                    param.Role = role;
                }
            }
            encoding::safe_yaml::try_parse_field(node, "proxy_url", param.ProxyUrl);

            // 策略配置
            if (node["strategies"] && node["strategies"].IsSequence()) {
                param.Strategies.clear();
                for (const auto& item : node["strategies"]) {
                    quant1x::config::StrategyParameter sp;
                    if (item.IsMap() && convert<quant1x::config::StrategyParameter>::decode(item, sp)) {
                        param.Strategies.push_back(std::move(sp));
                    }
                }
            }

            // 交易时段
            if (node["cancel"] && node["cancel"].IsMap()) {
                quant1x::config::TradingSession session;
                if (convert<quant1x::config::TradingSession>::decode(node["cancel"], session)) {
                    param.CancelSession = session;
                }
            }

            // 其他参数
            encoding::safe_yaml::try_parse_field(node, "undertake_ratio", param.UndertakeRatio);

            return true;
        }
    };
} // namespace YAML

#endif //QUANT1X_CONFIG_DETAIL_TRADER_PARAMETER_H
