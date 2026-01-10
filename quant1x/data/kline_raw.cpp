#include "kline_raw.h"
#include <quant1x/factors/base.h>
#include <quant1x/config/cache.h>
#include <quant1x/std/filepath.h>

namespace data {

    namespace detail {
        // 拉取数据
        std::vector<level1::SecurityBar> fetch_kline(const std::string &code, u16 start, u16 count, level1::KLineType kline_type) {
            try {
                auto conn = level1::get_std_conn();
                //constexpr auto category = kline_type;
                level1::SecurityBarsRequest request(code, kline_type, start, count);
                level1::SecurityBarsResponse response(request.isIndex, kline_type);
                auto err = level1::process(conn->socket(), request, response);
                if (err) {
                    throw std::runtime_error(fmt::format("Process error: {}", err.message()));
                }
                return response.List;
            } catch (const std::exception &e) {  // 其他标准异常
                spdlog::error("[data::KLine] - 标准异常: {} (type: {})", e.what(), typeid(e).name());
                // 对于system_error可以记录更多信息
                if (auto se = dynamic_cast<const std::system_error *>(&e)) {
                    spdlog::error("[data::KLine] Error code: {}, category: {}", se->code().value(), se->code().category().name());
                }
            } catch (...) {
                spdlog::error("[data::KLine] 获取日K线异常");
            }
            return {};
        }
    } // namespace

    namespace {

        void save_kline_raw(const std::string &filename, const std::vector<KLineRaw>& values) {
            auto ec = filepath::check_filepath(filename, true);
            ec.clear();
            io::CSVWriter writer(filename);
            writer.write_row("date", "open", "close", "high", "low", "volume", "amount", "up", "down", "datetime");
            for (const auto &row: values) {
                writer.write_row(row.date, row.open, row.close, row.high, row.low, row.volume, row.amount,
                                 row.up, row.down, row.datetime);
            }
        }
    }

    // void KLineRaw::adjust(const factors::CumulativeAdjustment &adj) {
    //     auto m = adj.m;
    //     auto a = adj.a;
    //     this->Open = this->Open * m + a;
    //     this->Close = this->Close * m + a;
    //     this->High = this->High * m + a;
    //     this->Low = this->Low * m + a;
    //     // 成交量复权
    //     // 1. 计算均价
    //     auto ap = this->Amount / this->Volume;
    //     // 2. 均价复权
    //     ap = ap * m + a;
    //     // 3. 以成交金额为基准, 用复权均价计算成交量
    //     this->Volume = this->Amount / ap;
    // }

    std::vector<KLineRaw> read_kline_raw_from_csv(const std::string& filename) {
        std::vector<KLineRaw> klines;
        try {
            // 创建 CSV 读取器
            io::CSVReader<10> in(filename);  // 有 11 列数据

            // 设置表头字段名(用于自动匹配顺序)
            in.read_header(io::ignore_extra_column,
                           "date", "open", "close", "high", "low",
                           "volume", "amount", "up", "down", "datetime");

            KLineRaw row = {};
            while (in.read_row(row.date, row.open, row.close, row.high, row.low, row.volume, row.amount,
                               row.up, row.down, row.datetime)) {
                klines.emplace_back(row);
            }
        } catch(...) {
            // 忽略异常, 读csv文件失败, 返回空
        }
        return klines;
    }

    std::vector<KLineRaw> load_kline_raw(const std::string &code) {
        auto filename = config::get_kline_filename(code, false);
        spdlog::debug("[data::KLineRaw] kline file: {}", filename);
        return read_kline_raw_from_csv(filename);
    }

    void DataKLineRaw::Print(const std::string &code, const std::vector<exchange::timestamp> &dates)  {
        (void)code;
        (void)dates;
    }

