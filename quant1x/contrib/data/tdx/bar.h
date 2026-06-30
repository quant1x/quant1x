#pragma once
#ifndef QUANT1X_TDX_BAR_H
#define QUANT1X_TDX_BAR_H 1

#include <quant1x/data/adapter.h>
#include <quant1x/data/base.h>
#include <quant1x/data/schema/bar.h>
#include <quant1x/data/schema/adjustment.h>
#include <quant1x/data/meta/instrument.h>
#include <quant1x/data/meta/timestamp.h>
#include <quant1x/contrib/data/tdx/level1/std/xdxr_info.h>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace quant1x::contrib::data::tdx {

// =============================
// K线缓存 I/O
// =============================

/// 获取K线缓存文件名 (对应 Python get_bar_filename)
std::string get_bar_filename(const quant1x::data::meta::Instrument& inst);

/// 从CSV文件读取K线数据 (对应 Python read_bar_from_csv)
std::vector<quant1x::data::schema::Bar> read_bar_from_csv(const std::string& filename);

/// 保存K线数据到CSV文件 (对应 Python save_bar)
void save_bar(const std::string& filename, const std::vector<quant1x::data::schema::Bar>& bars);

/// 加载某只证券的K线缓存 (对应 Python load_bar)
std::vector<quant1x::data::schema::Bar> load_bar(const quant1x::data::meta::Instrument& inst);

// =============================
// 复权因子
// =============================

/// 通过证券代码获取最新的除权除息列表 (对应 Python get_xdxr_list)
std::vector<XdxrInfo> get_xdxr_list(const quant1x::data::meta::Instrument& inst);

/// 通过证券代码字符串获取除权除息列表 (便捷重载)
std::vector<XdxrInfo> get_xdxr_list(const std::string& security_code);

/// 从除权除息的列表提取IPO日期 (对应 Python ipo_date_from_xdxrs)
std::optional<std::string> ipo_date_from_xdxrs(std::span<const XdxrInfo> xdxrs);

/// 聚合给定一个时间范围内的复权因子 (对应 Python combine_adjustments_in_period)
std::vector<quant1x::data::schema::CumulativeAdjustment> combine_adjustments_in_period(
    std::span<const XdxrInfo> xdxrs,
    const quant1x::data::meta::Timestamp& start_date,
    const quant1x::data::meta::Timestamp& end_date);

/// 聚合给定一个时间范围内的复权因子 — 日期字符串便捷重载
std::vector<quant1x::data::schema::CumulativeAdjustment> combine_adjustments_in_period(
    const std::vector<XdxrInfo>& xdxrs,
        const std::string& start_date,
        const std::string& end_date);

/// 对K线数据进行一次性前复权 (对应 Python apply_forward_adjustment_incrementally)
void apply_forward_adjustments_once(
        std::vector<quant1x::data::schema::Bar>& bars,
        std::span<const XdxrInfo> xdxrs,
        const quant1x::data::meta::Timestamp& start_date,
        const quant1x::data::meta::Timestamp& end_date,
        bool should_truncate = true);

/// 对K线数据进行前复权计算 (对应 Python calculate_pre_adjust)
void calculate_pre_adjust(
        std::vector<quant1x::data::schema::Bar>& bars,
        const std::vector<XdxrInfo>& dividends);

/// 对增量K线应用前复权 (对应 Python apply_forward_adjustment_for_event)
void apply_forward_adjustment_for_event(
        std::vector<quant1x::data::schema::Bar>& bars,
        const quant1x::data::meta::Timestamp& start_date,
        const std::vector<XdxrInfo>& dividends);

/// 检查给定日期在K线数据中的偏移位置 (对应 Python check_bar_offset)
template <typename T>
int check_bar_offset(const std::vector<T>& bars, const std::string& date) {
    size_t rows = bars.size();
    int offset = 0;
    for (size_t i = 0; i < rows; i++) {
        std::string bar_date = bars[rows - 1 - i].date;
        if (bar_date < date) {
            return -1;
        } else if (bar_date == date) {
            break;
        } else {
            offset++;
        }
    }
    if (size_t(offset) + 1 >= rows) {
        return -1;
    }
    return offset;
}

// =============================
// 数据适配器: 前复权K线
// =============================

/// 获取指定证券代码截至指定日期的前复权K线数据 (对应 Python/Rust get_cross_section_forward_adjusted_bars)
///   - 如果缓存文件不存在, 先通过 DataKLine 从服务器拉取并生成缓存
///   - 从已复权的 K 线缓存 CSV 读取, 按 as_of_date 过滤
std::vector<quant1x::data::schema::Bar> get_cross_section_forward_adjusted_bars(
    const quant1x::data::meta::Instrument& inst, const std::string& as_of_date);

/// 前复权K线适配器 (对应 Python DataKLine)
class DataKLine : public quant1x::data::DataAdapter {
public:
    quant1x::data::Kind Kind() const override { return quant1x::data::BaseKLine; }
    std::string Owner() const override { return quant1x::data::DefaultDataProvider; }
    std::string Key() const override { return "day"; }
    std::string Name() const override { return "前复权K线"; }
    std::string Usage() const override { return "前复权K线数据"; }

    void Print(const quant1x::data::meta::Instrument& inst, const quant1x::data::meta::Timestamp& date) override;
    void Update(const quant1x::data::meta::Instrument& inst, const quant1x::data::meta::Timestamp& date) override;
};

// =============================
// 向后兼容的便捷函数
// =============================

/// 从缓存检出指定日期 K 线 (便捷重载, 自动解析证券代码)
/// 对齐旧版 checkout_bars(code, date)
inline std::vector<quant1x::data::schema::Bar> checkout_bars(const std::string& code, const std::string& date) {
    (void)date;
    auto inst = quant1x::data::detect_symbol(code);
    return load_bar(inst);
}

/// 获取前复权 K 线截至指定日期 (便捷重载, 自动解析证券代码)
/// 对齐旧版 bars_forward_adjusted_to_date(code, date)
inline std::vector<quant1x::data::schema::Bar> bars_forward_adjusted_to_date(const std::string& code, const std::string& date) {
    auto inst = quant1x::data::detect_symbol(code);
    return get_cross_section_forward_adjusted_bars(inst, date);
}

} // namespace quant1x::contrib::data::tdx

#endif // QUANT1X_TDX_BAR_H
