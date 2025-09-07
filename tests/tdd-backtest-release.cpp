#include <q1x/datasets/kline.h>
#include <users/no1.h>
#include <q1x/backtest/backtest.h>
#include <indicators/progress_bar.hpp>

int main() {
    runtime::global_init();
    // 1. 准备回测配置
    backtest::BacktestConfig config{};
    config.initial_capital = 100000.0;  // 初始资金10万
    config.commission_rate = 0.0005;    // 0.05%手续费
    config.slippage_rate = 0.0002;    // 0.02%滑点

    // 设置回测时间范围(示例)
    using namespace std::chrono;
    config.start_time = system_clock::now() - hours(24 * 365);  // 1年前
    config.end_time = system_clock::now();                    // 现在

    exchange::timestamp timestamp = exchange::timestamp::parse("2025-05-28").pre_market_time();
    StrategyManager &manager = StrategyManager::Instance();

    StrategyPtr s1 = std::make_shared<HousNo1Strategy>();
    manager.Register(s1);

    std::cout << "已注册策略:\n" << manager.UsageStrategyList() << std::endl;

    auto strategy = manager.GetStrategy(ModelHousNo1);
    std::cout << strategy->DebugString() << std::endl;

    strategy->setTimestamp(timestamp);
    // 2. 创建回测引擎
    backtest::BacktestEngine engine(config, strategy);
    engine.initAccount();

    // 3. 加载市场数据 - 生成更有波动性的数据
    auto all_codes = exchange::GetCodeList();
    auto codeCount = all_codes.size();
    {
        indicators::ProgressBar bar{
            indicators::option::BarWidth{50},
            indicators::option::ForegroundColor{indicators::Color::cyan},
            indicators::option::Start{"["},
            indicators::option::Fill{"="},
            indicators::option::Lead{">"},
            indicators::option::Remainder{" "},
            indicators::option::End{"]"},
            indicators::option::ShowElapsedTime{true},
            indicators::option::ShowRemainingTime{true},
            indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}},
            indicators::option::ShowPercentage{true},
            indicators::option::ShowSpeed{true},
            indicators::option::MaxProgress{codeCount + 0},
            //indicators::option::Stream{std::cerr}
        };
        int processed_codes = 0;
        for (auto const &code: all_codes) {
            size_t current = ++processed_codes;
            std::string codePrefix = std::format("{}({}/{})", code, current, codeCount);
            bar.set_option(indicators::option::PrefixText{codePrefix + ""});
            // 4. 运行回测
            std::string securityCode = exchange::CorrectSecurityCode(code);
            // result.date = feature_date;
            if (exchange::AssertStockBySecurityCode(securityCode)) {
                engine.run(code);
            }
            bar.tick();
            //std::cout << "code: " << code << std::endl;
        }
        //bar.mark_as_completed();
    }
    runtime::console_set_utf8();
    //std::locale::global(std::locale("en_US.UTF-8"));
    // std::cout.imbue(std::locale());
    // 设置 stdout 为 UTF-8 输出模式（仅适用于 Windows）
    //_setmode(_fileno(stdout), _O_U8TEXT);
    // 5. 输出结果
    //engine.calculateResults();  // 重新计算绩效
    engine.printResults();
//    // 打印交易记录
//    const auto &backtest_data = engine.getBacktestData();
//    std::wcout << L"\n=== 交易记录 ===" << std::endl;
//    for (const auto &trade: backtest_data.trades) {
//        auto tt = trade.trade_time;
//        auto wtt = std::wstring(reinterpret_cast<const wchar_t *>(tt.c_str()));
//        std::wcout << wtt << L"  " << (trade.direction == TradeDirection::LONG ? L"买入" : L"卖出") << L" " << trade.quantity
//                  << L"股 @ " << trade.price << L" (手续费: " << trade.fee << L")" << std::endl;
//    }
    return 0;
}