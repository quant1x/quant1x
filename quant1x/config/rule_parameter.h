#pragma once
#ifndef QUANT1X_CONFIG_DETAIL_RULE_PARAMETER_H
#define QUANT1X_CONFIG_DETAIL_RULE_PARAMETER_H 1

#include <quant1x/encoding/yaml.h>
#include <quant1x/std/strings.h>
#include <quant1x/std/numeric.h>

namespace config {

    // RuleParameter 规则参数
    struct RuleParameter {
        // 是否启用板块过滤, false代表全市场扫描
        bool SectorsFilter = false;

        // 最多关联多少个板块, 默认3个
        int SectorsTopN = 3;

        // 板块内个股排名前N
        int StockTopNInSector = 5;

        // 忽略规则组合
        std::vector<int> IgnoreRuleGroup;

        // 忽略的证券代码段, 默认忽略科创板和北交所全部
        std::vector<std::string> IgnoreCodes = {"sh68", "bj"};

        // 20.00 5日累计最大涨幅
        double MaximumIncreaseWithin5days = 20.00;

        // 70.00 10日累计最大涨幅
        double MaximumIncreaseWithin10days = 70.00;

        // -1000 最大流出1000万
        double MaxReduceAmount = -1000;

        // 80 通达信安全分最小值
        numeric::number_range<int> SafetyScore = numeric::number_range<int>(80);

        // 1.800 成交量放大不能超过1.8
        numeric::number_range<double> VolumeRatio = numeric::number_range<double>(0.382, 2.800);

        // 流通股本, 默认0.5亿~20亿
        numeric::number_range<double> Capital = numeric::number_range<double>(0.5, 20);

        // 流通市值, 默认4亿~600亿
        numeric::number_range<double> MarketCap = numeric::number_range<double>(4, 600);

        // 股价: 4.9E-324~1.7976931348623157e+308
        numeric::number_range<double> Price = numeric::number_range<double>(2);

        // 开盘涨幅, 默认不限制
        numeric::number_range<double> OpenChangeRate = numeric::number_range<double>();

        // 开盘量比, 默认不限制
        numeric::number_range<double> OpenQuantityRatio = numeric::number_range<double>();

        // 开盘换手, 默认不限制
        numeric::number_range<double> OpenTurnZ = numeric::number_range<double>();

        // 盘中策略涨幅范围, 默认不限制
        numeric::number_range<double> ChangeRate = numeric::number_range<double>();

        // 波动率, 默认不限制
        numeric::number_range<double> Vix = numeric::number_range<double>();

        // 换手率范围, 默认不限制
        numeric::number_range<double> TurnoverRate = numeric::number_range<double>();

        // 振幅范围, 默认不限制
        numeric::number_range<double> AmplitudeRatio = numeric::number_range<double>();

        // 5档行情委托平均值范围, 默认不限制
        numeric::number_range<double> BiddingVolume = numeric::number_range<double>();

        // 情绪范围
        numeric::number_range<double> Sentiment = numeric::number_range<double>();

        // 买入是否允许跳空低开, 默认是允许
        bool GapDown = true;

        // 是否检测每股收益, 默认不检测
        bool CheckEPS = false;

        // 是否检测每股净资产, 默认不检测
        bool CheckBPS = false;

        // 是否检测安全分
        bool CheckSafetyScore = false;

        // 融资余额占比阀值, 过滤超过阀值的标的
        double FinancingBalanceRatio = 10;

        // 冗详模式
        bool Verbose = false;

        // 构造函数, 初始化NumberRange字段
        RuleParameter() {
            SafetyScore = numeric::number_range<int>("80~");
            VolumeRatio = numeric::number_range<double>("0.382~2.800");
            Capital = numeric::number_range<double>("0.5~20");
            MarketCap = numeric::number_range<double>("4~600");
            Price = numeric::number_range<double>("2~");
            Sentiment = numeric::number_range<double>("38.2~61.80");
        }

