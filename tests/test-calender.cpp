#include <quant1x/test/test.h>
#include <quant1x/exchange.h>

TEST_CASE("update-calendar", "[calendar]") {
    runtime::global_init();
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("calendar-1");
    exchange::update_calendar();
    spdlog::debug("calendar-2");
//    auto list = util::js_decode(text);
//    for(const auto & v: list) {
//        std::cout << v << std::endl;
//    }
}

#include <cmath>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

class CalendarDecoder {
private:
    // 添加成员变量保存分支值
    int branch_type = 0;

    std::string                encoded_data;
    std::vector<int>           indices;
    std::string                base64_chars;
    int                        e = 0, o = 0, n = 0;
    std::map<std::string, int> r;
    std::vector<long long>     h;
    int                        s      = 0;
    int                        u_val  = 7657;
    long long                  l_val  = 86400000;
    long long                  d_mask = ~(3LL << 30);
    long long                  f_mask = 1LL << 30;
    std::vector<int>           p      = {0, 3, 5, 6, 9, 10, 12, 15, 17, 18, 20, 23, 24, 27, 29, 30};

    void init_base64() {
        base64_chars.clear();
        // A-Z (65-90)
        for (int i = 0; i < 26; i++) {
            base64_chars += char(i + 65);
        }
        // a-z (97-122)
        for (int i = 0; i < 26; i++) {
            base64_chars += char(i + 97);
        }
        // 0-9 (48-57)
        for (int i = 0; i < 10; i++) {
            base64_chars += char(i + 48);
        }
        base64_chars += "+/";
    }

    void init_powers() {
        h.resize(64);
        for (int i = 0; i < 64; i++) {
            h[i] = 1LL << i;
        }
    }

    void decode_base64() {
        indices.clear();
        for (char c : encoded_data) {
            size_t pos = base64_chars.find(c);
            indices.push_back(pos != std::string::npos ? static_cast<int>(pos) : 0);
        }
        n = static_cast<int>(indices.size());
    }

    bool y() {
        if (e >= n)
            return false;
        bool t = (indices[e] & (1 << o)) != 0;
        o++;
        if (o >= 6) {
            o -= 6;
            e++;
        }
        return t;
    }

    int N() {
        bool t = y();
        int  e = 1;
        while (y()) {
            e++;
        }
        return e * (2 * t - 1);
    }

    std::vector<long long>
    w(const std::vector<int> &t, const std::vector<int> &r_param = {}, const std::vector<int> &a_param = {}) {
        std::vector<long long> l;
        std::vector<int>       r_local = r_param;
        std::vector<int>       a_local = a_param;

        if (r_local.empty())
            r_local.resize(t.size(), 0);
        if (a_local.empty())
            a_local.resize(t.size(), 0);

        for (size_t i = 0; i < t.size(); i++) {
            long long u = 0;
            int       c = t[i];

            if (c) {
                if (e >= n) {
                    l.resize(t.size(), 0);
                    return l;
                }

                if (c <= 0) {
                    u = 0;
                } else if (c <= 30) {
                    // 在春节期间添加详细调试
                    bool is_spring_debug = (c == 21);
                    if (is_spring_debug) {
                        std::cout << ">>> w() 详细调试 c=21 <<<" << std::endl;
                        std::cout << "开始前: e=" << e << ", o=" << o << std::endl;
                        std::cout << "r_local[" << i << "]=" << (i < r_local.size() ? r_local[i] : -1) << std::endl;
                    }
                    
                    while (c > 0) {
                        int d = 6 - o;
                        d     = (c > d) ? d : c;
                        long long bits = ((long long)(indices[e] >> o) & ((1 << d) - 1));
                        long long shift_amount = t[i] - c;
                        u |= bits << shift_amount;
                        
                        if (is_spring_debug) {
                            std::cout << "  循环: c=" << c << ", d=" << d 
                                      << ", indices[" << e << "]=" << indices[e]
                                      << ", bits=" << bits 
                                      << ", shift=" << shift_amount
                                      << ", u=" << u << std::endl;
                        }
                        
                        o += d;
                        if (o >= 6) {
                            o -= 6;
                            e++;
                        }
                        c -= d;
                    }
                    
                    if (i < r_local.size() && r_local[i] && u >= h[t[i] - 1]) {
                        long long old_u = u;
                        u -= h[t[i]];
                        if (is_spring_debug) {
                            std::cout << "  应用符号位修正: " << old_u << " - " << h[t[i]] << " = " << u << std::endl;
                        }
                    }
                    
                    if (is_spring_debug) {
                        std::cout << "最终结果: u=" << u << std::endl;
                        std::cout << ">>> w() 详细调试结束 <<<" << std::endl;
                    }
                } else {
                    std::vector<int> sub_t      = {30, c - 30};
                    std::vector<int> sub_r      = {0, (i < r_local.size()) ? r_local[i] : 0};
                    auto             sub_result = w(sub_t, sub_r);
                    if (i < a_local.size() && !a_local[i]) {
                        u = sub_result[0] + sub_result[1] * h[30];
                    } else {
                        u = sub_result[0];
                    }
                }
            } else {
                u = 0;
            }
            l.push_back(u);
        }

        std::cout << "w(";
        for (size_t i = 0; i < t.size(); i++) {
            if (i > 0)
                std::cout << ",";
            std::cout << t[i];
        }
        std::cout << ") => ";
        for (size_t i = 0; i < l.size(); i++) {
            if (i > 0)
                std::cout << ",";
            std::cout << l[i];
        }
        std::cout << std::endl;

        return l;
    }

