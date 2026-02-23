#include <quant1x/std/api.h>
#include <quant1x/std/strings.h>
#include <quant1x/std/util.h>
#include <quant1x/std/filesystem.h>
#include <quant1x/config/base.h>
#include <quant1x/market/instruments.h>
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
            std::string expandedPath = filesystem::expand_user(path);
            filesystem::mkdirs(expandedPath);
            global_config().homeDir = std::move(expandedPath);
        } catch (const std::exception &e) {
            std::cerr << "路径初始化失败: " << e.what() << std::endl;
            std::terminate();
        }
    }

    /**
     * @brief 延迟初始化全局配置，仅在第一次调用时执行
     *
     * 该函数负责初始化全局配置，包括：
     * - 设置默认数据路径
     * - 加载并解析YAML配置文件
     * - 初始化日志目录
     * - 设置调试标志
     *
     * @note 该函数是线程安全的，使用静态局部变量确保只初始化一次
     * @note 如果配置文件解析失败，会使用默认配置继续运行
     *
     * @throws std::exception 当YAML文件解析失败时会捕获并打印异常信息
     *
     * 配置项包括：
     * - basedir: 基础目录路径
     * - debug: 调试模式标志
     * - 其他BaseConfig中定义的配置项
     */
    static void lazy_init() noexcept {
        static int count = 0;
        spdlog::info("lazy_init called: {}", ++count);
        init_path(defaultQuant1xDataPath);
        auto config_filename = filesystem::expand_user(global_config().homeDir + "/quant1x.yaml");
        global_config().filename = std::move(config_filename);
        try {
            YAML::Node yaml = YAML::LoadFile(global_config().filename);
            std::string base_dir;
            encoding::safe_yaml::parse_field(yaml, "basedir", base_dir, global_config().homeDir);
            global_config().cacheDir = filesystem::expand_user(base_dir);
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
        auto err = filesystem::mkdirs(global_config().logsDir, true);
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
} // namespace config
