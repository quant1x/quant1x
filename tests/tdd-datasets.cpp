#include <quant1x/test/test.h>
#include <quant1x/data/meta/timestamp.h>
#include <quant1x/data/meta/session.h>
#include <quant1x/proto/data.h>
#include <random>

#include "quant1x/data/trans.h"

TEST_CASE("lower-upper", "[strings]") {
    spdlog::set_level(spdlog::level::debug);
    std::string s("SH000001");
    strings::strtolc_inplace_branchless(s.data());
    spdlog::debug("lower = {}", s);
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

// 测试参数
const size_t stringLength = 1000000; // 字符串长度
const int iterations = 100;          // 迭代次数
// 生成随机字符串
std::string v1generateRandomString(size_t length) {
    static const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        result += charset[rand() % (sizeof(charset) - 1)];
    }
    return result;
}

// 生成随机字符串
std::string generateRandomString(size_t length) {
    static const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    static const size_t charsetSize = sizeof(charset) - 1;

    // 使用 std::random_device 获取高熵随机种子
    static std::random_device rd;

    static std::mt19937 generator(rd()); // Mersenne Twister 随机数生成器
    static std::uniform_int_distribution<> distribution(0, charsetSize - 1);

    std::string result;
    result.reserve(length);

    for (size_t i = 0; i < length; ++i) {
        result += charset[distribution(generator)];
    }

    return result;
}

TEST_CASE("bm-lowe-upper", "[strings]") {
    srand(static_cast<unsigned int>(time(nullptr))); // 初始化随机数种子

    // 生成随机字符串
    std::string randomString = generateRandomString(stringLength);

    BENCHMARK("Standard toUpper") {
                                     return strings::to_upper(randomString);
                                 };

    BENCHMARK("No-branch toUpper") {
                                 return strings::strtouc_inplace_branchless(randomString.data());
                             };
    BENCHMARK("Standard toLower") {
                                      return strings::to_lower(randomString);
                                  };

    BENCHMARK("No-branch toLower") {
                                       return strings::strtolc_inplace_branchless(randomString.data());
                                   };
}

// 基准测试函数
void benchmark() {
    // 测试参数
    //const size_t stringLength = 1000000; // 字符串长度
    //const int iterations = 100;          // 迭代次数

    // 生成随机字符串
    std::string randomString = generateRandomString(stringLength);

    // 无分支实现: 转大写
    {
        std::string temp = randomString;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            strings::strtouc_inplace_branchless(temp.data());
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "No-branch toUpper: " << elapsed.count() << " seconds\n";
    }

    // 标准库实现: 转大写
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            strings::to_upper(randomString);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Standard toUpper: " << elapsed.count() << " seconds\n";
    }

    // 无分支实现: 转小写
    {
        std::string temp = randomString;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            strings::strtolc_inplace_branchless(temp.data());
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "No-branch toLower: " << elapsed.count() << " seconds\n";
    }

    // 标准库实现: 转小写
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            strings::to_lower(randomString);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Standard toLower: " << elapsed.count() << " seconds\n";
    }
}

TEST_CASE("bm-strings-2", "[string]") {
    srand(static_cast<unsigned>(time(nullptr))); // 初始化随机数种子
    benchmark();
}