    std::string x(int t) {
        if (t > 1) {
            // e = 0; // This line seems unused in original
        }
        for (int i = 0; i < t; i++) {
            r["d"]++;
            int n = r["d"] % 7;
            if (n == 3 || n == 4) {
                r["d"] += 5 - n;
            }
        }

        long long   timestamp = (u_val + r["d"]) * l_val;
        std::time_t time      = timestamp / 1000;
        std::tm tm_info;
#if defined(_WIN32) || defined(_WIN64)
        gmtime_s(&tm_info, &time);
#else
        gmtime_r(&time, &tm_info);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm_info, "%Y-%m-%d");
        return oss.str();
    }

    std::vector<std::map<std::string, std::string>> S() {
        std::vector<std::map<std::string, std::string>> result;
        if (s >= 1)
            return result;

        auto init_data = w({18}, {1});
        r["d"]         = static_cast<int>(init_data[0] - 1);

        auto a  = w({3, 3, 30, 6});
        r["p"]  = static_cast<int>(a[0]);
        r["ld"] = static_cast<int>(a[1]);
        r["cd"] = static_cast<int>(a[2]);
        r["c"]  = static_cast<int>(a[3]);
        r["m"]  = static_cast<int>(std::pow(10, r["p"]));
        r["pc"] = r["cd"] / r["m"];

        std::cout << "初始参数: d=" << r["d"] << " p=" << r["p"] << " ld=" << r["ld"] << " cd=" << r["cd"]
                  << " c=" << r["c"] << std::endl;

        int t = 0;
        while (true) {
            std::map<std::string, int> day_data;
            day_data["d"] = 1;

            if (y()) {
                auto a_val = w({3});
                if (a_val[0] == 0) {
                    day_data["d"] = static_cast<int>(w({6})[0]);
                } else if (a_val[0] == 1) {
                    r["d"]        = static_cast<int>(w({18})[0]);
                    day_data["d"] = 0;
                } else {
                    day_data["d"] = static_cast<int>(a_val[0]);
                }
            }

            std::map<std::string, std::string> l;
            l["day"] = x(day_data["d"]);

            if ( y() ) {
                r["ld"] += N();
            }

            auto a_close = w({3 * r["ld"]}, {1});
            r["cd"] += static_cast<int>(a_close[0]);
            l["close"] = std::to_string((double)r["cd"] / r["m"]);

            result.push_back(l);

            if ( e >= n || ( e == n - 1 && ( 63 & ( r["c"] ^ ( t + 1 ) ) ) == 0 ) ) {
                break;
            }
            t++;
        }

        if ( !result.empty() ) {
            result[0]["prevclose"] = std::to_string((double)r["pc"]);
        }

        // 打印解码出来的日期
        for ( size_t i = 0; i < result.size(); i++ ) {
            std::cout << "date[" << i << "]: " << result[i]["day"] << std::endl;
        }

        return result;
    }

    std::vector<std::map<std::string, std::string>> _() {
        // 分时数据解码逻辑
        std::vector<std::map<std::string, std::string>> result;
        if (s > 2)
            return result;

        // 实现分时数据解码逻辑（简化版本）
        return result;
    }

    std::vector<std::map<std::string, std::string>> T() {
        // K线数据解码逻辑
        std::vector<std::map<std::string, std::string>> result;
        if (s >= 1)
            return result;

        // 实现K线数据解码逻辑（简化版本）
        return result;
    }

    std::vector<std::string> k() {
        std::vector<std::string> result;
        if (s > 1) return result;

        r["l"] = 0;
        int n_count = -1;
        bool t_initialized = false;  // 🔧 关键：跟踪result是否已初始化
        
        r["d"] = static_cast<int>(w({18})[0] - 1);
        int target_date = static_cast<int>(w({18})[0]);
        
        while (r["d"] < target_date) {
            std::string current_date = x(1);
            
            if (n_count <= 0) {
                if (y()) {
                    r["l"] += N();
                }
                
                auto count_data = w({3 * r["l"]}, {0});
                n_count = static_cast<int>(count_data[0]) + 1;
                
                // 🔧 关键修正：只有第一次才初始化
                if (!t_initialized) {
                    result.push_back(current_date);
                    n_count--;
                    t_initialized = true;
                }
                // 后续所有n_count <= 0的情况都跳过
            } else {
                result.push_back(current_date);
            }
            
            n_count--;
        }

        return result;
    }

    std::vector<std::vector<long long>> _mi_run() {
        // 自定义数据解码逻辑
        std::vector<std::vector<long long>> result;
        if (s >= 1)
            return result;

        // 实现自定义数据解码逻辑（简化版本）
        return result;
    }

