#include <quant1x/command.h>
#include <quant1x/cache/adapter.h>
#include <quant1x/cache.h>

namespace quant1x {

    // 更新数据, 只更新最近一个交易日的数据
    int update(argparse::ArgumentParser &sub_parser) {
        std::vector<cache::DataAdapter*> adapters;
        std::cout << "全部数据 = " << updateAll.value << "," << sub_parser.is_used(cmd_flag_all) << std::endl;
        std::cout << "基础数据 = " << updateBase.value << "," << sub_parser.is_used(cmd_flag_base) << std::endl;
        std::cout << "特征数据 = " << updateFeatures.value << "," << sub_parser.is_used(cmd_flag_features) << std::endl;

        // 全部数据
        if(sub_parser.is_used(cmd_flag_all)) {
            auto all = cache::Plugins();
            adapters.insert(adapters.end(), all.begin(), all.end());
        } else {
            // 判断基础数据
            if (sub_parser.is_used(cmd_flag_base)) {
                std::vector<cache::DataAdapter *> base;
                if (updateBase.value == default_all) {
                    base = cache::Plugins(cache::PluginMaskBaseData);
                } else {
                    std::vector<std::string> names = strings::split(updateBase.value, ",");
                    base = cache::PluginsWithName(cache::PluginMaskBaseData, names);
                }
                adapters.insert(adapters.end(), base.begin(), base.end());
            }

            // 判断特征数据
            if (sub_parser.is_used(cmd_flag_features)) {
                std::vector<cache::DataAdapter *> features;
                if (updateFeatures.value == default_all) {
                    features = cache::Plugins(cache::PluginMaskFeature);
                } else {
                    std::vector<std::string> names = strings::split(updateFeatures.value, ",");
                    features = cache::PluginsWithName(cache::PluginMaskFeature, names);
                }
                adapters.insert(adapters.end(), features.begin(), features.end());
            }
        }
        if(adapters.empty()) {
            spdlog::warn("没有需要更新的数据适配器");
            return 0;
        }
        // 判断开始日期
        std::string tmp_start_date = exchange::last_trading_day().only_date();
        if(sub_parser.is_used(cmd_flag_start)) {
            tmp_start_date = updateStartDate.value;
        }
        std::cout << "开始日期 = " << tmp_start_date << std::endl;
        // 判断结束日期
        std::string tmp_end_date = exchange::last_trading_day().only_date();
        if(sub_parser.is_used(cmd_flag_end)) {
            tmp_end_date = updateEndDate.value;
        }
        std::cout << "结束日期 = " << tmp_end_date << std::endl;
        // 标准化日期
        exchange::timestamp start_date = exchange::timestamp::parse(tmp_start_date).pre_market_time();
        start_date = exchange::last_trading_day(start_date);
        exchange::timestamp end_date = exchange::timestamp::parse(tmp_end_date).pre_market_time();
        end_date = exchange::last_trading_day(end_date);
        // 矫正日期
        if(!start_date.is_same_date(end_date)) {
            // 以传入的开始日期为cache日期, 特征日期要取前一天
            start_date = exchange::prev_trading_day(start_date);
//            // 以传入的结束日期为特征日期, 矫正结束日期为最近一个有数据的交易日
//            end_date = exchange::last_trading_day(end_date);
        }
        auto const & dates = exchange::date_range(start_date, end_date);
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