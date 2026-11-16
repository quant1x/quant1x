// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#include <quant1x/app.h>
#include <sstream>
#include <quant1x/command.h>
#include <private/build-info.h>
#include <quant1x/config/config.h>
#include <quant1x/cache.h>
#include <quant1x/base/api.h>
#include <quant1x/base/argparser/parser.hpp>
#include <quant1x/base/filesystem.h>
#include <quant1x/runtime/core.h>

// TDX 数据适配器 — 显式注册以强制 linker 拉入对应目标文件
#include <quant1x/contrib/data/tdx/bar.h>
#include <quant1x/contrib/data/tdx/bar_minute.h>
#include <quant1x/contrib/data/tdx/trans.h>
#include <quant1x/contrib/data/tdx/minute.h>
#include <quant1x/contrib/data/tdx/xdxr.h>
#include <quant1x/contrib/data/tdx/bar_raw.h>
// #if HAVE_MIMALLOC
// #include <mimalloc.h>
// #endif

namespace {
    /// 取选项的文本取值; 未给出时返回空串
    std::string option_text(const ::argparser::Namespace &namespace_value, std::string_view key) {
        const auto *value = namespace_value.find(key);
        if (value == nullptr) {
            return {};
        }
        const auto *text = value->as_string();
        return text == nullptr ? std::string{} : *text;
    }

    /// 取开关的真值; 未给出或不是布尔时返回 false
    bool option_flag(const ::argparser::Namespace &namespace_value, std::string_view key) {
        const auto *value = namespace_value.find(key);
        if (value == nullptr) {
            return false;
        }
        const auto *flag = value->as_bool();
        return flag != nullptr && *flag;
    }
}  // namespace

static std::string build_version_info() {
    std::ostringstream oss;
    oss << "  quant1x project: "<< filesystem::executable_name() << " - Build Info\n";
    oss << "--------------------------------------------------------------------------------\n";
    oss << "               Version : " << VERSION_STRING << "\n";
    oss << "                Author : " << GIT_AUTHOR_NAME << " <" << GIT_AUTHOR_EMAIL << ">\n";
    oss << "           Commit Hash : " << GIT_COMMIT_HASH << "\n";
    oss << "            Latest Tag : " << GIT_LATEST_TAG << "\n";
    oss << "            Build Date : " << GIT_BUILD_DATE << "\n";
    oss << "--------------------------------------------------------------------------------\n";
    oss << "      Operating System : " << CXX_OS << "\n";
    oss << "          Build System : " << CXX_SYSTEM << "\n";
    oss << "Processor Architecture : " << CXX_ARCH << "\n";
    oss << "          C++ Standard : " << CXX_STANDARD << "\n";
    oss << " C++ Standard Required : " << CXX_STANDARD_REQUIRED << "\n";
    oss << "        C++ Extensions : " << CXX_EXTENSIONS << "\n";
    oss << "       C++ Compiler ID : " << CXX_COMPILER_ID << "\n";
    oss << "  C++ Compiler version : " << CXX_COMPILER_VERSION << "\n";
    oss << "             C++ flags : " << CXX_FLAGS << "\n";
    auto build_type = strings::to_lower(BUILD_TYPE);
    if(build_type == "debug") {
        oss << "      C++ flags(Debug) : ";
        oss << CXX_FLAGS_DEBUG << "\n";
        oss << "   Linker flags(Debug) : ";
        oss << EXE_LINKER_FLAGS_DEBUG << "\n";
    } else {
        oss << "    C++ flags(Release) : ";
        oss << CXX_FLAGS_RELEASE << "\n";
        oss << " Linker flags(Release) : ";
        oss << EXE_LINKER_FLAGS_RELEASE << "\n";
    }
    oss << "Target Compile Options : " << TARGET_COMPILE_OPTIONS << "\n";
    return oss.str();
}