TEST_CASE("calendar", "[exchange]") {
    runtime::logger_set(true, true);

    // 当前交易日
    std::string a = meta::Timestamp::now;
    std::string b = meta::Timestamp::now.get().toString();
    spdlog::debug("current_day={},  ts_today_init={}", a, b);

    spdlog::debug("------------------------------");

    // 上一个交易日
    meta::Timestamp base;
    meta::Timestamp debug_timestamp;
    meta::Timestamp ts;
    base = meta::Timestamp(2025,5,19).pre_market_time();
    debug_timestamp = meta::Timestamp(2025,5,20,8,59);
    ts = meta::prev_trading_day(base, debug_timestamp);
    spdlog::debug("prev_trading_day={}", ts.toString());
    spdlog::debug("----------");
    base = meta::Timestamp(2025,5,19).pre_market_time();
    debug_timestamp = meta::Timestamp(2025,5,20,9,0);
    ts = meta::prev_trading_day(base, debug_timestamp);
    spdlog::debug("prev_trading_day={}", ts.toString());
    spdlog::debug("----------");
    base = meta::Timestamp(2025,5,20).pre_market_time();
    debug_timestamp = meta::Timestamp(2025,5,20,9,0);
    ts = meta::prev_trading_day(base, debug_timestamp);
    spdlog::debug("prev_trading_day={}", ts.toString());
    spdlog::debug("----------");
    base = meta::Timestamp(2025,5,20).pre_market_time();
    debug_timestamp = meta::Timestamp(2025,5,20,9,1);
    ts = meta::prev_trading_day(base, debug_timestamp);
    spdlog::debug("prev_trading_day={}", ts.toString());
    spdlog::debug("----------");
    base = meta::Timestamp(2025,5,20).pre_market_time();
    debug_timestamp = meta::Timestamp(2025,5,21,8,89);
    ts = meta::prev_trading_day(base, debug_timestamp);
    spdlog::debug("prev_trading_day={}", ts.toString());
    spdlog::debug("----------");
    base = meta::Timestamp(2025,5,20).pre_market_time();
    debug_timestamp = meta::Timestamp(2025,5,21,9,0);
    ts = meta::prev_trading_day(base, debug_timestamp);
    spdlog::debug("prev_trading_day={}", ts.toString());
    spdlog::debug("----------");
    base = meta::Timestamp(2025,5,20).pre_market_time();
    debug_timestamp = meta::Timestamp(2025,5,21,9,0,1);
    ts = meta::prev_trading_day(base, debug_timestamp);
    spdlog::debug("prev_trading_day={}", ts.toString());

    spdlog::debug("------------------------------");

    // 下一个交易日
    base = meta::Timestamp(2025,5,19).pre_market_time();
    debug_timestamp = meta::Timestamp(2025,5,20,8,59);
    ts = meta::next_trading_day(base, debug_timestamp);
    spdlog::debug("next_trading_day={}", ts.toString());
    spdlog::debug("----------");
    base = meta::Timestamp(2025,5,20).pre_market_time();
    debug_timestamp = meta::Timestamp(2025,5,20,9,0);
    ts = meta::next_trading_day(base, debug_timestamp);
    spdlog::debug("next_trading_day={}", ts.toString());
    spdlog::debug("----------");
    base = meta::Timestamp(2025,5,20).pre_market_time();
    debug_timestamp = meta::Timestamp(2025,5,20,9,1);
    ts = meta::next_trading_day(base, debug_timestamp);
    spdlog::debug("next_trading_day={}", ts.toString());
    spdlog::debug("----------");
    base = meta::Timestamp(2025,5,20).pre_market_time();
    debug_timestamp = meta::Timestamp(2025,5,21,8,59);
    ts = meta::next_trading_day(base, debug_timestamp);
    spdlog::debug("next_trading_day={}", ts.toString());
    spdlog::debug("----------");
    base = meta::Timestamp(2025,5,20).pre_market_time();
    debug_timestamp = meta::Timestamp(2025,5,21,9,1);
    ts = meta::next_trading_day(base, debug_timestamp);
    spdlog::debug("next_trading_day={}", ts.toString());
    spdlog::debug("----------");
    base = meta::Timestamp(2025,5,21).pre_market_time();
    debug_timestamp = meta::Timestamp(2025,5,21,8,1);
    ts = meta::next_trading_day(base, debug_timestamp);
    spdlog::debug("next_trading_day={}", ts.toString());
    spdlog::debug("----------");
    base = meta::Timestamp(2025,5,21).pre_market_time();
    debug_timestamp = meta::Timestamp(2025,5,21,9);
    ts = meta::next_trading_day(base, debug_timestamp);
    spdlog::debug("next_trading_day={}", ts.toString());
    spdlog::debug("----------");
    base = meta::Timestamp(2025,5,21).pre_market_time();
    debug_timestamp = meta::Timestamp(2025,5,21,9,1);
    ts = meta::next_trading_day(base, debug_timestamp);
    spdlog::debug("next_trading_day={}", ts.toString());
}

TEST_CASE("all-codes", "[exchange]") {
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("------------------------------");
//    auto xx = data::correct_security_code();
//    (void)xx;
    auto codes = instruments::get_code_list();
    spdlog::debug("------------------------------");
    for(auto const &v : codes) {
        spdlog::debug(v);
    }
}

