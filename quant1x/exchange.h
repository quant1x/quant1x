#pragma once
#ifndef QUANT1X_EXCHANGE_H
#define QUANT1X_EXCHANGE_H 1

#include <quant1x/exchange/security.h>
#include <quant1x/runtime/config.h>
#include <quant1x/runtime/core.h>
#include <quant1x/runtime/once.h>
#include <quant1x/exchange/code.h>
#include <quant1x/exchange/calendar.h>
#include <quant1x/exchange/blocks.h>
#include <quant1x/exchange/margin-trading.h>
#include <quant1x/level1/client.h>
#include <quant1x/io/http.h>
#include <quant1x/io/csv-writer.h>
#include <quant1x/io/file.h>
#include <quant1x/std/time.h>
#include <string>
#include <random>
#include <vector>
#include <optional>
#include <chrono>
#include <format> // for C++20的格式化方法
#include <iostream>
#include <array>
#include <string_view>

//============================================================
// exchange 证券代码整合                                      //
//============================================================

namespace exchange {

    // A股指数列表
    static const std::vector<std::string> AShareIndexList = {
            "sh000001", // 上证综合指数
            "sh000002", // 上证A股指数
            "sh000300", // 沪深300指数
            "sh000688", // 科创50指数
            "sh000905", // 中证500指数
            "sz399001", // 深证成份指数
            "sz399006", // 创业板指
            "sz399107", // 深证A指
            "sh880005", // 通达信板块-涨跌家数
            "sh510050", // 上证50ETF
            "sh510300", // 沪深300ETF
            "sh510900", // H股ETF
    };

    /// 证券代码是否需要忽略, 这是一个不参与数据和策略处理的开关
    bool IsNeedIgnore(const std::string& code);

    /// 获取证券代码列表, 过滤退市、摘牌和ST标记的个股
    std::vector<std::string> GetStockCodeList();

    /// 加载全部指数、板块和个股的代码
    std::vector<std::string> GetCodeList();
}


#endif //QUANT1X_EXCHANGE_H
