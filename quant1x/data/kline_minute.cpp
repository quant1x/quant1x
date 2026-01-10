#include <quant1x/data/kline_minute.h>
#include <quant1x/data/kline_raw.h>
#include <quant1x/pandas/rule.h>
#include <quant1x/std/filepath.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <ranges>
#include <quant1x/config/base.h>
#include <quant1x/config/cache.h>

namespace data {

    namespace {

        void save_kline(const std::string &filename, const std::vector<MinuteKLine> &values) {
            filepath::check_filepath(filename, true);
            io::CSVWriter writer(filename);
            writer.write_row("date",
                             "open",
                             "close",
                             "high",
                             "low",
                             "volume",
                             "amount",
                             "up",
                             "down",
                             "datetime",
                             "adjustment_count");
            for (const auto &row : values) {
                writer.write_row(row.date,
                                 row.open,
                                 row.close,
                                 row.high,
                                 row.low,
                                 row.volume,
                                 row.amount,
                                 row.up,
                                 row.down,
                                 row.datetime,
                                 row.adjustment_count);
            }
        }

        // 通过比较 Amount/Vol（每个报告成交量单位对应的金额）和典型价格（OCHL）来推断 `Vol` 字段的单位。
        // (Amount/Vol) / price 大致等于单位倍数。不同品种可能使用 1、10、100、1000 等单位。
        // 使用第一条有效的 SecurityBar 记录进行检测，并以 bar 的 High 作为锚点，向上取整到 10 的次幂作为单位。
        f64 infer_bar_vol_unit(const std::vector<std::vector<level1::SecurityBar>> &hs) {
            // 使用第一条有效的 SecurityBar 记录推断单位。
            for (const auto &vec : hs) {
                for (const auto &row : vec) {
                    if (row.Amount <= 0 || row.Vol <= 0) {
                        continue;
                    }
                    // 典型价格：优先使用 OCHL 的平均价（Open/Close/High/Low 的平均）
                    f64 typical = (row.Open + row.Close + row.High + row.Low) / 4.0;
                    if (typical <= 0) {
                        typical = row.Close;
                    }
                    if (typical <= 0) {
                        continue;
                    }
                    f64 implied = row.Amount / row.Vol;  // 每个报告成交量单位对应的金额
                    if (implied <= 0) {
                        continue;
                    }
                    // 以 bar 的 High 为锚点。隐含的每单位金额不应大于 High * unit，
                    // 因此选择满足 implied <= High * unit 的最小 10 的次幂 unit。
                    // 即 unit = 10^ceil(log10(implied / High))。
                    if (!std::isfinite(implied) || implied <= 0.0 || row.High <= 0.0) {
                        return 1.0;
                    }
                    if (implied <= row.High) {
                        return 1.0;  // fits within one share
                    }
                    double ratio = implied / row.High;
                    if (!std::isfinite(ratio) || ratio <= 1.0) {
                        return 1.0;
                    }
                    // exponent = ceil(log10(ratio))
                    double expd = std::ceil(std::log10(ratio));
                    int    expi = static_cast<int>(expd);
                    if (expi < 0)
                        expi = 0;
                    if (expi > 9)
                        expi = 9;  // clamp to reasonable max (1e9)
                    return std::pow(10.0, expi);
                }
            }
            return 1.0;  // default assume shares
        }
    }  // namespace

    // void MinuteKLine::adjust(double m, double a, int number) {
    //     Open = Open * m + a;
    //     Close = Close * m + a;
    //     High = High * m + a;
    //     Low = Low * m + a;
    //     // 成交量复权
    //     // 1. 计算均价
    //     auto ap = Amount / Volume;
    //     // 2. 均价复权
    //     ap = ap * m + a;
    //     // 3. 以成交金额为基准, 用复权均价计算成交量
    //     Volume = Amount / ap;
    //     AdjustmentCount += number;
    // }

    std::vector<MinuteKLine> read_minute_kline_from_csv(const std::string &filename) {
        std::vector<MinuteKLine> klines;
        try {
            // 创建 CSV 读取器
            io::CSVReader<11> in(filename);  // 有 11 列数据

            // 设置表头字段名(用于自动匹配顺序)
            in.read_header(io::ignore_extra_column,
                           "date",
                           "open",
                           "close",
                           "high",
                           "low",
                           "volume",
                           "amount",
                           "up",
                           "down",
                           "datetime",
                           "adjustment_count");

            MinuteKLine row = {};
            while (in.read_row(row.date,
                               row.open,
                               row.close,
                               row.high,
                               row.low,
                               row.volume,
                               row.amount,
                               row.up,
                               row.down,
                               row.datetime,
                               row.adjustment_count)) {
                klines.emplace_back(row);
            }
        } catch (...) {
            // 忽略异常, 读csv文件失败, 返回空
        }
        return klines;
    }

