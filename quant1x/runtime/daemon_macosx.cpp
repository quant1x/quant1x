#include <dirent.h>
#include <fcntl.h>
#include <mach-o/dyld.h>
#include <pwd.h>
#include <quant1x/runtime/core.h>
#include <quant1x/runtime/service.h>
#include <quant1x/base/safe.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

namespace service {

    namespace {
        ServiceConfig     g_api_service_config;
        const std::string g_mac_launch_service_name = "com.quant1x.q1x.service";
    }  // namespace

    // 获取当前可执行文件路径
    std::string get_self_executable_path() {
        char     result[PATH_MAX];
        uint32_t size = sizeof(result);
        if (_NSGetExecutablePath(result, &size) == 0) {
            return std::string(result);
        }
        return "";
    }

    // 获取当前用户名
    std::string get_current_username() {
        auto user = safe::getenv("USER");
        if (user) {
            return *user;
        }

        uid_t          uid = getuid();
        struct passwd *pw  = getpwuid(uid);
        if (pw) {
            return std::string(pw->pw_name);
        }
        return "unknown";
    }

    // 检查是否具有 root 权限
    bool is_root() {
        return getuid() == 0;
    }

    // 请求提权并重启(使用 sudo)
    bool require_root_and_relaunch(const std::string &choice) {
        if (is_root()) {
            return true;
        }

        spdlog::warn("[+] 正在请求 root 权限...");

        std::string self_path = get_self_executable_path();
        if (self_path.empty()) {
            spdlog::error("[-] 无法获取自身路径");
            return false;
        }

        std::vector<std::string> args = {"sudo", self_path, "service", choice};
        std::vector<char *>      c_args;
        for (auto &arg : args)
            c_args.push_back(const_cast<char *>(arg.c_str()));
        c_args.push_back(nullptr);

        execvp("sudo", c_args.data());

        spdlog::error("[-] 提权失败, 请手动使用 sudo 执行");
        return false;
    }

    // 获取 plist 文件路径
    std::string get_plist_path() {
        std::string user = get_current_username();
        return "/Users/" + user + "/Library/LaunchAgents/" + g_mac_launch_service_name + ".plist";
    }

    // 获取日志目录路径
    std::string get_log_dir() {
        std::string user = get_current_username();
        return "/Users/" + user + "/.q1x/logs/";
    }

    // 初始化日志系统
    void init_logger() {
        std::string log_dir = get_log_dir();

        // 创建日志目录
        struct stat st{};
        if (stat(log_dir.c_str(), &st) == -1) {
            mkdir(log_dir.c_str(), 0755);
        }

        auto logger = spdlog::basic_logger_mt("q1x_logger", log_dir + "q1x.log");
        logger->set_level(spdlog::level::debug);
        logger->flush_on(spdlog::level::debug);

        spdlog::set_default_logger(logger);
    }

    // 安装服务(创建 .plist 文件)
    void install() {
        // if (!require_root_and_relaunch("install")) {
        //     return;
        // }

        std::string executable_path = get_self_executable_path();
        if (executable_path.empty()) {
            std::cout << "[-] 无法获取可执行文件路径" << std::endl;
            spdlog::error("[-] 无法获取可执行文件路径");
            return;
        }

        std::string plist_path = get_plist_path();

        std::ofstream file(plist_path);
        if (!file.is_open()) {
            std::cout << "[-] 无法创建 plist 文件: " << plist_path << std::endl;
            spdlog::error("[-] 无法创建 plist 文件: {}", plist_path);
            return;
        }

        file << R"(
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>)"
             << g_mac_launch_service_name << R"(</string>

    <key>ProgramArguments</key>
    <array>
        <string>)"
             << executable_path << R"(</string>
        <string>service</string>
        <string>run</string>
    </array>

    <key>RunAtLoad</key>
    <true/>

    <key>KeepAlive</key>
    <true/>

    <key>WorkingDirectory</key>
    <string>/usr/local/var</string>

    <key>StandardOutPath</key>
    <string>/usr/local/var/log/)"
             << g_mac_launch_service_name << R"(.log</string>

    <key>StandardErrorPath</key>
    <string>/usr/local/var/log/)"
             << g_mac_launch_service_name << R"(.err</string>
