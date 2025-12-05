#include <quant1x/quant1x.h>
#include <sstream>
#include "user/no0.h"
#include "user/strategy-no0.h"
#include <quant1x/command.h>
#include <quant1x/datasets.h>
#include <private/build-info.h>
#include <quant1x/config/config.h>
#include <quant1x/cache.h>
// #if HAVE_MIMALLOC
// #include <mimalloc.h>
// #endif

static std::string build_version_info() {
    std::ostringstream oss;
    oss << "  quant1x project: "<< io::executable_name() << " - Build Info\n";
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
//     // --- mimalloc status (compile-time conservative check) ---
// #ifdef DEPENDENCY_MIMALLOC
//     oss << "--------------------------------------------------------------------------------\n";
//     oss << "        mimalloc dependency: " << DEPENDENCY_MIMALLOC << "\n";
// #else
//     oss << "--------------------------------------------------------------------------------\n";
//     oss << "        mimalloc dependency: (not configured)\n";
// #endif
// #if HAVE_MIMALLOC
//     oss << "        mimalloc mode : " << MIMALLOC_MODE << "\n";
// #if defined(MI_OVERRIDE)
//     oss << "  new/delete routed to mimalloc: yes (global override)\n";
// #elif defined(MI_OVERRIDE_NEW_DELETE)
//     oss << "  new/delete routed to mimalloc: yes (new/delete override)\n";
// #else
//     oss << "  new/delete routed to mimalloc: no (mimalloc present but not configured to override new/delete)\n";
// #endif
// #else
//     oss << "        mimalloc mode : (not enabled)\n";
//     oss << "  new/delete routed to mimalloc: no\n";
// #endif
//     // Runtime proof: print mimalloc stats and exercise operator new/delete.
// #if HAVE_MIMALLOC
//     oss << "\n[mimalloc-runtime-check] printing stats (initial) to stdout\n";
//     mi_stats_print(NULL);
//     try {
//         char *p = new char[1 << 20]; // 1 MiB
//         (void)p[0];
//         delete [] p;
//     } catch(...) {
//         // ignore
//     }
//     oss << "[mimalloc-runtime-check] printing stats (after new/delete) to stdout\n";
//     mi_stats_print(NULL);
// #else
//     oss << "[mimalloc-runtime-check] mimalloc not available at build time (HAVE_MIMALLOC=0)\n";
// #endif
    return oss.str();
}

// 应用入口
int main(const int argc, const char *const argv[]) {
    // 提取默认的 program_name
    std::string program_name = io::executable_name();
    std::string program_version = build_version_info();
    // 创建主解析器，使用动态的 program_name
    argparse::ArgumentParser program(program_name, program_version);
    bool verbose = false;
    program.add_argument("--verbose")
        .help("显示日志信息到终端")
        .default_value(false)
        .store_into(verbose)
        .nargs(0);
    bool debug = false;
    program.add_argument("--debug")
        .help("打开日志的调试模式")
        .default_value(false)
        .store_into(debug)
        .nargs(0);

    argparse::ArgumentParser service_command("service");
    service_command.add_description("Manage the service.");
    service_command.add_argument("action")
        .help("install | uninstall | start | stop | status | run")
        .choices("install", "uninstall", "start", "stop", "status", "run");
    service_command.add_argument("--pipe");

    program.add_subparser(service_command);

    // 存储子命令解析器，确保其生命周期与主解析器一致
    std::vector<std::unique_ptr<argparse::ArgumentParser>> subparsers;

    // 存储子命令名称与对应解析器的映射
    std::map<std::string, argparse::ArgumentParser *> subparser_map;
    runtime::global_init();
    datasets::init();
    {
        auto const &config = config::TraderConfig();
        (void)config;
    }
    // 动态注册子命令
    for (auto &subcommand: quant1x::subcommands) {
        // 创建子命令解析器，并使用 unique_ptr 管理其生命周期
        auto sub_parser = std::make_unique<argparse::ArgumentParser>(subcommand.name);
        sub_parser->add_description(subcommand.help);
        for(auto flag : subcommand.args) {
            sub_parser->add_argument(flag->use)
                .help(flag->usage)
                .default_value(flag->defaultValue)
                .store_into(flag->value) // 不起作用的原因是default_value方法要提前执行
                ;
        }

        // 将子命令解析器添加到主解析器中
        program.add_subparser(*sub_parser);

        // 将子解析器存储到容器中，确保其生命周期足够长
        subparser_map[subcommand.name] = sub_parser.get();  // 保存子命令名称与解析器的映射
        subparsers.push_back(std::move(sub_parser));
    }

    // 解析命令行参数
    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }
    // 设置日志信息
    runtime::logger_set(verbose, debug);
    quant1x::engine::init([] {
        std::cout << "这里执行定制的初始化工作" << std::endl;
        // 注册1号特征
        cache::Register(std::make_unique<DataNo0>());
        // 注册策略
        StrategyManager& manager = StrategyManager::Instance();
        StrategyPtr s0 = std::make_shared<No0Strategy>();
        manager.Register(s0);
        
    });

    if(program.is_subcommand_used("service")) {
        return quant1x::engine::daemon(service_command);
    }

    // 判断激活的子命令并执行对应逻辑
    bool command_found = false;
    for (const auto &subcommand: quant1x::subcommands) {
        if (program.is_subcommand_used(subcommand.name)) {
            command_found = true;

            // 找到对应的子解析器
            if (auto it = subparser_map.find(subcommand.name); it != subparser_map.end()) {
                subcommand.handler(*(it->second));  // 执行与子命令绑定的逻辑
                spdlog::default_logger()->flush();
            }
            break; // 只处理一个子命令
        }
    }
    spdlog::default_logger()->flush();
    // 如果没有找到子命令
    if (!command_found) {
        for(int i = 0; i< argc; ++i) {
            std::cerr << argv[i]<< " ";
        }
        std::cerr<< std::endl;
        std::cerr << "Error: No command provided." << std::endl;
        std::cerr << program;
        return 1;
    }
    return 0;
}