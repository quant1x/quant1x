#include <quant1x/command.h>
#include <quant1x/data/adapter.h>
#include <quant1x/cache.h>

namespace quant1x {

    // 更新数据, 只更新最近一个交易日的数据
    int update(argparse::ArgumentParser &sub_parser) {
        std::vector<data::DataAdapter*> adapters;
        std::cout << "全部数据 = " << updateAll.value << "," << sub_parser.is_used(cmd_flag_all) << std::endl;
        std::cout << "基础数据 = " << updateBase.value << "," << sub_parser.is_used(cmd_flag_base) << std::endl;
        std::cout << "特征数据 = " << updateFeatures.value << "," << sub_parser.is_used(cmd_flag_features) << std::endl;

        // 全部数据
        if(sub_parser.is_used(cmd_flag_all)) {
            auto all = data::Plugins();
            adapters.insert(adapters.end(), all.begin(), all.end());
        } else {
            // 判断基础数据
            if (sub_parser.is_used(cmd_flag_base)) {
                std::vector<data::DataAdapter *> base;
                if (updateBase.value == default_all) {
                    base = data::Plugins(data::PluginMaskBaseData);
                } else {
                    std::vector<std::string> names = strings::split(updateBase.value, ",");
                    base = data::PluginsWithName(data::PluginMaskBaseData, names);
                }
                adapters.insert(adapters.end(), base.begin(), base.end());
            }

            // 判断特征数据
            if (sub_parser.is_used(cmd_flag_features)) {
                std::vector<data::DataAdapter *> features;
                if (updateFeatures.value == default_all) {
                    features = data::Plugins(data::PluginMaskFeature);
                } else {
                    std::vector<std::string> names = strings::split(updateFeatures.value, ",");
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
        std::string tmp_start_date = meta::last_trading_day().only_date();
        if(sub_parser.is_used(cmd_flag_start)) {
            tmp_start_date = updateStartDate.value;
        }
        std::cout << "开始日期 = " << tmp_start_date << std::endl;
        // 判断结束日期
        std::string tmp_end_date = meta::last_trading_day().only_date();
        if(sub_parser.is_used(cmd_flag_end)) {
            tmp_end_date = updateEndDate.value;
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
//            // 以传入的结束日期为特征日期, 矫正结束日期为最近一个有数据的交易日
//            end_date = meta::last_trading_day(end_date);
        }
        auto const & dates = meta::date_range(start_date, end_date);
        int count = 0;
        size_t length = dates.size();
        if(length > 0) {
            //std::cout << length << std::endl;
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