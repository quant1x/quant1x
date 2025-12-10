#pragma once
#ifndef QUANT1X_CONFIG_DETAIL_TRADING_SESSION_H
#define QUANT1X_CONFIG_DETAIL_TRADING_SESSION_H 1

#include <quant1x/std/strings.h>
#include <quant1x/std/safe.h>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace config {

    namespace detail {
        constexpr const char *const TransactionStartTime = "09:30:00"; // 交易开始时间
        // 值范围正则表达式
        inline std::regex valueRangePattern("[~]\\s*");
        // 数组正则表达式
        inline std::regex arrayPattern("[,]\\s*");
    }

    // 错误定义
    class TimeFormatError : public std::runtime_error {
    public:
        TimeFormatError() : std::runtime_error("时间格式错误") {}
    };

    // TimeRange 时间范围
    struct TimeRange {
        std::string begin = "09:30:00"; // 默认开始时间
        std::string end = "15:00:00";   // 默认结束时间

        // 获取当前交易时间戳
        static std::string getTradingTimestamp() {
            auto now = std::chrono::system_clock::now();
            std::time_t now_time = std::chrono::system_clock::to_time_t(now);
            std::tm tm = safe::localtime(now_time);

            char buffer[9];
            std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
            return std::string(buffer);
        }

        // 补全时间格式为HH:MM:SS
        static std::string completeTime(const std::string& time) {
            if (time.empty()) throw TimeFormatError();
            std::string tmpTime = time;
            std::vector<std::string> parts;
            size_t pos = 0;
            while ((pos = tmpTime.find(':')) != std::string::npos) {
                parts.push_back(tmpTime.substr(0, pos));
                parts.push_back(":");
                tmpTime = time.substr(pos + 1);
            }
            parts.push_back(tmpTime);

            // 根据已有部分补全
            if (parts.size() == 2) { // HH:MM
                return parts[0] + ":" + parts[1] + ":00";
            } else if (parts.size() == 1) { // HH
                return parts[0] + ":00:00";
            }
            return time; // 已经是完整格式
        }

        TimeRange() = default;

        std::string ToString() const {
            std::ostringstream oss;
            oss << "{begin:" << begin << ", end:" << end << "}";
            return oss.str();
        }

        void Parse(const std::string& text) noexcept {
            try {
                // 使用regex_token_iterator分割字符串
                std::sregex_token_iterator iter(text.begin(), text.end(), detail::valueRangePattern, -1);
                std::sregex_token_iterator end_iter;

                std::vector<std::string> parts;
                while (iter != end_iter) {
                    parts.push_back(*iter);
                    ++iter;
                }

                // 只有成功分割成两部分时才更新
                if (parts.size() >= 2) {  // 改为>=2更安全
                    begin = strings::trim(parts[0]);
                    end = strings::trim(parts[1]);
                }
            } catch (...) {
                // 保持默认值
            }
        }

        // 是否交易时段
        bool IsTrading(const std::string& timestamp = "") const {
            std::string tm = timestamp.empty() ? getTradingTimestamp() : strings::trim(timestamp);
            return tm >= begin && tm <= end;
        }

        // 获取开始时间
        const std::string& Begin() const { return begin; }

        // 获取结束时间
        const std::string& End() const { return end; }

        // 重载小于运算符
        bool operator<(const TimeRange& other) const {
            if (begin < other.begin) return true;
            if (begin > other.begin) return false;
            return end < other.end;
        }
    };

    // TradingSession 交易时段
    struct TradingSession {
        std::vector<TimeRange> sessions;

        // 获取当前交易时间戳
        static std::string getTradingTimestamp() {
            auto        now      = std::chrono::system_clock::now();
            std::time_t now_time = std::chrono::system_clock::to_time_t(now);
            std::tm    tm       = safe::localtime(now_time);

            char buffer[16]{};
            std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
            return std::string(buffer);
        }

        TradingSession() = default;

        std::string ToString() const {
            std::ostringstream oss;
            oss << "[";
            for (size_t i = 0; i < sessions.size(); ++i) {
                if (i != 0)
                    oss << ",";
                oss << sessions[i].ToString();
            }
            oss << "]";
            return oss.str();
        }

        void Parse(const std::string &text) {
            std::string trimmed = strings::trim(text);
            if (trimmed.empty()) {
                throw std::runtime_error("empty time range");
            }

            std::vector<TimeRange>     tempSessions;
            std::sregex_token_iterator iter(trimmed.begin(), trimmed.end(), detail::arrayPattern, -1);
            std::sregex_token_iterator end;

            for (; iter != end; ++iter) {
                std::string part = *iter;
                if (part.empty()) {
                    continue;
                }
                TimeRange tr;
                tr.Parse(part);
                tempSessions.push_back(tr);
            }

            // 排序时间段
            std::sort(tempSessions.begin(), tempSessions.end());

            if (tempSessions.empty()) {
                throw std::runtime_error("no valid time ranges");
            }

            sessions = std::move(tempSessions);
        }

        // 获取时段总数
        size_t Size() const { return sessions.size(); }

        // 判断timestamp是第几个交易时段
        int Index(const std::string &timestamp = "") const {
            std::string tm = timestamp.empty() ? getTradingTimestamp() : strings::trim(timestamp);

            for (size_t i = 0; i < sessions.size(); ++i) {
                if (sessions[i].IsTrading(tm)) {
                    return static_cast<int>(i);
                }
            }
            return -1;
        }

        // 是否交易时段
        bool IsTrading(const std::string &timestamp = "") const { return Index(timestamp) >= 0; }

        // 当前时段是否今天最后一个交易时段
        bool IsTodayLastSession(const std::string &timestamp = "") const {
            int index = Index(timestamp);
            return index >= 0 && (index + 1) >= static_cast<int>(sessions.size());
        }

        // 当前时段是否可以进行止损操作
        bool CanStopLoss(const std::string &timestamp = "") const {
            int n     = static_cast<int>(sessions.size());
            int index = Index(timestamp);

            // 1个时段, 立即止损
            bool c1 = (n == 1);
            // 2个时段, 在第二个时间止损
            bool c2 = (n == 2 && index == 1);
            // 3个以上时段, 在倒数第2个时段止损
            bool c3 = (n >= 3 && (index + 2) == n);

            return c1 || c2 || c3;
        }

        // 当前时段是否可以止盈
        bool CanTakeProfit(const std::string &timestamp = "") const {
            return true;
        }

        // 是否盘前交易时段
        bool IsPreMarket(const std::string &timestamp = "") const {
            std::string tm = timestamp.empty() ? getTradingTimestamp() : strings::trim(timestamp);
            return tm < detail::TransactionStartTime;
        }

        // 为TradingSession添加缺失的GetSessions和SetSessions方法
        const std::vector<TimeRange>& GetSessions() const {
            return sessions;
        }

        void SetSessions(const std::vector<TimeRange>& newSessions) {
            sessions = newSessions;
            std::sort(sessions.begin(), sessions.end());
        }
    };

}  // namespace config

