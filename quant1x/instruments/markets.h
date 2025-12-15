#pragma once
#ifndef QUANT1X_INSTRUMENTS_MARKETS_H
#define QUANT1X_INSTRUMENTS_MARKETS_H 1

#include <quant1x/exchange/blocks.h>
#include <quant1x/exchange/calendar.h>
#include <quant1x/exchange/code.h>
#include <quant1x/exchange/margin_trading.h>
#include <quant1x/instruments/security.h>
#include <quant1x/io/csv-writer.h>
#include <quant1x/io/file.h>
#include <quant1x/io/http.h>
#include <quant1x/level1/client.h>
#include <quant1x/config/config.h>
#include <quant1x/runtime/core.h>
#include <quant1x/runtime/once.h>
#include <quant1x/std/time.h>

#include <string>
#include <vector>

//============================================================
// instruments 证券代码整合                                   //
//============================================================

namespace instruments {

    /// 证券代码是否需要忽略, 这是一个不参与数据和策略处理的开关
    bool IsNeedIgnore(const std::string &code);

    /// 获取证券代码列表, 过滤退市、摘牌和ST标记的个股
    std::vector<std::string> GetStockCodeList();

    /// 加载全部指数、板块和个股的代码
    std::vector<std::string> GetCodeList();
}  // namespace instruments

#endif  // QUANT1X_INSTRUMENTS_MARKETS_H
