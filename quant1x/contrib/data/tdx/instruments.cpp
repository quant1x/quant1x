// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// tdx/instruments — TDX 证券信息缓存读取 + 初始化
// 与 Python contrib/data/tdx/instruments.py 和 Rust instruments.rs 对齐

#include "instruments.h"
#include <quant1x/config/base.h>
#include <quant1x/data/market.h>
#include <quant1x/data/meta/calendar.h>
#include <quant1x/data/status.h>
#include <quant1x/contrib/data/tdx/client.h>
#include <quant1x/contrib/data/tdx/protocol.h>
#include <quant1x/contrib/data/tdx/level1/security_list.h>
#include <quant1x/io/csv-reader.h>
#include <quant1x/io/csv-writer.h>
#include <quant1x/runtime/once.h>
#include <mutex>
#include <tsl/robin_map.h>
#include <algorithm>
#include <filesystem>

namespace tdx {
namespace instruments {

// ============================================================
// 常量
// ============================================================
constexpr int SECURITY_LIST_PRE_REQUEST_MAX = 1600;  ///< 单次请求最大证券数量

// ============================================================
// Exchange → TDX 市场 ID 映射
// 对齐 Python _EXCHANGE_TO_MARKET / Rust TDX_MARKET_* 常量
// ============================================================
inline int exchange_to_tdx_market(meta::Exchange ex) {
    switch (ex) {
        case meta::Exchange::SZSE: return 0;   // 深圳
        case meta::Exchange::SSE:  return 1;   // 上海
        case meta::Exchange::BSE:  return 2;   // 北京
        case meta::Exchange::HKEX: return 31;  // 香港 (扩展行情)
        case meta::Exchange::HKFE: return 27;  // 香港期货 (扩展行情)
        case meta::Exchange::USA:  return 74;  // 美国 (扩展行情)
        default:                   return -1;  // 不支持
    }
}

// ============================================================
// 内存缓存: symbol → Instrument
// RollingOnce 保证每日首次调用时初始化, 长期运行每天自动重新加载
// ============================================================
static auto security_once = RollingOnce::create("tdx-instruments", meta::cron_expr_daily_9am);
static std::mutex g_security_mutex;
static tsl::robin_map<std::string, meta::Instrument> g_security_map;

// ============================================================
// load_securities() — 从 CSV 加载到内存 (调用方负责通过 RollingOnce 控制时机)
// ============================================================
bool load_securities() {
    std::lock_guard<std::mutex> lock(g_security_mutex);

    std::string fname = config::get_meta_path() + "/securities.csv";
    spdlog::debug("[tdx/instruments] Loading securities from {}", fname);

    g_security_map.clear();

    try {
        io::CSVReader<9> in(fname);
        in.read_header(io::ignore_extra_column, "exchange", "type", "code", "name",
                       "lot_size", "price_precision", "ext_market", "ext_category", "alias_ticker");
        std::string exchange_str, type_str, ticker, name;
        std::string lot_str, prec_str, extm_str, extc_str, alias;
        int count = 0;
        while (in.read_row(exchange_str, type_str, ticker, name, lot_str, prec_str, extm_str, extc_str, alias)) {
            try {
                meta::Instrument inst;
                inst.exchange = meta::exchange_from_abbr(exchange_str);
                inst.type = meta::instype_from_string(type_str);
                for (auto& c : ticker) { c = static_cast<char>(::tolower(static_cast<unsigned char>(c))); }
                inst.ticker = ticker;
                inst.name = name;
                inst.lot_size = lot_str.empty() ? 100 : std::stoi(lot_str);
                inst.price_precision = prec_str.empty() ? 2 : std::stoi(prec_str);
                inst.ext_market = extm_str.empty() ? 0 : std::stoi(extm_str);
                inst.ext_category = extc_str.empty() ? 0 : std::stoi(extc_str);
                inst.alias_ticker = alias;

                std::string symbol = inst.symbol();
                g_security_map[symbol] = inst;
                count++;
            } catch (const std::exception& e) {
                spdlog::debug("[tdx/instruments] skip row: {} {} {} — {}", exchange_str, type_str, ticker, e.what());
            }
        }
        spdlog::info("[tdx/instruments] loaded {} instruments from {}", count, fname);
        return count > 0;
    } catch (const std::exception& e) {
        spdlog::warn("[tdx/instruments] cannot open or parse {}: {}", fname, e.what());
        return false;
    }
}

// ============================================================
// fetch_security_list() — 从 TDX 标准行情服务器获取一页证券
// 对齐 Python instruments.fetch_security_list() (标准行情部分)
// ============================================================
static std::vector<meta::Instrument> fetch_security_list(meta::Exchange exchange, int start, int count) {
    std::vector<meta::Instrument> result;
    int market_id = exchange_to_tdx_market(exchange);
    if (market_id < 0) {
        spdlog::error("[tdx/instruments] unsupported exchange for std market: {} (code={})",
                      meta::exchange_label(exchange), meta::exchange_code(exchange));
        return result;
    }

    try {
        auto conn = level1::get_std_conn();
        auto& sock = conn->socket();

        level1::SecurityList msg(market_id, start, count);
        auto err = level1::process(sock, msg);
        if (err) {
            spdlog::error("[tdx/instruments] SecurityList fetch failed: {} {}",
                          meta::exchange_code(exchange), err.message());
            return result;
        }

        spdlog::debug("[tdx/instruments] SecurityList {} start={} count={} got {} records",
                      meta::exchange_code(exchange), start, count, msg.Count);

        for (const auto& sec : msg.List) {
            meta::Instrument inst;
            inst.exchange = exchange;
            inst.type = meta::InstrumentType::Stock; // A 股默认 Stock 类型
            // 复制代码并转小写
            std::string code_lower = sec.Code;
            for (auto& c : code_lower) { c = static_cast<char>(::tolower(static_cast<unsigned char>(c))); }
            inst.ticker = code_lower;
            inst.name = sec.Name;
            inst.lot_size = static_cast<int>(sec.VolUnit);
            inst.price_precision = static_cast<int>(sec.DecimalPoint);
            inst.ext_market = market_id;      // 与 Python 对齐: ext_market = helpers.exchange_to_market(exchange)
            inst.ext_category = 0;

            result.push_back(inst);
        }

    } catch (const std::exception& e) {
        spdlog::error("[tdx/instruments] fetch_security_list exception: {}", e.what());
    }

    return result;
}

// ============================================================
// write_securities_csv() — 将证券列表写入 CSV
// 对齐 Rust write_securities_csv()
// ============================================================
static void write_securities_csv(const std::string& fname, const std::vector<meta::Instrument>& instruments) {
    // Ensure parent directory exists
    std::filesystem::path p(fname);
    if (p.has_parent_path()) {
        std::filesystem::create_directories(p.parent_path());
    }

    try {
        io::CSVWriter writer(fname);
        writer.write_row("exchange", "type", "code", "name", "lot_size", "price_precision",
                         "ext_market", "ext_category", "alias_ticker");

        for (const auto& inst : instruments) {
            writer.write_row(
                meta::exchange_identifier(inst.exchange),
                meta::instype_to_string(inst.type),
                inst.ticker,
                inst.name,
                inst.lot_size,
                inst.price_precision,
                inst.ext_market,
                inst.ext_category,
                inst.alias_ticker
            );
        }
        spdlog::info("[tdx/instruments] wrote {} instruments to {}", instruments.size(), fname);
    } catch (const std::exception& e) {
        spdlog::error("[tdx/instruments] cannot create {}: {}", fname, e.what());
    }
}

// ============================================================
// do_init_securities() — 实际初始化逻辑, 由 RollingOnce::Do 调用
// ============================================================
static void do_init_securities() {
    std::string fname = config::get_meta_path() + "/securities.csv";

    // Step 1: 检查是否需要更新
    bool create_or_update = data::should_initialize_file(fname);
    if (!create_or_update) {
        // CSV 存在且是今天的, 尝试加载
        create_or_update = !load_securities();
    }
    spdlog::debug("[tdx/instruments] init_securities create_or_update={}", create_or_update);

    if (!create_or_update) {
        return; // 已加载, 无需更新
    }

    // Step 2: 从 TDX 服务器拉取
    std::vector<meta::Instrument> instruments;

    // 2a. 标准行情: A 股 (SSE/SZSE/BSE)
    // 对齐 Python: markets = [Exchange.SSE, Exchange.SZSE, Exchange.BSE]
    std::vector<meta::Exchange> std_markets = {
        meta::Exchange::SSE,
        meta::Exchange::SZSE,
        meta::Exchange::BSE,
    };

    for (auto m : std_markets) {
        int start = 0;
        std::vector<meta::Instrument> rows;
        while (true) {
            auto page = fetch_security_list(m, start, SECURITY_LIST_PRE_REQUEST_MAX);
            if (page.empty()) {
                break;
            }
            size_t page_size = page.size();
            rows.insert(rows.end(), page.begin(), page.end());
            if (page_size < static_cast<size_t>(SECURITY_LIST_PRE_REQUEST_MAX)) {
                break;
            }
            start += SECURITY_LIST_PRE_REQUEST_MAX;
        }

        // 相同市场按代码排序 (对齐 Python: rows.sort(key=lambda x: x.ticker))
        std::sort(rows.begin(), rows.end(),
                  [](const meta::Instrument& a, const meta::Instrument& b) {
                      return a.ticker < b.ticker;
                  });

        spdlog::info("[tdx/instruments] fetched {} instruments from {}", rows.size(),
                     meta::exchange_label(m));
        instruments.insert(instruments.end(), rows.begin(), rows.end());
    }

    // 2b. TODO: 扩展行情 (HKEX 等)
    // 需要 ExtensionProtocolHandler + ExtensionConnectionPool + InstrumentInfo 消息类
    // 等 ext 协议基础设施完成后接入
    // 对齐 Python instruments.py init_securities() 第 127-158 行
    if (true) {
        // 以下为预留框架, 待 ext 协议层完成后实现:
        // std::vector<meta::Exchange> ext_markets = { meta::Exchange::HKEX };
        // for (auto m : ext_markets) { ... }
        spdlog::info("[tdx/instruments] ext market (HKEX) not yet implemented — requires ext protocol handler");
    }

    // Step 3: 写入 CSV
    if (!instruments.empty()) {
        write_securities_csv(fname, instruments);
    } else {
        spdlog::warn("[tdx/instruments] no instruments fetched — CSV not written");
    }

    // Step 4: 加载到内存 (load_securities 内部会加锁并覆盖 g_security_map)
    bool ok = load_securities();
    if (!ok) {
        spdlog::error("[tdx/instruments] failed to load securities after initialization");
    }
}

// ============================================================
// init_securities() — 通过 RollingOnce 保证每日首次调用时初始化
// 对齐 Python init_securities() / Rust init_securities()
// ============================================================
void init_securities() {
    security_once->Do(do_init_securities);
}

// ============================================================
// get_code_list() — 返回所有 symbol 字符串
// ============================================================
std::vector<std::string> get_code_list() {
    init_securities(); // RollingOnce 保证每天只执行一次

    std::lock_guard<std::mutex> lock(g_security_mutex);
    std::vector<std::string> codes;
    codes.reserve(g_security_map.size());
    for (const auto& [symbol, _] : g_security_map) {
        codes.push_back(symbol);
    }
    return codes;
}

// ============================================================
// get_instrument_info() — 查找单个证券
// ============================================================
std::optional<meta::Instrument> get_instrument_info(const std::string& symbol) {
    std::string security_code = data::correct_security_code(symbol);
    spdlog::debug("[tdx/instruments] get_instrument_info: symbol={}, security_code={}", symbol, security_code);

    init_securities(); // RollingOnce 保证每天只执行一次

    std::lock_guard<std::mutex> lock(g_security_mutex);
    auto it = g_security_map.find(security_code);
    if (it != g_security_map.end()) {
        return it->second;
    }
    return std::nullopt;
}

// ============================================================
// EnsureSecuritiesInitialized() — 供外部在策略启动时调用
// ============================================================
void EnsureSecuritiesInitialized() {
    init_securities();
}

} // namespace instruments
} // namespace tdx