// 应用入口
int main(const int argc, const char *const argv[]) {
    // 提取默认的 program_name
    std::string program_name = filesystem::executable_name();
    std::string program_version = build_version_info();

    runtime::global_init();
    quant1x::app::init_datasource();
    {
        auto const &config = quant1x::config::TraderConfig();
        (void)config;
    }

    // 创建主解析器, 使用动态的 program_name
    ::argparser::Parser program(program_name);
    program.add_argument({"--verbose"}, ::argparser::Argument{
                                             .action = ::argparser::Action::STORE_TRUE,
                                             .help   = "显示日志信息到终端",
                                         });
    program.add_argument({"--debug"}, ::argparser::Argument{
                                           .action = ::argparser::Action::STORE_TRUE,
                                           .help   = "打开日志的调试模式",
                                       });
    program.add_argument({"--version"}, ::argparser::Argument{
                                             .action = ::argparser::Action::STORE_TRUE,
                                             .help   = "Print build version information and exit",
                                         });

    auto &subparsers = program.add_subparsers("command", "命令", false);

    // 服务子命令
    auto &service_command = subparsers.add_parser("service", "Manage the service.");
    ::argparser::Argument action_argument;
    action_argument.help    = "install | uninstall | start | stop | status | run";
    action_argument.choices = std::vector<::argparser::Choice>{
        {::argparser::Value("install"), ""},
        {::argparser::Value("uninstall"), ""},
        {::argparser::Value("start"), ""},
        {::argparser::Value("stop"), ""},
        {::argparser::Value("status"), ""},
        {::argparser::Value("run"), ""},
    };
    service_command.add_argument({"action"}, action_argument);
    service_command.add_argument({"--pipe"}, ::argparser::Argument{});

    // 存储子命令名称与对应解析器的映射
    std::map<std::string, ::argparser::Parser *> subparser_map;
    // 动态注册子命令
    for (auto &subcommand: quant1x::subcommands) {
        auto &sub_parser = subparsers.add_parser(subcommand.name, subcommand.help);
        for(auto *flag : subcommand.args) {
            ::argparser::Argument argument;
            if (flag->is_switch) {
                argument.action = ::argparser::Action::STORE_TRUE;
            } else if (!flag->defaultValue.empty()) {
                argument.default_value = ::argparser::DefaultValue::of(::argparser::Value(flag->defaultValue));
            }
            if (!flag->usage.empty()) {
                argument.help = flag->usage;
            }
            sub_parser.add_argument({flag->use}, argument);
        }
        subparser_map[subcommand.name] = &sub_parser;  // 保存子命令名称与解析器的映射
    }

    // 解析命令行参数: 帮助 / 版本退 0, 用法错误退 2
    std::vector<std::string> arguments(argv + 1, argv + argc);
    auto outcome = program.parse_or_exit(arguments, std::cout, std::cerr);
    if (std::holds_alternative<int>(outcome)) {
        return std::get<int>(outcome);
    }
    auto const &namespace_value = std::get<::argparser::Namespace>(outcome);

    // 如果请求, 打印构建/版本信息并提前退出
    if (option_flag(namespace_value, "version")) {
        std::cout << program_version << std::endl;
        return 0;
    }

    // 设置日志信息
    runtime::logger_set(option_flag(namespace_value, "verbose"), option_flag(namespace_value, "debug"));
    quant1x::app::init([] {
        namespace data = quant1x::data;

        std::cout << "这里执行定制的初始化工作" << std::endl;
        // TODO: API migration — user strategy files masked
        // // 注册1号特征
        // data::Register(std::make_unique<DataNo0>());
        // // 注册策略
        // StrategyManager& manager = StrategyManager::Instance();
        // StrategyPtr s0 = std::make_shared<No0Strategy>();
        // manager.Register(s0);

        // TDX 基础数据适配器已在 init_datasource() 中统一注册, 此处不再重复注册
    });

    const std::string command_name = option_text(namespace_value, "command");
    if (command_name == "service") {
        return quant1x::app::daemon(namespace_value);
    }

    // 判断激活的子命令并执行对应逻辑
    for (const auto &subcommand: quant1x::subcommands) {
        if (command_name == subcommand.name) {
            // 找到对应的子解析器
            if (auto it = subparser_map.find(subcommand.name); it != subparser_map.end()) {
                subcommand.handler(*(it->second), namespace_value);  // 执行与子命令绑定的逻辑
                spdlog::default_logger()->flush();
            }
            spdlog::default_logger()->flush();
            return 0;
        }
    }

    // 如果没有找到子命令
    spdlog::default_logger()->flush();
    for(int i = 0; i< argc; ++i) {
        std::cerr << argv[i]<< " ";
    }
    std::cerr<< std::endl;
    std::cerr << "Error: No command provided." << std::endl;
    program.print_help(std::cerr);
    return 1;
}
