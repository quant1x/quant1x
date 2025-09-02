#include <quant1x/exchange/calendar.h>

#include <quant1x/runtime/config.h>
#include <quant1x/runtime/cache1d.h>
#include <quant1x/std/time.h>
#include <quant1x/std/util.h>
#include <quant1x/io/file.h>
#include <quant1x/io/http.h>
#include <quant1x/io/csv-reader.h>
#include <quant1x/io/csv-writer.h>
#include <quant1x/exchange/timestamp.h>
#include <duktape.h>

//============================================================
// exchange 交易日历相关                                      //
//============================================================

namespace exchange {

    //============================================================
    // JS解码                                                    //
    //============================================================
    namespace js_sina {

        const char * const javascript_decoder = R"(function d(t) {
    var e, i, n, r, a, o, s, l = (arguments,
            864e5), u = 7657, c = [], h = [], d = ~(3 << 30), f = 1 << 30,
        p = [0, 3, 5, 6, 9, 10, 12, 15, 17, 18, 20, 23, 24, 27, 29, 30], m = Math, g = function () {
            var l, u;
            for (l = 0; 64 > l; l++)
                h[l] = m.pow(2, l),
                26 > l && (c[l] = v(l + 65),
                    c[l + 26] = v(l + 97),
                10 > l && (c[l + 52] = v(l + 48)));
            for (c.push("+", "/"),
                     c = c.join(""),
                     i = t.split(""),
                     n = i.length,
                     l = 0; n > l; l++)
                i[l] = c.indexOf(i[l]);
            return r = {},
                e = o = 0,
                a = {},
                u = w([12, 6]),
                s = 63 ^ u[1],
            {
                _1479: T,
                _136: _,
                _200: S,
                _139: k,
                _197: _mi_run
            }["_" + u[0]] || function () {
                return []
            }
        }, v = String.fromCharCode, b = function (t) {
            return t === {}._
        }, N = function () {
            var t, e;
            for (t = y(),
                     e = 1; ;) {
                if (!y())
                    return e * (2 * t - 1);
                e++
            }
        }, y = function () {
            var t;
            return e >= n ? 0 : (t = i[e] & 1 << o,
                o++,
            o >= 6 && (o -= 6,
                e++),
                !!t)
        }, w = function (t, r, a) {
            var s, l, u, c, d;
            for (l = [],
                     u = 0,
                 r || (r = []),
                 a || (a = []),
                     s = 0; s < t.length; s++)
                if (c = t[s],
                    u = 0,
                    c) {
                    if (e >= n)
                        return l;
                    if (t[s] <= 0)
                        u = 0;
                    else if (t[s] <= 30) {
                        for (; d = 6 - o,
                                   d = c > d ? d : c,
                                   u |= (i[e] >> o & (1 << d) - 1) << t[s] - c,
                                   o += d,
                               o >= 6 && (o -= 6,
                                   e++),
                                   c -= d,
                                   !(0 >= c);)
                            ;
                        r[s] && u >= h[t[s] - 1] && (u -= h[t[s]])
                    } else
                        u = w([30, t[s] - 30], [0, r[s]]),
                        a[s] || (u = u[0] + u[1] * h[30]);
                    l[s] = u
                } else
                    l[s] = 0;
            return l
        }, x = function (t) {
            var e, i, n;
            for (t > 1 && (e = 0),
                     e = 0; t > e; e++)
                r.d++,
                    n = r.d % 7,
                (3 == n || 4 == n) && (r.d += 5 - n);
            return i = new Date,
                i.setTime((u + r.d) * l),
                i
        }, S = function () {
            var t, i, a, o, l;
            if (s >= 1)
                return [];
            for (r.d = w([18], [1])[0] - 1,
                     a = w([3, 3, 30, 6]),
                     r.p = a[0],
                     r.ld = a[1],
                     r.cd = a[2],
                     r.c = a[3],
                     r.m = m.pow(10, r.p),
                     r.pc = r.cd / r.m,
                     i = [],
                     t = 0; o = {
                d: 1
            },
                 y() && (a = w([3])[0],
                     0 == a ? o.d = w([6])[0] : 1 == a ? (r.d = w([18])[0],
                         o.d = 0) : o.d = a),
                     l = {
                         day: x(o.d)
                     },
                 y() && (r.ld += N()),
                     a = w([3 * r.ld], [1]),
                     r.cd += a[0],
                     l.close = r.cd / r.m,
                     i.push(l),
                 !(e >= n) && (e != n - 1 || 63 & (r.c ^ t + 1)); t++)
                ;
            return i[0].prevclose = r.pc,
                i
        }, _ = function () {
            var t, i, a, o, l, u, c, h, d, f, p;
            if (s > 2)
                return [];
            for (c = [],
                     d = {
                         v: "volume",
                         p: "price",
                         a: "avg_price"
                     },
                     r.d = w([18], [1])[0] - 1,
                     h = {
                         day: x(1)
                     },
                     a = w(1 > s ? [3, 3, 4, 1, 1, 1, 5] : [4, 4, 4, 1, 1, 1, 3]),
                     t = 0; 7 > t; t++)
                r[["la", "lp", "lv", "tv", "rv", "zv", "pp"][t]] = a[t];
            for (r.m = m.pow(10, r.pp),
                     s >= 1 ? (a = w([3, 3]),
                         r.c = a[0],
                         a = a[1]) : (a = 5,
                         r.c = 2),
                     r.pc = w([6 * a])[0],
                     h.pc = r.pc / r.m,
                     r.cp = r.pc,
                     r.da = 0,
                     r.sa = r.sv = 0,
                     t = 0; !(e >= n) && (e != n - 1 || 7 & (r.c ^ t)); t++) {
                for (l = {},
                         o = {},
                         f = r.tv ? y() : 1,
                         i = 0; 3 > i; i++)
                    if (p = ["v", "p", "a"][i],
                    (f ? y() : 0) && (a = N(),
                        r["l" + p] += a),
                        u = "v" == p && r.rv ? y() : 1,
                        a = w([3 * r["l" + p] + ("v" == p ? 7 * u : 0)], [!!i])[0] * (u ? 1 : 100),
                        o[p] = a,
                    "v" == p) {
                        if (!(l[d[p]] = a) && (s > 1 || 241 > t) && (r.zv ? !y() : 1)) {
                            o.p = 0;
                            break
                        }
                    } else
                        "a" == p && (r.da = (1 > s ? 0 : r.da) + o.a);
                r.sv += o.v,
                    l[d.p] = (r.cp += o.p) / r.m,
                    r.sa += o.v * r.cp,
                    l[d.a] = b(o.a) ? t ? c[t - 1][d.a] : l[d.p] : r.sv ? ((m.floor((r.sa * (2e3 / r.m) + r.sv) / r.sv) >> 1) + r.da) / 1e3 : l[d.p] + r.da / 1e3,
                    c.push(l)
            }
            return c[0].date = h.day,
                c[0].prevclose = h.pc,
                c
        }, T = function () {
            var t, e, i, n, a, o, l;
            if (s >= 1)
                return [];
            for (r.lv = 0,
                     r.ld = 0,
                     r.cd = 0,
                     r.cv = [0, 0],
                     r.p = w([6])[0],
                     r.d = w([18], [1])[0] - 1,
                     r.m = m.pow(10, r.p),
                     a = w([3, 3]),
                     r.md = a[0],
                     r.mv = a[1],
                     t = []; a = w([6]),
                     a.length;) {
                if (i = {
                    c: a[0]
                },
                    n = {},
                    i.d = 1,
                32 & i.c)
                    for (; ;) {
                        if (a = w([6])[0],
                        63 == (16 | a)) {
                            l = 16 & a ? "x" : "u",
                                a = w([3, 3]),
                                i[l + "_d"] = a[0] + r.md,
                                i[l + "_v"] = a[1] + r.mv;
                            break
                        }
                        if (32 & a) {
                            o = 8 & a ? "d" : "v",
                                l = 16 & a ? "x" : "u",
                                i[l + "_" + o] = (7 & a) + r["m" + o];
                            break
                        }
                        if (o = 15 & a,
                            0 == o ? i.d = w([6])[0] : 1 == o ? (r.d = o = w([18])[0],
                                i.d = 0) : i.d = o,
                            !(16 & a))
                            break
                    }
                n.date = x(i.d);
                for (o in {
                    v: 0,
                    d: 0
                })
                    b(i["x_" + o]) || (r["l" + o] = i["x_" + o]),
                    b(i["u_" + o]) && (i["u_" + o] = r["l" + o]);
                for (i.l_l = [i.u_d, i.u_d, i.u_d, i.u_d, i.u_v],
                         l = p[15 & i.c],
                     1 & i.u_v && (l = 31 - l),
                     16 & i.c && (i.l_l[4] += 2),
                         e = 0; 5 > e; e++)
                    l & 1 << 4 - e && i.l_l[e]++,
                        i.l_l[e] *= 3;
                i.d_v = w(i.l_l, [1, 0, 0, 1, 1], [0, 0, 0, 0, 1]),
                    o = r.cd + i.d_v[0],
                    n.open = o / r.m,
                    n.high = (o + i.d_v[1]) / r.m,
                    n.low = (o - i.d_v[2]) / r.m,
                    n.close = (o + i.d_v[3]) / r.m,
                    a = i.d_v[4],
                "number" == typeof a && (a = [a, a >= 0 ? 0 : -1]),
                    r.cd = o + i.d_v[3],
                    l = r.cv[0] + a[0],
                    r.cv = [l & d, r.cv[1] + a[1] + !!((r.cv[0] & d) + (a[0] & d) & f)],
                    n.volume = (r.cv[0] & f - 1) + r.cv[1] * f,
                    t.push(n)
            }
            return t
        }, k = function () {
            var t, e, i, n;
            if (s > 1)
                return [];
            for (r.l = 0,
                     n = -1,
                     r.d = w([18])[0] - 1,
                     i = w([18])[0]; r.d < i;)
                e = x(1),
                    0 >= n ? (y() && (r.l += N()),
                        n = w([3 * r.l], [0])[0] + 1,
                    t || (t = [e],
                        n--)) : t.push(e),
                    n--;
            return t
        };
    return _mi_run = function () {
        var t, i, a, o;
        if (s >= 1)
            return [];
        for (r.f = w([6])[0],
                 r.c = w([6])[0],
                 a = [],
                 r.dv = [],
                 r.dl = [],
                 t = 0; t < r.f; t++)
            r.dv[t] = 0,
                r.dl[t] = 0;
        for (t = 0; !(e >= n) && (e != n - 1 || 7 & (r.c ^ t)); t++) {
            for (o = [],
                     i = 0; i < r.f; i++)
                y() && (r.dl[i] += N()),
                    r.dv[i] += w([3 * r.dl[i]], [1])[0],
                    o[i] = r.dv[i];
            a.push(o)
        }
        return a
    }
        ,
        g()()
})";

        /// 预处理http接口返回的js文本, 去除赋值双引号等
        static std::string preprocess(const std::string& text) {
            std::string processed = text;
            size_t eqPos = processed.find('=');
            if (eqPos != std::string::npos) {
                processed = processed.substr(eqPos + 1);
            }
            size_t semiPos = processed.find(';');
            if (semiPos != std::string::npos) {
                processed = processed.substr(0, semiPos);
            }
            processed.erase(std::remove(processed.begin(), processed.end(), '"'), processed.end());
            return processed;
        }

        /// js解码
        static std::vector<std::string> decode(const std::string &text) {
            duk_context *ctx = duk_create_heap_default();

            if (duk_peval_string(ctx, javascript_decoder) != DUK_EXEC_SUCCESS) {
                spdlog::error("Error in JavaScript code: {}", duk_safe_to_string(ctx, -1));
                duk_destroy_heap(ctx);
                return {};
            }

            // 调用函数
            duk_get_global_string(ctx, "d");
            std::string input = preprocess(text);
            duk_push_string(ctx, input.c_str());
            if (duk_pcall(ctx, 1) != DUK_EXEC_SUCCESS) {
                spdlog::error("Error: {}", duk_safe_to_string(ctx, -1));
                duk_destroy_heap(ctx);
                return {};
            }

            // 检查返回值是否为数组
            if (!duk_is_array(ctx, -1)) {
                spdlog::error("Return value is not an array!");
                duk_destroy_heap(ctx);
                return {};
            }

            // 获取数组长度
            auto len = duk_get_length(ctx, -1);
            if (len <= 0) {
                return {};
            }
            std::vector<std::string> result;
            result.reserve(len);
            // 遍历数组元素
            for (duk_uarridx_t i = 0; i < len; i++) {
                duk_get_prop_index(ctx, -1, i);
//            std::cout << "Element(" << duk_get_type(ctx, -1) <<")" << i << ": ";
//            if (duk_is_number(ctx, -1)) {
//                std::cout << "number: " << duk_get_number(ctx, -1);
//            } else if (duk_is_string(ctx, -1)) {
//                std::cout << "string: " << duk_get_string(ctx, -1);
//            } else if (duk_is_boolean(ctx, -1)) {
//                std::cout << "boolean: " << duk_get_boolean(ctx, -1);
//            } else if (duk_is_object(ctx, -1)) {
//                const char *date_str = duk_safe_to_string(ctx, -1);
//                std::cout << "object: " << date_str << std::endl;
//            } else {
//                std::cout << "[unknown type]";
//            }
//            std::cout << std::endl;
                auto v = duk_safe_to_string(ctx, -1);
                result.emplace_back(v, 0, 10);
                duk_pop(ctx);  // 弹出当前元素
            }
            duk_destroy_heap(ctx);
            return result;
        }
    } // namespace js_sina

    inline auto global_calendar_once = RollingOnce::create("exchange-calendar", cron_expr_daily_9am);
    inline std::vector<std::string> global_calendars_string={};
    inline std::vector<timestamp> global_calendars_timestamp={};

    static const char * const urlSinaRealstockCompanyKlcTdSh = "https://finance.sina.com.cn/realstock/company/klc_td_sh.txt";
    //static const char * const urlSinaRealstockCompanyKlcTdSz = "https://finance.sina.com.cn/realstock/company/klc_td_sz.txt";
    static const char * const calendarMissingDate            = "1992-05-04"; // TODO:已知缺失的交易日期, 现在已经能自动甄别缺失的交易日期

    /// 同步交易日历
    void update_calendar() {
        auto cache_filename = config::get_calendar_filename();;
        auto modified = io::last_modified_time(cache_filename);
        auto [text, tm] = io::request(urlSinaRealstockCompanyKlcTdSh, modified);
        if(!text.empty()) {
            auto list = js_sina::decode(text);
            auto it = std::lower_bound(list.begin(), list.end(), calendarMissingDate);
            if (it != list.end() || *it != calendarMissingDate) {
                list.insert(it, calendarMissingDate);
            }
            {
                auto ec = util::check_filepath(cache_filename, true);
                ec.clear();
                io::CSVWriter writer(cache_filename);
                writer.write_row("date", "source");
                for (auto const &v: list) {
                    writer.write_row(v, "sina");
                }
            }
            io::last_modified_time(cache_filename, tm);
        }
    }

    // 交易日历
    void lazy_load_calendar() {
        spdlog::info("初始化交易日历...");
        update_calendar();
        auto cache_filename = config::get_calendar_filename();
        auto ec = util::check_filepath(cache_filename, true);
        ec.clear();
        io::CSVReader<1> in(cache_filename);
        in.read_header(io::ignore_extra_column, "date");
        std::string date;
        while (in.read_row(date)) {
            global_calendars_string.push_back(date);
            timestamp ts = date;
            ts = ts.pre_market_time();
            global_calendars_timestamp.emplace_back(ts);
        }
        spdlog::info("初始化交易日历...OK");
    }

    // 这里简单的封装一层, 以后扩展动态更新加载
    std::vector<std::string> get_calendar_list() {
        global_calendar_once->Do(lazy_load_calendar);
        if (global_calendars_string.empty()) {
            throw std::runtime_error("exchange calendar is empty");
        }
        return global_calendars_string;
    }

    // 获取最近一个交易日
    //[[deprecated("获取最后一个交易日的函数, 自0.1.0版本起废弃. 使用 last_trade_day() 代替")]]
    std::string get_last_trading_day(const std::string &date, const std::string & debug_timestamp) {
        spdlog::debug("[exchange::calendar] date={}, debug_timestamp={}",date,debug_timestamp);
        auto tradeDates = get_calendar_list();
        auto it = std::upper_bound(tradeDates.begin(), tradeDates.end(), date);
        if (it != tradeDates.begin()) {
            --it;
        }
        // 判断是否盘前
        std::string last_timestamp = to_timestamp(*it);
        std::string current_timestamp = debug_timestamp.empty() || debug_timestamp=="1970-01-01"? api::get_timestamp() : debug_timestamp;
        if (current_timestamp < last_timestamp && it != tradeDates.begin()) {
            --it;
        }
        return *it;
    }

    timestamp last_trading_day(const timestamp& date, const timestamp & debug_timestamp) {
        global_calendar_once->Do(lazy_load_calendar);
        spdlog::debug("[exchange::calendar] date={}, debug_timestamp={}", date.toString(), debug_timestamp.toString());
        const std::vector<timestamp> &trade_dates = global_calendars_timestamp;
        auto it = std::upper_bound(trade_dates.begin(), trade_dates.end(), date);
        if (it != trade_dates.begin()) {
            --it;
        }
        // 判断是否盘前
        const timestamp& last_timestamp = *it;
        const timestamp& current_timestamp = debug_timestamp.empty() ? timestamp::now() : debug_timestamp;
        if (current_timestamp < last_timestamp && it != trade_dates.begin()) {
            --it;
        }
        auto ts = *it;
        spdlog::debug("[exchange::calendar] last_trading_day={}",ts.toString());
        return ts;
    }

    // 获取上一个交易日
    timestamp prev_trading_day(const timestamp& date, const timestamp& debug_timestamp) {
        global_calendar_once->Do(lazy_load_calendar);
        spdlog::debug("[exchange::calendar] date={}, debug_timestamp={}", date.toString(), debug_timestamp.toString());
        const std::vector<timestamp> &trade_dates = global_calendars_timestamp;
        auto it = std::lower_bound(trade_dates.begin(), trade_dates.end(), date);
        if (it != trade_dates.begin()) {
            --it;
        }
        // 判断是否盘前
        const timestamp& last_timestamp = *it;
        const timestamp& current_timestamp = debug_timestamp.empty() ? timestamp::now() : debug_timestamp;
        if (current_timestamp < last_timestamp && it != trade_dates.begin()) {
            --it;
        }
        auto ts = *it;
        spdlog::debug("[exchange::calendar] prev_trading_day={}",ts.toString());
        return ts;
    }

    // 获取下一个交易日
    timestamp next_trading_day(const timestamp& date, const timestamp& debug_timestamp) {
        global_calendar_once->Do(lazy_load_calendar);
        spdlog::debug("[exchange::calendar] date={}, debug_timestamp={}", date.toString(), debug_timestamp.toString());

        const auto& trade_dates = global_calendars_timestamp;
        // 获取当前时间（用于判断是否“已过今日”）
        const timestamp current_time = debug_timestamp.empty() ? timestamp::now() : debug_timestamp;

        // 找到第一个大于 date.pre_market_time() 的交易日
        auto it = std::lower_bound(trade_dates.begin(), trade_dates.end(), date);

        // 如果没有比 date 更大的交易日，就返回最后一个交易日
        if (it == trade_dates.end()) {
            if (!trade_dates.empty()) {
                spdlog::debug("[exchange::calendar] 已达交易日历尾部，返回最后一个交易日");
                return trade_dates.back();
            } else {
                // 没有交易日数据，返回空
                return timestamp{};
            }
        }

        const timestamp& candidate_day = *it;
        spdlog::debug("[exchange::calendar] candidate_day={}, current_time={}", candidate_day.toString(), current_time.toString());
        // 如果当前时间已经过了候选交易日的盘前时间，则取下一个
        if (current_time >= candidate_day && it != trade_dates.end()) {
            ++it;
            if (it == trade_dates.end()) {
                spdlog::debug("[exchange::calendar] 已达交易日历尾部，返回最后一个交易日");
                return trade_dates.back();
            }
            return *it;
        }

        // 否则返回当前找到的交易日
        spdlog::debug("[exchange::calendar] next_trading_day={}",candidate_day.toString());
        return candidate_day;
    }

    // 获取日期范围
    std::vector<std::string> get_date_range(const std::string &begin, const std::string &end, bool skipToday) {
        if (begin > end) {
            return {}; // 起始日期不能大于结束日期
        }

        auto tradeDates = get_calendar_list();
        if (tradeDates.empty()) {
            return {}; // 交易日历为空
        }

        // 查找起始索引
        auto itStart = std::lower_bound(tradeDates.begin(), tradeDates.end(), begin);
        auto is = static_cast<int>(std::distance(tradeDates.begin(), itStart));

        // 查找结束索引
        auto itEnd = std::lower_bound(tradeDates.begin(), tradeDates.end(), end);
        auto ie = static_cast<int>(std::distance(tradeDates.begin(), itEnd));

        // 调整结束索引
        if (skipToday) {
            if (static_cast<size_t>(ie) < tradeDates.size()) { // 确保索引有效
                std::string today_ = current_day;
                const std::string& lastDay = tradeDates[ie];
                if (lastDay > today_ || lastDay > end) {
                    --ie; // 如果最后一天大于今天或结束日期，则向前调整
                }
            }
        } else {
            // 确保ie在有效范围内并调整到<= end的最大日期
            while (ie >= 0 && static_cast<size_t>(ie) < tradeDates.size() && tradeDates[ie] > end) {
                --ie; // 向前调整索引
            }
        }

        // 检查索引有效性
        if (is < 0 || ie < 0 || is > ie || static_cast<size_t>(ie) >= tradeDates.size()) {
            return {}; // 索引无效时返回空结果
        }

        // 返回日期范围
        return {tradeDates.begin() + is, tradeDates.begin() + ie + 1};
    }

    // 获取日期范围
    std::vector<timestamp> date_range(const timestamp &begin, const timestamp &end, bool skipToday) {
        // 1. 检查无效输入
        if (begin > end) {
            return {};
        }

        // 2. 确保日历数据已加载
        global_calendar_once->Do(lazy_load_calendar);
        const std::vector<timestamp> &trade_dates = global_calendars_timestamp;
        if (trade_dates.empty()) {
            return {};
        }

        // 3. 使用更清晰的变量名
        const auto first = trade_dates.begin();
        const auto last = trade_dates.end();

        // 4. 查找范围边界
        auto lower = std::lower_bound(first, last, begin);
        auto upper = std::upper_bound(first, last, end);

        // 5. 处理skipToday逻辑
        if (skipToday && upper != last) {
            const timestamp today = ts_today_init;
            if (*upper > today || *upper > end) {
                --upper;
            }
        } else {
            // 调整upper到最后一个<=end的日期
            while (upper != first && *(upper-1) > end) {
                --upper;
            }
        }

        // 6. 检查有效范围
        if (lower >= upper || lower == last || upper == first) {
            return {};
        }

        // 7. 返回结果
        return {lower, upper};
    }

    /// 获取当前时间戳
    std::string get_current_timestamp() {
        return api::get_timestamp();
    }
}