    std::vector<MinuteKLine> load_minute_kline(const std::string &code, const std::string &freq) {
        auto [minutes, frequency] = pandas::parse_frequency(freq);
        auto filename             = config::get_kline_filename_ex(code, frequency);
        spdlog::debug("[data::MinuteKLine] kline file: {}", filename);
        return read_minute_kline_from_csv(filename);
    }

    void DataMinuteKLine::Print(const std::string &code, const std::vector<exchange::timestamp> &dates) {
        (void)code;
        (void)dates;
    }

    void DataMinuteKLine::Update(const std::string &code, const exchange::timestamp &date) {
        if (!mkc_.enabled) {
            return;
        }
        //        if(date != exchange::last_trading_day()) {
        //            return;
        //        }
        const std::string freq_ = mkc_.frequency;
        (void)date;
        // 1. 确定本地有效数据最后1条数据作为拉取数据的开始日期
        auto current_start_date = market_first_date;
        try {
            std::string              cache_filename    = config::get_kline_filename_ex(code, freq_);
            std::vector<MinuteKLine> cacheMinuteKLines = read_minute_kline_from_csv(cache_filename);
            auto                     klines_length     = cacheMinuteKLines.size();
            int                      adjust_times      = 0;  // 除权除息的次数
            auto                     period            = 1;
            auto                     numberOfDay       = 1;
            level1::KLineType        kline_type        = level1::_1MIN;
            if (mkc_.enabled) {
                period      = mkc_.minutes;
                numberOfDay = detail::CN_DEFAULT_TOTALFZNUM / period;
                switch (period) {
                    case 5:
                        kline_type = level1::_5MIN;
                        break;
                    case 15:
                        kline_type = level1::_15MIN;
                        break;
                    case 30:
                        kline_type = level1::_30MIN;
                        break;
                    case 60:
                        kline_type = level1::_1HOUR;
                        break;
                    default:
                        kline_type = level1::_1MIN;
                        break;
                }
            }
            const size_t min_fixed_offset = detail::MAX_KLINE_LOOKBACK_DAYS * numberOfDay;
            auto klines_offset = min_fixed_offset;
            //size_t klines_aligned_length = 0;
            if (klines_length > 0) {
                if (klines_offset > klines_length) {
                    klines_offset = klines_length;
                }
                // candidate: 原始候选起点索引
                size_t candidate = (klines_length > klines_offset) ? (klines_length - klines_offset) : 0;
                // 使用 floor 对齐到 min_fixed_offset 的倍数，确保 (klines_length - klines_offset) 为该块大小的整数倍
                size_t aligned = (candidate / min_fixed_offset) * min_fixed_offset;
                // 边界保护
                if (aligned >= klines_length) {
                    aligned = 0;
                }
                // 重新计算 klines_offset，使得 klines_length - klines_offset == aligned
                klines_offset = klines_length - aligned;
                // 根据对齐后的索引取出对应的日期作为拉取起点
                const auto &kline  = cacheMinuteKLines[aligned];
                current_start_date = kline.date;  // 修正本次更新的开始日期
                adjust_times       = kline.adjustment_count;

                // 如果 aligned 看起来不是某个交易日的首条记录，记录警告以便人工审查。
                // （业务上若要求严格为交易日首条，应在此处添加向前回退到当天首条的逻辑；
                // 但那将打破"(klines_length - klines_offset) 为整块大小"的约束，两者需明确优先级。）
                if (aligned > 0 && cacheMinuteKLines[aligned - 1].date == cacheMinuteKLines[aligned].date) {
                    spdlog::warn("[data::MinuteKLine] aligned index {} is not day-first for {} (date={})",
                                 aligned,
                                 code,
                                 cacheMinuteKLines[aligned].date);
                }
            }
            // 2. 确定结束日期
            auto current_trading_date = exchange::timestamp::now().pre_market_time();
            spdlog::debug("[data::MinuteKLine] [{}]: from {} to {}",
                          code,
                          current_start_date.only_date(),
                          current_trading_date.only_date());
            auto ts         = exchange::date_range(current_start_date, current_trading_date);
            auto total_days = ts.size();
            auto max_       = 65535;
            auto max_days   = max_ / numberOfDay;
            auto days_      = std::min(max_days, int(total_days));
            // 计算需要拉取的新增分钟K线数量
            auto incremental_total = days_ * numberOfDay;
            current_start_date     = ts[total_days - days_];
            auto current_end_date  = ts[total_days - 1];
            spdlog::debug("[data::MinuteKLine] [{}]: from {} to {}",
                          code,
                          current_start_date.only_date(),
                          current_end_date.only_date());
            u16 step  = level1::security_bars_max;
            u16 start = 0;
            // 3. 拉取数据
            std::vector<std::vector<level1::SecurityBar>> hs;
            size_t                                        elementCount = 0;
            do {
                u16 count = step;
                if (incremental_total - start >= step) {
                    count = step;
                } else {
                    count = u16(incremental_total - start);
                }
                auto reply = detail::fetch_kline(code, start, count, kline_type);
                if (reply.empty()) {
                    break;
                }
                elementCount += reply.size();
                hs.emplace_back(reply);
                if (reply.size() < count) {
                    break;
                }
                start += count;
            } while (start < incremental_total);
            // 4. 由于K线数据，每次获取数据是从后往前获取, 所以这里需要反转历史数据的切片
            std::reverse(hs.begin(), hs.end());
            // 5. 调整成交量, 单位从手改成股, vol字段 * 100
            std::vector<MinuteKLine> incremental_klines;
            incremental_klines.reserve(elementCount);
            // 从获取到的 bars 中推断成交量单位（可能为 1、10、100、1000 等）
            f64 bar_vol_unit = infer_bar_vol_unit(hs);
            // std::cout << "Inferred bar volume unit: " << bar_vol_unit << "\n";
            for (const auto &vec : hs) {
                for (const auto &row : vec) {
                    auto dateTime = exchange::timestamp(row.Year, row.Month, row.Day).pre_market_time();
                    if (dateTime < current_start_date || dateTime > current_trading_date) {
                        // 不在本地更新范围内的记录, 忽略掉
                        continue;
                    }
                    auto kx = MinuteKLine{
                        .date            = dateTime.only_date(),    // 日期
                        .open            = row.Open,                // 开盘价
                        .close           = row.Close,               // 收盘价
                        .high            = row.High,                // 最高价
                        .low             = row.Low,                 // 最低价
                        .volume          = row.Vol * bar_vol_unit,  // 成交量(股)
                        .amount          = row.Amount,              // 成交金额(元)
                        .up              = row.UpCount,             // 上涨家数 / 外盘
                        .down            = row.DownCount,           // 下跌家数 / 内盘
                        .datetime        = row.DateTime,            // 时间
                        .adjustment_count = 0                        // 新增：除权除息次数
                    };
                    incremental_klines.emplace_back(kx);
                }
            }
            // 6. K线数据转换成MinuteKLine结构
            // 6.1 判断是否已除权的依据
            // 6.1.1 当前更新K线只有1条记录, 则是当前日期, 那么本次更新为当前日期内的多次更新,
            // 需要判断这条新数据是否需要更新缓存以及是否复权 6.1.2 如果隔日更新, 会有2条数据,
            // 缓存中因为偏移是有一条从服务器获取的未复权数据, 第二条数据是当日不需要前复权的记录 6.1.3
            // 只需要判断缓存中的最后一条数据是否除权, 即增量的日线数据的第一条是否需要除权, 如果已除权,
            // 说明缓存内的数据已经全部复权,
            //       只需要复权增量数据的复权即可, 如果没有除权, 则需要对全部的K线数据进行全量处理是否复权
            bool isFreshFetchRequireAdjustment = /*incremental_klines.size() == 1 && */ adjust_times == 1;
            auto dividends                     = load_xdxr(code);
            if (isFreshFetchRequireAdjustment) {
                // 只除权除息最新的一条记录
                detail::apply_forward_adjustment_for_event(incremental_klines, current_start_date, dividends);
            }
            // 6.2 只前复权当日数据
            // 7. 拼接缓存和新增的数据
            std::vector<MinuteKLine> klines;
            // 7.1 先截取本地缓存的数据
            if (klines_length > klines_offset) {
                // 注意：iterator 的差值类型为 signed difference_type, 这里显式转换以避免窄化警告
                klines.insert(klines.end(),
                              cacheMinuteKLines.begin(),
                              cacheMinuteKLines.begin() + static_cast<std::ptrdiff_t>(klines_length - klines_offset));
            }
            // 7.2 拼接新增的数据
            if (klines.empty()) {
                klines = incremental_klines;
            } else {
                klines.insert(klines.end(), incremental_klines.begin(), incremental_klines.end());
            }
            // 8. 前复权
            if (!isFreshFetchRequireAdjustment) {
                detail::apply_forward_adjustment_for_event(klines, current_start_date, dividends);
            }
            // 9. 刷新缓存文件
            save_kline(cache_filename, klines);
        } catch (const std::exception &e) {  // 其他标准异常
            spdlog::error("[data::MinuteKLine] - 标准异常: {} (type: {})", e.what(), typeid(e).name());
            // 对于system_error可以记录更多信息
            if (auto se = dynamic_cast<const std::system_error *>(&e)) {
                spdlog::error("[dataset::MinuteKLine] Error code: {}, category: {}",
                              se->code().value(),
                              se->code().category().name());
            }
        } catch (...) {
            spdlog::error("[data::MinuteKLine] 获取分钟级别K线异常");
        }
    }

}  // namespace data