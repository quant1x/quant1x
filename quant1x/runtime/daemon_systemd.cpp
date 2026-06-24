#include <fcntl.h>
#include <pwd.h>
#include <quant1x/runtime/core.h>
#include <quant1x/runtime/service.h>
#include <quant1x/std/safe.h>
#include <signal.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

namespace service {
    namespace {
        ServiceConfig g_api_service_config;
    }

    // 获取当前可执行文件路径
    std::string get_self_executable_path() {
        char    result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        if (count != -1) {
            return std::string(result, count);
        }
        return "";
    }

    // 获取当前用户名
    std::string get_current_username() {
        auto sudo_user = safe::getenv("SUDO_USER");
        if (sudo_user) {
            return *sudo_user;
        }

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

    // 安装服务(创建 .service 文件)
    void install() {
        if (!require_root_and_relaunch("install")) {
            return;
        }

        auto executable_path = get_self_executable_path();
        if (executable_path.empty()) {
            spdlog::error("[-] 无法获取可执行文件路径");
            return;
        }

        const std::string service_name = g_api_service_config.service_name;
        const std::string service_file = "/etc/systemd/system/" + service_name + ".service";

        std::ofstream file(service_file);
        if (!file.is_open()) {
            spdlog::error("[-] 无法创建服务文件: {}", service_file);
            return;
        }

        std::string user = get_current_username();

        file << "[Unit]\n"
             << "Description=" << g_api_service_config.description << "\n"
             << "After=network.target\n\n"

             << "[Service]\n"
             << "ExecStart=" << executable_path << " service run\n"
             << "WorkingDirectory=/\n"
             << "Restart=always\n"
             << "User=" << user << "\n";  // 👈 以当前用户运行

        // 如果服务需要访问 GUI/X11, 可以启用下面这行
        // file << "Environment=\"DISPLAY=:0\" \"XAUTHORITY=/home/" << user << "/.Xauthority\"\n";

        file << "\n[Install]\n"
             << "WantedBy=multi-user.target\n";

        file.close();

        spdlog::info("[+] 服务文件已创建于: {}", service_file);

        system("systemctl daemon-reexec");
        std::string enable_cmd = "systemctl enable " + service_name + ".service";
        system(enable_cmd.c_str());
        spdlog::info("[+] 服务安装完成, 并设置为开机自启");
    }

    // 卸载服务
    void uninstall() {
        if (!require_root_and_relaunch("uninstall")) {
            return;
        }

        const std::string service_name = g_api_service_config.service_name;
        const std::string service_file = "/etc/systemd/system/" + service_name + ".service";

        std::string disable_cmd = "systemctl disable " + service_name + ".service";
        system(disable_cmd.c_str());

        if (unlink(service_file.c_str()) == 0) {
            spdlog::info("[+] 成功删除服务文件: {}", service_file);
        } else {
            spdlog::error("[-] 删除服务文件失败");
        }

        system("systemctl daemon-reexec");
        spdlog::info("[+] 服务卸载完成");
    }

    // 启动服务
    void start() {
        if (!require_root_and_relaunch("start")) {
            return;
        }

        const std::string service_name = g_api_service_config.service_name;
        std::string       cmd          = "systemctl start " + service_name + ".service";
        system(cmd.c_str());
        spdlog::info("[+] 服务已启动: {}", service_name);
    }

    // 停止服务
    void stop() {
        if (!require_root_and_relaunch("stop")) {
            return;
        }

        const std::string service_name = g_api_service_config.service_name;
        std::string       cmd          = "systemctl stop " + service_name + ".service";
        system(cmd.c_str());
        spdlog::info("[+] 服务已停止: {}", service_name);
    }

    // 查询服务状态
    void query_status() {
        if (!require_root_and_relaunch("status")) {
            return;
        }

        const std::string service_name = g_api_service_config.service_name;
        std::string       cmd          = "systemctl status " + service_name + ".service";
        system(cmd.c_str());
    }

    // 守护进程主逻辑(带日志)
    void run_daemon() {
        //        // 第一次 fork: 创建子进程, 父进程退出
        //        pid_t pid = fork();
        //        if (pid < 0) {
        //            std::cerr << "[-] 第一次 fork 失败" << std::endl;
        //            exit(EXIT_FAILURE);
        //        }
        //        if (pid > 0) {
        //            exit(EXIT_SUCCESS);  // 父进程退出
        //        }
        //
        //        // 创建新会话
        //        pid_t sid = setsid();
        //        if (sid < 0) {
        //            std::cerr << "[-] setsid 失败" << std::endl;
        //            exit(EXIT_FAILURE);
        //        }
        //
        //        // 忽略挂断信号
        //        signal(SIGCHLD, SIG_IGN);
        //        signal(SIGHUP, SIG_IGN);
        //
        //        // 第二次 fork: 确保不会重新获得控制终端
        //        pid = fork();
        //        if (pid < 0) {
        //            std::cerr << "[-] 第二次 fork 失败" << std::endl;
        //            exit(EXIT_FAILURE);
        //        }
        //        if (pid > 0) {
        //            exit(EXIT_SUCCESS);  // 子进程退出
        //        }
        //
        //        // 设置 umask
        //        umask(0);
        //
        //        // 切换工作目录到根目录或用户目录
        //        chdir("/");
        //
        //        // 关闭标准输入输出
        //        close(STDIN_FILENO);
        //        close(STDOUT_FILENO);
        //        close(STDERR_FILENO);
        //
        //        // 重定向 stdin/stdout/stderr 到 /dev/null
        //        int dev_null_fd = open("/dev/null", O_RDWR);
        //        if (dev_null_fd != -1) {
        //            dup2(dev_null_fd, STDIN_FILENO);
        //            dup2(dev_null_fd, STDOUT_FILENO);
        //            dup2(dev_null_fd, STDERR_FILENO);
        //            close(dev_null_fd);
        //        }

        // 获取当前用户名(用于日志路径)
        uid_t          uid      = getuid();
        struct passwd *pw       = getpwuid(uid);
        std::string    username = pw ? pw->pw_name : "unknown";

        // 日志路径: 放在用户目录下, 避免权限问题
        std::string log_path = "/home/" + username + "/logs/q1x/";

        // 创建日志目录(如果不存在)
        struct stat st{};
        if (stat(log_path.c_str(), &st) == -1) {
            mkdir(log_path.c_str(), 0755);
        }

        // 初始化日志系统(文件日志)
        auto file_logger = spdlog::basic_logger_mt("file_logger", log_path + "q1x.log");
        file_logger->set_level(spdlog::level::debug);
        file_logger->flush_on(spdlog::level::debug);  // 每次都刷新, 避免缓冲区延迟

        // (可选)初始化控制台日志(仅用于调试时查看输出)
        // auto console_logger = spdlog::stdout_color_mt("console");
        // console_logger->set_level(spdlog::level::info);

        // 示例日志
        file_logger->info("[*] 守护进程已启动");
        // console_logger->info("[*] 守护进程已启动");
        runtime::logger_set(false, config::is_debug());

        runtime::wait_for_exit();
    }
}  // namespace service