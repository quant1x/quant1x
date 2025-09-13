#include <quant1x/std/util.h>

#include <duktape.h>
#include <algorithm> // for find_xxx
#include <string>
#include <vector>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <filesystem>
#include <iostream>
#include <spdlog/spdlog.h>
#include <quant1x/std/strings.h>

namespace util {

    // 跨平台安全获取环境变量
    static std::optional<std::string> getenv_str(const char *name) {
#ifdef _WIN32
        char  *value = nullptr;
        size_t len   = 0;
        if (_dupenv_s(&value, &len, name) == 0 && value != nullptr) {
            std::string result(value);
            free(value);
            return result;
        }
        return std::nullopt;
#else
        const char *value = std::getenv(name);
        if (value) {
            return std::string(value);
        }
        return std::nullopt;
#endif
    }

    //============================================================
    // 文件系统                                                   //
    //============================================================

    namespace fs = std::filesystem;

    std::string homedir() {
        auto homeOpt = getenv_str("QUANT1X_HOME");
        if (homeOpt) {
            return *homeOpt;
        }
        homeOpt = getenv_str("GOX_HOME");
        if (homeOpt) {
            return *homeOpt;
        }
        homeOpt = getenv_str("HOME");
        if (homeOpt) {
            return *homeOpt;
        }
#ifdef _WIN32
        homeOpt = getenv_str("USERPROFILE");
        if (homeOpt) {
            return *homeOpt;
        }
#endif
        return fs::temp_directory_path().generic_string();
    }

    std::string expand_homedir(const std::string &filename) {
        std::string filepath = strings::trim(filename);
        if (filepath.empty() || filepath[0] != '~') {
            return filepath;
        }

        std::string home = homedir();
        if (filepath.size() == 1 || filepath[1] == '/') {
            return std::string(home) + filepath.substr(1);
        }

        return filepath;
    }

    std::error_code mkdirs(const std::string &path, bool notExistToCreate) {
        std::string filepath = strings::trim(path);
        if (filepath.empty()) {
            return {}; // 根目录或无父目录，视为存在
        }

        std::error_code ec;
        bool exists = fs::exists(filepath, ec);

        // 检查是否存在或错误类型
        if (!ec && exists) {
            return {}; // 目录存在
        }

        // 处理错误，仅当错误是文件不存在时才继续
        if (ec && ec != std::errc::no_such_file_or_directory) {
            return ec;
        }

        // 若不需要创建，返回不存在错误
        if (!notExistToCreate) {
            return std::make_error_code(std::errc::no_such_file_or_directory);
        }

        // 使用枚举组合权限(0755)
        constexpr fs::perms perms =
            fs::perms::owner_all |     // 所有者：读 + 写 + 执行
            fs::perms::group_read | fs::perms::group_exec |   // 组：读 + 执行
            fs::perms::others_read | fs::perms::others_exec;  // 其他：读 + 执行
        // 递归创建目录
        bool created = fs::create_directories(filepath);
        if (created) {
            fs::permissions(filepath, perms, ec);
            return ec;
        }

        // 确认目录已创建且为目录类型
        auto dir_status = fs::status(filepath, ec);
        if (ec || !fs::is_directory(dir_status)) {
            return ec ? ec : std::make_error_code(std::errc::not_a_directory);
        }

        return {};
    }

    std::error_code check_filepath(const std::string &filename, bool notExistToCreate) {
        std::string expanded = expand_homedir(filename);
        fs::path file_path(expanded);
        fs::path dir_path = file_path.parent_path();

        return mkdirs(dir_path.string(), notExistToCreate);
    }

    // std::tm 转 time_point (假设tm为本地时间)
    std::chrono::system_clock::time_point tm_to_time_point(const std::tm& tm)
    {
        std::tm tmp = tm;
        const std::time_t t = mktime(&tmp);
        if(t == -1) {
            throw std::runtime_error("Invalid tm structure");
        }
        return std::chrono::system_clock::from_time_t(t);
    }

    uint64_t get_thread_id(const std::thread::id &tid) {
        return static_cast<uint64_t>(std::hash<std::thread::id>{}(tid));
    }
} // namespace util