    void DataKLineRaw::Update(const std::string &code, const exchange::timestamp &date) {
//        if(date != exchange::last_trading_day()) {
//            return;
//        }
        (void)date;
        // 1. 确定本地有效数据最后1条数据作为拉取数据的开始日期
        auto startDate = market_first_date;
        try {
            std::string cache_filename = config::get_kline_filename(code, false);
            std::vector<KLineRaw> cacheKLines = read_kline_raw_from_csv(cache_filename);
            auto kLength = cacheKLines.size();
            auto klineDaysOffset = detail::MAX_KLINE_LOOKBACK_DAYS;
            if(kLength > 0) {
                if (klineDaysOffset > kLength) {
                    klineDaysOffset = kLength;
                }
                startDate = cacheKLines[kLength-klineDaysOffset].date;
            }
            // 2. 确定结束日期
            auto endDate = exchange::timestamp::now().pre_market_time();
            spdlog::debug("[data::KLineRaw] [{}]: from {} to {}", code, startDate.only_date(), endDate.only_date());
            auto ts = exchange::date_range(startDate, endDate);
            auto total = u16(ts.size());
            startDate = ts[0];
            endDate = ts[total-1];
            spdlog::debug("[dataset::KLineRaw] [{}]: from {} to {}", code, startDate.only_date(), endDate.only_date());
            u16 step = level1::security_bars_max;
            u16 start = 0;
            //u16 category = level1::RI_K;
            // 3. 拉取数据
            std::vector<std::vector<level1::SecurityBar>> hs;
            //std::vector<level1::SecurityBar> history;
            size_t elementCount = 0;
            do {
                u16 count = step;
                if(total - start >= step) {
                    count = step;
                } else {
                    count = total - start;
                }
                auto reply = detail::fetch_kline(code, start, count);
                if (reply.empty()) {
                    break;
                }
                elementCount += reply.size();
                //hs.insert(hs.end(), reply.begin(), reply.end());
                hs.emplace_back(reply);
                if (reply.size() < count) {
                    break;
                }
                start += count;
            } while (start < total);
            // 4. 由于K线数据，每次获取数据是从后往前获取, 所以这里需要反转历史数据的切片
            std::reverse(hs.begin(), hs.end());
            // 5. 调整成交量, 单位从手改成股, vol字段 * 100
            std::vector<KLineRaw> incremental_klines;
            incremental_klines.reserve(elementCount);
            for(const auto & vec : hs) {
                for (const auto & row : vec) {
                    auto dateTime = exchange::timestamp(row.Year, row.Month, row.Day).pre_market_time();
                    if (dateTime < startDate || dateTime > endDate) {
                        continue;
                    }
                    auto kx = KLineRaw{
                        .date = dateTime.only_date(), // 日期
                        .open = row.Open,             // 开盘价
                        .close = row.Close,           // 收盘价
                        .high = row.High,             // 最高价
                        .low = row.Low,               // 最低价
                        .volume = row.Vol * 100,      // 成交量(股)
                        .amount = row.Amount,         // 成交金额(元)
                        .up = row.UpCount,            // 上涨家数 / 外盘
                        .down = row.DownCount,        // 下跌家数 / 内盘
                        .datetime = row.DateTime,     // 时间
                    };
                    incremental_klines.emplace_back(kx);
                }
            }
            // 6. K线数据转换成KLine结构
            // 6.1 判断是否已除权的依据是当前更新K线只有1条记录
//            bool adjusted = incremental_klines.size() == 1;
//            auto dividends = load_xdxr(code);
//            if (adjusted) {
//                calculate_pre_adjust(incremental_klines, startDate, dividends);
//            }
            // 6.2 只前复权当日数据
            // 7. 拼接缓存和新增的数据
            std::vector<KLineRaw> klines;
            // 7.1 先截取本地缓存的数据
            if (kLength > klineDaysOffset) {
                klines.insert(klines.end(), cacheKLines.begin(), cacheKLines.begin()+(kLength-klineDaysOffset));
            }
            // 7.2 拼接新增的数据
            if (klines.empty()) {
                klines = incremental_klines;
            } else {
                klines.insert(klines.end(), incremental_klines.begin(), incremental_klines.end());
            }
            // 8. 前复权
//            if(!adjusted) {
//                calculate_pre_adjust(klines, startDate, dividends);
//            }
            // 9. 刷新缓存文件
            save_kline_raw(cache_filename, klines);
        } catch (const std::exception &e) {  // 其他标准异常
            spdlog::error("[data::KLineRaw] - 标准异常: {} (type: {})", e.what(), typeid(e).name());
            // 对于system_error可以记录更多信息
            if (auto se = dynamic_cast<const std::system_error *>(&e)) {
                spdlog::error("[data::KLineRaw] Error code: {}, category: {}", se->code().value(), se->code().category().name());
            }
        } catch (...) {
            spdlog::error("[data::KLineRaw] 获取日K线异常");
        }
    }


} // namespace data