#pragma once
#ifndef QUANT1X_TDX_INSTRUMENTS_H
#define QUANT1X_TDX_INSTRUMENTS_H 1

/// tdx/instruments — TDX 证券信息缓存读取
/// 与 Python contrib/data/tdx/instruments.py 和 Rust instruments.rs 对齐
/// 属于 TDX 具体实现层, 不放在通用 data/ 层

#include <quant1x/data/meta/instrument.h>
#include <string>
#include <vector>
#include <optional>

namespace quant1x::contrib::data::tdx {
namespace instruments {

    /// 从 securities.csv 加载证券列表到内存
    /// 对齐 Python _load_securities() / Rust load_securities()
    /// @return true 加载成功, false 文件不存在或解析失败
    bool load_securities();

    /// 初始化证券列表 — 通过 RollingOnce 保证每日首次调用时执行
    /// 长期运行的应用每天自动触发重新下载/加载
    /// 对齐 Python init_securities() / Rust init_securities()
    void init_securities();

    /// 获取所有证券代码列表 (symbol 格式, 如 sh600000)
    /// 对齐 Python instruments.py 中遍历 _SECURITY_MAP 获取 keys 的行为
    std::vector<std::string> get_code_list();

    /// 根据证券代码获取证券信息
    /// 对齐 Python get_instrument_info() / Rust get_instrument_info()
    /// @param symbol 证券代码 (支持 sh600000 / 600000.sh / 600000 等格式)
    /// @return 找到返回 Instrument, 否则 std::nullopt
    std::optional<quant1x::data::meta::Instrument> get_instrument_info(const std::string& symbol);

    /// 确保证券缓存已初始化 (供外部在策略启动时调用)
    /// 对齐 Rust ensure_securities_initialized()
    void ensure_securities_initialized();

} // namespace instruments
} // namespace quant1x::contrib::data::tdx

#endif // QUANT1X_TDX_INSTRUMENTS_H
