#include <quant1x/datasets/kline_minute.h>
#include <quant1x/datasets/kline_raw.h>
#include <ranges>

#include <quant1x/pandas/rule.h>

namespace datasets {

    namespace {

        void save_kline(const std::string &filename, const std::vector<MinuteKLine>& values) {
            util::check_filepath(filename, true);
            io::CSVWriter writer(filename);
            writer.write_row("Date", "Open", "Close", "High", "Low", "Volume", "Amount", "Up", "Down", "Datetime", "AdjustmentCount");
            for (const auto &row: values) {
                writer.write_row(row.Date, row.Open, row.Close, row.High, row.Low, row.Volume, row.Amount,
                                 row.Up, row.Down, row.Datetime, row.AdjustmentCount);
            }
        }

        void calculate_pre_adjust(std::vector<MinuteKLine> &klines, const exchange::timestamp &startDate, const std::vector<level1::XdxrInfo> &dividends) {
            if(klines.empty()) {
                return;
            }
            // 最后一根K线的日期
            auto const& last_day = klines[klines.size()-1].Date;
            // 转成时间戳且对齐时间
            auto const& ts_last_day = exchange::timestamp::parse(last_day).pre_market_time();
            // 计算最后一根K线的下一个交易日的日期, 除权除息是不包括除权除息当日的, 所以要计算下一个交易日与除权除息的列表去匹配
            // 300773拉卡拉, 2025年6月6日除权, 数据公布于6月3日之前, 那么在6月6日之前的6月4日收盘前是不能除权除息的，6月5日收盘可以除权
            auto const& last_day_next = exchange::next_trading_day(ts_last_day).only_date();
            auto start_date = startDate.only_date();
            auto xdxr_infos = dividends | std::views::filter([&last_day_next](const level1::XdxrInfo & x) {return last_day_next >= x.Date && x.Category == 1;});
            //int times = 0; // 除权除息次数
            size_t count = std::ranges::distance(xdxr_infos); // 除权除息总次数
            // 时间越早的记录除权除息次数越多, 第一条数据时时总的除权除息次数
            auto times = count;
            for(auto const & info : xdxr_infos) {
                if(info.Date <= start_date) {
                    // 除权除息数据在日线第一条数据之前, 也就是ipo上市日期之前的数据, 不能用作复权
                    //continue;
                } else {
                    auto [m, a] = info.adjustFactor();
                    auto klines_size = klines.size();
                    for (size_t i = 0; i < klines_size; ++i) {
                        auto kl = &(klines[i]);
                        if (kl->Date >= info.Date) {
                            break;
                        }
                        if (kl->Date < info.Date) {
                            kl->Open = kl->Open * m + a;
                            kl->Close = kl->Close * m + a;
                            kl->High = kl->High * m + a;
                            kl->Low = kl->Low * m + a;
                            // 成交量复权
                            // 1. 计算均价
                            auto ap = kl->Amount / kl->Volume;
                            // 2. 均价复权
                            ap = ap * m + a;
                            // 3. 以成交金额为基准, 用复权均价计算成交量
                            kl->Volume = kl->Amount / ap;
                            kl->AdjustmentCount += 1;
                        }
                    }
                }
                --times;
                (void)times;
            }
        }

        // config::MinuteKLineConfig kline_config() {
        //     config::MinuteKLineConfig config{};
        //     auto const &local_cfg = config::global_config().data.cache.kline;
        //     if (local_cfg.size() != 1) {
        //         throw std::runtime_error("kline config size must be exactly one");
        //     }
        //     const auto minute_kline_config = local_cfg.begin();
        //     const auto key = minute_kline_config->first;
        //     const auto value = minute_kline_config->second;
        //     const auto d = pandas::ParseTimeRule(key);
        //     const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(d);
        //     config.minutes = minutes.count();
        //     config.frequency = key;
        //     config.enabled = value;
        //     return config;
        // }

    }

