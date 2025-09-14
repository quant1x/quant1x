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

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <ctime>

class Decoder {
private:
    static constexpr int64_t l = 86400000;     // ms per day
    static constexpr int64_t u_base = 7657;
    static constexpr int32_t d_mask = ~(3 << 30);
    static constexpr int64_t f_val = 1LL << 30;

    inline static const std::string base64_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::vector<int64_t> h;
    std::vector<int64_t> i_data;
    int64_t e = 0, o = 0, n = 0;

    struct State {
        int64_t d = 0, p = 0, ld = 0, c = 0;
        int64_t cd = 0;
        int m = 0;

        // For k()
        int64_t lk = 0;

        // For S(), _, T(), etc.
        int la = 0, lp = 0, lv = 0, tv = 0, rv = 0, zv = 0, pp = 0;
        int64_t pc = 0, cp = 0, da = 0, sa = 0, sv = 0;
        int md = 0, mv = 0;
        std::vector<int64_t> cv = {0, 0};
        int f = 0;
        std::vector<int> dv, dl;
    } r;

    int64_t s = 0;
    int64_t u_header = 0;

public:
    struct DailyClose {
        std::string day;
        double close;
        bool has_prevclose = false;
        double prevclose = 0.0;
    };

    struct DateOnly {
        std::string day;
    };

    struct Trade {
        std::string date;
        double price, avg_price;
        int64_t volume;
        double prevclose = 0.0;
    };

    struct Candle {
        std::string day;
        double open, high, low, close;
        int64_t volume;
        double prevclose = 0.0;
    };

    struct MultiField {
        std::vector<int> values;
    };

private:
    void initialize(const std::string& t) {
        h.assign(64, 0);
        for (int i = 0; i < 64; ++i)
            h[i] = 1LL << i;

        i_data.clear();
        for (char ch : t) {
            size_t pos = base64_alphabet.find(ch);
            i_data.push_back(pos == std::string::npos ? -1 : static_cast<int>(pos));
        }
        n = static_cast<int>(i_data.size());
        e = 0; o = 0;
    }

    bool y() {
        if (e >= n) return false;
        bool bit = (i_data[e] >> o) & 1;
        if (++o >= 6) { o = 0; ++e; }
        return bit;
    }

    int64_t N() {
        int64_t t = y() ? 1 : 0, e_count = 1;
        while (y()) ++e_count;
        return e_count * (2 * t - 1);
    }

    std::vector<int64_t> w(const std::vector<int64_t>& t,
                           const std::vector<int64_t>& r_flag = {},
                           const std::vector<int64_t>& a_flag = {}) {
        std::vector<int64_t> res;
        std::vector<int64_t> r_use = r_flag.empty() ? std::vector<int64_t>(t.size(), 0) : r_flag;
        std::vector<int64_t> a_use = a_flag.empty() ? std::vector<int64_t>(t.size(), 0) : a_flag;

        for (size_t i = 0; i < t.size(); ++i) {
            int64_t len = t[i];
            if (len == 0) { res.push_back(0); continue; }
            if (e >= n) break;

            int64_t val = 0;
            int64_t bits = len;
            while (bits > 0 && e < n) {
                int64_t take = std::min(bits, 6 - o);
                uint8_t mask = (1 << take) - 1;
                val |= ((i_data[e] >> o) & mask) << (len - bits);
                o += take; bits -= take;
                if (o >= 6) { o = 0; ++e; }
            }

            if (len <= 30 && r_use[i] && val >= h[len-1])
                val -= h[len];
            else if (len > 30) {
                auto sub = w({30, len-30}, {0, r_use[i]});
                if (!a_use[i]) val = sub[0] + sub[1] * h[30];
                else { res.push_back(sub[0]); res.push_back(sub[1]); continue; }
            }
            res.push_back(val);
        }
        return res;
    }

    std::string x(int64_t offset) {
        int64_t total_days = u_base + r.d + offset;
        time_t sec = (total_days * l) / 1000;
        tm ptm = q1x::safe::gmtime(sec);
        std::ostringstream oss;
        oss << (1900 + ptm.tm_year) << '-'
            << std::setfill('0') << std::setw(2) << (ptm.tm_mon + 1) << '-'
            << std::setfill('0') << std::setw(2) << ptm.tm_mday;
        return oss.str();
    }

