#include <quant1x/std/api.h>
#include <quant1x/std/strings.h>
#include <quant1x/std/util.h>
#include <quant1x/runtime/config.h>
#include <quant1x/exchange.h>
#include <quant1x/encoding/yaml.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>

namespace config {
    namespace fs = std::filesystem;

    // 默认的数据路径
    constexpr const char *const defaultQuant1xDataPath = "~/.q1x";

    // 懒加载标志
    std::once_flag global_cache_once;

    // 全局配置实例
    //BaseConfig global_quant1x_config;

    BaseConfig &global_config() {
        static BaseConfig instance;
        return instance;
    }


    // 初始化路径
    static void init_path(const std::string &path) {
        try {
            std::string expandedPath = util::expand_homedir(path);
            util::mkdirs(expandedPath);
            global_config().homeDir = std::move(expandedPath);
        } catch (const std::exception &e) {
            std::cerr << "路径初始化失败: " << e.what() << std::endl;
            std::terminate();
        }
    }

    // 懒加载初始化
    static void lazy_init() noexcept {
        static int count = 0;
        spdlog::info("lazy_init called: {}", ++count);
        init_path(defaultQuant1xDataPath);
        auto config_filename = util::expand_homedir(global_config().homeDir + "/quant1x.yaml");
        global_config().filename = std::move(config_filename);
        try {
            YAML::Node yaml = YAML::LoadFile(global_config().filename);
            std::string base_dir;
            encoding::safe_yaml::parse_field(yaml, "basedir", base_dir, global_config().homeDir);
            global_config().cacheDir = util::expand_homedir(base_dir);
            // 读取配置文件顶层的debug设置, 如果解析异常, 当作false处理
            bool in_debug = false;
            encoding::safe_yaml::parse_field(yaml, "debug", in_debug, false);
            global_config().running_in_debug = in_debug; // 设置全局调试标志
            auto const & baseConfig = encoding::yaml::deserialize<BaseConfig>(yaml);
            global_config().data = baseConfig.data;
        } catch (const std::exception &e) {
            // 解析yaml失败
            std::cerr << e.what() << std::endl;
            global_config().cacheDir = std::string(global_config().homeDir);
        }

        global_config().logsDir = global_config().cacheDir + "/logs";
        auto err = util::mkdirs(global_config().logsDir, true);
        err.clear();

        std::cerr << "lazy_init config_filename = " << &global_config().filename << ",[" << global_config().filename << "]\n";
    }

    static inline std::once_flag global_config_once;
    static inline std::shared_ptr<TraderParameter> global_trader_parameter; // 交易配置

    config::TraderParameter load_config_from_yaml(const std::string &filename) {
        spdlog::info("config file: {}", filename);
        config::TraderParameter config{};
        try {
            YAML::Node yaml = YAML::LoadFile(filename);
            config = yaml["trader"].as<config::TraderParameter>();
        } catch (const YAML::Exception &e) {
            std::cerr << "YAML解析错误: " << e.what() << std::endl;
            spdlog::error("YAML解析错误: {}", e.what());
        } catch (const std::exception &e) {
            std::cerr << "YAML解析错误: " << e.what() << std::endl;
            spdlog::error("YAML解析错误: {}", e.what());
        } catch (...) {
            std::cerr << "YAML解析错误: 未知" << std::endl;
            spdlog::error("YAML解析错误: 未知");
        }
        return config;
    }

    void lazy_load_trader_config() {
        // 先检查内存
        //check_memory_guard();
        std::cerr << "lazy_load_trader_config config_filename = " << &global_config().filename << ",[" << global_config().filename << "]\n";
        auto tmp_config_filename = config::config_filename();
        auto config = load_config_from_yaml(tmp_config_filename);
        global_trader_parameter = std::make_shared<TraderParameter>(config);
    }

    std::shared_ptr<TraderParameter> v2_TraderConfig() {
        static std::once_flag init_flag;
        static std::shared_ptr<TraderParameter> config;
        auto tmp_config_filename = config::config_filename();
        std::call_once(init_flag, [&] {
            config = std::make_shared<TraderParameter>(load_config_from_yaml(tmp_config_filename));
        });

        return config;
    }

    std::shared_ptr<TraderParameter> TraderConfig() {
        std::call_once(global_config_once, lazy_load_trader_config);
        return global_trader_parameter;
    }

    std::string config_filename() {
        std::call_once(global_cache_once, lazy_init);
        return global_config().filename;
    }

    bool is_debug() noexcept {
        std::call_once(global_cache_once, lazy_init);
        return global_config().running_in_debug;
    }

    // 获取用户主路径
    std::string default_home_path() {
        std::call_once(global_cache_once, lazy_init);
        return global_config().homeDir;
    }

    // 获取默认缓存路径
    std::string default_cache_path() {
        std::call_once(global_cache_once, lazy_init);
        return global_config().cacheDir;
    }

    // 获取元数据路径
    std::string get_meta_path() {
        fs::path p(default_home_path());
        p /= "meta";
        return p.string();
    }

    // 获取日志路径
    std::string get_logs_path() {
        fs::path p(default_cache_path());
        p /= "logs";
        return p.string();
    }

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
        if (!orderPath.empty() && !util::check_filepath(orderPath, true)) {
            qmtOrderPath = orderPath;
        }
        return qmtOrderPath;
    }
} // namespace config