TEST_CASE("check-realtime", "[exchange]") {
    meta::Timestamp now = meta::Timestamp::now();
    auto ts = meta::check_trading_timestamp(now);
    spdlog::info("realtime update: {}", ts.updateInRealTime);
}

TEST_CASE("timestamp", "[exchange]") {
    spdlog::set_level(spdlog::level::debug);
    auto ts = meta::Timestamp::now();
    spdlog::debug("timestamp:now = {}", ts.value());
}

TEST_CASE("year-month-day", "[exchange]") {
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("------------------------------");
    auto ts = meta::Timestamp::now();
    ts.today();
    spdlog::debug("timestamp:now = {}, string={}", ts.value(), ts.toString());
    auto [year, month, day] = ts.extract();
    spdlog::debug("timestamp: year={}, month={}, day={}", year, month, day);
    spdlog::debug("------------------------------");
}

TEST_CASE("base-session", "[exchange]") {
    runtime::logger_set(true, true);
    auto ts = meta::Timestamp::midnight();
    auto rs = meta::check_trading_timestamp(ts.offset(8));
    std::cout<< rs << std::endl;
    rs = meta::check_trading_timestamp(ts.offset(9));
    std::cout<< rs << std::endl;
    rs = meta::check_trading_timestamp(ts.offset(9,14,59));
    std::cout<< rs << std::endl;
    rs = meta::check_trading_timestamp(ts.offset(9,15,00));
    std::cout<< rs << std::endl;
}

TEST_CASE("trade-session", "[exchange]") {
    // 构造多个交易时段
    meta::TradingSession session({
                                             meta::TimeRange(34209000, 41400000, meta::TimeStatus::ExchangeTrading), // 09:30:00 ~ 11:30:00
                                             meta::TimeRange(46808000, 54000000, meta::TimeStatus::ExchangeTrading)  // 13:00:00 ~ 15:00:00
                           });
    std::cout << session << std::endl;

    // 测试时间点
    meta::Timestamp ts1 = 30000000; // 08:20:00(全天交易未开始)
    meta::Timestamp ts2 = 36000000; // 10:00:00(交易时段内)
    meta::Timestamp ts3 = 55000000; // 15:16:40(全天交易已结束)

    // 测试是否在交易时段内
    std::cout << "ts1 在交易时段内: " << session.in(ts1) << std::endl;
    std::cout << "ts2 在交易时段内: " << session.in(ts2) << std::endl;
    std::cout << "ts3 在交易时段内: " << session.in(ts3) << std::endl;

    // 测试全天交易是否未开始
    std::cout << "ts1 全天交易未开始: " << session.is_trading_not_started(ts1) << std::endl;
    std::cout << "ts2 全天交易未开始: " << session.is_trading_not_started(ts2) << std::endl;
    std::cout << "ts3 全天交易未开始: " << session.is_trading_not_started(ts3) << std::endl;

    // 测试全天交易是否已结束
    std::cout << "ts1 全天交易已结束: " << session.is_trading_ended(ts1) << std::endl;
    std::cout << "ts2 全天交易已结束: " << session.is_trading_ended(ts2) << std::endl;
    std::cout << "ts3 全天交易已结束: " << session.is_trading_ended(ts3) << std::endl;
}

// 时间解析
TEST_CASE("timestamp-parse2", "[exchange]") {
    auto tm = meta::Timestamp::parse("2025-04-22");
    std::cout << "Direct output: " << tm.toString() << std::endl;
}

// 交易时间戳判断
TEST_CASE("session-check", "[exchange]") {
    spdlog::set_level(spdlog::level::debug);
    std::cout << meta::Timestamp::now.get() << std::endl;
    std::cout << meta::Timestamp::now << std::endl;
    std::cout << meta::TradingSession << std::endl;
    spdlog::debug("------------------------------");
    auto now = meta::Timestamp::now();
    std::cout << "               now = " << now.toString() << std::endl;
    auto modified = now.since(9, 0, 0, 0).offset(0,0,0,-1);
    std::cout << "          modified = " << modified.toString() << std::endl;
    spdlog::debug("------------------------------");
    auto [beforeLastTradeDay, isHoliday, beforeInitTime, cacheAfterInitTime, updateInRealTime, status] = meta::check_trading_timestamp(modified);
    std::cout << "beforeLastTradeDay = " << beforeLastTradeDay << std::endl;
    std::cout << "         isHoliday = " << isHoliday << std::endl;
    std::cout << "    beforeInitTime = " << beforeInitTime << std::endl;
    std::cout << "cacheAfterInitTime = " << cacheAfterInitTime << std::endl;
    std::cout << "  updateInRealTime = " << updateInRealTime << std::endl;
    std::cout << "            status = " << status << std::endl;
}

