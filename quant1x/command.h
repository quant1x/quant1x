// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#pragma once
#ifndef QUANT1X_COMMAND_H
#define QUANT1X_COMMAND_H 1

//============================================================
// quant1x命令字                                             //
//============================================================

// 先于 Windows 头文件引入: 避免 min/max/OPTIONAL 等同名宏污染 argparser 的声明
#include <quant1x/base/argparser/parser.hpp>

#include <quant1x/base/api.h>

#include <boost/pfr/core.hpp>
#include <csv2/writer.hpp>
#include <indicators/dynamic_progress.hpp>
#include <indicators/progress_bar.hpp>

#include <quant1x/data/meta/timestamp.h>

namespace quant1x {

    namespace meta = quant1x::data::meta;

    constexpr const char *const default_all       = "all";         ///< 默认值, 表示全部数据
    constexpr const char *const cmd_flag_all      = "--all";       ///< 命令行标志, 用于表示全部数据
    constexpr const char *const cmd_flag_base     = "--base";      ///< 命令行标志, 用于表示基础数据
    constexpr const char *const cmd_flag_features = "--features";  ///< 命令行标志, 用于表示特征数据
    constexpr const char *const cmd_flag_start    = "--start";     ///< 命令行标志, 用于表示开始日期
    constexpr const char *const cmd_flag_end      = "--end";       ///< 命令行标志, 用于表示结束日期

    /// 一个子命令选项的声明: 名字 / 默认值 / 说明 / 是否开关
    struct cmdFlags {
        std::string use;                       ///< 选项名, 含前导 `--`
        std::string defaultValue;              ///< 默认值; 开关项忽略
        std::string usage;                     ///< 帮助里的说明
        bool        is_switch = false;         ///< 为真时登记成 `store_true`

        friend std::ostream &operator<<(std::ostream &os, const cmdFlags &flags) {
            os << "use: " << flags.use << " defaultValue: " << flags.defaultValue
               << " usage: " << flags.usage << " switch: " << flags.is_switch;
            return os;
        }
    };

    // 定义子命令的结构体
    struct SubCommand {
        std::string                                                          name;     // 子命令名称
        std::string                                                          help;     // 子命令的帮助信息
        std::function<void(::argparser::Parser &, const ::argparser::Namespace &)> handler;  // 子命令的处理逻辑
        std::vector<cmdFlags *>                                              args;
    };

    inline cmdFlags updateAll = {.use = cmd_flag_all, .defaultValue = "", .usage = "全部", .is_switch = true};
    inline cmdFlags updateBase = {.use = cmd_flag_base, .defaultValue = "", .usage = "基础数据"};

    inline cmdFlags updateFeatures = {
        .use = cmd_flag_features, .defaultValue = "", .usage = "特征数据"};

    inline cmdFlags updateStartDate = {.use          = cmd_flag_start,
                                       .defaultValue = meta::Timestamp::now().only_date(),
                                       .usage        = "开始日期"};

    inline cmdFlags updateEndDate = {
        .use = cmd_flag_end, .defaultValue = meta::Timestamp::now().only_date(), .usage = "结束日期"};

    inline std::vector<cmdFlags *> updateFlags = {
        &updateAll,
        &updateBase,
        &updateFeatures,
        &updateStartDate,
        &updateEndDate,
    };

    int update(::argparser::Parser &sub_parser, const ::argparser::Namespace &namespace_value);

    // 定义子命令及其逻辑
    inline std::vector<quant1x::SubCommand> subcommands = {{
        .name    = "update",    // 子命令名称
        .help    = "更新数据",  // 子命令帮助信息
        .handler = quant1x::update,
        .args    = quant1x::updateFlags,
    }};

}  // namespace quant1x

#endif  // QUANT1X_COMMAND_H
