#include <quant1x/config/cache.h>
#include <quant1x/config/base.h>
#include <quant1x/exchange/markets.h>
#include <quant1x/std/filepath.h>

namespace config {
    namespace fs = std::filesystem;

    // 获取交易日历的缓存文件名
    std::string get_calendar_filename() {
        return get_meta_path() + "/calendar";
    }

    // 获取证券列表的缓存文件名
    std::string get_security_filename() {
        return get_meta_path() + "/securities.csv";
    }

    // 获取板块列表的缓存文件名${~/.quant1x/meta/blocks.${YYYY-mm-dd}}
    std::string get_sector_filename(const std::string &date) {
        // 板块文件是每天一个文件
        std::string filename = "blocks." + date;
        auto normalized = (fs::path(get_meta_path()) / filename).lexically_normal();
        return normalized.generic_string();
    }

    // 历史成交记录
    // 目录结构${trans}/${YYYY}/${YYYYMMDD}/${SecurityCode}.csv
    std::string get_historical_trade_filename(const std::string &code, const std::string &cache_date) {
        ASSERT(code.length() == 8, INVALID_SECURITY_CODE_MSG);
        std::string year = cache_date.substr(0, 4);
        std::string date = strings::replace_all(cache_date, "-", "");
        auto path = fs::path(default_cache_path()) / "trans";
        path /= year;
        path /= date;
        path /= (code + ".csv");
        auto normalized = path.lexically_normal();
        return normalized.generic_string();
    }

    // 筹码分布
    // 目录结构${trans}/${YYYY}/${YYYYMMDD}/${SecurityCode}.cd
    std::string get_chip_distribution_filename(const std::string &code, const std::string &cache_date) {
        ASSERT(code.length() == 8, INVALID_SECURITY_CODE_MSG);
        std::string year = cache_date.substr(0, 4);
        std::string date = strings::replace_all(cache_date, "-", "");
        auto path = fs::path(default_cache_path()) / "trans";
        path /= year;
        path /= date;
        path /= (code + ".cd");
        auto normalized = path.lexically_normal();
        return normalized.generic_string();
    }

    // 板块数据文件路径
    std::string get_block_path() {
        return get_meta_path();
    }

    // 除权除息文件路径
    std::string get_xdxr_path() {
        return default_cache_path() + "/xdxr";
    }

    // 日K线文件路径
    std::string get_day_path() {
        return default_cache_path() + "/day";
    }

    // 通用K线文件路径
    std::string get_kline_path(const std::string &freq) {
        return default_cache_path() + "/" + freq;
    }

    // 分时数据路径
    std::string get_minute_path() {
        return default_cache_path() + "/minutes";
    }

    constexpr int suffix_length = 3;

    static inline std::string subpath(const std::string &code) {
        auto length = code.length();
        if (length <= suffix_length) {
            return "";
        }
        return code.substr(0, length - suffix_length);
    }

    std::string get_xdxr_filename(const std::string &code) {
        ASSERT(code.length() == 8, INVALID_SECURITY_CODE_MSG);
        auto sub = subpath(code);
        auto path = fs::path(get_xdxr_path()) / sub;
        path /= (code + ".csv");
        auto normalized = path.lexically_normal();
        return normalized.generic_string();
    }

    std::string get_kline_filename(const std::string &code, bool forward) {
        ASSERT(code.length() == 8, INVALID_SECURITY_CODE_MSG);
        auto sub = subpath(code);
        auto path = fs::path(get_day_path()) / sub;
        path /= (code + "." + (forward ? "csv" : "raw"));
        auto normalized = path.lexically_normal();
        return normalized.generic_string();
    }

    std::string get_kline_filename_ex(const std::string &code, const std::string &freq) {
        ASSERT(code.length() == 8, INVALID_SECURITY_CODE_MSG);
        auto sub = subpath(code);
        auto path = fs::path(get_kline_path(freq)) / sub;
        path /= (code + ".csv");
        auto normalized = path.lexically_normal();
        return normalized.generic_string();
    }

    std::string get_minute_filename(const std::string &code, const std::string &cache_date) {
        ASSERT(code.length() == 8, INVALID_SECURITY_CODE_MSG);
        ASSERT(cache_date.length() == 8, INVALID_DATE_FORMAT_YMD_COMPACT_MSG);
        std::string year = cache_date.substr(0, 4);
        std::string date = strings::replace_all(cache_date, "-", "");
        auto path = fs::path(get_minute_path());
        path /= year;
        path /= date;
        path /= (code + ".csv");
        auto normalized = path.lexically_normal();
        return normalized.generic_string();
    }

    namespace detail {
        // CacheId 通过代码构建目录结构
        std::string CacheId(const std::string &code) {
            auto [_, marketCode, code_] = exchange::DetectMarket(code);
            return marketCode + code_;
        }

        // CacheIdPath code从后保留3位, 市场缩写+从头到倒数第3的代码, 确保每个目录只有000~999个代码
        std::string CacheIdPath(const std::string &code) {
            const size_t N = 3;
            std::string cacheId = CacheId(code);
            size_t length = cacheId.length();

            if (length <= N) {
                return cacheId; // 如果长度不足，直接返回整个字符串
            }

            std::string prefix = cacheId.substr(0, length - N);
            return prefix + "/" + cacheId;
        }
    } // namespace detail

    std::string GetHoldingPath() {
        return default_cache_path() + "/holding";
    }

    // top10_holders_filename 前十大流通股股东缓存文件名
    std::string top10_holders_filename(const std::string &code, const std::string &date) {
        auto idPath = detail::CacheIdPath(code);
        // 使用std::ignore忽略不需要的返回值
        std::string quarter;
        std::tie(quarter, std::ignore, std::ignore) = api::GetQuarterByDate(date);
        return GetHoldingPath() + "/" + quarter + "/" + idPath + ".csv";
    }

    std::string quarterly_cache_path(const std::string &date) {
        auto [q, x1, x2] = api::GetQuarterByDate(date);
        std::string path = default_cache_path() + "/infoq/" + q;
        return path;
    }

    std::string quarterly_filename(const std::string &date, const std::string &keyword) {
        return quarterly_cache_path(date) + "/" + keyword + ".csv";
    }

    std::string reports_filename(const std::string &date) {
        return quarterly_filename(date, "reports");
    }

    std::string defaultQmtCachePath() {
        return default_cache_path() + "/qmt";
    }

    std::string get_qmt_cache_path() {
        auto qmtOrderPath = defaultQmtCachePath();
        auto const &traderParameter = TraderConfig();
        auto &orderPath = traderParameter->OrderPath;
        if (!orderPath.empty() && !filepath::check_filepath(orderPath, true)) {
            qmtOrderPath = orderPath;
        }
        return qmtOrderPath;
    }
}