#include <quant1x/data/xdxr.h>

TEST_CASE("xdxr-factor", "[data]") {
    spdlog::set_level(spdlog::level::debug);
    std::string code = "sz000048";
    auto list = data::load_xdxr(code);
    for (auto const & v: list) {
        std::cout << v << std::endl;
    }
}

#include <filesystem>
#include <quant1x/data/kline_raw.h>

namespace fs = std::filesystem;

void read_file(const fs::path& filename, std::vector<uint8_t>& buf) {
    std::ifstream in(filename, std::ios::binary);
    buf.assign(std::istreambuf_iterator<char>(in),
               std::istreambuf_iterator<char>());
}

std::vector<uint8_t> read_file_binary(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary | std::ios::ate);
    if(!in) {
        return {};
    }

    const auto file_size = in.tellg();
    in.seekg(0);

    std::vector<uint8_t> buf;
    buf.reserve(file_size);

    char tmp[8192];
    while(in.read(tmp, sizeof(tmp))) {
        buf.insert(buf.end(), tmp, tmp + in.gcount());
    }

    // 处理最后一块
    if(in.gcount() > 0) {
        buf.insert(buf.end(), tmp, tmp + in.gcount());
    }

    return buf;
}

void write_file_binary(const std::string& filename, const std::vector<u8> &data) {
    std::ofstream out(filename,std::ios::binary|std::ios::out | std::ios::trunc);
    out.write(reinterpret_cast<const char *>(data.data()), data.size());
}

// 拉取数据
std::vector<tdx::SecurityBar> fetch(const std::string &code, u16 start, u16 count) {
    try {
        auto conn = tdx::get_std_conn();
        auto category = tdx::KLineType::RI_K;
        tdx::SecurityBarsContext request(code, category, start, count);
        tdx::SecurityBarsResponse response(request.isIndex, category);
        auto err = tdx::transact_message_sync(conn->socket(), request, response);
        if (err) {
            spdlog::error("Process error: {}", err.message());
            return {};
        }
        return response.List;
    } catch (const std::exception &e) {  // 其他标准异常
        spdlog::error("全局捕获 - 标准异常: {} (type: {})", e.what(), typeid(e).name());
        // 对于system_error可以记录更多信息
        if (auto se = dynamic_cast<const std::system_error *>(&e)) {
            spdlog::error("Error code: {}, category: {}", se->code().value(), se->code().category().name());
        }
    } catch (...) {
        spdlog::error("获取日K线异常");
    }
    return {};
}

