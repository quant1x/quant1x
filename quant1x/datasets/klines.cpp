#include <quant1x/datasets/kline.h>
#include <quant1x/datasets/kline_raw.h>
#include <ranges>

namespace datasets {

    namespace {

        void save_kline(const std::string &filename, const std::vector<KLine>& values) {
            util::check_filepath(filename, true);
            io::CSVWriter writer(filename);
            writer.write_row("Date", "Open", "Close", "High", "Low", "Volume", "Amount", "Up", "Down", "Datetime", "AdjustmentCount");
            for (const auto &row: values) {
                writer.write_row(row.Date, row.Open, row.Close, row.High, row.Low, row.Volume, row.Amount,
                                 row.Up, row.Down, row.Datetime, row.AdjustmentCount);
            }
        }

        void calculate_pre_adjust(std::vector<KLine> &klines, const exchange::timestamp &startDate, const std::vector<level1::XdxrInfo> &dividends) {
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
            auto xdxrs = dividends | std::views::filter([&last_day_next](const level1::XdxrInfo & x) {return last_day_next >= x.Date && x.Category == 1;});
            //int times = 0; // 除权除息次数
            size_t count = std::ranges::distance(xdxrs); // 除权除息总次数
            // 时间越早的记录除权除息次数越多, 第一条数据时时总的除权除息次数
            auto times = count;
            for(auto const & xdxr : xdxrs) {
                if(xdxr.Date <= start_date) {
                    // 除权除息数据在日线第一条数据之前, 也就是ipo上市日期之前的数据, 不能用作复权
                    //continue;
                } else {
                    auto [m, a] = xdxr.adjustFactor();
                    for (size_t i = 0; i < klines.size(); ++i) {
                        auto kl = &(klines[i]);
                        if (kl->Date >= xdxr.Date) {
                            break;
                        }
                        if (kl->Date < xdxr.Date) {
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
    }

    void KLine::adjust(double m, double a, int number) {
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

    std::vector<KLine> read_kline_from_csv(const std::string& filename) {
        std::vector<KLine> klines;
        try {
            // 创建 CSV 读取器
            io::CSVReader<11> in(filename);  // 有 11 列数据

            // 设置表头字段名(用于自动匹配顺序)
            in.read_header(io::ignore_extra_column,
                           "Date", "Open", "Close", "High", "Low",
                           "Volume", "Amount", "Up", "Down", "Datetime", "AdjustmentCount");

            KLine row = {};
            while (in.read_row(row.Date, row.Open, row.Close, row.High, row.Low, row.Volume, row.Amount,
                               row.Up, row.Down, row.Datetime, row.AdjustmentCount)) {
                klines.emplace_back(row);
            }
        } catch(...) {
            // 忽略异常, 读csv文件失败, 返回空
        }
        return klines;
    }

    std::vector<KLine> load_kline(const std::string &code) {
        auto filename = config::get_kline_filename(code);
        spdlog::debug("[dataset::KLine] kline file: {}", filename);
        return read_kline_from_csv(filename);
    }

    void DataKLine::Print(const std::string &code, const std::vector<exchange::timestamp> &dates)  {
        (void)code;
        (void)dates;
    }

    void DataKLine::Update(const std::string &code, const exchange::timestamp &date) {
//        if(date != exchange::last_trading_day()) {
//            return;
//        }
        (void)date;
        // 1. 确定本地有效数据最后1条数据作为拉取数据的开始日期
        auto current_start_date = market_first_date;
        try {
            std::string cache_filename = config::get_kline_filename(code);
            std::vector<KLine> cacheKLines = read_kline_from_csv(cache_filename);
            auto klines_length = cacheKLines.size();
            auto klines_offset_days = detail::MAX_KLINE_LOOKBACK_DAYS;
            int adjust_times = 0; // 除权除息的次数
            if(klines_length > 0) {
                if (klines_offset_days > klines_length) {
                    klines_offset_days = klines_length;
                }
                // 根据最大可以偏移的K线天数, 从缓存中截取对应的日期, 作为从服务器获取数据的起始日期
                const auto& kline = cacheKLines[klines_length-klines_offset_days];
                current_start_date = kline.Date; // 修正本次更新的开始日期
                adjust_times = kline.AdjustmentCount;
            }
            // 2. 确定结束日期
            auto current_end_date = exchange::timestamp::now().pre_market_time();
            spdlog::debug("[dataset::KLine] [{}]: from {} to {}", code, current_start_date.only_date(), current_end_date.only_date());
            auto ts = exchange::date_range(current_start_date, current_end_date);
            auto total = ts.size();
            current_start_date = ts[0];
            current_end_date = ts[total-1];
            spdlog::debug("[dataset::KLine] [{}]: from {} to {}", code, current_start_date.only_date(), current_end_date.only_date());
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
                auto reply = detail::fetch_kline(code, start, count);
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
            std::vector<KLine> incremental_klines;
            incremental_klines.reserve(elementCount);
            for(const auto & vec : hs) {
                for (const auto & row : vec) {
                    auto dateTime = exchange::timestamp(row.Year, row.Month, row.Day).pre_market_time();
                    if (dateTime < current_start_date || dateTime > current_end_date) {
                        // 不在本地更新范围内的记录, 忽略掉
                        continue;
                    }
                    auto kx = KLine{
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
            // 6. K线数据转换成KLine结构
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
            std::vector<KLine> klines;
            // 7.1 先截取本地缓存的数据
            if (klines_length > klines_offset_days) {
                klines.insert(klines.end(), cacheKLines.begin(), cacheKLines.begin()+(klines_length-klines_offset_days));
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
            spdlog::error("[dataset::KLine] - 标准异常: {} (type: {})", e.what(), typeid(e).name());
            // 对于system_error可以记录更多信息
            if (auto se = dynamic_cast<const std::system_error *>(&e)) {
                spdlog::error("[dataset::KLine] Error code: {}, category: {}", se->code().value(), se->code().category().name());
            }
        } catch (...) {
            spdlog::error("[dataset::KLine] 获取日K线异常");
        }
    }

} // namespace datasets