namespace YAML {
    // TimeRange的YAML转换
    template<>
    struct convert<config::TimeRange> {
        static Node encode(const config::TimeRange& range) {
            return Node(range.Begin() + "~" + range.End());
        }

        static bool decode(const Node& node, config::TimeRange& range) {
            try {
                if (node.IsScalar()) {
                    range.Parse(node.as<std::string>());
                } else if (node.IsSequence() && node.size() == 2) {
                    range.Parse(node[0].as<std::string>() + "~" + node[1].as<std::string>());
                } else {
                    return false;
                }
                return true;
            } catch (const config::TimeFormatError&) {
                return false;
            }
        }
    };

    // TradingSession的YAML转换
    template<>
    struct convert<config::TradingSession> {
        static Node encode(const config::TradingSession& session) {
            Node node;
            for (const auto& range : session.GetSessions()) {
                node.push_back(range.Begin() + "~" + range.End());
            }
            return node;
        }

        static bool decode(const Node& node, config::TradingSession& session) {
            try {
                if (node.IsScalar()) {
                    session.Parse(node.as<std::string>());
                } else if (node.IsSequence()) {
                    std::vector<config::TimeRange> ranges;
                    for (const auto& item : node) {
                        config::TimeRange range;
                        if (convert<config::TimeRange>::decode(item, range)) {
                            ranges.push_back(range);
                        }
                    }
                    session.SetSessions(ranges);
                } else {
                    return false;
                }
                return true;
            } catch (const std::exception&) {
                return false;
            }
        }
    };

} // namespace YAML

#endif  // QUANT1X_CONFIG_DETAIL_TRADING_SESSION_H
