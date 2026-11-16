// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#include <quant1x/command.h>
#include <quant1x/data/adapter.h>
#include <quant1x/cache.h>

namespace quant1x {

    namespace {
        /// 取选项的文本取值; 未给出时为空
        std::optional<std::string> option_text(const ::argparser::Namespace &namespace_value, std::string_view key) {
            const auto *value = namespace_value.find(key);
            if (value == nullptr) {
                return std::nullopt;
            }
            const auto *text = value->as_string();
            if (text == nullptr) {
                return std::nullopt;
            }
            return *text;
        }

        /// 取开关的真值; 未给出或不是布尔时返回 false
        bool option_flag(const ::argparser::Namespace &namespace_value, std::string_view key) {
            const auto *value = namespace_value.find(key);
            if (value == nullptr) {
                return false;
            }
            const auto *flag = value->as_bool();
            return flag != nullptr && *flag;
        }
    }  // namespace

    // 更新数据, 只更新最近一个交易日的数据
    int update([[maybe_unused]] ::argparser::Parser &sub_parser, const ::argparser::Namespace &namespace_value) {
        std::vector<data::DataAdapter*> adapters;
        const bool                  all_enabled    = option_flag(namespace_value, "all");
        const std::optional<std::string> base_value     = option_text(namespace_value, "base");
        const std::optional<std::string> features_value = option_text(namespace_value, "features");
        std::cout << "全部数据 = " << all_enabled << std::endl;
        std::cout << "基础数据 = " << base_value.value_or("") << std::endl;
        std::cout << "特征数据 = " << features_value.value_or("") << std::endl;

        // 全部数据
        if(all_enabled) {
            auto all = data::Plugins();
            adapters.insert(adapters.end(), all.begin(), all.end());
        } else {
            // 判断基础数据
            if (base_value.has_value()) {
                std::vector<data::DataAdapter *> base;
                if (base_value->empty() || *base_value == default_all) {
                    base = data::Plugins(data::PluginMaskBaseData);
                } else {
                    std::vector<std::string> names = strings::split(*base_value, ",");
                    base = data::PluginsWithName(data::PluginMaskBaseData, names);
                }
                adapters.insert(adapters.end(), base.begin(), base.end());
            }

            // 判断特征数据
            if (features_value.has_value()) {
                std::vector<data::DataAdapter *> features;
                if (features_value->empty() || *features_value == default_all) {
                    features = data::Plugins(data::PluginMaskFeature);
                } else {
                    std::vector<std::string> names = strings::split(*features_value, ",");
                    features = data::PluginsWithName(data::PluginMaskFeature, names);
                }
                adapters.insert(adapters.end(), features.begin(), features.end());
            }
        }
        if(adapters.empty()) {
            spdlog::warn("没有需要更新的数据适配器");
            return 0;
        }
        // 判断开始日期
        std::string tmp_start_date = option_text(namespace_value, "start").value_or("");
        if (tmp_start_date.empty()) {
            tmp_start_date = meta::last_trading_day().only_date();
        }
        std::cout << "开始日期 = " << tmp_start_date << std::endl;
        // 判断结束日期
        std::string tmp_end_date = option_text(namespace_value, "end").value_or("");
        if (tmp_end_date.empty()) {
            tmp_end_date = meta::last_trading_day().only_date();
        }
        std::cout << "结束日期 = " << tmp_end_date << std::endl;
        // 标准化日期
        meta::Timestamp start_date = meta::Timestamp::parse(tmp_start_date).pre_market_time();
        start_date = meta::last_trading_day(start_date);
        meta::Timestamp end_date = meta::Timestamp::parse(tmp_end_date).pre_market_time();
        end_date = meta::last_trading_day(end_date);
        // 矫正日期
        if(!start_date.is_same_date(end_date)) {
            // 以传入的开始日期为cache日期, 特征日期要取前一天
            start_date = meta::prev_trading_day(start_date);
        }
        auto const & dates = meta::date_range(start_date, end_date);
        int count = 0;
        size_t length = dates.size();
        if(length > 0) {
            fmt::println("from: {} to {}, count={}", dates[0].only_date(), dates[length - 1].only_date(), length);
            for (size_t i = 0; i < length; ++i) {
                auto const &timestamp = dates[i];
                fmt::println("sample date: {}({}/{})", timestamp.only_date(), i + 1, length);
                count += cache::update_with_adapters(adapters, timestamp);
            }
        } else {
            std::cout << "日期范围没有交易数据" << std::endl;
        }

        return count;
    }

} // namespace quant1x
