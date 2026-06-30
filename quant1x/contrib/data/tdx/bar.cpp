#include <quant1x/contrib/data/tdx/bar.h>
#include <quant1x/contrib/data/tdx/client.h>
#include <quant1x/contrib/data/tdx/instruments.h>
#include <quant1x/contrib/data/tdx/bar_raw.h>
#include <quant1x/contrib/data/tdx/xdxr.h>
#include <quant1x/contrib/data/tdx/level1/std/security_bars.h>
#include <quant1x/contrib/data/tdx/level1/std/xdxr_info.h>
#include <quant1x/config/base.h>
#include <quant1x/data/base.h>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

namespace quant1x::contrib::data::tdx {

// =============================
// 常量 (对齐 Python)
// =============================

/// 增量更新缓存清理的最大天数 (对齐 Python MaxCachedDaysToDropOnIncrementalUpdate)
constexpr int kMaxCachedDaysToDropOnIncrementalUpdate = 1;

/// 中国资本市场首个交易日 (对齐 Python MarketCnFirstListTime)
constexpr const char* kMarketCnFirstListTime = "1990-12-19";

// =============================
// K-line cache I/O
// =============================

std::string get_bar_filename(const quant1x::data::meta::Instrument& inst) {
    return config::default_cache_path() + "/day/" + inst.cache_dir() + "/" + inst.symbol() + ".csv";
}

std::vector<quant1x::data::schema::Bar> read_bar_from_csv(const std::string& filename) {
    std::vector<quant1x::data::schema::Bar> bars;
    if (!std::filesystem::exists(filename)) {
        return bars;
    }

    std::ifstream in(filename);
    if (!in) {
        spdlog::warn("[read_bar_from_csv] cannot open: {}", filename);
        return bars;
    }

    std::string line;
    // 跳过 header
    if (!std::getline(in, line)) {
        return bars;
    }

    while (std::getline(in, line)) {
        if (line.empty()) continue;

        std::istringstream ss(line);
        quant1x::data::schema::Bar bar;
        std::string token;

        auto next_token = [&]() -> std::string {
            std::string t;
            if (!std::getline(ss, t, ',')) return "";
            // trim
            t.erase(0, t.find_first_not_of(" \t\r\n"));
            t.erase(t.find_last_not_of(" \t\r\n") + 1);
            return t;
        };

        bar.date = next_token();
        bar.open = std::stod(next_token());
        bar.close = std::stod(next_token());
        bar.high = std::stod(next_token());
        bar.low = std::stod(next_token());
        bar.volume = std::stod(next_token());
        bar.amount = std::stod(next_token());
        bar.up = std::stoi(next_token());
        bar.down = std::stoi(next_token());
        bar.timestamp = next_token();
        bar.adjustment_count = std::stoi(next_token());

        bars.push_back(std::move(bar));
    }

    return bars;
}

void save_bar(const std::string& filename, const std::vector<quant1x::data::schema::Bar>& bars) {
    if (bars.empty()) return;

    auto dir = std::filesystem::path(filename).parent_path().string();
    std::filesystem::create_directories(dir);

    std::ofstream out(filename);
    if (!out) {
        spdlog::error("[save_bar] cannot write: {}", filename);
        return;
    }

    out << "date,open,close,high,low,volume,amount,up,down,timestamp,adjustment_count\n";
    for (auto const& bar : bars) {
        out << bar.date << ","
            << bar.open << "," << bar.close << "," << bar.high << "," << bar.low << ","
            << bar.volume << "," << bar.amount << ","
            << bar.up << "," << bar.down << ","
            << bar.timestamp << ","
            << bar.adjustment_count << "\n";
    }
    out.close();
}

std::vector<quant1x::data::schema::Bar> load_bar(const quant1x::data::meta::Instrument& inst) {
    auto filename = get_bar_filename(inst);
    spdlog::debug("[load_bar] file: {}", filename);
    return read_bar_from_csv(filename);
}

// =============================
// 获取XDXR数据
// =============================

std::vector<XdxrInfo> get_xdxr_list(const std::string& security_code) {
    auto inst_opt = instruments::get_instrument_info(security_code);
    if (!inst_opt) return {};
    return get_xdxr_list(*inst_opt);
}

std::vector<XdxrInfo> get_xdxr_list(const quant1x::data::meta::Instrument& inst) {
    // 从 xdxr 缓存文件读取
    std::string filename = config::default_cache_path() + "/xdxr/" + inst.cache_dir() + "/" + inst.symbol() + ".csv";
    std::vector<XdxrInfo> result;

    std::ifstream in(filename);
    if (!in) {
        spdlog::debug("[get_xdxr_list] no cache file: {}", filename);
        return result;
    }

    std::string line;
    // 跳过 header
    if (!std::getline(in, line)) {
        return result;
    }

    auto parse_double = [](const std::string& s) -> double {
        try { return std::stod(s); } catch (...) { return 0.0; }
    };
    auto parse_int = [](const std::string& s) -> int {
        try { return std::stoi(s); } catch (...) { return 0; }
    };

    while (std::getline(in, line)) {
        if (line.empty()) continue;

        std::istringstream ss(line);
        std::string token;
        auto next = [&]() -> std::string {
            std::string t;
            std::getline(ss, t, ',');
            t.erase(0, t.find_first_not_of(" \t\r\n"));
            t.erase(t.find_last_not_of(" \t\r\n") + 1);
            return t;
        };

        XdxrInfo info;
        info.Date         = next();
        info.Category     = static_cast<u16>(parse_int(next()));
        info.Name         = next();
        info.FenHong      = parse_double(next());
        info.PeiGuJia     = parse_double(next());
        info.SongZhuanGu  = parse_double(next());
        info.PeiGu        = parse_double(next());
        info.SuoGu        = parse_double(next());
        info.QianLiuTong  = parse_double(next());
        info.HouLiuTong   = parse_double(next());
        info.QianZongGuBen = parse_double(next());
        info.HouZongGuBen  = parse_double(next());
        info.FenShu       = parse_double(next());
        info.XingQuanJia  = parse_double(next());

        result.push_back(std::move(info));
    }

    // 按日期排序
    std::sort(result.begin(), result.end(),
              [](const XdxrInfo& a, const XdxrInfo& b) { return a.Date < b.Date; });

    return result;
}

std::optional<std::string> ipo_date_from_xdxrs(std::span<const XdxrInfo> xdxrs) {
    for (auto const& v : xdxrs) {
        if (v.Category != 5) continue;
        if (v.QianLiuTong == 0 && v.QianZongGuBen == 0 && v.HouLiuTong > 0 && v.HouZongGuBen > 0) {
            return v.Date;
        }
    }
    return std::nullopt;
}

// =============================
// Adjustment aggregation (Python combine_adjustments_in_period)
// =============================

std::vector<quant1x::data::schema::CumulativeAdjustment> combine_adjustments_in_period(
        std::span<const XdxrInfo> xdxrs,
        const quant1x::data::meta::Timestamp& start_date,
        const quant1x::data::meta::Timestamp& end_date) {

    std::vector<quant1x::data::schema::CumulativeAdjustment> result;

    for (const auto& info : xdxrs) {
        // 只处理除权除息 (Category == 1)
        if (info.Category != 1) continue;

        // 转换为盘前时间
        quant1x::data::meta::Timestamp event_ts = quant1x::data::meta::Timestamp::parse(info.Date).pre_market_time();
        if (event_ts < start_date || event_ts > end_date) continue;

        auto [m, a] = info.adjustFactor();
        double event_monetary_adjustment = info.computeMonetaryAdjustment();
        double event_share_adjustment_ratio = info.computeShareAdjustmentRatio();

        for (auto& factor : result) {
            // 叠加复权因子
            factor.m *= m;
            factor.a = m * factor.a + a;
            factor.no += 1;

            double old_monetary = factor.monetary_adjustment;
            double old_share = factor.share_adjustment_ratio;
            double new_share = old_share + event_share_adjustment_ratio +
                               old_share * event_share_adjustment_ratio;
            double new_monetary = old_monetary +
                                  event_monetary_adjustment * (1.0 + old_share);
            factor.monetary_adjustment = new_monetary;
            factor.share_adjustment_ratio = new_share;
        }

        quant1x::data::schema::CumulativeAdjustment entry{};
        entry.timestamp             = event_ts;
        entry.m                     = m;
        entry.a                     = a;
        entry.no                    = 1;
        entry.monetary_adjustment   = event_monetary_adjustment;
        entry.share_adjustment_ratio = event_share_adjustment_ratio;
        result.push_back(entry);
    }

    return result;
}

// 日期字符串便捷重载
std::vector<quant1x::data::schema::CumulativeAdjustment> combine_adjustments_in_period(
        const std::vector<XdxrInfo>& xdxrs,
        const std::string& start_date,
        const std::string& end_date) {
    auto ts_start = quant1x::data::meta::Timestamp::parse(start_date).pre_market_time();
    auto ts_end   = quant1x::data::meta::Timestamp::parse(end_date).pre_market_time();
    return combine_adjustments_in_period(std::span(xdxrs), ts_start, ts_end);
}

// =============================
// One-shot forward adjustment (Python apply_forward_adjustment_incrementally)
// =============================

void apply_forward_adjustments_once(
        std::vector<quant1x::data::schema::Bar>& bars,
        std::span<const XdxrInfo> xdxrs,
        const quant1x::data::meta::Timestamp& start_date,
        const quant1x::data::meta::Timestamp& end_date,
        bool should_truncate) {

    if (bars.empty()) return;

    auto ts_start = start_date;
    auto ts_end   = end_date;
    auto factors  = combine_adjustments_in_period(xdxrs, ts_start, ts_end);

    if (factors.empty()) return;

    size_t factors_count = factors.size();
    size_t i = 0;
    size_t rows = 0;
    size_t bars_count = bars.size();

    for (size_t idx = 0; idx < bars_count; ++idx) {
        auto& bar = bars[idx];
        auto current_date = quant1x::data::meta::Timestamp(bar.date).pre_market_time();
        auto factor = factors[i];

        if (current_date > ts_end) {
            break;
        }

        while (i + 1 < factors_count && current_date >= factor.timestamp) {
            ++i;
            factor = factors[i];
        }

        if (current_date < factor.timestamp) {
            bar.adjust(factor);
        } else if (!should_truncate) {
            break;
        }

        ++rows;
    }

    if (should_truncate) {
        bars.resize(rows);
    }
}

// =============================
// 前复权计算 (对应 Python calculate_pre_adjust)
// =============================

void calculate_pre_adjust(
        std::vector<quant1x::data::schema::Bar>& bars,
        const std::vector<XdxrInfo>& dividends) {

    if (bars.empty()) return;

    auto start_ts = quant1x::data::meta::Timestamp(bars[0].date).pre_market_time();
    auto end_ts   = quant1x::data::meta::Timestamp(bars.back().date).pre_market_time();
    apply_forward_adjustments_once(bars, dividends, start_ts, end_ts, true);
}

// =============================
// Incremental forward adjustment (Python apply_forward_adjustment_for_event)
// =============================

void apply_forward_adjustment_for_event(
        std::vector<quant1x::data::schema::Bar>& bars,
        const quant1x::data::meta::Timestamp& current_start_date,
        const std::vector<XdxrInfo>& dividends) {

    if (bars.empty()) return;

    // 最后一根K线的日期
    auto& last = bars.back();
    auto ts_last_day = quant1x::data::meta::Timestamp(last.date).pre_market_time();

    // 使用 next_trading_day 的逻辑: 这里简化为 last_date_next = ts_last_day + 1day
    // 对齐 Python: last_day_next = next_trading_day(ts_last_day).only_date()
    auto last_day_next = ts_last_day;
    auto start_date_str = current_start_date.only_date();

    for (const auto& info : dividends) {
        if (info.Category != 1) continue;
        if (info.Date > last_day_next.only_date()) continue;

        if (info.Date <= start_date_str) {
            // IPO之前的事件跳过
            continue;
        }

        auto [m, a] = info.adjustFactor();
        double share_ratio = info.computeShareAdjustmentRatio();

        for (auto& bar : bars) {
            if (bar.date >= info.Date) break;
            if (bar.date < info.Date) {
                bar.open  = bar.open  * m + a;
                bar.close = bar.close * m + a;
                bar.high  = bar.high  * m + a;
                bar.low   = bar.low   * m + a;

                if (bar.volume != 0) {
                    double ap = bar.amount / bar.volume;
                    double ap_adjusted = ap * m + a;
                    bar.volume *= (1.0 + share_ratio);
                    bar.amount = bar.volume * ap_adjusted;
                }

                bar.adjustment_count += 1;
            }
        }
    }
}

// =============================
// get_cross_section_forward_adjusted_klines (对应 Python/Rust)
// =============================

std::vector<quant1x::data::schema::Bar> get_cross_section_forward_adjusted_bars(
        const quant1x::data::meta::Instrument& inst, const std::string& as_of_date) {

    auto filename = get_bar_filename(inst);
    spdlog::debug("[get_cross_section_forward_adjusted_bars] loading for {} from {}",
                  inst.symbol(), filename);

    // 如果缓存文件不存在，先通过 DataKLine adapter 拉取并生成缓存
    if (!std::filesystem::exists(filename)) {
        spdlog::info("[get_cross_section_forward_adjusted_bars] cache not found for {}, triggering DataKLine update",
                     inst.symbol());
        DataKLine adapter;
        adapter.Update(inst, quant1x::data::meta::Timestamp());
    }

    auto all_bars = read_bar_from_csv(filename);
    if (all_bars.empty()) {
        return {};
    }

    // 过滤 as_of_date 及之前的K线
    std::vector<quant1x::data::schema::Bar> result;
    for (auto& bar : all_bars) {
        if (bar.date <= as_of_date) {
            result.push_back(std::move(bar));
        }
    }
    return result;
}

// =============================
// DataKLine 适配器实现
// =============================

void DataKLine::Print(const quant1x::data::meta::Instrument& inst, const quant1x::data::meta::Timestamp& date) {
    auto bars = load_bar(inst);
    if (bars.empty()) {
        fmt::print("\n=== {}: {} ===\n  (no data)\n", Name(), inst.symbol());
        return;
    }
    // filter by date if specified
    if (!date.empty()) {
        std::string date_str = date.only_date();
        bars.erase(std::remove_if(bars.begin(), bars.end(),
            [&](auto const& b) { return b.date > date_str; }), bars.end());
    }
    fmt::print("\n=== {}: {} ({} rows) ===\n", Name(), inst.symbol(), bars.size());
    fmt::print("{:<12} {:>8} {:>8} {:>8} {:>8} {:>12} {:>14} {:>4} {:>4} {:>3}\n",
               "date", "open", "close", "high", "low", "volume", "amount", "up", "dn", "adj");
    fmt::print("{:-<90}\n", "");
    size_t head = std::min<size_t>(bars.size(), 10);
    for (size_t i = 0; i < head; ++i) {
        auto const& b = bars[i];
        fmt::print("{:<12} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>12.0f} {:>14.0f} {:>4} {:>4} {:>3}\n",
                   b.date, b.open, b.close, b.high, b.low,
                   b.volume, b.amount, b.up, b.down, b.adjustment_count);
    }
    if (bars.size() > 20) {
        fmt::print("  ... {} rows omitted ...\n", bars.size() - 20);
        head = std::min<size_t>(10, bars.size());
        for (size_t i = bars.size() - head; i < bars.size(); ++i) {
            auto const& b = bars[i];
            fmt::print("{:<12} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>12.0f} {:>14.0f} {:>4} {:>4} {:>3}\n",
                       b.date, b.open, b.close, b.high, b.low,
                       b.volume, b.amount, b.up, b.down, b.adjustment_count);
        }
    } else if (bars.size() > 10) {
        for (size_t i = 10; i < bars.size(); ++i) {
            auto const& b = bars[i];
            fmt::print("{:<12} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>12.0f} {:>14.0f} {:>4} {:>4} {:>3}\n",
                       b.date, b.open, b.close, b.high, b.low,
                       b.volume, b.amount, b.up, b.down, b.adjustment_count);
        }
    }
}

void DataKLine::Update(const quant1x::data::meta::Instrument& inst, const quant1x::data::meta::Timestamp& date) {
    (void)date;
    auto code = inst.symbol();

    // 1. 确定起始日期 - 从本地缓存读取
    quant1x::data::meta::Timestamp current_start_date = quant1x::data::meta::Timestamp::parse(kMarketCnFirstListTime);
    auto cache_filename = get_bar_filename(inst);
    auto cache_bars = read_bar_from_csv(cache_filename);

    size_t bars_length = cache_bars.size();
    size_t bars_offset_days = kMaxCachedDaysToDropOnIncrementalUpdate;
    int adjust_times = 0;

    if (bars_length > 0) {
        if (bars_offset_days > bars_length) {
            bars_offset_days = bars_length;
        }
        auto& bar = cache_bars[bars_length - bars_offset_days];
        current_start_date = quant1x::data::meta::Timestamp(bar.date);
        adjust_times = bar.adjustment_count;
    }

    // 2. 确定结束日期 = 当前盘前时间
    auto current_end_date = quant1x::data::meta::Timestamp::now().pre_market_time();

    spdlog::debug("[DataKLine] [{}]: from {} to {}",
                  code, current_start_date.only_date(), current_end_date.only_date());

    // 3. 分页拉取原始日线数据 -> fetch_bar_raw 返回 domain Bar (对齐 Python: reply = fetch_bar_raw(inst, start, count, freq))
    int32_t step = security_bars_max;
    int32_t start = 0;
    std::vector<std::vector<quant1x::data::schema::Bar>> batches;
    while (true) {
        int32_t count = step;
        auto reply = fetch_bar_raw(inst, start, count, static_cast<u16>(BarFreq::FreqDaily));
        if (reply.empty()) break;

        // 记录最后一根bar的日期用于判断循环终止 (对齐 Python: last_bar = reply[-1]; last_bar_date = Timestamp.parse(last_bar.date).get_pre_market_time())
        auto& last_bar = reply.back();
        auto last_bar_date = quant1x::data::meta::Timestamp::parse(last_bar.date).pre_market_time();

        batches.push_back(std::move(reply));

        if (last_bar_date < current_start_date) break;
        if (reply.size() < static_cast<size_t>(count)) break;

        start += count;
    }

    // 对齐 Python: 如果首次请求就失败, 直接返回
    if (batches.empty()) {
        spdlog::debug("[DataKLine] no data from server for {}", code);
        return;
    }

    // 4. 反转批次并过滤日期范围
    std::reverse(batches.begin(), batches.end());

    std::vector<quant1x::data::schema::Bar> incremental_bars;
    for (auto& batch : batches) {
        for (auto& bar : batch) {
            auto date_time = quant1x::data::meta::Timestamp(bar.date).pre_market_time();
            if (date_time < current_start_date || date_time > current_end_date) continue;
            incremental_bars.push_back(std::move(bar));
        }
    }

    size_t inc_len = incremental_bars.size();
    if (inc_len == 0) {
        spdlog::debug("[DataKLine] no new data for {}", code);
        return;
    }

    // 5. 获取除权除息数据
    auto dividends = get_xdxr_list(inst);

    // 6. 增量复权判断
    bool is_fresh_fetch_require_adjustment = (adjust_times == 1);

    if (is_fresh_fetch_require_adjustment) {
        apply_forward_adjustment_for_event(incremental_bars, current_start_date, dividends);
    }

    // 7. 合并旧缓存和新数据
    std::vector<quant1x::data::schema::Bar> bars;
    if (bars_length > bars_offset_days) {
        bars.insert(bars.end(),
                      cache_bars.begin(),
                      cache_bars.begin() + (bars_length - bars_offset_days));
    }
    bars.insert(bars.end(), incremental_bars.begin(), incremental_bars.end());

    // 8. 非首次拉取时对整个合并结果做前复权
    if (!is_fresh_fetch_require_adjustment) {
        apply_forward_adjustment_for_event(bars, current_start_date, dividends);
    }

    // 9. 保存到缓存
    save_bar(cache_filename, bars);
    spdlog::info("[DataKLine] updated {} ({} bars) -> {}",
                 code, bars.size(), cache_filename);
}

} // namespace quant1x::contrib::data::tdx