public:
    CalendarDecoder(const std::string &data) : encoded_data(data) {
        init_base64();
        init_powers();
        decode_base64();

        r.clear();
        e = o = 0;

        // Use d_mask and f_mask to avoid -Wunused-private-field being treated as an error
        (void)d_mask;
        (void)f_mask;

        auto u = w({12, 6});
        s      = 63 ^ static_cast<int>(u[1]);
        branch_type = static_cast<int>(u[0]);  // 保存分支值

        std::cout << "u[0]=" << u[0] << " 分支:";
        std::map<std::string, std::string> branches = {
            {"_1479", "T"}, {"_136", "_"}, {"_200", "S"}, {"_139", "k"}, {"_197", "_mi_run"}};
        std::string key = "_" + std::to_string(u[0]);
        if (branches.find(key) != branches.end()) {
            std::cout << branches[key] << std::endl;
        } else {
            std::cout << "unknown" << std::endl;
        }
    }

    std::vector<std::map<std::string, std::string>> decode() {
        // 直接使用保存的分支值，不再重复调用 w({12, 6})
        int branch = branch_type;

        switch (branch) {
            case 1479:
                return T();
            case 136:
                return _();
            case 200:
                return S();
            case 139: {
                auto                                            dates = k();
                std::vector<std::map<std::string, std::string>> result;
                for ( const auto &date : dates ) {
                    std::map<std::string, std::string> item;
                    item["date"] = date;
                    result.push_back(item);
                }
                return result;
            }
            case 197: {
                auto                                            data = _mi_run();
                std::vector<std::map<std::string, std::string>> result;
                for ( size_t i = 0; i < data.size(); i++ ) {
                    std::map<std::string, std::string> item;
                    item["index"] = std::to_string(i);
                    result.push_back(item);
                }
                return result;
            }
            default:
                return {};
        }
    }
};


TEST_CASE("calendar-without-vm", "[calendar]") {
    std::string encoded_data =
        "LC/AAAf8CXCw6mHbaPgkryxXv10eAJP1LW0SD39aT7+NV44Xba3PxCgTdrp5BkYVAc11hWvg0c/"
        "19UAc7jNtHQyWBAu2xmGuZI1NVAc3FepphjnTBw1X4hmGu+"
        "ypVAcvFenpBXPqCc6F4ZmGueLFwbIN8QTDXPsCc1FepphjvOoCc8FepphjvcgFO3CP00wxXXWhrkUdZrIJpw9X3ThrlEp6hlGc88Kcem0VeFpZ"
        "M46VV4MrTC2KScKc811U4aLXUdlzINc9lTrwFW3T52KPj0mDueVFuUR1RtiEoCXfdgFOOSGRXnUhrXWhb0kt6Rk2pU44JV4SrTyU9wSDHPwCnX"
        "dP1FuiUM44r7qwdKqcYrIZpw1DqgrlU5IrHRawxjrwBaqcbrIt9gr3UhDtOpyVNjEnCHPnC3royNWvi0gj/";
    CalendarDecoder decoder(encoded_data);
    auto            dates = decoder.decode();

    std::cout << "解码结果:" << std::endl;
    for (const auto &item : dates) {
        for (const auto &pair : item) {
            std::cout << pair.first << ": " << pair.second << " ";
        }
        std::cout << std::endl;
    }
}



#include <duktape.h>
//============================================================
// JS解码                                                    //
//============================================================
namespace js_sina {

    const char *const javascript_decoder = R"(function d(t) {
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
        //std::string input = preprocess(text);
        duk_push_string(ctx, text.c_str());
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
}

TEST_CASE("calendar-decode", "[calendar]") {
    std::string encoded_data =
        "LC/AAAf8CXCw6mHbaPgkryxXv10eAJP1LW0SD39aT7+NV44Xba3PxCgTdrp5BkYVAc11hWvg0c/"
        "19UAc7jNtHQyWBAu2xmGuZI1NVAc3FepphjnTBw1X4hmGu+"
        "ypVAcvFenpBXPqCc6F4ZmGueLFwbIN8QTDXPsCc1FepphjvOoCc8FepphjvcgFO3CP00wxXXWhrkUdZrIJpw9X3ThrlEp6hlGc88Kcem0VeFpZ"
        "M46VV4MrTC2KScKc811U4aLXUdlzINc9lTrwFW3T52KPj0mDueVFuUR1RtiEoCXfdgFOOSGRXnUhrXWhb0kt6Rk2pU44JV4SrTyU9wSDHPwCnX"
        "dP1FuiUM44r7qwdKqcYrIZpw1DqgrlU5IrHRawxjrwBaqcbrIt9gr3UhDtOpyVNjEnCHPnC3royNWvi0gj/";

    auto dates = js_sina::decode(encoded_data);
    std::cout << "解码结果:" << std::endl;
    for (const auto &item : dates) {
        std::cout << item << std::endl;
    }
}
