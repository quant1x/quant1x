#include <quant1x/contrib/data/tdx/bar_raw.h>
#include <quant1x/contrib/data/tdx/client.h>
#include <quant1x/contrib/data/tdx/instruments.h>
#include <quant1x/contrib/data/tdx/level1/std/security_bars.h>
#include <quant1x/contrib/data/tdx/level1/ext/instrument_bars.h>
#include <quant1x/config/base.h>
#include <quant1x/data/base.h>
#include <quant1x/data/meta/exchange.h>
#include <quant1x/io/csv-reader.h>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <quant1x/io/csv-writer.h>
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace config = quant1x::config;
namespace data = quant1x::data;
namespace csvio = ::io;
namespace meta = quant1x::data;
using quant1x::contrib::data::tdx::KLineType;
using quant1x::contrib::data::tdx::SecurityBarsContext;

namespace quant1x::contrib::data::tdx {

// =============================
// 常量 (对齐 Python kline_raw.py)
// =============================

/// 每页请求的最大K线数量 (对齐 Python SECURITY_BARS_PRE_REQUEST_MAX)
constexpr int kSecurityBarsPreRequestMax = 800;

/// 增量更新缓存清理的最大天数 (对齐 Python MaxCachedDaysToDropOnIncrementalUpdate)
constexpr int kMaxCachedDaysToDrop = 1;

/// 全局默认起始日期 (对齐 Python GLOBAL_DEFAULT_START_DATE)
constexpr const char* kGlobalDefaultStartDate = "1990-12-19";

// =============================
// BarRaw — 缓存格式 (对齐 Python kline_raw.py BarRaw)
// =============================

struct BarRaw {
    std::string date;
    double      open = 0.0;
    double      close = 0.0;
    double      high = 0.0;
    double      low = 0.0;
    double      volume = 0.0;
    double      amount = 0.0;
    int         up = 0;
    int         down = 0;
    std::string timestamp;

