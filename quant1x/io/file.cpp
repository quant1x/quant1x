#include <quant1x/io/file.h>

// macOS 特有宏定义必须放在最前面
#ifdef __APPLE__
#define _DARWIN_C_SOURCE
#include <fcntl.h>

// 替换 st_mtim
#define st_mtim st_mtimespec

// 自定义 clock_cast 替代
template<typename ToClock, typename Rep, typename Period>
auto manual_clock_cast(const std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<Rep, Period>>& tp)
    -> std::chrono::time_point<ToClock, typename ToClock::duration>
{
    using Duration = typename ToClock::duration;
    auto d = tp.time_since_epoch();
    return std::chrono::time_point<ToClock, Duration>(std::chrono::duration_cast<Duration>(d));
}

#endif // __APPLE__

#include <chrono>
#include <string>
#include <system_error>
#include <type_traits>

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>
#include <sys/time.h>
#endif

#include <quant1x/std/time.h>
#include <filesystem>

#include <iostream>
#include <fstream>
#include <sys/stat.h>  // 用于 _utime 或 utimes（取决于平台）
// 跨平台兼容性处理
#if defined(_MSC_VER) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include <sys/utime.h>  // Windows 下使用 _utime
#else
#include <sys/time.h>  // Linux/macOS 使用 utimes
#endif
#include <quant1x/std/util.h>

namespace {

    template<typename ToClock, typename Clock, typename Duration>
    auto clock_cast(const std::chrono::time_point<Clock, Duration>& tp)
        -> std::chrono::time_point<ToClock, typename ToClock::duration>
    {
    #if defined(__cpp_lib_chrono_caster) && __cpp_lib_chrono_caster >= 201907L
        return std::chrono::clock_cast<ToClock>(tp);
    #else
        // 手动转换（适用于 macOS Clang）
        auto d = tp.time_since_epoch();
        return std::chrono::time_point<ToClock, typename ToClock::duration>(
            std::chrono::duration_cast<typename ToClock::duration>(d)
        );
    #endif
    }
//#ifdef _WIN32
//    std::chrono::system_clock::time_point getWindowsModificationTime(const std::string& filePath)
//    {
//        HANDLE hFile = CreateFileA(
//            filePath.c_str(),
//            GENERIC_READ,
//            FILE_SHARE_READ | FILE_SHARE_WRITE,
//            nullptr,
//            OPEN_EXISTING,
//            FILE_ATTRIBUTE_NORMAL,
//            nullptr);
//
//        if (hFile == INVALID_HANDLE_VALUE) {
//            throw std::system_error(GetLastError(), std::system_category());
//        }
//
//        FILETIME ftCreate, ftAccess, ftWrite;
//        if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite)) {
//            CloseHandle(hFile);
//            throw std::system_error(GetLastError(), std::system_category());
//        }
//
//        CloseHandle(hFile);
//
//        // 将FILETIME转换为system_clock::time_point
//        ULARGE_INTEGER uli;
//        uli.LowPart = ftWrite.dwLowDateTime;
//        uli.HighPart = ftWrite.dwHighDateTime;
//
//        // Windows时间（1601-01-01）到Unix时间（1970-01-01）的转换
//        constexpr uint64_t WINDOWS_TICK = 10000000;
//        constexpr uint64_t UNIX_EPOCH = 11644473600ULL * WINDOWS_TICK;
//
//        uint64_t nanoseconds = (uli.QuadPart - UNIX_EPOCH) * 100;
//        return std::chrono::system_clock::time_point(
//            std::chrono::duration_cast<std::chrono::system_clock::duration>(
//                std::chrono::nanoseconds(nanoseconds)
//            )
//        );
//    }
//
//    void setWindowsFileTimes(
//        const std::string& filePath,
//        const std::chrono::system_clock::time_point& createTime,
//        const std::chrono::system_clock::time_point& modifyTime,
//        const std::chrono::system_clock::time_point& accessTime)
//    {
//        // 转换时间格式
//        const auto toFileTime = [](const auto& tp) {
//            using namespace std::chrono;
//            // 定义目标时区（例如 Asia/Shanghai）
//            const auto target_tz = locate_zone("Asia/Shanghai");
//
//            // 将 UTC 时间点转换为目标时区的本地时间
//            auto current = target_tz->to_local(tp);
//            // 获取自 1970-01-01 以来的持续时间
//            auto since_epoch = current.time_since_epoch();
//
//            // 转换为以 100 纳秒为单位的持续时间
//            auto ft_duration = duration_cast<duration<int64_t, std::ratio<1, 10'000'000>>>(since_epoch);
//
//            // 计算从 1601-01-01 到 1970-01-01 的偏移量（100 纳秒单位）
//            constexpr int64_t offset = 116444736000000000LL;
//            int64_t total_100ns = ft_duration.count() + offset;
//
//            // 构造 FILETIME 结构
//            FILETIME ft;
//            ft.dwLowDateTime = static_cast<DWORD>(total_100ns & 0xFFFFFFFF);
//            ft.dwHighDateTime = static_cast<DWORD>((total_100ns >> 32) & 0xFFFFFFFF);
//            return ft;
//        };
//
//        // 打开文件句柄
//        HANDLE hFile = CreateFileA(
//            filePath.c_str(),
//            FILE_WRITE_ATTRIBUTES,
//            FILE_SHARE_READ | FILE_SHARE_WRITE,
//            nullptr,
//            OPEN_EXISTING,
//            FILE_ATTRIBUTE_NORMAL,
//            nullptr);
//
//        if (hFile == INVALID_HANDLE_VALUE) {
//            throw std::system_error(GetLastError(), std::system_category());
//        }
//
//        // 设置文件时间
//        FILETIME creationTime = toFileTime(createTime);
//        FILETIME lastAccessTime = toFileTime(accessTime);
//        FILETIME lastWriteTime = toFileTime(modifyTime);
//
//        if (!SetFileTime(hFile, &creationTime, &lastAccessTime, &lastWriteTime)) {
//            CloseHandle(hFile);
//            throw std::system_error(GetLastError(), std::system_category());
//        }
//
//        CloseHandle(hFile);
//    }
//#else
//    static std::chrono::system_clock::time_point getPosixModificationTime(const std::string& filePath)
//    {
//        struct stat fileStat;
//        if (stat(filePath.c_str(), &fileStat) != 0) {
//            throw std::system_error(errno, std::generic_category());
//        }
//
//        // 转换timespec到time_point
//        auto duration = std::chrono::seconds(fileStat.st_mtime) +
//                       std::chrono::nanoseconds(fileStat.st_mtim.tv_nsec);
//
//        return std::chrono::system_clock::time_point(
//            std::chrono::duration_cast<std::chrono::system_clock::duration>(duration)
//        );
//    }
//
//    static void setPosixFileTimes(
//        const std::string& filePath,
//        const std::chrono::system_clock::time_point& modifyTime,
//        const std::chrono::system_clock::time_point& accessTime)
//    {
//        // 转换时间格式
//        const auto toTimeSpec = [](const auto& tp) {
//            auto duration = tp.time_since_epoch();
//            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
//            auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - seconds);
//
//            timespec ts;
//            ts.tv_sec = seconds.count();
//            ts.tv_nsec = nanoseconds.count();
//            return ts;
//        };
//
//        timespec times[2];
//        times[0] = toTimeSpec(accessTime); // 访问时间
//        times[1] = toTimeSpec(modifyTime); // 修改时间
//
//        if (utimensat(AT_FDCWD, filePath.c_str(), times, 0) != 0) {
//            throw std::system_error(errno, std::generic_category());
//        }
//    }
//#endif
}