// void update_pb(const std::string &code, const std::string &date) {
//     (void)date;
//     // 1. 确定本地有效数据最后1条数据作为拉取数据的开始日期
//     auto startDate = data::market_first_date;
//     try {
//         std::string cache_filename = config::get_kline_filename(code) + ".pb";
//         KLine cacheKLines = {};
//         {
//             std::ifstream input(cache_filename, std::ios::in | std::ios::binary);
//             cacheKLines.ParseFromIstream(&input);
//         }
//
//         auto kLength = cacheKLines.datetime_size();
//         auto klineDaysOffset = static_cast<int>(datasets::detail::MAX_KLINE_LOOKBACK_DAYS);
//         if(kLength > 0) {
//             if (klineDaysOffset > kLength) {
//                 klineDaysOffset = kLength;
//             }
//             startDate = cacheKLines.datetime(kLength-klineDaysOffset);
//         }
//         // 2. 确定结束日期
//         auto endDate = meta::Timestamp::now().pre_market_time();
//         spdlog::debug("[{}]: from {} to {}", code, startDate.only_date(), endDate.only_date());
//         auto ts = meta::date_range(startDate, endDate);
//         auto total = ts.size();
//         startDate = ts[0];
//         endDate = ts[total-1];
//         spdlog::debug("[{}]: from {} to {}", code, startDate.only_date(), endDate.only_date());
//         size_t step = tdx::security_bars_max;
//         u16 start = 0;
//         //u16 category = tdx::RI_K;
//         // 3. 拉取数据
//         std::vector<std::vector<tdx::SecurityBar>> hs;
//         //std::vector<tdx::SecurityBar> history;
//         size_t elementCount = 0;
//         do {
//             u16 count = u16(step);
//             if(total - start >= step) {
//                 count = u16(step);
//             } else {
//                 count = u16(total - start);
//             }
//             auto reply = fetch(code, start, count);
//             if (reply.empty()) {
//                 break;
//             }
//             elementCount += reply.size();
//             //hs.insert(hs.end(), reply.begin(), reply.end());
//             hs.emplace_back(reply);
//             if (reply.size() < count) {
//                 break;
//             }
//             start += count;
//         } while (start < total);
//         (void)elementCount;
//         // 4. 由于K线数据, 每次获取数据是从后往前获取, 所以这里需要反转历史数据的切片
//         std::reverse(hs.begin(), hs.end());
//         // 5. 调整成交量, 单位从手改成股, vol字段 * 100
//         //std::vector<KLine> newKLines;
//         KLine newKLines = {};
//         //newKLines.reserve(elementCount);
//         for(const auto & vec : hs) {
//             for (const auto & row : vec) {
//                 auto dateTime = meta::Timestamp(row.Year, row.Month, row.Day).pre_market_time();
//                 if (dateTime < startDate || dateTime > endDate) {
//                     continue;
//                 }
//                 newKLines.add_datetime(dateTime); // 时间
//                 newKLines.add_open(row.Open); // 开盘价
//                 newKLines.add_close(row.Close); // 收盘价
//                 newKLines.add_high(row.High); // 最高价
//                 newKLines.add_low(row.Low); // 最低价
//                 newKLines.add_volume(row.Vol * 100); // 成交量(股)
//                 newKLines.add_amount(row.Amount); // 成交金额(元)
//                 newKLines.add_up(row.UpCount); // 上涨家数 / 外盘
//                 newKLines.add_down(row.DownCount); // 下跌家数 / 内盘
//                 newKLines.add_adjustmentcount(0); // 新增: 除权除息次数
//             }
//         }
//         // 6. K线数据转换成KLine结构
//         // 6.1 判断是否已除权的依据是当前更新K线只有1条记录
//         bool adjusted = newKLines.datetime_size() == 1;
// //        auto dividends = load_xdxr(code);
// //        if (adjusted) {
// //            calculate_pre_adjust(newKLines, startDate, dividends);
// //        }
//         (void) adjusted;
//         // 6.2 只前复权当日数据
//         // 7. 拼接缓存和新增的数据
//         //std::vector<KLine> klines;
//         KLine klines = {};
//         // 7.1 先截取本地缓存的数据
//         if (kLength > klineDaysOffset) {
//             klines.mutable_open()->Add(cacheKLines.open().begin(), cacheKLines.open().begin()+(kLength-klineDaysOffset));
//             klines.mutable_close()->Add(cacheKLines.close().begin(), cacheKLines.close().begin()+(kLength-klineDaysOffset));
//             klines.mutable_high()->Add(cacheKLines.high().begin(), cacheKLines.high().begin()+(kLength-klineDaysOffset));
//             klines.mutable_low()->Add(cacheKLines.low().begin(), cacheKLines.low().begin()+(kLength-klineDaysOffset));
//             klines.mutable_volume()->Add(cacheKLines.volume().begin(), cacheKLines.volume().begin()+(kLength-klineDaysOffset));
//             klines.mutable_amount()->Add(cacheKLines.amount().begin(), cacheKLines.amount().begin()+(kLength-klineDaysOffset));
//             klines.mutable_up()->Add(cacheKLines.up().begin(), cacheKLines.up().begin()+(kLength-klineDaysOffset));
//             klines.mutable_down()->Add(cacheKLines.down().begin(), cacheKLines.down().begin()+(kLength-klineDaysOffset));
//             klines.mutable_datetime()->Add(cacheKLines.datetime().begin(), cacheKLines.datetime().begin()+(kLength-klineDaysOffset));
//             klines.mutable_adjustmentcount()->Add(cacheKLines.adjustmentcount().begin(), cacheKLines.adjustmentcount().begin()+(kLength-klineDaysOffset));
//         }
//         // 7.2 拼接新增的数据
//         if (klines.datetime().empty()) {
//             klines = newKLines;
//         } else {
//             klines.mutable_open()->Add(newKLines.open().begin(), newKLines.open().end());
//             klines.mutable_close()->Add(newKLines.close().begin(), newKLines.close().end());
//             klines.mutable_high()->Add(newKLines.high().begin(), newKLines.high().end());
//             klines.mutable_low()->Add(newKLines.low().begin(), newKLines.low().end());
//             klines.mutable_volume()->Add(newKLines.volume().begin(), newKLines.volume().end());
//             klines.mutable_amount()->Add(newKLines.amount().begin(), newKLines.amount().end());
//             klines.mutable_up()->Add(newKLines.up().begin(), newKLines.up().end());
//             klines.mutable_down()->Add(newKLines.down().begin(), newKLines.down().end());
//             klines.mutable_datetime()->Add(newKLines.datetime().begin(), newKLines.datetime().end());
//             klines.mutable_adjustmentcount()->Add(newKLines.adjustmentcount().begin(), newKLines.adjustmentcount().end());
//         }
// //        // 8. 前复权
// //        if(!adjusted) {
// //            calculate_pre_adjust(klines, startDate, dividends);
// //        }
//         // 9. 刷新缓存文件
//         {
//             std::ofstream output(cache_filename, std::ios::out | std::ios::binary);
//             klines.SerializeToOstream(&output);
//             output.close();
//         }
//     } catch (const std::exception &e) {  // 其他标准异常
//         spdlog::error("全局捕获 - 标准异常: {} (type: {})", e.what(), typeid(e).name());
//         // 对于system_error可以记录更多信息
//         if (auto se = dynamic_cast<const std::system_error *>(&e)) {
//             spdlog::error("Error code: {}, category: {}", se->code().value(), se->code().category().name());
//         }
//     } catch (...) {
//         spdlog::error("获取日K线异常");
//     }
// }
//
// TEST_CASE("pb-kline", "[datasets]") {
//     spdlog::set_level(spdlog::level::debug);
//     update_pb("sz000048", "2025-05-06");
// }