    void MinuteKLine::adjust(double m, double a, int number) {
        Open = Open * m + a;
        Close = Close * m + a;
        High = High * m + a;
        Low = Low * m + a;
        // 成交量复权
        // 1. 计算均价
        auto ap = Amount / Volume;
        // 2. 均价复权
        ap = ap * m + a;
        // 3. 以成交金额为基准, 用复权均价计算成交量
        Volume = Amount / ap;
        AdjustmentCount += number;
    }

    std::vector<MinuteKLine> read_minute_kline_from_csv(const std::string& filename) {
        std::vector<MinuteKLine> klines;
        try {
            // 创建 CSV 读取器
            io::CSVReader<11> in(filename);  // 有 11 列数据

            // 设置表头字段名(用于自动匹配顺序)
            in.read_header(io::ignore_extra_column,
                           "Date", "Open", "Close", "High", "Low",
                           "Volume", "Amount", "Up", "Down", "Datetime", "AdjustmentCount");

            MinuteKLine row = {};
            while (in.read_row(row.Date, row.Open, row.Close, row.High, row.Low, row.Volume, row.Amount,
                               row.Up, row.Down, row.Datetime, row.AdjustmentCount)) {
                klines.emplace_back(row);
            }
        } catch(...) {
            // 忽略异常, 读csv文件失败, 返回空
        }
        return klines;
    }

    std::vector<MinuteKLine> load_minute_kline(const std::string &code, const std::string &freq) {
        auto [minutes, frequency] = pandas::parse_frequency(freq);
        auto filename = config::get_kline_filename_ex(code, frequency);
        spdlog::debug("[dataset::MinuteKLine] kline file: {}", filename);
        return read_minute_kline_from_csv(filename);
    }

