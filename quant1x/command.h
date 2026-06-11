#pragma once
#ifndef QUANT1X_COMMAND_H
#define QUANT1X_COMMAND_H 1

//============================================================
// quant1x命令字                                             //
//============================================================

#include <quant1x/std/api.h>

#include <argparse/argparse.hpp>
#include <boost/pfr/core.hpp>
#include <csv2/writer.hpp>
#include <indicators/dynamic_progress.hpp>
#include <indicators/progress_bar.hpp>

#include <quant1x/data/market/instruments.h>

namespace quant1x {

    constexpr const char *const default_all       = "all";         ///< 默认值，表示全部数据
    constexpr const char *const cmd_flag_all      = "--all";       ///< 命令行标志，用于表示全部数据
    constexpr const char *const cmd_flag_base     = "--base";      ///< 命令行标志，用于表示基础数据
    constexpr const char *const cmd_flag_features = "--features";  ///< 命令行标志，用于表示特征数据
    constexpr const char *const cmd_flag_start    = "--start";     ///< 命令行标志，用于表示开始日期
    constexpr const char *const cmd_flag_end      = "--end";       ///< 命令行标志，用于表示结束日期

    struct cmdFlags {
        std::string use;
        std::string value;
        std::string defaultValue;
        std::string usage;

        friend std::ostream &operator<<(std::ostream &os, const cmdFlags &flags) {
            os << "use: " << flags.use << " value: " << flags.value << " defaultValue: " << flags.defaultValue
               << " usage: " << flags.usage;
            return os;
        }
    };

    // 定义子命令的结构体
    struct SubCommand {
        std::string                                     name;     // 子命令名称
        std::string                                     help;     // 子命令的帮助信息
        std::function<void(argparse::ArgumentParser &)> handler;  // 子命令的处理逻辑
        std::vector<cmdFlags *>                         args;
    };

    inline cmdFlags updateAll  = {.use = cmd_flag_all, .value = "", .defaultValue = "", .usage = "全部"};
    inline cmdFlags updateBase = {.use = cmd_flag_base, .value = "", .defaultValue = default_all, .usage = "基础数据"};

    inline cmdFlags updateFeatures = {
        .use = cmd_flag_features, .value = "", .defaultValue = default_all, .usage = "特征数据"};

    inline cmdFlags updateStartDate = {.use          = cmd_flag_start,
                                       .value        = "",
                                       .defaultValue = exchange::timestamp::now().only_date(),
                                       .usage        = "开始日期"};

    inline cmdFlags updateEndDate = {
        .use = cmd_flag_end, .value = "", .defaultValue = exchange::timestamp::now().only_date(), .usage = "结束日期"};

    inline std::vector<cmdFlags *> updateFlags = {
        &updateAll,
        &updateBase,
        &updateFeatures,
        &updateStartDate,
        &updateEndDate,
    };

    int update(argparse::ArgumentParser &sub_parser);

    // 定义子命令及其逻辑
    inline std::vector<quant1x::SubCommand> subcommands = {{
        .name    = "update",    // 子命令名称
        .help    = "更新数据",  // 子命令帮助信息
        .handler = quant1x::update,
        .args    = quant1x::updateFlags,
    }};

}  // namespace quant1x

#endif  // QUANT1X_COMMAND_H
