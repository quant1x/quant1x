#pragma once
#ifndef QUANT1X_TDX_KLINE_H
#define QUANT1X_TDX_KLINE_H 1

#include <quant1x/data/adapter.h>
#include <quant1x/data/base.h>
#include <quant1x/data/schema/bar.h>
#include <quant1x/data/schema/adjustment.h>
#include <quant1x/data/meta/instrument.h>
#include <quant1x/data/meta/timestamp.h>
#include <quant1x/contrib/data/tdx/level1/xdxr_info.h>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace tdx {

// =============================
// K线缓存 I/O
// =============================

/// 获取K线缓存文件名 (对应 Python get_kline_filename)
std::string get_kline_filename(const meta::Instrument& inst);

/// 从CSV文件读取K线数据 (对应 Python read_kline_from_csv)
std::vector<meta::schema::Bar> read_kline_from_csv(const std::string& filename);

/// 保存K线数据到CSV文件 (对应 Python save_kline)
void save_kline(const std::string& filename, const std::vector<meta::schema::Bar>& klines);

/// 加载某只证券的K线缓存 (对应 Python load_kline)
std::vector<meta::schema::Bar> load_kline(const meta::Instrument& inst);

// =============================
// 复权因子
// =============================

/// 通过证券代码获取最新的除权除息列表 (对应 factor:: / Python get_xdxr_list)
std::vector<level1::XdxrInfo> get_xdxr_list(const meta::Instrument& inst);

/// 从除权除息的列表提取IPO日期 (对应 Python ipo_date_from_xdxrs)
std::optional<std::string> ipo_date_from_xdxrs(std::span<const level1::XdxrInfo> xdxrs);

/// 聚合给定一个时间范围内的复权因子 (对应 Python combine_adjustments_in_period)
std::vector<meta::schema::CumulativeAdjustment> combine_adjustments_in_period(
        std::span<const level1::XdxrInfo> xdxrs,
        const meta::Timestamp& start_date,
        const meta::Timestamp& end_date);

/// 对K线数据进行一次性前复权 (对应 Python apply_forward_adjustment_incrementally)
void apply_forward_adjustments_once(
        std::vector<meta::schema::Bar>& klines,
        std::span<const level1::XdxrInfo> xdxrs,
        const meta::Timestamp& start_date,
        const meta::Timestamp& end_date,
        bool should_truncate = true);

/// 对K线数据进行前复权计算 (对应 Python calculate_pre_adjust)
void calculate_pre_adjust(
        std::vector<meta::schema::Bar>& klines,
        const std::vector<level1::XdxrInfo>& dividends);

/// 对增量K线应用前复权 (对应 Python apply_forward_adjustment_for_event)
void apply_forward_adjustment_for_event(
        std::vector<meta::schema::Bar>& klines,
        const meta::Timestamp& start_date,
        const std::vector<level1::XdxrInfo>& dividends);

/// 检查给定日期在K线数据中的偏移位置 (对应 Python check_kline_offset)
template <typename T>
int check_kline_offset(const std::vector<T>& klines, const std::string& date) {
    size_t rows = klines.size();
    int offset = 0;
    for (size_t i = 0; i < rows; i++) {
        std::string kline_date = klines[rows - 1 - i].date;
        if (kline_date < date) {
            return -1;
        } else if (kline_date == date) {
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

/// 前复权K线适配器 (对应 Python DataKLine)
class DataKLine : public data::DataAdapter {
public:
    data::Kind Kind() const override { return data::BaseKLine; }
    std::string Owner() override { return data::DefaultDataProvider; }
    std::string Key() const override { return "day"; }
    std::string Name() const override { return "前复权K线"; }
    std::string Usage() const override { return "前复权K线数据"; }

    void Print(const meta::Instrument& inst, const std::vector<meta::Timestamp>& dates = {}) override;
    void Update(const meta::Instrument& inst, const meta::Timestamp& date = meta::Timestamp()) override;
};

} // namespace tdx

#endif // QUANT1X_TDX_KLINE_H
