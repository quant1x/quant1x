#include <fmt/format.h>
#include <quant1x/backtest/backtest.h>
#include <quant1x/contrib/data/tdx/bar.h>
#include <quant1x/contrib/data/tdx/instruments.h>
#include <user/no0.h>
#include <user/strategy-no0.h>

namespace instruments = quant1x::contrib::data::tdx::instruments;

#include <indicators/dynamic_progress.hpp>
#include <indicators/progress_bar.hpp>
#include <regex>

int main(int argc, char **argv) {
    namespace mpb = indicators;
    runtime::global_init();
    // Enable console logger and debug level to see strategy/engine spdlog output during tests
    runtime::logger_set(/*verbose=*/true, /*debug=*/true);

    // 1. 准备回测配置
    backtest::BacktestConfig config{};
    config.initial_capital = 100000.0;  // 初始资金10万
    config.commission_rate = 0.0005;    // 0.05%手续费
    config.slippage_rate   = 0.0002;    // 0.02%滑点
    config.verbose         = true;

    // 默认回测时间范围: 2025-09-01 到 2025-09-30
    std::string start_str = "2025-09-01";
    std::string end_str   = "2025-09-30";

    // 支持可选的命令行参数: [start_date end_date] 格式为 YYYY-MM-DD
    if (argc == 3) {
        start_str = argv[1];
        end_str   = argv[2];
    } else if (argc != 1) {
        std::cerr << "Usage: tdd-backtest-release [start_date end_date]\n";
        std::cerr << "  Dates must be in YYYY-MM-DD format, e.g. 2025-01-01 2025-09-30.\n";
        std::cerr << "  Example: tdd-backtest-release 2025-01-01 2025-09-30\n";
        return 1;
    }

    // 验证日期格式和范围
    std::regex ymd_re(R"(^\d{4}-\d{2}-\d{2}$)");
    if (argc == 3) {
        if (!std::regex_match(start_str, ymd_re) || !std::regex_match(end_str, ymd_re)) {
            std::cerr << "Error: Dates must be in YYYY-MM-DD format.\n";
            std::cerr << "  Received: '" << start_str << "' '" << end_str << "'\n";
            std::cerr << "  Example: tdd-backtest-release 2025-01-01 2025-09-30\n";
            return 2;
        }

        try {
            auto t0 = meta::Timestamp::parse(start_str);
            auto t1 = meta::Timestamp::parse(end_str);
            if (t0.value() > t1.value()) {
                std::cerr << "Error: start_date must be <= end_date.\n";
                std::cerr << "  Received: start='" << start_str << "' end='" << end_str << "'\n";
                return 3;
            }
            config.start_time = std::chrono::system_clock::time_point(std::chrono::milliseconds(t0.value()));
            config.end_time   = std::chrono::system_clock::time_point(std::chrono::milliseconds(t1.value()));

            // use parsed end timestamp for strategy timestamp (pre_market_time)
            meta::Timestamp timestamp = t1.pre_market_time();
            (void)timestamp;  // keep compiler happy until used below
        } catch (const std::exception &ex) {
            std::cerr << "Error: Failed to parse dates: " << ex.what() << "\n";
            return 2;
        }
    } else {
        // no args -> use defaults
        auto t0                       = meta::Timestamp(2025, 9, 1);
        auto t1                       = meta::Timestamp(2025, 9, 30);
        config.start_time             = std::chrono::system_clock::time_point(std::chrono::milliseconds(t0.value()));
        config.end_time               = std::chrono::system_clock::time_point(std::chrono::milliseconds(t1.value()));
        meta::Timestamp timestamp = t1.pre_market_time();
        (void)timestamp;
    }

    // Print accepted date range before running
    std::cout << "回测日期区间: " << start_str << " 至 " << end_str << std::endl;

    // parse end_str to get a timestamp for strategy time (we re-parse here to get the value in both branches)
    meta::Timestamp timestamp = meta::Timestamp::parse(end_str).pre_market_time();
    StrategyManager    &manager   = StrategyManager::Instance();

    // Register strategy and keep the shared_ptr locally (avoids cross-translation lookup issues)
    // StrategyPtr is an alias to std::shared_ptr<StrategyBase>
    StrategyPtr s0 = std::static_pointer_cast<StrategyBase>(std::make_shared<No0Strategy>());
    manager.Register(s0);

    std::cout << "已注册策略:\n" << manager.UsageStrategyList() << std::endl;
    std::cout << s0->DebugString() << std::endl;
    s0->setTimestamp(timestamp);

    // 2. 创建回测引擎
    backtest::BacktestEngine engine(config, s0);
    engine.initAccount();

    // 3. 加载市场数据
    auto all_codes = instruments::get_code_list();
    // For faster debugging restrict to a handful of codes so logs are manageable
    size_t                   debug_limit = 5;
    std::vector<std::string> debug_codes;
    for (size_t i = 0; i < all_codes.size() && debug_codes.size() < debug_limit; ++i) {
        auto        code         = all_codes[i];
        std::string securityCode = data::correct_security_code(code);
        if (data::assert_stock_by_security_code(securityCode)) {
            debug_codes.push_back(code);
        }
    }
    auto codeCount = static_cast<int64_t>(debug_codes.size());

    // Progress bar (pattern follows tests/test-multibar.cpp)
    mpb::DynamicProgress<mpb::ProgressBar> bars;
    mpb::ProgressBar                       bar{
        indicators::option::BarWidth{50},
        indicators::option::Start{"["},
        indicators::option::Fill{"="},
        indicators::option::Lead{">"},
        indicators::option::Remainder{" "},
        indicators::option::End{" ]"},
        indicators::option::PostfixText{"Backtesting"},
        indicators::option::ForegroundColor{indicators::Color::cyan},
        indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}},
        indicators::option::ShowPercentage{true},
        indicators::option::ShowSpeed{true},
        indicators::option::ShowElapsedTime{true},
        indicators::option::ShowRemainingTime{true}};
    bars.push_back(bar);
    bars[0].set_option(indicators::option::MaxProgress{codeCount});

    int processed_codes = 0;
    for (auto const &code : debug_codes) {
        ++processed_codes;
        bars[0].set_option(indicators::option::PrefixText{fmt::format("{}({}/{})", code, processed_codes, codeCount)});

        // 4. 运行回测
        std::string securityCode = data::correct_security_code(code);
        std::cout << "--- Backtesting " << securityCode << " ---\n";
        if (data::assert_stock_by_security_code(securityCode)) {
            engine.run(code);
        } else {
            std::cout << "skipping non-stock: " << securityCode << "\n";
        }

        bars[0].tick();
    }

    runtime::console_set_utf8();

    engine.printResults();
    return 0;
}