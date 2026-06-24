//#include <quant1x/test/test.h>
//
//// ==============================
//// 分笔成交记录 - 大单阀值计算
//// ==============================
//#include <vector>
//#include <algorithm>
//#include <iostream>
//#include <cmath>
//
//// 一, 百分位法(Percentile cmd_id)
//// 对历史成交量或成交金额进行排序, 取其高分位数作为大单阈值. 
//
//// 计算百分位数(线性插值)
//double percentile(const std::vector<double>& data, double percent) {
//    if (data.empty()) return 0;
//
//    std::vector<double> sorted = data;
//    std::sort(sorted.begin(), sorted.end());
//
//    double index = percent * double(sorted.size() - 1);
//    int left = static_cast<int>(index);
//    double frac = index - left;
//
//    if (left >= int(sorted.size()) - 1) return sorted.back();
//    return sorted[left] + frac * (sorted[left + 1] - sorted[left]);
//}
//
//TEST_CASE("Threshold-Percentile", "[tick-by-tick]") {
//    // 示例: 历史逐笔成交量(单位: 股)
//    std::vector<double> volumes = {100, 200, 300, 500, 800, 1000, 1200, 1500, 2000, 2500, 3000};
//
//    double threshold_90 = percentile(volumes, 0.90);  // 90%
//    double threshold_95 = percentile(volumes, 0.95);  // 95%
//
//    std::cout << "90% Percentile Threshold: " << threshold_90 << " 股" << std::endl;
//    std::cout << "95% Percentile Threshold: " << threshold_95 << " 股" << std::endl;
//}
//
//// 二, 固定比例法(Fixed Ratio cmd_id)
//// 根据流通市值或日均成交额设定固定比例, 如万分之一或5%. 
//#include <iostream>
//
//struct StockInfo {
//    double circulatingMarketCap; // 流通市值(元)
//    double dailyAvgAmount;       // 日均成交额(元)
//};
//
//// 计算大单金额阈值
//double calcLargeOrderThreshold(const StockInfo& info) {
//    double capThreshold = info.circulatingMarketCap * 0.0001;  // 万分之一
//    double avgThreshold = info.dailyAvgAmount * 0.05;          // 5%
//    return std::max(capThreshold, avgThreshold);
//}
//
//TEST_CASE("Threshold-Fixed-Ratio", "[tick-by-tick]") {
//    StockInfo stock = {
//        .circulatingMarketCap = 2e8,  // 2亿流通市值
//        .dailyAvgAmount = 2e6         // 日均成交额 200万
//    };
//
//    double threshold = calcLargeOrderThreshold(stock);
//    std::cout << "建议的大单金额阈值为: " << threshold << " 元" << std::endl;
//}
//
//// 三, 经验法(Empirical cmd_id)
//// 基于股票类型设定固定的成交量或金额门槛. 
//
//#include <iostream>
//#include <string>
//
//enum class StockType {
//    SmallCap,
//    MidCap,
//    LargeCap,
//    SciTechOrGEM
//};
//
//// 返回建议的大单成交量阈值(手)
//int getVolumeThresholdByType(StockType type) {
//    switch (type) {
//        case StockType::SmallCap: return 500;     // 小盘股 ≥ 500 手
//        case StockType::MidCap:   return 1000;    // 中盘股 ≥ 1000 手
//        case StockType::LargeCap: return 3000;    // 大盘股 ≥ 3000 手
//        case StockType::SciTechOrGEM: return 500; // 科创板/创业板 ≥ 500 手
//        default: return 1000;
//    }
//}
//
//TEST_CASE("Threshold-Empirical", "[tick-by-tick]") {
//    StockType type = StockType::SmallCap;
//    int threshold = getVolumeThresholdByType(type);
//
//    std::cout << "当前股票类型下的大单成交量阈值为: " << threshold << " 手" << std::endl;
//}
//
//// 根据流通市值和日均成交额动态计算大单金额阈值
//double calcDynamicThreshold(double marketCap, double dailyAvgAmount) {
//    double capBased = marketCap * 0.0001;     // 流通市值万分之一
//    double avgBased = dailyAvgAmount * 0.05;  // 日均成交额5%
//    return std::max(capBased, avgBased);
//}
//
//TEST_CASE("Threshold-Dynamic", "[tick-by-tick]") {
//    double marketCap = 2e8;     // 流通市值 2亿
//    double dailyAvgAmount = 2e6; // 日均成交额 200万
//
//    double threshold = calcDynamicThreshold(marketCap, dailyAvgAmount);
//    std::cout << "动态计算的大单金额阈值为: " << threshold << " 元" << std::endl;
//}
//
//#include <iostream>
//#include <fstream>
//#include <sstream>
//#include <vector>
//#include <unordered_map>
//#include <string>
//#include <quant1x/runtime/config.h>
//#include <quant1x/contrib/data/tdx/level1/transaction_data.h>
//
//// ================== 原始 Tick 结构体(来自券商 API, 不可修改)==================
//struct Tick {
//    std::string time;
//    double price;
//    long long vol;
//    long long num;
//    double amount;
//    int buyOrSell;
//};
//
//std::vector<Tick> readTicksFromFile(const std::string& filename) {
//    std::ifstream file(filename);
//    std::vector<Tick> ticks;
//    std::string line;
//    bool is_first_row = true;
//    while (std::getline(file, line)) {
//        if(is_first_row) {
//            is_first_row = false;
//            continue;
//        }
//        std::istringstream ss(line);
//        std::vector<std::string> tokens;
//        std::string token;
//
//        while (std::getline(ss, token, ',')) {
//            tokens.push_back(token);
//        }
//
//        if (tokens.size() >= 6) {
//            Tick tick;
//            tick.time = tokens[0];
//            tick.price = std::stod(tokens[1]);
//            tick.vol = std::stoll(tokens[2]);
//            tick.num = std::stoll(tokens[3]);
//            tick.amount = std::stod(tokens[4]);
//            tick.buyOrSell = std::stoi(tokens[5]);
//
//            ticks.push_back(tick);
//        }
//    }
//
//    return ticks;
//}
//
//// ================== 资金分类枚举(独立封装)==================
//
//enum class OrderCategory {
//    Small,
//    Medium,
//    Large,
//    SuperLarge
//};
//
//const std::unordered_map<OrderCategory, std::string> CategoryToString = {
//    {OrderCategory::Small, "小单"},
//    {OrderCategory::Medium, "中单"},
//    {OrderCategory::Large, "大单"},
//    {OrderCategory::SuperLarge, "超大单"}
//};
//
//// ================== 分类判定函数(纯外部逻辑)==================
//
//namespace OrderThreshold {
//    constexpr double SuperLarge = 1'000'000.0; // ≥ 100万
//    constexpr double Large      = 200'000.0;   // ≥ 50万
//    constexpr double Medium     = 50'000.0;    // ≥ 5万
//}
//
//OrderCategory getCategory(const Tick& tick) {
//    double amount = tick.amount;
//
//    if (amount >= OrderThreshold::SuperLarge) return OrderCategory::SuperLarge;
//    else if (amount >= OrderThreshold::Large) return OrderCategory::Large;
//    else if (amount >= OrderThreshold::Medium) return OrderCategory::Medium;
//    else return OrderCategory::Small;
//}
//
//// ================== 统计结构体 ==================
//
//struct CategoryStats {
//    int buyCount = 0;
//    int sellCount = 0;
//    double buyAmount = 0.0;
//    double sellAmount = 0.0;
//};
//
//// ================== 主程序: 统计 + 输出 ==================
//
//TEST_CASE("total-v1", "[tick-by-tick]") {
//    std::string code = "sh600600";
//    std::string date = "2025-06-19";
//    std::string filename = config::get_historical_trade_filename(code, date);
//    auto ticks = readTicksFromFile(filename);
//
//    std::unordered_map<OrderCategory, CategoryStats> stats;
//    double lastPrice = 0.0;
//    for (const auto& tick : ticks) {
//        OrderCategory cat = getCategory(tick);
//        if(lastPrice == 0) {
//            lastPrice = tick.price;
//        }
//        auto direction = tick.buyOrSell;
////        if(direction != tdx::tick_buy && direction != tdx::tick_sell) {
////            if (tick.price > lastPrice) {
////                direction = tdx::tick_buy;
////            } else if (tick.price < lastPrice) {
////                direction = tdx::tick_sell;
////            } else {
////                direction = tdx::tick_neutral;
////            }
////        }
//
//        if (direction == tdx::tick_buy) {
//            stats[cat].buyCount++;
//            stats[cat].buyAmount += tick.amount;
//        } else if (direction == tdx::tick_sell) {
//            stats[cat].sellCount++;
//            stats[cat].sellAmount += tick.amount;
//        } else {
//            stats[cat].buyCount++;
//            stats[cat].buyAmount += tick.amount/2;
//            stats[cat].sellCount++;
//            stats[cat].sellAmount += tick.amount - tick.amount/2;
//        }
//    }
//
//    // 输出统计结果
//    std::cout << "【成交分类统计 - 按资金划分】\n";
//    std::cout << "类别\t|\t主动买笔数\t|\t主动买金额(万元)\t|\t主动卖笔数\t|\t主动卖金额(万元)\n";
//
//    for (const auto& [cat, stat] : stats) {
//        const std::string& categoryName = CategoryToString.at(cat);
//        double buyAmtWan = stat.buyAmount / 1e4;
//        double sellAmtWan = stat.sellAmount / 1e4;
//
//        std::cout << categoryName << "\t|\t"
//                  << stat.buyCount << "\t|\t"
//                  << buyAmtWan << "\t|\t"
//                  << stat.sellCount << "\t|\t"
//                  << sellAmtWan << "\n";
//    }
//
//    // 可选: 输出到 CSV 文件
//    std::ofstream outFile("output.csv");
//    outFile << "类别,主动买笔数,主动买金额(万元),主动卖笔数,主动卖金额(万元)\n";
//
//    for (const auto& [cat, stat] : stats) {
//        const std::string& categoryName = CategoryToString.at(cat);
//        double buyAmtWan = stat.buyAmount / 1e4;
//        double sellAmtWan = stat.sellAmount / 1e4;
//
//        outFile << categoryName << ","
//                << stat.buyCount << ","
//                << buyAmtWan << ","
//                << stat.sellCount << ","
//                << sellAmtWan << "\n";
//    }
//
//    outFile.close();
//
//    std::cout << "\n✅ 统计结果已保存至 output.csv\n";
//}
//
//// ================== 时间段划分函数 ==================
//std::string getWindowStart(const std::string& timeStr, int windowSizeMinutes = 10) {
//    int hour = std::stoi(timeStr.substr(0, 2));
//    int minute = std::stoi(timeStr.substr(3, 2));
//
//    int totalMin = hour * 60 + minute;
//    int windowStartMin = (totalMin / windowSizeMinutes) * windowSizeMinutes;
//
//    int startHour = windowStartMin / 60;
//    int startMin = windowStartMin % 60;
//
//    char buffer[6];
//    sprintf(buffer, "%02d:%02d", startHour, startMin);
//    return std::string(buffer);
//}
//
//std::string getNextWindowStart(const std::string& windowStart, int windowSizeMinutes = 10) {
//    int hour = std::stoi(windowStart.substr(0, 2));
//    int minute = std::stoi(windowStart.substr(3, 2));
//    int totalMin = hour * 60 + minute + windowSizeMinutes;
//
//    int newHour = totalMin / 60;
//    int newMinute = totalMin % 60;
//    char buffer[6];
//    sprintf(buffer, "%02d:%02d", newHour, newMinute);
//    return std::string(buffer);
//}
//
//// ================== 是否属于主力资金 ==================
//bool isMainForce(const Tick& tick) {
//    return tick.amount >= OrderThreshold::SuperLarge; // 大单及以上(大单+超大单)
//}
//
//TEST_CASE("total-v2", "[tick-by-tick]") {
//    std::string code = "sh000001";
//    std::string date = "2025-06-19";
//    std::string filename = config::get_historical_trade_filename(code, date);
//    auto ticks = readTicksFromFile(filename);
//
//    std::map<std::string, double> netInflowByWindow;
//    double lastPrice = 0.0;
//    for (const auto& tick : ticks) {
//        if(lastPrice == 0) {
//            lastPrice = tick.price;
//        }
//        if (!isMainForce(tick)) continue;
//
//        std::string windowStart = getWindowStart(tick.time, 10);
//        std::string windowEnd = getNextWindowStart(windowStart, 10); // 使用辅助函数获取结束时间
//
//        std::string windowRange = windowStart + "-" + windowEnd;
//
//        double amount = tick.amount;
//
//        auto direction = tick.buyOrSell;
//        if(direction != tdx::tick_buy && direction != tdx::tick_sell) {
//            if (tick.price > lastPrice) {
//                direction = tdx::tick_buy;
//            } else if (tick.price < lastPrice) {
//                direction = tdx::tick_sell;
//            } else {
//                direction = tdx::tick_neutral;
//            }
//        }
//
//        if (direction == tdx::tick_buy) {
//            netInflowByWindow[windowRange] += amount;
//        } else if (direction == tdx::tick_sell) {
//            netInflowByWindow[windowRange] -= amount;
//        }
//    }
//
//    // 输出统计结果
//    std::ofstream outFile("main_force_active_windows.csv");
//    outFile << "时间段,主力净流入(万元),累计净流入(万元)\n";
//
//    std::cout << "【主力活跃时间段统计】\n";
//    std::cout << "时间段\t|\t主力净流入(万元)\t|\t累计净流入(万元)\n";
//
//    double cumulative = 0.0;
//
//    for (const auto& [window, inflow] : netInflowByWindow) {
//        double inflowWan = inflow / 1e4;
//        cumulative += inflowWan;
//
//        std::cout << window << "\t|\t" << inflowWan << "\t|\t" << cumulative << "\n";
//        outFile << window << "," << inflowWan << "," << cumulative << "\n";
//    }
//
//    outFile.close();
//    std::cout << "\n✅ 统计结果已保存至 main_force_active_windows.csv\n";
//}