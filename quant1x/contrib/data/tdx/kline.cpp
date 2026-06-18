#include <quant1x/contrib/data/tdx/kline.h>
#include <quant1x/contrib/data/tdx/client.h>
#include <quant1x/contrib/data/tdx/instruments.h>
#include <quant1x/contrib/data/tdx/kline_raw.h>
#include <quant1x/contrib/data/tdx/xdxr.h>
#include <quant1x/contrib/data/tdx/level1/std/security_bars.h>
#include <quant1x/contrib/data/tdx/level1/std/xdxr_info.h>
#include <quant1x/config/base.h>
#include <quant1x/data/base.h>
#include <spdlog/spdlog.h>
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

std::string get_kline_filename(const quant1x::data::meta::Instrument& inst) {
    return config::default_cache_path() + "/day/" + inst.cache_dir() + "/" + inst.symbol() + ".csv";
}

std::vector<quant1x::data::meta::schema::Bar> read_kline_from_csv(const std::string& filename) {
    std::vector<quant1x::data::meta::schema::Bar> klines;
    if (!std::filesystem::exists(filename)) {
        return klines;
    }

    std::ifstream in(filename);
    if (!in) {
        spdlog::warn("[read_kline_from_csv] cannot open: {}", filename);
        return klines;
    }

    std::string line;
    // 跳过 header
    if (!std::getline(in, line)) {
        return klines;
    }

    while (std::getline(in, line)) {
        if (line.empty()) continue;

        std::istringstream ss(line);
        quant1x::data::meta::schema::Bar bar;
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

        klines.push_back(std::move(bar));
    }

    return klines;
}

void save_kline(const std::string& filename, const std::vector<quant1x::data::meta::schema::Bar>& klines) {
    if (klines.empty()) return;

    auto dir = std::filesystem::path(filename).parent_path().string();
    std::filesystem::create_directories(dir);

    std::ofstream out(filename);
    if (!out) {
        spdlog::error("[save_kline] cannot write: {}", filename);
        return;
    }

    out << "date,open,close,high,low,volume,amount,up,down,timestamp,adjustment_count\n";
    for (auto const& bar : klines) {
        out << bar.date << ","
            << bar.open << "," << bar.close << "," << bar.high << "," << bar.low << ","
            << bar.volume << "," << bar.amount << ","
            << bar.up << "," << bar.down << ","
            << bar.timestamp << ","
            << bar.adjustment_count << "\n";
    }
    out.close();
}