</dict>
</plist>
)";

        file.close();
        std::cout << "[+] 已生成 macOS plist 文件: " << plist_path << std::endl;
        spdlog::info("[+] 已生成 macOS plist 文件: {}", plist_path);

        std::string cmd_load = "launchctl load " + plist_path;
        system(cmd_load.c_str());

        std::string cmd_start = "launchctl start " + g_mac_launch_service_name;
        system(cmd_start.c_str());

        std::cout << "[+] 服务安装并启动成功" << std::endl;
        spdlog::info("[+] 服务安装并启动成功");
    }

    // 卸载服务
    void uninstall() {
        // if (!require_root_and_relaunch("uninstall")) {
        //     return;
        // }

        std::string plist_path = get_plist_path();

        std::string cmd_stop   = "launchctl stop " + g_mac_launch_service_name;
        std::string cmd_remove = "launchctl remove " + g_mac_launch_service_name;

        system(cmd_stop.c_str());
        system(cmd_remove.c_str());

        if (unlink(plist_path.c_str()) == 0) {
            std::cout << "[+] 成功删除 plist 文件: " << plist_path << std::endl;
            spdlog::info("[+] 成功删除 plist 文件: {}", plist_path);
        } else {
            std::cout << "[-] 删除 plist 文件失败" << std::endl;
            spdlog::error("[-] 删除 plist 文件失败");
        }
        std::cout << "[+] 服务卸载完成" << std::endl;
        spdlog::info("[+] 服务卸载完成");
    }

    // 启动服务
    void start() {
        // if (!require_root_and_relaunch("start")) {
        //     return;
        // }

        std::string cmd = "launchctl load " + get_plist_path();
        system(cmd.c_str());
        std::cout << "[+] 服务已启动" << std::endl;
        spdlog::info("[+] 服务已启动");
    }

    // 停止服务
    void stop() {
        // if (!require_root_and_relaunch("stop")) {
        //     return;
        // }

        std::string cmd = "launchctl unload " + get_plist_path();
        system(cmd.c_str());
        std::cout << "[+] 服务已停止" << std::endl;
        spdlog::info("[+] 服务已停止");
    }

    // 检查 launchd 服务是否正在运行
    std::pair<std::string, bool> check_service_running(const std::string &service_name) {
        // 构建命令: launchctl list com.quant1x.q1x.service
        std::string cmd = "launchctl list " + service_name;

        // 使用 popen 执行命令并读取输出
        std::array<char, 128> buffer;
        std::string           result;
        FILE                 *pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            std::cerr << "Failed to run command" << std::endl;
            return {"Error executing command", false};
        }

        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }

        int exit_code = pclose(pipe);

        if (exit_code != 0) {
            return {"Service is stopped", false};
        }

        // 检查输出中是否包含服务名称(PID 存在说明运行中)
        std::regex service_regex(service_name);
        if (!std::regex_search(result, service_regex)) {
            return {"Service is stopped", false};
        }

        // 提取 PID(格式如 "PID" = 1234;)
        std::regex  pid_regex(R"(\"PID\" = ([0-9]+);)");
        std::smatch match;
        if (std::regex_search(result, match, pid_regex) && match.size() > 1) {
            std::string pid = match[1].str();
            return {"Service (pid " + pid + ") is running...", true};
        }

        return {"Service is running...", true};
    }

    // 查询服务状态
    void query_status() {
        // if (!require_root_and_relaunch("status")) {
        //     return;
        // }

        // std::string cmd = "launchctl list com.quant1x.q1x.service";
        // system(cmd.c_str());
        auto [status, running] = check_service_running(g_mac_launch_service_name);
        std::cout << status << std::endl;
    }

    // 守护进程主逻辑(被 launchd 管理时无需 fork)
    void run_daemon() {
        // 初始化日志系统
        init_logger();

        spdlog::info("[*] 守护进程已启动");

        runtime::logger_set(false, true);  // 示例设置日志器
        runtime::wait_for_exit();          // 主循环或等待退出信号
    }

}  // namespace service