    std::vector<std::string> k() {
        if (s > 1) return {};
        r.lk = 0;
        int64_t count = -1;
        r.d = w({18})[0] - 1;
        int64_t end_day = w({18})[0];

        std::vector<std::string> res;
        std::string first = x(1);

        while (r.d < end_day) {
            std::string today = x(1);
            if (count <= 0) {
                if (y()) r.lk += N();
                auto tmp = w({3 * r.lk}, {0});
                count = tmp[0] + 1;
                if (res.empty()) {
                    res.push_back(first);
                    count--;
                } else {
                    res.push_back(today);
                }
            } else {
                res.push_back(today);
            }
            count--;
            r.d++;
        }
        if (res.empty()) res.push_back(first);
        return res;
    }

    std::vector<DailyClose> S() {
        if (s >= 1) return {};
        r.d = w({18}, {1})[0] - 1;
        auto a = w({3,3,30,6});
        r.p = a[0]; r.ld = a[1]; r.cd = a[2]; r.c = a[3];
        r.m = static_cast<int>(std::pow(10, r.p));
        double init_pc = r.cd / static_cast<double>(r.m);

        std::vector<DailyClose> res;
        int idx = 0;
        while (true) {
            if (!y()) break;
            int64_t cmd = w({3})[0], off = 1;
            if (cmd == 0) off = w({6})[0];
            else if (cmd == 1) { r.d = w({18})[0]; off = 0; }
            else off = cmd;

            std::string day = x(off);
            if (y()) r.ld += N();
            auto dc = w({3 * r.ld}, {1})[0];
            r.cd += dc;
            double close = r.cd / static_cast<double>(r.m);

            DailyClose bar{day, close, false, 0.0};
            if (idx == 0) {
                bar.has_prevclose = true;
                bar.prevclose = init_pc;
            }
            res.push_back(bar);

            if (e >= n) break;
            if (e == n - 1 && ((r.c ^ (idx + 1)) & 63) == 0) break;
            idx++;
        }
        return res;
    }

    // 简化其他函数占位（避免干扰）
    template<typename T> std::vector<T> placeholder() { return {}; }

public:
    template<typename T>
    std::vector<T> decode(const std::string& input) {
        initialize(input);
        auto u = w({12,6});
        u_header = u[0];
        s = 63 ^ u[1];

        // 输出分支信息
        std::string name;
        switch (u_header) {
            case 1479: name = "T"; break;
            case 136:  name = "_"; break;
            case 200:  name = "S"; break;
            case 139:  name = "k"; break;
            case 197:  name = "_mi_run"; break;
            default:   name = "unknown";
        }
        std::cout << "u[0]=" << u_header << " 分支: " << name << "\n";

        if constexpr (std::is_same_v<T, std::string>)
            return u_header == 139 ? k() : std::vector<std::string>();
        else if constexpr (std::is_same_v<T, DailyClose>)
            return u_header == 200 ? S() : std::vector<DailyClose>();
        else
            return {}; // 其他类型暂不展开
    }
};

// 测试主函数
TEST_CASE("c1", "[calendar]") {
    std::string encoded_data = "LC/AAAf8CXCw6mHbaPgkryxXv10eAJP1LW0SD39aT7+NV44Xba3PxCgTdrFc3FepphjnTBw1X4hmGu+ypVAcvFenpBXPqCc6F4ZmGueLFwbIN8QTDXPsCc1FepphjvOoCc8FepphjvcgFO3CP00wxXXWhrkUdZrIJpw9X3ThrlEp6hlGc88Kcem0VeFpZM46VV4MrTC2KScKc811U4aLXUdlzINc9lTrwFW3T52KPj0mDueVFuUR1RtiEoCXfdgFOOSGRXnUhrXWhb0kt6Rk2pU44JV4SrTyU9wSDHPwCnXdP1FuiUM44r7qwdKqcYrIZpw1DqgrlU5IrHRawxjrwBaqcbrIt9gr3UhDtOpyVNjEnCHPnC3royNWvi0gj/";

    Decoder dec;

    // Try decoding as SimpleDate (likely S function)
    auto dates = dec.decode<std::string>(encoded_data);
    size_t length = dates.size();
    std::cout << "交易日总数: " << length << std::endl;
    std::cout << "--- Decoded Dates ---" << std::endl;
    for (size_t i = 0; i < length; i++) {
        std::cout << i << ", " << dates[i] << "\n";
    }
}