    void DataMinuteKLine::Print(const std::string &code, const std::vector<exchange::timestamp> &dates)  {
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
            std::string cache_filename = config::get_kline_filename_ex(code, freq_);
            std::vector<MinuteKLine> cacheMinuteKLines = read_minute_kline_from_csv(cache_filename);
            auto klines_length = cacheMinuteKLines.size();
            int adjust_times = 0; // 除权除息的次数
            auto period = 1;
            auto numberOfDay = 1;
            level1::KLineType kline_type = level1::_1MIN;
            if (mkc_.enabled) {
                period = mkc_.minutes;
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
            auto klines_offset = detail::MAX_KLINE_LOOKBACK_DAYS * numberOfDay;
            if(klines_length > 0) {
                if (klines_offset > klines_length) {
                    klines_offset = klines_length;
                }
                // 根据最大可以偏移的K线天数, 从缓存中截取对应的日期, 作为从服务器获取数据的起始日期
                const auto& kline = cacheMinuteKLines[klines_length-klines_offset];
                current_start_date = kline.Date; // 修正本次更新的开始日期
                adjust_times = kline.AdjustmentCount;
            }
            // 2. 确定结束日期
            auto current_trading_date = exchange::timestamp::now().pre_market_time();
            spdlog::debug("[dataset::MinuteKLine] [{}]: from {} to {}", code, current_start_date.only_date(), current_trading_date.only_date());
            auto ts = exchange::date_range(current_start_date, current_trading_date);
            auto max_ = 65535;
            auto total = int(ts.size());
            auto max_days = max_/numberOfDay;
            auto days_ = std::min(max_days, total);
            total = days_ * numberOfDay;
            current_start_date = ts[0];
            auto current_end_date = ts[days_-1];
            spdlog::debug("[dataset::MinuteKLine] [{}]: from {} to {}", code, current_start_date.only_date(), current_end_date.only_date());
            u16 step = level1::security_bars_max;
            u16 start = 0;
            // 3. 拉取数据
            std::vector<std::vector<level1::SecurityBar>> hs;
            size_t elementCount = 0;
            do {
                u16 count = step;
                if(total - start >= step) {
                    count = step;
                } else {
                    count = u16(total - start);
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
            } while (start < total);
            // 4. 由于K线数据，每次获取数据是从后往前获取, 所以这里需要反转历史数据的切片
            std::reverse(hs.begin(), hs.end());
            // 5. 调整成交量, 单位从手改成股, vol字段 * 100
            std::vector<MinuteKLine> incremental_klines;
            incremental_klines.reserve(elementCount);
            for(const auto & vec : hs) {
                for (const auto & row : vec) {
                    auto dateTime = exchange::timestamp(row.Year, row.Month, row.Day).pre_market_time();
                    if (dateTime < current_start_date || dateTime > current_trading_date) {
                        // 不在本地更新范围内的记录, 忽略掉
                        continue;
                    }
                    auto kx = MinuteKLine{
                        .Date = dateTime.only_date(), // 日期
                        .Open = row.Open,             // 开盘价
                        .Close = row.Close,           // 收盘价
                        .High = row.High,             // 最高价
                        .Low = row.Low,               // 最低价
                        .Volume = row.Vol * 100,      // 成交量(股)
                        .Amount = row.Amount,         // 成交金额(元)
                        .Up = row.UpCount,            // 上涨家数 / 外盘
                        .Down = row.DownCount,        // 下跌家数 / 内盘
                        .Datetime = row.DateTime,     // 时间
                        .AdjustmentCount = 0      // 新增：除权除息次数
                    };
                    incremental_klines.emplace_back(kx);
                }
            }
            // 6. K线数据转换成MinuteKLine结构
            // 6.1 判断是否已除权的依据
            // 6.1.1 当前更新K线只有1条记录, 则是当前日期, 那么本次更新为当前日期内的多次更新, 需要判断这条新数据是否需要更新缓存以及是否复权
            // 6.1.2 如果隔日更新, 会有2条数据, 缓存中因为偏移是有一条从服务器获取的未复权数据, 第二条数据是当日不需要前复权的记录
            // 6.1.3 只需要判断缓存中的最后一条数据是否除权, 即增量的日线数据的第一条是否需要除权, 如果已除权, 说明缓存内的数据已经全部复权,
            //       只需要复权增量数据的复权即可, 如果没有除权, 则需要对全部的K线数据进行全量处理是否复权
            bool isFreshFetchRequireAdjustment = /*incremental_klines.size() == 1 && */ adjust_times == 1;
            auto dividends = load_xdxr(code);
            if (isFreshFetchRequireAdjustment) {
                // 只除权除息最新的一条记录
                calculate_pre_adjust(incremental_klines, current_start_date, dividends);
            }
            // 6.2 只前复权当日数据
            // 7. 拼接缓存和新增的数据
            std::vector<MinuteKLine> klines;
            // 7.1 先截取本地缓存的数据
            if (klines_length > klines_offset) {
                klines.insert(klines.end(), cacheMinuteKLines.begin(), cacheMinuteKLines.begin()+(klines_length-klines_offset));
            }
            // 7.2 拼接新增的数据
            if (klines.empty()) {
                klines = incremental_klines;
            } else {
                klines.insert(klines.end(), incremental_klines.begin(), incremental_klines.end());
            }
            // 8. 前复权
            if (!isFreshFetchRequireAdjustment) {
                calculate_pre_adjust(klines, current_start_date, dividends);
            }
            // 9. 刷新缓存文件
            save_kline(cache_filename, klines);
        } catch (const std::exception &e) {  // 其他标准异常
            spdlog::error("[dataset::MinuteKLine] - 标准异常: {} (type: {})", e.what(), typeid(e).name());
            // 对于system_error可以记录更多信息
            if (auto se = dynamic_cast<const std::system_error *>(&e)) {
                spdlog::error("[dataset::MinuteKLine] Error code: {}, category: {}", se->code().value(), se->code().category().name());
            }
        } catch (...) {
            spdlog::error("[dataset::MinuteKLine] 获取分钟级别K线异常");
        }
    }

} // namespace datasets