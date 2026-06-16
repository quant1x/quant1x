// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// tdx/instruments — TDX 证券信息缓存读取 + 初始化
// 与 Python contrib/data/tdx/instruments.py 和 Rust instruments.rs 对齐

#include "instruments.h"
#include <quant1x/config/base.h>
#include <quant1x/data/market.h>
#include <quant1x/data/status.h>
#include <quant1x/contrib/data/tdx/client.h>
#include <quant1x/contrib/data/tdx/protocol.h>
#include <quant1x/contrib/data/tdx/level1/security_list.h>
#include <fstream>
#include <sstream>
#include <mutex>
#include <unordered_map>
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
// ============================================================
static std::mutex g_security_mutex;
static std::unordered_map<std::string, meta::Instrument> g_security_map;
static bool g_security_loaded = false;

// ============================================================
// load_securities() — 从 CSV 加载到内存
// ============================================================
bool load_securities() {
    std::lock_guard<std::mutex> lock(g_security_mutex);
    if (g_security_loaded) {
        return !g_security_map.empty();
    }

    std::string fname = config::get_meta_path() + "/securities.csv";
    spdlog::debug("[tdx/instruments] Loading securities from {}", fname);

    g_security_map.clear();

    std::ifstream file(fname);
    if (!file.is_open()) {
        spdlog::warn("[tdx/instruments] cannot open {}", fname);
        g_security_loaded = true;
        return false;
    }

    std::string line;
    // Skip header: exchange,type,code,name,lot_size,price_precision,ext_market,ext_category,alias_ticker
    if (!std::getline(file, line)) {
        g_security_loaded = true;
        return false;
    }

    int count = 0;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string exchange_str, type_str, ticker, name;
        std::string lot_str, prec_str, extm_str, extc_str, alias;

        std::getline(ss, exchange_str, ',');
        std::getline(ss, type_str, ',');
        std::getline(ss, ticker, ',');
        std::getline(ss, name, ',');
        std::getline(ss, lot_str, ',');
        std::getline(ss, prec_str, ',');
        std::getline(ss, extm_str, ',');
        std::getline(ss, extc_str, ',');
        std::getline(ss, alias, ',');

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
            spdlog::debug("[tdx/instruments] skip row: {} — {}", line, e.what());
        }
    }

    g_security_loaded = true;
    spdlog::info("[tdx/instruments] loaded {} instruments from {}", count, fname);
    return count > 0;
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

    std::ofstream file(fname, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        spdlog::error("[tdx/instruments] cannot create {}", fname);
        return;
    }

    // Header: exchange,type,code,name,lot_size,price_precision,ext_market,ext_category,alias_ticker
    file << "exchange,type,code,name,lot_size,price_precision,ext_market,ext_category,alias_ticker\n";

    for (const auto& inst : instruments) {
        file << meta::exchange_identifier(inst.exchange) << ","
             << meta::instype_to_string(inst.type) << ","
             << inst.ticker << ","
             << inst.name << ","
             << inst.lot_size << ","
             << inst.price_precision << ","
             << inst.ext_market << ","
             << inst.ext_category << ","
             << inst.alias_ticker << "\n";
    }

    file.close();
    spdlog::info("[tdx/instruments] wrote {} instruments to {}", instruments.size(), fname);
}

// ============================================================
// init_securities() — 初始化证券列表
// 对齐 Python init_securities() / Rust init_securities()
//
// 流程:
//   1. 检查 CSV 是否过期 (data::should_initialize_file)
//   2. 若文件未过期, 尝试从 CSV 加载
//   3. 若需要更新:
//      a. 从标准行情获取 A 股列表 (SSE/SZSE/BSE)
//      b. TODO: 从扩展行情获取港股等 (HKEX) — 需要 ext 协议基础设施
//      c. 写入 CSV
//      d. 加载到内存
// ============================================================
void init_securities() {
    std::string fname = config::get_meta_path() + "/securities.csv";

    // Step 1: 检查是否需要更新
    bool ensure_updated = data::should_initialize_file(fname);
    if (!ensure_updated) {
        // CSV 存在且是今天的, 尝试加载
        ensure_updated = !load_securities();
    }
    spdlog::debug("[tdx/instruments] init_securities ensure_updated={}", ensure_updated);

    if (!ensure_updated) {
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

    // Step 4: 加载到内存
    g_security_loaded = false; // 强制重新加载
    g_security_map.clear();
    bool ok = load_securities();
    if (!ok) {
        spdlog::error("[tdx/instruments] failed to load securities after initialization");
    }
}

// ============================================================
// GetCodeList() — 返回所有 symbol 字符串
// 首次调用时若缓存为空, 自动触发 init_securities()
// ============================================================
std::vector<std::string> GetCodeList() {
    if (!load_securities()) {
        // 缓存为空或文件不存在, 尝试初始化
        spdlog::info("[tdx/instruments] cache empty, triggering init_securities()...");
        init_securities();
        // 重新加载
        load_securities();
    }

    std::lock_guard<std::mutex> lock(g_security_mutex);
    std::vector<std::string> codes;
    codes.reserve(g_security_map.size());
    for (const auto& [symbol, _] : g_security_map) {
        codes.push_back(symbol);
    }
    return codes;
}

// ============================================================
// GetInstrumentInfo() — 查找单个证券
// ============================================================
std::optional<meta::Instrument> GetInstrumentInfo(const std::string& symbol) {
    std::string security_code = data::correct_security_code(symbol);
    spdlog::debug("[tdx/instruments] GetInstrumentInfo: symbol={}, security_code={}", symbol, security_code);

    load_securities();

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