#include <xsimd/xsimd.hpp>
#include <iostream>

TEST_CASE("simd-check", "[datasets]") {
    // 输出当前支持的 SIMD 架构
    std::cout << "Current architecture: " << xsimd::default_arch().name() << std::endl;

    // 检查是否启用了 SIMD 加速(不是 GENERIC)
    if (std::string(xsimd::default_arch().name()) == "GENERIC") {
        std::cout << "No SIMD acceleration available!" << std::endl;
    } else {
        std::cout << "SIMD acceleration is active." << std::endl;
    }
}

TEST_CASE("simd-add", "[datasets]") {
    alignas(xsimd::default_arch::alignment()) float a[8] = {1.0f, 2.0f, 3.0f, 4.0f};
    alignas(xsimd::default_arch::alignment()) float b[8] = {5.0f, 6.0f, 7.0f, 8.0f};
    alignas(xsimd::default_arch::alignment()) float res[8];

    using batch_type = xsimd::batch<float>;

    auto va = batch_type::load_aligned(a);
    auto vb = batch_type::load_aligned(b);

    auto vres = va + vb;

    vres.store_aligned(res);

    for (int i = 0; i < 4; ++i) {
        std::cout << res[i] << " ";
    }
    std::cout << std::endl;
}

#define XTENSOR_USE_XSIMD

#include <xtensor/containers/xarray.hpp>
#include <xtensor/io/xio.hpp>
#include <iostream>

TEST_CASE("xtensor-add", "[xtensor]") {
    xt::xarray<double> a = xt::ones<double>({1000});
    xt::xarray<double> b = xt::ones<double>({1000}) * 2.0;

    // 所有元素相加, 自动使用 xsimd SIMD 加速
    auto c = a + b;

    std::cout << "First element: " << c(0) << std::endl;
    std::cout << "Size of result: " << c.shape()[0] << std::endl;
}

#include <immintrin.h> // SSE / AVX intrinsics

