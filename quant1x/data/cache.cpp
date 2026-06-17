#include "cache.h"
#include <quant1x/std/filesystem.h>
#include <algorithm>
#include <cctype>

namespace data {

    meta::Timestamp get_today_initialized_time() {
        auto now = meta::Timestamp::now();
        return now.pre_market_time();
    }

    meta::Timestamp get_filename_modified_time(const std::string &fname) {
        try {
            int64_t ms = filesystem::last_modified_time(fname);
            if (ms <= 0) {
                return meta::Timestamp::zero();
            }
            return meta::Timestamp(ms);
        } catch (...) {
            // 可能因权限, 竞争条件(文件被删除)等导致 stat 失败
            return meta::Timestamp::zero();
        }
    }

    std::string get_period_name(const std::string &period) {
        if (period.empty()) {
            return period;
        }
        std::string upper = period;
        std::transform(upper.begin(), upper.end(), upper.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        if (upper == "W") return "周";
        if (upper == "M") return "月";
        if (upper == "Q") return "季";
        if (upper == "Y") return "年";
        if (upper == "D") return "日";
        return upper;
    }

    std::string date_format(const std::string &date, const std::string &layout) {
        // 尝试多种常见日期格式解析
        static const char *formats[] = {
            "%Y-%m-%d",
            "%Y/%m/%d",
            "%Y.%m.%d",
            "%Y%m%d",
            "%B %d, %Y",
            "%b %d, %Y",
        };

        for (const char *fmt : formats) {
            std::tm tm = {};
            std::istringstream ss(date);
            ss >> std::get_time(&tm, fmt);
            if (!ss.fail()) {
                char buf[64] = {};
                std::strftime(buf, sizeof(buf), layout.c_str(), &tm);
                return std::string(buf);
            }
        }

        // 解析失败, 返回原字符串
        return date;
    }

}  // namespace data
