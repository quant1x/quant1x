#include <quant1x/data/meta/timestamp.h>
#include <quant1x/factors/base.h>
#include <quant1x/contrib/data/tdx/kline.h>
#include <quant1x/contrib/data/tdx/instruments.h>
#include <quant1x/data/schema/bar.h>
#include <mutex>
#include <unordered_map>

namespace factors {

    namespace {
        // -- 前复权K线缓存 (供 checkout_klines / klines_forward_adjusted_to_date 使用) --
        std::mutex mutex_klines;
        std::unordered_map<std::string, std::vector<data::KLine>> routineLocal_klines;

        // -- 原始K线缓存 --
        std::mutex mutex_raw_klines;
        std::unordered_map<std::string, std::vector<meta::schema::Bar>> routineLocal_raw_klines;

        void update_cache_klines(const std::string& security_code, const std::vector<data::KLine>& klines) {
            if (klines.empty()) return;
            std::lock_guard<std::mutex> lock(mutex_klines);
            routineLocal_klines[security_code] = klines;
        }

        void update_cache_raw_klines(const std::string& security_code, const std::vector<meta::schema::Bar>& klines) {
            if (klines.empty()) return;
            std::lock_guard<std::mutex> lock(mutex_raw_klines);
            routineLocal_raw_klines[security_code] = klines;
        }
    } // namespace

    // 从 Bar 转换为 data::KLine (向后兼容)
    static data::KLine bar_to_kline(const meta::schema::Bar& bar, const std::string& code) {
        data::KLine kline;
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

    // 捡出指定日期的K线数据
    std::vector<data::KLine> checkout_klines(const std::string& code, const std::string& date) {
        std::string security_code = data::correct_security_code(code);
        meta::Timestamp ts(date);
        std::string fixed_date = ts.only_date();

        // 1. 取缓存的K线
        std::vector<data::KLine> cache_klines;
        {
            std::lock_guard<std::mutex> lock(mutex_klines);
            auto it = routineLocal_klines.find(security_code);
            if (it != routineLocal_klines.end()) {
                cache_klines = it->second;
            }
        }

        if (cache_klines.empty()) {
            // 从 tdx 缓存文件加载
            auto inst_opt = tdx::instruments::GetInstrumentInfo(security_code);
            if (inst_opt) {
                auto bars = tdx::load_kline(*inst_opt);
                for (auto const& bar : bars) {
                    cache_klines.push_back(bar_to_kline(bar, security_code));
                }
            }
            update_cache_klines(security_code, cache_klines);
        }

        size_t rows = cache_klines.size();
        if (rows == 0) return {};

        // 1.1 检查是否最新数据
        if (cache_klines[rows - 1].date < fixed_date) {
            auto inst_opt = tdx::instruments::GetInstrumentInfo(security_code);
            if (inst_opt) {
                auto bars = tdx::load_kline(*inst_opt);
                cache_klines.clear();
                for (auto const& bar : bars) {
                    cache_klines.push_back(bar_to_kline(bar, security_code));
                }
                update_cache_klines(security_code, cache_klines);
            }
        }

        // 2. 对齐日期
        int offset = tdx::check_kline_offset(cache_klines, fixed_date);
        if (offset < 0) return {};

        // 3. 返回
        std::vector<data::KLine> result(cache_klines.begin(), cache_klines.end() - offset);
        return result;
    }

    // 原始数据一次性复权
    std::vector<data::KLine> klines_forward_adjusted_to_date(const std::string& code, const std::string& date) {
        std::string security_code = data::correct_security_code(code);
        meta::Timestamp ts(date);
        std::string fixed_date = ts.only_date();

        // 1. 取缓存的原始K线
        std::vector<meta::schema::Bar> cache_raw;
        {
            std::lock_guard<std::mutex> lock(mutex_raw_klines);
            auto it = routineLocal_raw_klines.find(security_code);
            if (it != routineLocal_raw_klines.end()) {
                cache_raw = it->second;
            }
        }

        if (cache_raw.empty()) {
            auto inst_opt = tdx::instruments::GetInstrumentInfo(security_code);
            if (inst_opt) {
                cache_raw = tdx::load_kline(*inst_opt);
            }
            update_cache_raw_klines(security_code, cache_raw);
        }

        size_t rows = cache_raw.size();
        if (rows == 0) return {};

        // 1.1 检查是否最新
        if (cache_raw[rows - 1].date < fixed_date) {
            auto inst_opt = tdx::instruments::GetInstrumentInfo(security_code);
            if (inst_opt) {
                cache_raw = tdx::load_kline(*inst_opt);
            }
            update_cache_raw_klines(security_code, cache_raw);
        }

        // 2. 对齐日期
        int offset = tdx::check_kline_offset(cache_raw, fixed_date);
        if (offset < 0) return {};

        auto sliced = std::vector<meta::schema::Bar>(cache_raw.begin(), cache_raw.end() - offset);

        // 3. 获取XDXR并应用前复权
        auto inst_opt = tdx::instruments::GetInstrumentInfo(security_code);
        if (!inst_opt) return {};
        auto xdxrs = tdx::get_xdxr_list(*inst_opt);
        auto ts_start = meta::Timestamp(sliced[0].date).pre_market_time();
        auto ts_end   = meta::Timestamp(sliced.back().date).pre_market_time();
        tdx::apply_forward_adjustments_once(sliced, xdxrs, ts_start, ts_end);

        // 4. 转换为 data::KLine
        std::vector<data::KLine> result;
        result.reserve(sliced.size());
        for (auto const& bar : sliced) {
            result.push_back(bar_to_kline(bar, security_code));
        }
        return result;
    }

} // namespace factors
