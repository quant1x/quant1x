#pragma once
#ifndef INDICATORS_PROGRESS_BAR
#define INDICATORS_PROGRESS_BAR 1

#include <indicators/details/stream_helper.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <indicators/color.hpp>
#include <indicators/setting.hpp>
#include <indicators/terminal_size.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>

namespace indicators {

    class ProgressBar {
        using Settings =
                std::tuple<option::BarWidth, option::PrefixText, option::PostfixText,
                        option::Start, option::End, option::Fill, option::Lead,
                        option::Remainder, option::MaxPostfixTextLen,
                        option::Completed, option::ShowPercentage, option::ShowSpeed,
                        option::ShowElapsedTime, option::ShowRemainingTime,
                        option::SavedStartTime, option::ForegroundColor,
                        option::FontStyles, option::MinProgress, option::MaxProgress,
                        option::ProgressType, option::Stream>;

    public:
        template<typename... Args,
                typename std::enable_if<
                        details::are_settings_from_tuple<
                                Settings, typename std::decay<Args>::type...>::value,
                        void *>::type = nullptr>
        explicit ProgressBar(Args &&... args)
                : settings_(
                details::get<details::ProgressBarOption::bar_width>(
                        option::BarWidth{50}, std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::prefix_text>(
                        option::PrefixText{}, std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::postfix_text>(
                        option::PostfixText{}, std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::start>(
                        option::Start{"["}, std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::end>(
                        option::End{"]"}, std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::fill>(
                        option::Fill{"="}, std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::lead>(
                        option::Lead{">"}, std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::remainder>(
                        option::Remainder{" "}, std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::max_postfix_text_len>(
                        option::MaxPostfixTextLen{0}, std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::completed>(
                        option::Completed{false}, std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::show_percentage>(
                        option::ShowPercentage{true}, std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::show_speed>(
                    option::ShowSpeed{true}, std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::show_elapsed_time>(
                        option::ShowElapsedTime{true}, std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::show_remaining_time>(
                        option::ShowRemainingTime{true}, std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::saved_start_time>(
                        option::SavedStartTime{false}, std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::foreground_color>(
                        option::ForegroundColor{Color::unspecified},
                        std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::font_styles>(
                        option::FontStyles{std::vector<FontStyle>{FontStyle::bold}},
                        std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::min_progress>(
                        option::MinProgress{0}, std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::max_progress>(
                        option::MaxProgress{100}, std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::progress_type>(
                        option::ProgressType{ProgressType::incremental},
                        std::forward<Args>(args)...),
                details::get<details::ProgressBarOption::stream>(
                        option::Stream{std::cout}, std::forward<Args>(args)...)) {

            // if progress is incremental, start from min_progress
            // else start from max_progress
            const auto type = get_value<details::ProgressBarOption::progress_type>();
            if (type == ProgressType::incremental) {
                progress_ = get_value<details::ProgressBarOption::min_progress>();
            } else {
                progress_ = get_value<details::ProgressBarOption::max_progress>();
            }
            save_start_time();
        }

        template<typename T, details::ProgressBarOption id>
        void set_option(details::Setting<T, id> &&setting) {
            static_assert(
                    !std::is_same<T, typename std::decay<decltype(details::get_value<id>(
                            std::declval<Settings>()))>::type>::value,
                    "Setting has wrong type!");
            std::lock_guard<std::mutex> lock(mutex_);
            get_value<id>() = std::move(setting).value;
        }

        template<typename T, details::ProgressBarOption id>
        void set_option(const details::Setting<T, id> &setting) {
            static_assert(
                    !std::is_same<T, typename std::decay<decltype(details::get_value<id>(
                            std::declval<Settings>()))>::type>::value,
                    "Setting has wrong type!");
            std::lock_guard<std::mutex> lock(mutex_);
            get_value<id>() = setting.value;
        }

        void set_option(const details::Setting<std::string, details::ProgressBarOption::postfix_text> &setting) {
            std::lock_guard<std::mutex> lock(mutex_);
            get_value<details::ProgressBarOption::postfix_text>() = setting.value;
            if (setting.value.length() > get_value<details::ProgressBarOption::max_postfix_text_len>()) {
                get_value<details::ProgressBarOption::max_postfix_text_len>() = setting.value.length();
            }
        }

        void set_option(details::Setting<std::string, details::ProgressBarOption::postfix_text> &&setting) {
            std::lock_guard<std::mutex> lock(mutex_);
            get_value<details::ProgressBarOption::postfix_text>() =
                    std::move(setting).value;
            auto &new_value = get_value<details::ProgressBarOption::postfix_text>();
            if (new_value.length() >
                get_value<details::ProgressBarOption::max_postfix_text_len>()) {
                get_value<details::ProgressBarOption::max_postfix_text_len>() =
                        new_value.length();
            }
        }

        void set_progress(size_t new_progress) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                progress_ = new_progress;
            }

            save_start_time();
            print_progress();
        }

        void tick() {
            {
                std::lock_guard<std::mutex> lock{mutex_};
                const auto type = get_value<details::ProgressBarOption::progress_type>();
                if (type == ProgressType::incremental) {
                    progress_ += 1;
                } else {
                    progress_ -= 1;
                }
            }
            save_start_time();
            print_progress();
        }

        size_t current() {
            std::lock_guard<std::mutex> lock{mutex_};
            return (std::min)(
                    progress_,
                    size_t(get_value<details::ProgressBarOption::max_progress>()));
        }

        bool is_completed() const {
            return get_value<details::ProgressBarOption::completed>();
        }

        void mark_as_started() {
            get_value<details::ProgressBarOption::completed>() = false;
            get_value<details::ProgressBarOption::saved_start_time>() = false;
        }

        void mark_as_completed() {
            get_value<details::ProgressBarOption::completed>() = true;
            print_progress();
        }

    private:
        template<details::ProgressBarOption id>
        auto get_value() -> decltype((details::get_value<id>(std::declval<Settings &>()).value)) {
            return details::get_value<id>(settings_).value;
        }

        template<details::ProgressBarOption id>
        auto get_value() const -> decltype((details::get_value<id>(std::declval<const Settings &>()).value)) {
            return details::get_value<id>(settings_).value;
        }

        size_t progress_{0};
        Settings settings_;
        std::chrono::nanoseconds elapsed_;
        std::chrono::time_point<std::chrono::system_clock> start_time_point_;
        std::chrono::time_point<std::chrono::system_clock> tp_last_update_; // 记录最后一次更新进度条的时间点
        std::mutex mutex_;

        template<typename Indicator, size_t count> friend
        class MultiProgress;

        template<typename Indicator> friend
        class DynamicProgress;

        std::atomic<bool> multi_progress_mode_{false};

        void save_start_time() {
            auto &show_elapsed_time = get_value<details::ProgressBarOption::show_elapsed_time>();
            auto &saved_start_time = get_value<details::ProgressBarOption::saved_start_time>();
            auto &show_remaining_time = get_value<details::ProgressBarOption::show_remaining_time>();
            if ((show_elapsed_time || show_remaining_time) && !saved_start_time) {
                start_time_point_ = std::chrono::system_clock::now();
                saved_start_time = true;
            }
        }

        std::pair<std::string, size_t> get_prefix_text() {
            std::stringstream os;
            os << get_value<details::ProgressBarOption::prefix_text>();
            const auto result = os.str();
            const auto result_size = unicode::display_width(result);
            return {result, result_size};
        }

        inline std::string format_percentage_fixed_width(double percent) {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2);
            ss << std::setw(5)<< percent;

            // 确保输出为7字符
            std::string result = ss.str();
            return result.substr(0, 5);
        }

        static constexpr int FIELD_WIDTH = 8; // 示例: "1234.56T/s"
        std::string format_speed(double speed) {
            // 单位定义表（阈值降序排列）
            static const std::vector<std::pair<double, std::string>> UNITS = {
                {1e12, "T/s"},  // 万亿
                {1e9,  "G/s"},  // 十亿
                {1e6,  "M/s"},  // 百万
                {1e3,  "k/s"},  // 千
                {1,    "/s"}   // 个（注意空格对齐）
            };

            // 查找匹配单位
            for (const auto& [threshold, unit] : UNITS) {
                if (speed >= threshold || threshold == 1) {
                    // 统一保留1位小数，右对齐，固定宽度10字符
                    std::ostringstream oss;
                    oss << std::fixed << std::setprecision(1)
                        << std::setw(FIELD_WIDTH-unit.length()) << std::right
                        << (speed / threshold) << unit;

                    // 精确截断（防止极端情况溢出）
                    std::string result = oss.str();
                    return result.substr(0, FIELD_WIDTH);
                }
            }
            return ""; // 不会执行到此处
        }

        std::pair<std::string, size_t> get_postfix_text() {
            std::stringstream os;
            const auto max_progress = get_value<details::ProgressBarOption::max_progress>();

            if (get_value<details::ProgressBarOption::show_percentage>()) {
                os << " "
                   << format_percentage_fixed_width((std::min)(double(progress_) / double(max_progress) * 100,
                                                               double(100)))
                   << "%";
            }

            if (get_value<details::ProgressBarOption::show_speed>()) {
                const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_).count();
                double speed = progress_;
                if (ms > 0) {
                    speed = static_cast<double>(progress_) * 1000 / ms;
                }
                os << " "
                   << format_speed(speed);
            }

            auto &saved_start_time = get_value<details::ProgressBarOption::saved_start_time>();

            if (get_value<details::ProgressBarOption::show_elapsed_time>()) {
                os << " [";
                if (saved_start_time) {
                    details::write_duration(os, elapsed_);
                } else {
                    os << "00:00s";
                }
            }

            if (get_value<details::ProgressBarOption::show_remaining_time>()) {
                if (get_value<details::ProgressBarOption::show_elapsed_time>()) {
                    os << "<";
                } else {
                    os << " [";
                }
                if (saved_start_time) {
                    auto eta = std::chrono::nanoseconds(
                            progress_ > 0
                            ? static_cast<long long>(std::ceil(double(elapsed_.count()) * max_progress / progress_))
                            : 0);
                    auto remaining = eta > elapsed_ ? (eta - elapsed_) : (elapsed_ - eta);
                    if (progress_ <= 1 || progress_ >= max_progress) {
                        remaining = std::chrono::nanoseconds(0);
                    }
                    details::write_duration(os, remaining);
                } else {
                    os << "00:00s";
                }

                os << "]";
            } else {
                if (get_value<details::ProgressBarOption::show_elapsed_time>())
                    os << "]";
            }

            os << " " << get_value<details::ProgressBarOption::postfix_text>();

            const auto result = os.str();
            const auto result_size = unicode::display_width(result);
            return {result, result_size};
        }

    public:
        void print_progress(bool from_multi_progress = false) {
            std::lock_guard<std::mutex> lock{mutex_};

            auto &os = get_value<details::ProgressBarOption::stream>();

            const auto type = get_value<details::ProgressBarOption::progress_type>();
            const auto min_progress = get_value<details::ProgressBarOption::min_progress>();
            const auto max_progress = get_value<details::ProgressBarOption::max_progress>();
            if (multi_progress_mode_ && !from_multi_progress) {
                if ((type == ProgressType::incremental && progress_ >= max_progress) ||
                    (type == ProgressType::decremental && progress_ <= min_progress)) {
                    get_value<details::ProgressBarOption::completed>() = true;
                }
                return;
            }
            auto now = std::chrono::system_clock::now();
            if (!get_value<details::ProgressBarOption::completed>()) {
                if (progress_ < max_progress && (now - tp_last_update_) < std::chrono::milliseconds(100)) {
                    return;
                }
                elapsed_ = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_time_point_);
                tp_last_update_ = now;
            }
            if (get_value<details::ProgressBarOption::foreground_color>() != Color::unspecified) {
                details::set_stream_color(os, get_value<details::ProgressBarOption::foreground_color>());
            }
            for (auto &style: get_value<details::ProgressBarOption::font_styles>()) {
                details::set_font_style(os, style);
            }

            const auto prefix_pair = get_prefix_text();
            const auto prefix_text = prefix_pair.first;
            const auto prefix_length = prefix_pair.second;
            os << "\r" << prefix_text;

            os << get_value<details::ProgressBarOption::start>();

            details::ProgressScaleWriter writer{
                    os, get_value<details::ProgressBarOption::bar_width>(),
                    get_value<details::ProgressBarOption::fill>(),
                    get_value<details::ProgressBarOption::lead>(),
                    get_value<details::ProgressBarOption::remainder>()};
            writer.write(double(progress_) / double(max_progress) * 100.0f);

            os << get_value<details::ProgressBarOption::end>();

            const auto postfix_pair = get_postfix_text();
            const auto postfix_text = postfix_pair.first;
            const auto postfix_length = postfix_pair.second;
            os << postfix_text;

            // Get length of prefix text and postfix text
            const auto start_length = get_value<details::ProgressBarOption::start>().size();
            const auto bar_width = get_value<details::ProgressBarOption::bar_width>();
            const auto end_length = get_value<details::ProgressBarOption::end>().size();
            const auto terminal_width = terminal_size().second;
            // prefix + bar_width + postfix should be <= terminal_width
            const int remaining =
                    terminal_width - (prefix_length + start_length + bar_width + end_length + postfix_length);
            if (remaining > 0) {
                os << std::string(remaining, ' ') << "\r";
            } else if (remaining < 0) {
                // Do nothing. Maybe in the future truncate postfix with ...
            }
            os.flush();

            if ((type == ProgressType::incremental && progress_ >= max_progress) ||
                (type == ProgressType::decremental && progress_ <= min_progress)) {
                get_value<details::ProgressBarOption::completed>() = true;
            }
            if (get_value<details::ProgressBarOption::completed>() &&
                !from_multi_progress) // Don't std::endl if calling from MultiProgress
                os << termcolor::reset << std::endl;
        }
    };

} // namespace indicators

#endif