    /// 从 domain Bar 构造 (对齐 Python: for row in vec: BarRaw(date=..., open=row.open, ...))
    static BarRaw from_bar(const schema::Bar& bar) {
        return BarRaw{
            bar.date,
            bar.open,
            bar.close,
            bar.high,
            bar.low,
            bar.volume,
            bar.amount,
            bar.up,
            bar.down,
            bar.timestamp
        };
    }
};

// =============================
// Raw K线缓存 I/O — BarRaw 格式 (对齐 Python kline_raw.py save_kline_raw / read_kline_raw_from_csv)
// =============================

static std::string get_kline_raw_filename(const meta::Instrument& inst) {
    return config::default_cache_path() + "/day_raw/" + inst.cache_dir() + "/" + inst.symbol() + ".raw";
}

static void save_kline_raw(const std::string& filename, const std::vector<BarRaw>& values) {
    if (values.empty()) return;
    auto dir = std::filesystem::path(filename).parent_path().string();
    std::filesystem::create_directories(dir);

    csvio::CSVWriter writer(filename);
    writer.write_row("date", "open", "close", "high", "low", "volume", "amount", "up", "down", "timestamp");
    for (const auto& v : values) {
        writer.write_row(v.date, v.open, v.close, v.high, v.low,
                         v.volume, v.amount, v.up, v.down, v.timestamp);
    }
}

static std::vector<BarRaw> read_kline_raw_from_csv(const std::string& filename) {
    std::vector<BarRaw> klines;
    try {
        csvio::CSVReader<10> in(filename);
        in.read_header(csvio::ignore_extra_column, "date", "open", "close", "high", "low",
                       "volume", "amount", "up", "down", "timestamp");
        BarRaw row = {};
        while (in.read_row(row.date, row.open, row.close, row.high, row.low,
                           row.volume, row.amount, row.up, row.down, row.timestamp)) {
            klines.emplace_back(std::move(row));
        }
    } catch (const std::exception& e) {
        spdlog::warn("[kline_raw] read_kline_raw_from_csv error: {}", e.what());
    }
    return klines;
}

// =============================
// fetch_kline_raw (对应 Python fetch_kline_raw — 返回 domain schema Bar)
// =============================

/// fetch_kline_raw_from_std — 从标准行情获取原始K线, 转换为 domain Bar
/// 对应 Python kline_raw.py fetch_kline_raw_from_std:
///   msg = SecurityBarsContext(inst.exchange, inst.ticker, kline_type, start, count, inst.type.is_index())
///   protocol.transact_message_sync(conn, msg)
///   return msg.list  # msg.list 已经是 List[Bar]
static std::vector<schema::Bar> fetch_kline_raw_from_std(
        const meta::Instrument& inst, int start, int count, u16 category) {
    try {
        auto conn = get_std_conn();
        SecurityBarsContext bars(inst, category,
                                  static_cast<u16>(start), static_cast<u16>(count));
        transact_message_sync(conn->socket(), bars);

        std::vector<schema::Bar> result;
        result.reserve(bars.List.size());
        for (auto const& raw : bars.List) {
            schema::Bar bar;
            bar.date             = raw.DateTime.substr(0, 10);
            bar.open             = raw.Open;
            bar.close            = raw.Close;
            bar.high             = raw.High;
            bar.low              = raw.Low;
            bar.volume           = raw.Vol * 100;    // 转换为股 (对齐 Python: volume * 100)
            bar.amount           = raw.Amount;
            bar.up               = static_cast<int>(raw.UpCount);
            bar.down             = static_cast<int>(raw.DownCount);
            bar.timestamp        = raw.DateTime;
            bar.adjustment_count = 0;
            result.push_back(std::move(bar));
        }

        spdlog::debug("[kline_raw] fetch_kline_raw_from_std: {} bars for {}",
                      result.size(), inst.symbol());
        return result;
    } catch (const std::exception& e) {
        spdlog::error("[kline_raw] fetch_kline_raw_from_std error for {}: {}",
                      inst.symbol(), e.what());
        return {};
    }
}

/// fetch_kline_raw_from_ext — 从扩展行情获取原始K线, 转换为 domain Bar (港股/美股等)
/// 对应 Python kline_raw.py fetch_kline_raw_from_ext:
///   with get_ext_conn() as conn:
///       bars = InstrumentBars(kline_type.value, inst.ext_market, ticker=code.upper(), start=start, count=count)
///       protocol.transact_message_sync(conn, bars)
///       return bars.reply  # bars.reply 已经是 List[Bar]
static std::vector<schema::Bar> fetch_kline_raw_from_ext(
        const meta::Instrument& inst, int start, int count, u16 category) {
    try {
        auto conn = get_ext_conn();
        if (!conn) {
            spdlog::warn("[kline_raw] fetch_kline_raw_from_ext: no ext connection for {}", inst.symbol());
            return {};
        }

        std::string ticker;
        if (!inst.alias_ticker.empty()) {
            ticker = inst.alias_ticker;
        } else {
            ticker = inst.ticker;
        }
        // 对齐 Python: ticker=code.upper()
        for (auto& c : ticker) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

        InstrumentBars bars(
            static_cast<u8>(inst.ext_market),
            ticker,
            category,
            static_cast<u32>(start),
            static_cast<u16>(count)
        );

        transact_message_sync(conn->socket(), bars);

        spdlog::debug("[kline_raw] fetch_kline_raw_from_ext: {} bars for {}",
                      bars.reply.size(), inst.symbol());
        return std::move(bars.reply);
    } catch (const std::exception& e) {
        spdlog::error("[kline_raw] fetch_kline_raw_from_ext error for {}: {}",
                      inst.symbol(), e.what());
        return {};
    }
}

std::vector<schema::Bar> fetch_kline_raw(
        const meta::Instrument& inst, int start, int count, u16 category) {
    if (exchange_is_std_quote(inst.exchange)) {
        return fetch_kline_raw_from_std(inst, start, count, category);
    } else if (exchange_is_ext_quote(inst.exchange)) {
        return fetch_kline_raw_from_ext(inst, start, count, category);
    }
    return {};
}

// =============================
// DataKLineRaw — 未复权日K线数据适配器
// 对应 Python class DataKLineRaw(DataAdapter)
// =============================

void DataKLineRaw::Print(const meta::Instrument& inst, const meta::Timestamp& date) {
    auto filename = get_kline_raw_filename(inst);
    auto klines = read_kline_raw_from_csv(filename);
    if (klines.empty()) {
        fmt::print("\n=== {}: {} ===\n  (no data)\n", Name(), inst.symbol());
        return;
    }
    if (!date.empty()) {
        std::string date_str = date.only_date();
        klines.erase(std::remove_if(klines.begin(), klines.end(),
            [&](auto const& b) { return b.date > date_str; }), klines.end());
    }
    fmt::print("\n=== {}: {} ({} rows) ===\n", Name(), inst.symbol(), klines.size());
    fmt::print("{:<12} {:>8} {:>8} {:>8} {:>8} {:>12} {:>14} {:>4} {:>4}\n",
               "date", "open", "close", "high", "low", "volume", "amount", "up", "dn");
    fmt::print("{:-<82}\n", "");
    size_t head = std::min<size_t>(klines.size(), 10);
    for (size_t i = 0; i < head; ++i) {
        auto const& b = klines[i];
        fmt::print("{:<12} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>12.0f} {:>14.0f} {:>4} {:>4}\n",
                   b.date, b.open, b.close, b.high, b.low,
                   b.volume, b.amount, b.up, b.down);
    }
    if (klines.size() > 20) {
        fmt::print("  ... {} rows omitted ...\n", klines.size() - 20);
        head = std::min<size_t>(10, klines.size());
        for (size_t i = klines.size() - head; i < klines.size(); ++i) {
            auto const& b = klines[i];
            fmt::print("{:<12} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>12.0f} {:>14.0f} {:>4} {:>4}\n",
                       b.date, b.open, b.close, b.high, b.low,
                       b.volume, b.amount, b.up, b.down);
        }
    } else if (klines.size() > 10) {
        for (size_t i = 10; i < klines.size(); ++i) {
            auto const& b = klines[i];
            fmt::print("{:<12} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>12.0f} {:>14.0f} {:>4} {:>4}\n",
                       b.date, b.open, b.close, b.high, b.low,
                       b.volume, b.amount, b.up, b.down);
        }
    }
}

void DataKLineRaw::Update(const meta::Instrument& inst, const meta::Timestamp& date) {
    (void)date;
    auto symbol = inst.symbol();

    // 1. 从本地缓存确定起始日期
    // 对齐 Python: current_start_date = Timestamp.parse(GLOBAL_DEFAULT_START_DATE)
    meta::Timestamp current_start_date = meta::Timestamp::parse(kGlobalDefaultStartDate);
    auto cache_filename = get_kline_raw_filename(inst);

    // 对齐 Python: cache_klines = read_kline_raw_from_csv(cache_filename)
    auto cache_klines = read_kline_raw_from_csv(cache_filename);

    size_t klines_length = cache_klines.size();
    size_t klines_offset_days = kMaxCachedDaysToDrop;

    if (klines_length > 0) {
        if (klines_offset_days > klines_length) {
            klines_offset_days = klines_length;
        }
        // 对齐 Python: kline = cache_klines[klines_length - klines_offset_days]; current_start_date = Timestamp.parse(kline.date)
        auto& kline = cache_klines[klines_length - klines_offset_days];
        current_start_date = meta::Timestamp::parse(kline.date);
    }

    // 2. 确定结束日期
    auto current_end_date = meta::Timestamp::now().pre_market_time();
    spdlog::debug("[DataKLineRaw] [{}]: from {} to {}",
                  symbol, current_start_date.only_date(), current_end_date.only_date());

    // 3. 分页拉取原始K线 — fetch_kline_raw 返回 domain Bar (对齐 Python: reply = fetch_kline_raw(inst, start, count, freq))
    int step = kSecurityBarsPreRequestMax;
    int start = 0;
    std::vector<std::vector<schema::Bar>> batches;
    while (true) {
        int count = step;
        auto reply = fetch_kline_raw(inst, start, count, static_cast<u16>(KLineType::DAILY));
        if (reply.empty()) break;

        // 对齐 Python: last_bar = reply[-1]; last_bar_date = Timestamp.parse(last_bar.date).get_pre_market_time()
        auto& last_bar = reply.back();
        auto last_bar_date = meta::Timestamp::parse(last_bar.date).pre_market_time();

        batches.push_back(std::move(reply));

        if (last_bar_date < current_start_date) break;
        if (reply.size() < static_cast<size_t>(count)) break;
        start += count;
    }

    // 4. 反转页面 (时间升序, 对齐 Python hs.reverse())
    std::reverse(batches.begin(), batches.end());

    // 5. 构建增量K线并过滤日期范围, 转为 BarRaw 缓存格式
    // 对齐 Python: for vec in reversed(hs): for row in vec: filter by date, create BarRaw(...)
    std::vector<BarRaw> incremental_klines;
    for (auto& batch : batches) {
        for (auto& bar : batch) {
            auto date_time = meta::Timestamp::parse(bar.date).pre_market_time();
            if (date_time < current_start_date || date_time > current_end_date) continue;

            // 对齐 Python: kx = BarRaw(date=date_time.only_date(), open=row.open, ..., volume=row.volume * 100, ...)
            BarRaw bx{
                date_time.only_date(),
                bar.open,
                bar.close,
                bar.high,
                bar.low,
                bar.volume * 100,   // 转换为股 (对齐 Python: volume = row.volume * 100)
                bar.amount,
                bar.up,
                bar.down,
                bar.timestamp
            };
            incremental_klines.push_back(std::move(bx));
        }
    }

    if (incremental_klines.empty()) {
        spdlog::debug("[DataKLineRaw] no new data for {}", symbol);
        return;
    }

    // 6. 合并旧缓存和新数据
    // 对齐 Python: klines = []; if klines_length > klines_offset_days: klines.extend(cache_klines[:...]); klines.extend(incremental_klines)
    std::vector<BarRaw> klines;
    if (klines_length > klines_offset_days) {
        klines.insert(klines.end(),
                      cache_klines.begin(),
                      cache_klines.begin() + (klines_length - klines_offset_days));
    }
    klines.insert(klines.end(), incremental_klines.begin(), incremental_klines.end());

    // 7. 保存到缓存文件 (对齐 Python: save_kline_raw(cache_filename, klines))
    save_kline_raw(cache_filename, klines);

    spdlog::info("[DataKLineRaw] updated {} ({} bars) -> {}",
                 symbol, klines.size(), cache_filename);
}

} // namespace quant1x::contrib::data::tdx