        friend std::ostream &operator<<(std::ostream &os, const RuleParameter &parameter) {
            os << "SectorsFilter: " << parameter.SectorsFilter
               << " SectorsTopN: " << parameter.SectorsTopN
               << " StockTopNInSector: " << parameter.StockTopNInSector
               << " IgnoreRuleGroup: " << parameter.IgnoreRuleGroup
               << " IgnoreCodes: " << parameter.IgnoreCodes
               << " MaximumIncreaseWithin5days: " << parameter.MaximumIncreaseWithin5days
               << " MaximumIncreaseWithin10days: " << parameter.MaximumIncreaseWithin10days
               << " MaxReduceAmount: " << parameter.MaxReduceAmount
               << " SafetyScore: " << parameter.SafetyScore
               << " VolumeRatio: " << parameter.VolumeRatio
               << " Capital: " << parameter.Capital
               << " MarketCap: " << parameter.MarketCap
               << " Price: " << parameter.Price
               << " OpenChangeRate: " << parameter.OpenChangeRate
               << " OpenQuantityRatio: " << parameter.OpenQuantityRatio
               << " OpenTurnZ: " << parameter.OpenTurnZ
               << " ChangeRate: " << parameter.ChangeRate
               << " Vix: " << parameter.Vix
               << " TurnoverRate: " << parameter.TurnoverRate
               << " AmplitudeRatio: " << parameter.AmplitudeRatio
               << " BiddingVolume: " << parameter.BiddingVolume
               << " Sentiment: " << parameter.Sentiment
               << " GapDown: " << parameter.GapDown
               << " CheckEPS: " << parameter.CheckEPS
               << " CheckBPS: " << parameter.CheckBPS
               << " CheckSafetyScore: " << parameter.CheckSafetyScore
               << " FinancingBalanceRatio: " << parameter.FinancingBalanceRatio
               << " Verbose: " << parameter.Verbose;
            return os;
        }
    };

} // namespace config

namespace YAML {

    using namespace numeric;

    struct number_range_yaml_parser {
        template <typename T>
        static void parse(const YAML::Node& node, number_range<T>& range) {
            if (node.IsScalar()) {
                range = number_range<T>(node.as<std::string>());
            } else if (node.IsMap()) {
                if (node["min"]) range.min_ = node["min"].as<T>();
                if (node["max"]) range.max_ = node["max"].as<T>();
            }
        }
    };


    template <typename T>
    void parse_yaml_range(const YAML::Node& node, const std::string& key, number_range<T>& out) {
        if (!node[key]) return;

        if (node[key].IsScalar()) {
            auto range_str = node[key].as<std::string>();
            out = number_range<T>(range_str);
        } else if (node[key].IsMap()) {
            out = node[key].as<number_range<T>>();
        }
    }

    // 重载 operator >>
    template <typename T>
    struct convert<number_range<T>> {
        static Node encode(const number_range<T>& rhs) {
            Node node;
            node["min"] = rhs.min_;
            node["max"] = rhs.max_;
            return node;
        }

        static bool decode(const Node& node, number_range<T>& rhs) {
            number_range_yaml_parser::parse(node, rhs);
            return true;
        }
    };