namespace io {

    std::string remove_extension(const std::string& path_str) {
        std::filesystem::path p(path_str);
        // 分解为路径 + 文件名（含可能的扩展名）
        auto parent_path = p.parent_path();
        auto filename = p.filename().generic_string();

        // 找到最后一个点的位置
        size_t dot_pos = filename.find_last_of('.');
        // 如果有点，并且不在开头或结尾，就去掉后缀
        while (dot_pos != std::string::npos && dot_pos > 0 && dot_pos < filename.length() - 1) {
            filename = filename.substr(0, dot_pos);
            dot_pos = filename.find_last_of('.');
        }

        return (parent_path / filename).string();
    }

    std::string executable_absolute_path() {
        std::filesystem::path executable_path;

        // Windows
#if defined(_WIN32)
        wchar_t buffer[MAX_PATH];
        GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        executable_path = buffer;

        // Linux
#elif defined(__linux__)
        executable_path = std::filesystem::read_symlink("/proc/self/exe");

    // macOS
#elif defined(__APPLE__)
        char buffer[PATH_MAX];
        uint32_t size = sizeof(buffer);
        if (_NSGetExecutablePath(buffer, &size) == 0) {
            executable_path = buffer;
        }

    // 其他平台（如无明确方法，回退到不可靠的 argv[0]）
#else
        executable_path = "unknown";
#endif
        std::string app_absolute_path = executable_path.lexically_normal().generic_string();
        std::string app_base_name = remove_extension(app_absolute_path);
        return app_base_name;
    }

    std::string executable_name() {
        std::filesystem::path executable_path = executable_absolute_path();
        auto exec_filename = executable_path.filename().string();
        return exec_filename;
    }

    int64_t last_modified_time(const std::string &filename) {
        if (!std::filesystem::exists(filename)) {
            return 0;
        }
        auto mtime = std::filesystem::last_write_time(filename);

        // 使用 clock_cast 转换为 system_clock 的时间点
        auto sys_time = clock_cast<std::chrono::system_clock>(mtime);

        // 获取 Unix 时间戳（毫秒）
        int64_t utc_ms = std::chrono::duration_cast<std::chrono::milliseconds>(sys_time.time_since_epoch()).count();

        // 转换为本地时间（假设你有现成的 api::ms_utc_to_local 函数）
        return api::ms_utc_to_local(utc_ms);
    }

    void last_modified_time(const std::string& filename, const int64_t& milliseconds) {
        // 转换成本地时间对应的 UTC 毫秒
        auto utc_ms = api::ms_local_to_utc(milliseconds);

        // 构造 system_clock 的 time_point
        auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(utc_ms));

        // 使用 clock_cast 转换为 file_time_type 所使用的时钟的时间点
        std::error_code ec;
        auto ftime = clock_cast<std::filesystem::file_time_type::clock>(tp);

        // 设置文件最后修改时间
        std::filesystem::last_write_time(filename, ftime, ec);
    }

    bool write_file(const std::string& filename, const char *data, size_t size) {
        try {
            util::check_filepath(filename, true);
            std::ofstream file(filename, std::ios::binary | std::ios::out | std::ios::trunc);
            if(!file.is_open()) {
                return false;
            }
            file.write(data, size);
            file.close();
            return true;
        } catch(...) {
            return false;
        }
    }
}