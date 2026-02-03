#include "filesystem.h"
#include "strings.h"
#include "safe.h"

#include <cstdlib>
#include <filesystem>
#include <optional>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace filesystem {

    namespace fs = std::filesystem;

    std::string homedir() {
        auto homeOpt = safe::getenv("QUANT1X_HOME");
        if (homeOpt) {
            return *homeOpt;
        }
        homeOpt = safe::getenv("GOX_HOME");
        if (homeOpt) {
            return *homeOpt;
        }
        homeOpt = safe::getenv("HOME");
        if (homeOpt) {
            return *homeOpt;
        }
#ifdef _WIN32
        homeOpt = safe::getenv("USERPROFILE");
        if (homeOpt) {
            return *homeOpt;
        }
#endif
        return fs::temp_directory_path().generic_string();
    }

    std::string expand_user(const std::string &path) {
        std::string filepath = strings::trim(path);
        if (filepath.empty() || filepath[0] != '~') {
            return filepath;
        }

        std::string home = homedir();
        if (filepath.size() == 1) {
            return home;
        }
        
        if (filepath[1] == '/' || filepath[1] == '\\') {
            // 使用 fs::path / 运算符处理路径拼接，确保分隔符正确
            // 注意：必须使用 substr(2) 去掉 "~/"，否则 "/xxx" 会被视为绝对路径而覆盖 home
            return (fs::path(home) / filepath.substr(2)).string();
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
        std::string expanded = expand_user(filename);
        fs::path file_path(expanded);
        fs::path dir_path = file_path.parent_path();

        return mkdirs(dir_path.string(), notExistToCreate);
    }

} // namespace filesystem