    // RuleParameter转换
    template<>
    struct convert<config::RuleParameter> {
        static bool decode(const Node& node, config::RuleParameter& rule) {
            if (!node.IsMap()) return false;

            // 基础配置
            encoding::safe_yaml::try_parse_field(node, "sectors_filter", rule.SectorsFilter);
            encoding::safe_yaml::try_parse_field(node, "sectors_top_n", rule.SectorsTopN);
            encoding::safe_yaml::try_parse_field(node, "stock_top_n_in_sector", rule.StockTopNInSector);

            // 忽略规则
            if (node["ignore_rule_group"] && node["ignore_rule_group"].IsSequence()) {
                rule.IgnoreRuleGroup.clear();
                for (const auto& item : node["ignore_rule_group"]) {
                    if (item.IsScalar()) {
                        rule.IgnoreRuleGroup.push_back(item.as<int>());
                    }
                }
            }

            if (node["ignore_codes"] && node["ignore_codes"].IsSequence()) {
                rule.IgnoreCodes.clear();
                for (const auto& item : node["ignore_codes"]) {
                    if (item.IsScalar()) {
                        rule.IgnoreCodes.push_back(item.as<std::string>());
                    }
                }
            }

            // 涨幅限制
            encoding::safe_yaml::try_parse_field(node, "maximum_increase_within_5d", rule.MaximumIncreaseWithin5days);
            encoding::safe_yaml::try_parse_field(node, "maximum_increase_within_10d", rule.MaximumIncreaseWithin10days);
            encoding::safe_yaml::try_parse_field(node, "max_reduce_amount", rule.MaxReduceAmount);

            // 数值范围配置(逐个处理NumberRange字段)
            if (node["safety_score"]) {
                number_range<int> nr;
                if (convert<number_range<int>>::decode(node["safety_score"], nr)) {
                    rule.SafetyScore = nr;
                }
            }
            if (node["volume_ratio"]) {
                number_range<double> nr;
                if (convert<number_range<double>>::decode(node["volume_ratio"], nr)) {
                    rule.VolumeRatio = nr;
                }
            }
            if (node["capital"]) {
                number_range<double> nr;
                if (convert<number_range<double>>::decode(node["capital"], nr)) {
                    rule.Capital = nr;
                }
            }
            if (node["market_cap"]) {
                number_range<double> nr;
                if (convert<number_range<double>>::decode(node["market_cap"], nr)) {
                    rule.MarketCap = nr;
                }
            }
            if (node["price"]) {
                number_range<double> nr;
                if (convert<number_range<double>>::decode(node["price"], nr)) {
                    rule.Price = nr;
                }
            }
            if (node["open_change_rate"]) {
                number_range<double> nr;
                if (convert<number_range<double>>::decode(node["open_change_rate"], nr)) {
                    rule.OpenChangeRate = nr;
                }
            }
            if (node["open_quantity_ratio"]) {
                number_range<double> nr;
                if (convert<number_range<double>>::decode(node["open_quantity_ratio"], nr)) {
                    rule.OpenQuantityRatio = nr;
                }
            }
            if (node["open_turn_z"]) {
                number_range<double> nr;
                if (convert<number_range<double>>::decode(node["open_turn_z"], nr)) {
                    rule.OpenTurnZ = nr;
                }
            }
            if (node["change_rate"]) {
                number_range<double> nr;
                if (convert<number_range<double>>::decode(node["change_rate"], nr)) {
                    rule.ChangeRate = nr;
                }
            }
            if (node["vix"]) {
                number_range<double> nr;
                if (convert<number_range<double>>::decode(node["vix"], nr)) {
                    rule.Vix = nr;
                }
            }
            if (node["turnover_rate"]) {
                number_range<double> nr;
                if (convert<number_range<double>>::decode(node["turnover_rate"], nr)) {
                    rule.TurnoverRate = nr;
                }
            }
            if (node["amplitude_ratio"]) {
                number_range<double> nr;
                if (convert<number_range<double>>::decode(node["amplitude_ratio"], nr)) {
                    rule.AmplitudeRatio = nr;
                }
            }
            if (node["bidding_volume"]) {
                number_range<double> nr;
                if (convert<number_range<double>>::decode(node["bidding_volume"], nr)) {
                    rule.BiddingVolume = nr;
                }
            }
            if (node["sentiment"]) {
                number_range<double> nr;
                if (convert<number_range<double>>::decode(node["sentiment"], nr)) {
                    rule.Sentiment = nr;
                }
            }

            // 交易控制
            encoding::safe_yaml::try_parse_field(node, "gap_down", rule.GapDown);
            encoding::safe_yaml::try_parse_field(node, "check_eps", rule.CheckEPS);
            encoding::safe_yaml::try_parse_field(node, "check_bps", rule.CheckBPS);
            encoding::safe_yaml::try_parse_field(node, "check_safety_score", rule.CheckSafetyScore);
            encoding::safe_yaml::try_parse_field(node, "financing_balance_ratio", rule.FinancingBalanceRatio);
            encoding::safe_yaml::try_parse_field(node, "verbose", rule.Verbose);

            return true;
        }
    };
} // namespace YAML

#endif //QUANT1X_CONFIG_DETAIL_RULE_PARAMETER_H
