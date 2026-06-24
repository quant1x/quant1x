#pragma once
#ifndef QUANT1X_CONTRIB_DATA_TDX_H
#define QUANT1X_CONTRIB_DATA_TDX_H 1

#include <string>
#include <vector>
#include <quant1x/data/meta/instrument.h>

namespace quant1x::contrib::data::tdx {

    /// 证券代码是否需要忽略 (对齐 Python datasource.is_need_ignore)
    /// 这是一个不参与数据和策略处理的开关:
    ///   1. 查不到 instrument → true
    ///   2. 名称含 "退" 或 "摘牌" → true
    ///   3. 否则 → false
    bool is_need_ignore(const std::string &code);

    /// 加载全部指数, 板块和个股的代码 (对齐 Python datasource.list_instruments)
    std::vector<quant1x::data::meta::Instrument> list_instruments();

} // namespace quant1x::contrib::data::tdx
#endif // !QUANT1X_CONTRIB_DATA_TDX_H