void vector_add_simd_hand_crafted(const double* a, const double* b, double* res, size_t N)
{
    size_t i = 0;
    constexpr size_t vec_size = 2; // SSE: 2 doubles per batch (128-bit)
    size_t aligned_N = N - (N % vec_size);

    for (; i < aligned_N; i += vec_size)
    {
        __m128d va = _mm_load_pd(&a[i]);     // Load 2 doubles
        __m128d vb = _mm_load_pd(&b[i]);
        __m128d vres = _mm_add_pd(va, vb);   // SIMD add
        _mm_store_pd(&res[i], vres);         // Store result
    }

    // Tail processing (scalar)
    for (; i < N; ++i)
    {
        res[i] = a[i] + b[i];
    }
}

void vector_add_avx_hand_crafted(const double* a, const double* b, double* res, size_t N)
{
    size_t i = 0;
    constexpr size_t vec_size = 4; // AVX: 4 doubles per batch (256-bit)
    const size_t aligned_N = (N / vec_size) * vec_size;

    // 主体: 使用 AVX 向量化加法
    for (; i < aligned_N; i += vec_size)
    {
        __m256d va = _mm256_loadu_pd(a + i);     // 安全加载(不对齐也兼容)
        __m256d vb = _mm256_loadu_pd(b + i);
        __m256d vres = _mm256_add_pd(va, vb);
        _mm256_storeu_pd(res + i, vres);         // 安全存储
    }

    // 尾部处理
    for (; i < N; ++i)
    {
        res[i] = a[i] + b[i];
    }
}
// xtensor + xsimd
#define XTENSOR_USE_XSIMD
#include <xtensor/containers/xarray.hpp>
#include <xtensor/io/xio.hpp>
#include <xtensor/views/xstrided_view.hpp>

// 基准数据大小
constexpr size_t N = 100001;

TEST_CASE("Vector Addition Benchmark", "[benchmark]") {
#ifdef XTENSOR_USE_XSIMD
    std::cout << "xsimd acceleration is enabled!" << std::endl;
#else
    std::cout << "xsimd acceleration is NOT enabled!" << std::endl;
#endif

#ifdef EIGEN_VECTORIZE
    std::cout << "Eigen SIMD acceleration is enabled!" << std::endl;
#else
    std::cout << "Eigen SIMD acceleration is NOT enabled!" << std::endl;
#endif

    std::vector<double> a(N, 1.0);
    std::vector<double> b(N, 2.0);
    std::vector<double> res(N);


    BENCHMARK("Loop") {
                          for (size_t i = 0; i < N; ++i) {
                              res[i] = a[i] + b[i];
                          }
                          return res[0];
                      };

    BENCHMARK("xsimd") {
                           using batch_type = xsimd::batch<double>;
                           constexpr size_t size_per_batch = batch_type::size;

                           size_t i = 0;
                           for (; i + size_per_batch <= N; i += size_per_batch)
                           {
                               auto va = batch_type::load_unaligned(&a[i]);
                               auto vb = batch_type::load_unaligned(&b[i]);
                               auto vres = va + vb;
                               vres.store_unaligned(&res[i]);
                           }

                           // 处理剩余元素
                           for (; i < N; ++i) {
                               res[i] = a[i] + b[i];
                           }

                           return res[0];
                       };

    BENCHMARK("Hand-Crafted-SSE") {
                                       vector_add_simd_hand_crafted(a.data(), b.data(), res.data(), N);
                                       return res[0];
                                   };

    BENCHMARK("Hand-Crafted-AVX") {
                                      vector_add_avx_hand_crafted(a.data(), b.data(), res.data(), N);
                                      return res[0];
                                  };
}

TEST_CASE("xtensor-add-v1", "[xtensor]") {
    xt::xarray<int> a = { 1, 2, 3, 4};
    xt::xarray<int> b = {10, 20};

    xt::xarray<int> result = a + 1;
    std::cout << result << std::endl;
    std::cout << result << std::endl;
}

TEST_CASE("trans-v1", "[data]") {
    runtime::global_init();
    std::string code = "sz300773";
    meta::Timestamp now = meta::last_trading_day();

    const auto adapter = std::make_unique<data::DataTrans>();
    auto inst = data::detect_symbol(code);
    adapter->Update(inst, now);
}