std::vector<quant1x::data::meta::schema::Bar> load_kline(const quant1x::data::meta::Instrument& inst) {
    auto filename = get_kline_filename(inst);
    spdlog::debug("[load_kline] file: {}", filename);
    return read_kline_from_csv(filename);
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

std::vector<quant1x::data::meta::schema::CumulativeAdjustment> combine_adjustments_in_period(
        std::span<const XdxrInfo> xdxrs,
        const quant1x::data::meta::Timestamp& start_date,
        const quant1x::data::meta::Timestamp& end_date) {

    std::vector<quant1x::data::meta::schema::CumulativeAdjustment> result;

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

        quant1x::data::meta::schema::CumulativeAdjustment entry{};
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
std::vector<quant1x::data::meta::schema::CumulativeAdjustment> combine_adjustments_in_period(
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
        std::vector<quant1x::data::meta::schema::Bar>& klines,
        std::span<const XdxrInfo> xdxrs,
        const quant1x::data::meta::Timestamp& start_date,
        const quant1x::data::meta::Timestamp& end_date,
        bool should_truncate) {

    if (klines.empty()) return;

    auto ts_start = start_date;
    auto ts_end   = end_date;
    auto factors  = combine_adjustments_in_period(xdxrs, ts_start, ts_end);

    if (factors.empty()) return;

    size_t factors_count = factors.size();
    size_t i = 0;
    size_t rows = 0;
    size_t klines_count = klines.size();

    for (size_t idx = 0; idx < klines_count; ++idx) {
        auto& kline = klines[idx];
        auto current_date = quant1x::data::meta::Timestamp(kline.date).pre_market_time();
        auto factor = factors[i];

        if (current_date > ts_end) {
            break;
        }

        while (i + 1 < factors_count && current_date >= factor.timestamp) {
            ++i;
            factor = factors[i];
        }

        if (current_date < factor.timestamp) {
            kline.adjust(factor);
        } else if (!should_truncate) {
            break;
        }

        ++rows;
    }

    if (should_truncate) {
        klines.resize(rows);
    }
}

// =============================
// 前复权计算 (对应 Python calculate_pre_adjust)
// =============================

void calculate_pre_adjust(
        std::vector<quant1x::data::meta::schema::Bar>& klines,
        const std::vector<XdxrInfo>& dividends) {

    if (klines.empty()) return;

    auto start_ts = quant1x::data::meta::Timestamp(klines[0].date).pre_market_time();
    auto end_ts   = quant1x::data::meta::Timestamp(klines.back().date).pre_market_time();
    apply_forward_adjustments_once(klines, dividends, start_ts, end_ts, true);
}

// =============================
// Incremental forward adjustment (Python apply_forward_adjustment_for_event)
// =============================

void apply_forward_adjustment_for_event(
        std::vector<quant1x::data::meta::schema::Bar>& klines,
        const quant1x::data::meta::Timestamp& current_start_date,
        const std::vector<XdxrInfo>& dividends) {

    if (klines.empty()) return;

    // 最后一根K线的日期
    auto& last = klines.back();
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

        for (auto& kline : klines) {
            if (kline.date >= info.Date) break;
            if (kline.date < info.Date) {
                kline.open  = kline.open  * m + a;
                kline.close = kline.close * m + a;
                kline.high  = kline.high  * m + a;
                kline.low   = kline.low   * m + a;

                if (kline.volume != 0) {
                    double ap = kline.amount / kline.volume;
                    double ap_adjusted = ap * m + a;
                    kline.volume *= (1.0 + share_ratio);
                    kline.amount = kline.volume * ap_adjusted;
                }

                kline.adjustment_count += 1;
            }
        }
    }
}

// =============================
// get_cross_section_forward_adjusted_klines (对应 Python/Rust)
// =============================

std::vector<quant1x::data::meta::schema::Bar> get_cross_section_forward_adjusted_klines(
        const quant1x::data::meta::Instrument& inst, const std::string& as_of_date) {

    auto filename = get_kline_filename(inst);
    spdlog::debug("[get_cross_section_forward_adjusted_klines] loading for {} from {}",
                  inst.symbol(), filename);

    // 如果缓存文件不存在，先通过 DataKLine adapter 拉取并生成缓存
    if (!std::filesystem::exists(filename)) {
        spdlog::info("[get_cross_section_forward_adjusted_klines] cache not found for {}, triggering DataKLine update",
                     inst.symbol());
        DataKLine adapter;
        adapter.Update(inst, quant1x::data::meta::Timestamp());
    }

    auto all_klines = read_kline_from_csv(filename);
    if (all_klines.empty()) {
        return {};
    }

    // 过滤 as_of_date 及之前的K线
    std::vector<quant1x::data::meta::schema::Bar> result;
    for (auto& kline : all_klines) {
        if (kline.date <= as_of_date) {
            result.push_back(std::move(kline));
        }
    }
    return result;
}

// =============================
// checkout_klines / klines_forward_adjusted_to_date
//   两者等效: DataKLine::Update 写入的缓存已是前复权数据
// =============================

/// 内部 helper: Bar -> quant1x::data::KLine
static quant1x::data::KLine bar_to_kline(const quant1x::data::meta::schema::Bar& bar, const std::string& code) {
    quant1x::data::KLine kline;
    kline.date   = bar.date;
    kline.code   = code;
    kline.open   = bar.open;
    kline.close  = bar.close;
    kline.high   = bar.high;
    kline.low    = bar.low;
    kline.volume = bar.volume;
    kline.amount = bar.amount;
    return kline;
}

static std::vector<quant1x::data::KLine> bars_to_klines(
        std::vector<quant1x::data::meta::schema::Bar>&& bars, const std::string& code) {
    std::vector<quant1x::data::KLine> result;
    result.reserve(bars.size());
    for (auto& bar : bars) {
        result.push_back(bar_to_kline(bar, code));
    }
    return result;
}

std::vector<quant1x::data::KLine> checkout_klines(const std::string& code, const std::string& date) {
    std::string sec_code = quant1x::data::correct_security_code(code);
    auto inst_opt = instruments::get_instrument_info(sec_code);
    if (!inst_opt) return {};
    auto bars = get_cross_section_forward_adjusted_klines(*inst_opt, date);
    return bars_to_klines(std::move(bars), sec_code);
}

std::vector<quant1x::data::KLine> klines_forward_adjusted_to_date(const std::string& code, const std::string& date) {
    // 与 checkout_klines 等效, 缓存中已是前复权数据
    return checkout_klines(code, date);
}

// =============================
// DataKLine 适配器实现
// =============================

void DataKLine::Print(const quant1x::data::meta::Instrument& inst, const std::vector<quant1x::data::meta::Timestamp>& dates) {
    (void)inst;
    (void)dates;
}

void DataKLine::Update(const quant1x::data::meta::Instrument& inst, const quant1x::data::meta::Timestamp& date) {
    (void)date;
    auto code = inst.symbol();

    // 1. 确定起始日期 - 从本地缓存读取
    quant1x::data::meta::Timestamp current_start_date = quant1x::data::meta::Timestamp::parse(kMarketCnFirstListTime);
    auto cache_filename = get_kline_filename(inst);
    auto cache_klines = read_kline_from_csv(cache_filename);

    size_t klines_length = cache_klines.size();
    size_t klines_offset_days = kMaxCachedDaysToDropOnIncrementalUpdate;
    int adjust_times = 0;

    if (klines_length > 0) {
        if (klines_offset_days > klines_length) {
            klines_offset_days = klines_length;
        }
        auto& kline = cache_klines[klines_length - klines_offset_days];
        current_start_date = quant1x::data::meta::Timestamp(kline.date);
        adjust_times = kline.adjustment_count;
    }

    // 2. 确定结束日期 = 当前盘前时间
    auto current_end_date = quant1x::data::meta::Timestamp::now().pre_market_time();

    spdlog::debug("[DataKLine] [{}]: from {} to {}",
                  code, current_start_date.only_date(), current_end_date.only_date());

    // 3. 分页拉取原始日线数据 -> fetch_kline_raw 返回 domain Bar (对齐 Python: reply = fetch_kline_raw(inst, start, count, freq))
    int32_t step = security_bars_max;
    int32_t start = 0;
    std::vector<std::vector<quant1x::data::meta::schema::Bar>> batches;
    size_t element_count = 0;

    while (true) {
        int32_t count = step;
        auto reply = fetch_kline_raw(inst, start, count, static_cast<u16>(KLineType::DAILY));
        if (reply.empty()) break;

        auto reply_size = reply.size();
        element_count += reply_size;

        // 记录最后一根bar的日期用于判断循环终止 (对齐 Python: last_bar = reply[-1]; last_bar_date = Timestamp.parse(last_bar.date).get_pre_market_time())
        auto& last_bar = reply.back();
        auto last_bar_date = quant1x::data::meta::Timestamp::parse(last_bar.date).pre_market_time();

        batches.push_back(std::move(reply));

        if (last_bar_date < current_start_date) break;
        if (reply_size < static_cast<size_t>(count)) break;

        start += count;
    }

    // 对齐 Python: 如果首次请求就失败, 直接返回
    if (batches.empty()) {
        spdlog::debug("[DataKLine] no data from server for {}", code);
        return;
    }

    // 4. 反转批次并过滤日期范围
    std::reverse(batches.begin(), batches.end());

    std::vector<quant1x::data::meta::schema::Bar> incremental_klines;
    for (auto& batch : batches) {
        for (auto& bar : batch) {
            auto date_time = quant1x::data::meta::Timestamp(bar.date).pre_market_time();
            if (date_time < current_start_date || date_time > current_end_date) continue;
            incremental_klines.push_back(std::move(bar));
        }
    }

    size_t inc_len = incremental_klines.size();
    if (inc_len == 0) {
        spdlog::debug("[DataKLine] no new data for {}", code);
        return;
    }

    // 5. 获取除权除息数据
    auto dividends = get_xdxr_list(inst);

    // 6. 增量复权判断
    bool is_fresh_fetch_require_adjustment = (adjust_times == 1);

    if (is_fresh_fetch_require_adjustment) {
        apply_forward_adjustment_for_event(incremental_klines, current_start_date, dividends);
    }

    // 7. 合并旧缓存和新数据
    std::vector<quant1x::data::meta::schema::Bar> klines;
    if (klines_length > klines_offset_days) {
        klines.insert(klines.end(),
                      cache_klines.begin(),
                      cache_klines.begin() + (klines_length - klines_offset_days));
    }
    klines.insert(klines.end(), incremental_klines.begin(), incremental_klines.end());

    // 8. 非首次拉取时对整个合并结果做前复权
    if (!is_fresh_fetch_require_adjustment) {
        apply_forward_adjustment_for_event(klines, current_start_date, dividends);
    }

    // 9. 保存到缓存
    save_kline(cache_filename, klines);
    spdlog::info("[DataKLine] updated {} ({} bars) -> {}",
                 code, klines.size(), cache_filename);
}

} // namespace quant1x::contrib::data::tdx
