#pragma once
#ifndef QUANT1X_TECHNICAL_ANALYSIS_PATTERNS_WAVES_H
#define QUANT1X_TECHNICAL_ANALYSIS_PATTERNS_WAVES_H 1

#include <quant1x/base/simd.h>

#include <cctype>
#include <set>
#include <stdexcept>
#include <vector>

#include "trend.h"

namespace ta::patterns {

    template <typename T>
    inline int compare(const T &a, const T &b) {
        if (a < b)
            return -1;
        else if (a > b)
            return 1;
        else
            return 0;
    }

    /**
     * @brief 检测波峰波谷
     * @param high 高点序列
     * @param low 低点序列
     * @return 包含波峰和波谷索引的pair, 波峰基于高价序列, 波谷基于低价序列
     * @remark 如果只检测一个序列, 比如收盘价, high和low可以传入相同的close序列
     */
    template <typename T>
    inline std::pair<std::vector<int>, std::vector<int>> basic_peaks_and_valleys(const std::vector<T> &high,
                                                                                 const std::vector<T> &low) {
        size_t           n = high.size();
        std::vector<int> diff_high(n, 0);  // 高价差分序列
        std::vector<int> diff_low(n, 0);   // 低价差分序列

        // 第一步: 计算一阶差分
        for (size_t i = 0; i < n - 1; ++i) {
            diff_high[i] = compare(high[i + 1], high[i]);
            diff_low[i]  = compare(low[i + 1], low[i]);
        }

        // 第二步: 处理平台区域(差分为0的情况)
        for (size_t i = 0; i < n - 1; ++i) {
            // 处理高价序列的平台
            if (diff_high[i] == 0) {
                if (i == 0) {  // 如果是第一个点
                    for (size_t j = i + 1; j < n - 1; ++j) {
                        if (diff_high[j] != 0) {
                            diff_high[i] = diff_high[j];
                            break;
                        }
                    }
                } else if (i == n - 2) {  // 如果是最后一个点
                    diff_high[i] = diff_high[i - 1];
                } else {  // 中间点
                    diff_high[i] = diff_high[i + 1];
                }
            }

            // 处理低价序列的平台
            if (diff_low[i] == 0) {
                if (i == 0) {
                    for (size_t j = i + 1; j < n - 1; ++j) {
                        if (diff_low[j] != 0) {
                            diff_low[i] = diff_low[j];
                            break;
                        }
                    }
                } else if (i == n - 2) {
                    diff_low[i] = diff_low[i - 1];
                } else {
                    diff_low[i] = diff_low[i + 1];
                }
            }
        }

        // 第三步: 识别波峰和波谷
        std::vector<int> peaks;    // 波峰索引
        std::vector<int> valleys;  // 波谷索引

        for (size_t i = 0; i < n - 1; ++i) {
            int d_high = diff_high[i + 1] - diff_high[i];
            int d_low  = diff_low[i + 1] - diff_low[i];

            int index = int(i) + 1;  // 波峰和波谷的索引是i+1
            if (d_high == -2) {      // 高价序列由上升到下降, 形成波峰
                peaks.push_back(index);
            }
            if (d_low == 2) {  // 低价序列由下降到上升, 形成波谷
                valleys.push_back(index);
            }
        }

        return make_pair(peaks, valleys);
    }

    /**
     * @brief 检测波峰波谷
     * @param high 高点序列
     * @param low 低点序列
     * @return 包含波峰和波谷索引的pair, 波峰基于高价序列, 波谷基于低价序列
     * @remark 如果只检测一个序列, 比如收盘价, high和low可以传入相同的close序列
     */
    std::pair<std::vector<point>, std::vector<point>> peaks_and_valleys(const xt::xarray<double> &high,
                                                                        const xt::xarray<double> &low);

}  // namespace ta::patterns

namespace ta::waves {
    // 约束: T 必须是算术类型(int, float, double 等)
    template <typename T>
    concept Arithmetic = std::is_arithmetic_v<T>;

    struct PeakValleyResult {
        std::vector<int> peaks;
        std::vector<int> valleys;
    };

    namespace detail {
        template <Arithmetic T>
        inline int argmax(std::span<const T> data) {
            return static_cast<int>(std::distance(data.begin(), std::max_element(data.begin(), data.end())));
        }

        template <Arithmetic T>
        inline int argmin(std::span<const T> data) {
            return static_cast<int>(std::distance(data.begin(), std::min_element(data.begin(), data.end())));
        }
    }  // namespace detail

    template <Arithmetic T>
    inline std::vector<int>
    find_monotonic_extremes(std::span<const T> data, const std::string &direction, const std::string &mode) {
        if (data.empty()) {
            return {};
        }
        if (data.size() == 1) {
            return {0};
        }
        auto compare = [](T a, T b, const std::string &m) -> bool {
            if (m == "peak")
                return a > b;
            if (m == "valley")
                return a < b;
            throw std::invalid_argument("mode must be 'peak' or 'valley'");
        };

        int start_idx, end_idx, step;
        if (direction == "left") {
            start_idx = 0;
            end_idx   = static_cast<int>(data.size());
            step      = 1;
        } else if (direction == "right") {
            start_idx = static_cast<int>(data.size()) - 1;
            end_idx   = -1;
            step      = -1;
        } else {
            throw std::invalid_argument("direction must be 'left' or 'right'");
        }

        int              prev_idx = start_idx;
        T                prev_val = data[start_idx];
        std::vector<int> extremes;

        for (int i = start_idx + step; i != end_idx; i += step) {
            if (compare(data[i], prev_val, mode)) {
                prev_idx = i;
                prev_val = data[i];
            } else if (!extremes.empty() && prev_val == data[extremes.back()]) {
                continue;
            } else {
                extremes.push_back(prev_idx);
            }
        }

        if (extremes.empty() || compare(prev_val, data[extremes.back()], mode)) {
            extremes.push_back(prev_idx);
        }

        if (direction == "right") {
            std::reverse(extremes.begin(), extremes.end());
        }

        return extremes;
    }

    template <Arithmetic T>
    inline std::vector<int> find_monotonic_peaks_around_max(std::span<const T> lst) {
        if (lst.empty())
            return {};

        int  max_idx   = detail::argmax(lst);
        auto left_part = lst.subspan(0, max_idx + 1);
        auto left      = find_monotonic_extremes(left_part, "left", "peak");

        std::vector<int> right;
        if (max_idx + 1 < static_cast<int>(lst.size())) {
            auto right_part = lst.subspan(max_idx + 1);
            auto right_raw  = find_monotonic_extremes(right_part, "right", "peak");
            right.reserve(right_raw.size());
            for (int idx : right_raw) {
                right.push_back(max_idx + 1 + idx);
            }
        }

        std::vector<int> raw = left;
        raw.insert(raw.end(), right.begin(), right.end());

        std::vector<int> peaks;
        for (int idx : raw) {
            if (peaks.empty()) {
                peaks.push_back(idx);
            } else {
                int last = peaks.back();
                if (idx != last + 1) {
                    peaks.push_back(idx);
                } else if (lst[idx] > lst[last]) {
                    peaks.back() = idx;
                }
            }
        }

        return peaks;
    }

    template <Arithmetic T>
    inline std::vector<int> find_monotonic_valleys_around_min(std::span<const T> lst) {
        if (lst.empty())
            return {};

        int  min_idx   = detail::argmin(lst);
        auto left_part = lst.subspan(0, min_idx + 1);
        auto left      = find_monotonic_extremes(left_part, "left", "valley");

        std::vector<int> right;
        if (min_idx + 1 < static_cast<int>(lst.size())) {
            auto right_part = lst.subspan(min_idx + 1);
            auto right_raw  = find_monotonic_extremes(right_part, "right", "valley");
            for (int idx : right_raw) {
                right.push_back(min_idx + 1 + idx);
            }
        }

        std::vector<int> raw = left;
        raw.insert(raw.end(), right.begin(), right.end());

        std::vector<int> valleys;
        for (int idx : raw) {
            if (valleys.empty()) {
                valleys.push_back(idx);
            } else {
                int last = valleys.back();
                if (idx != last + 1) {
                    valleys.push_back(idx);
                } else if (lst[idx] < lst[last]) {
                    valleys.back() = idx;
                }
            }
        }

        return valleys;
    }

    template <Arithmetic T>
    inline std::vector<int> refine_peaks_by_valleys(const std::vector<int> &peaks,
                                                    const std::vector<int> &valleys,
                                                    std::span<const T>      high_list) {
        if (peaks.empty())
            return {};

        std::vector<int> valid_peaks;
        for (size_t i = 0; i < peaks.size(); ++i) {
            if (i == 0) {
                valid_peaks.push_back(peaks[i]);
                continue;
            }

            int prev_peak = valid_peaks.back();
            int curr_peak = peaks[i];

            bool has_valley_between = false;
            for (int v : valleys) {
                if (prev_peak < v && v < curr_peak) {
                    has_valley_between = true;
                    break;
                }
            }

            if (has_valley_between) {
                valid_peaks.push_back(curr_peak);
            } else {
                if (high_list[curr_peak] > high_list[prev_peak]) {
                    valid_peaks.back() = curr_peak;
                }
            }
        }

        return valid_peaks;
    }

    template <Arithmetic T>
    inline std::vector<int> refine_valleys_by_peaks(const std::vector<int> &valleys,
                                                    const std::vector<int> &peaks,
                                                    std::span<const T>      low_list) {
        if (valleys.empty())
            return {};

        std::vector<int> valid_valleys;
        for (size_t i = 0; i < valleys.size(); ++i) {
            if (i == 0) {
                valid_valleys.push_back(valleys[i]);
                continue;
            }

            int prev_valley = valid_valleys.back();
            int curr_valley = valleys[i];

            bool has_peak_between = false;
            for (int p : peaks) {
                if (prev_valley < p && p < curr_valley) {
                    has_peak_between = true;
                    break;
                }
            }

            if (has_peak_between) {
                valid_valleys.push_back(curr_valley);
            } else {
                if (low_list[curr_valley] < low_list[prev_valley]) {
                    valid_valleys.back() = curr_valley;
                }
            }
        }

        return valid_valleys;
    }

    template <Arithmetic T>
    inline PeakValleyResult normalize_peaks_and_valleys(const std::vector<int> &peaks,
                                                        const std::vector<int> &valleys,
                                                        std::span<const T>      high_list,
                                                        std::span<const T>      low_list) {
        if (high_list.empty())
            return {{}, {}};

        std::set<int> peak_set(peaks.begin(), peaks.end());
        std::set<int> valley_set(valleys.begin(), valleys.end());
        int           n = static_cast<int>(high_list.size());

        std::vector<std::pair<std::string, int>> all_extremes;
        for (int i = 0; i < n; ++i) {
            if (peak_set.contains(i)) {
                all_extremes.emplace_back("peak", i);
            } else if (valley_set.contains(i)) {
                all_extremes.emplace_back("valley", i);
            }
        }

        if (all_extremes.empty())
            return {{}, {}};

        std::vector<std::pair<std::string, int>> cleaned;
        cleaned.push_back(all_extremes[0]);

        for (size_t i = 1; i < all_extremes.size(); ++i) {
            const auto &last = cleaned.back();
            const auto &curr = all_extremes[i];

            if (curr.first != last.first) {
                cleaned.push_back(curr);
            } else {
                if (curr.first == "peak") {
                    if (high_list[curr.second] > high_list[last.second]) {
                        cleaned.back() = curr;
                    }
                } else if (curr.first == "valley") {
                    if (low_list[curr.second] < low_list[last.second]) {
                        cleaned.back() = curr;
                    }
                }
            }
        }

        std::vector<int> final_peaks, final_valleys;
        for (const auto &[t, idx] : cleaned) {
            if (t == "peak") {
                final_peaks.push_back(idx);
            } else {
                final_valleys.push_back(idx);
            }
        }

        std::sort(final_peaks.begin(), final_peaks.end());
        std::sort(final_valleys.begin(), final_valleys.end());

        return {final_peaks, final_valleys};
    }

    template <Arithmetic T>
    inline PeakValleyResult detect_peaks_and_valleys(std::span<const T> high_list, std::span<const T> low_list) {
        if (high_list.empty() || high_list.size() != low_list.size()) {
            return {{}, {}};
        }

        if (high_list.size() == 1) {
            return {{0}, {0}};
        }

        auto peaks   = find_monotonic_peaks_around_max(high_list);
        auto valleys = find_monotonic_valleys_around_min(low_list);

        if (peaks.empty() || valleys.empty()) {
            std::sort(peaks.begin(), peaks.end());
            std::sort(valleys.begin(), valleys.end());
            return {peaks, valleys};
        }

        auto valid_peaks   = refine_peaks_by_valleys(peaks, valleys, high_list);
        auto valid_valleys = refine_valleys_by_peaks(valleys, peaks, low_list);

        return normalize_peaks_and_valleys(valid_peaks, valid_valleys, high_list, low_list);
    }

    template <Arithmetic T>
    inline PeakValleyResult detect_peaks_and_valleys(const std::vector<T> &high, const std::vector<T> &low) {
        return detect_peaks_and_valleys(std::span<const T>{high.data(), high.size()},
                                        std::span<const T>{low.data(), low.size()});
    }

    // 波浪段结构
    struct WaveSegment {
        int  start;      ///< 开始索引
        int  end;        ///< 结束索引
        int  level;      ///< 波浪层级
        bool is_rising;  ///< 是否上升

        // 用于排序
        bool operator<(const WaveSegment &other) const {
            if (level != other.level)
                return level < other.level;
            return start < other.start;
        }
    };

    template <Arithmetic T>
    std::vector<std::tuple<int, int, bool>> build_wave_segments(std::span<const T>      high_list,
                                                                std::span<const T>      low_list,
                                                                const std::vector<int> &peaks,
                                                                const std::vector<int> &valleys) {
        if (high_list.empty() || low_list.empty()) {
            return {};
        }

        int           n          = static_cast<int>(high_list.size());
        std::set<int> key_points = {0, n - 1};
        key_points.insert(peaks.begin(), peaks.end());
        key_points.insert(valleys.begin(), valleys.end());

        std::vector<int> sorted_points(key_points.begin(), key_points.end());
        std::sort(sorted_points.begin(), sorted_points.end());

        std::vector<std::tuple<int, int, bool>> segments;
        for (size_t i = 0; i + 1 < sorted_points.size(); ++i) {
            int start = sorted_points[i];
            int end   = sorted_points[i + 1];
            if (start >= end)
                continue;

            bool is_start_peak   = std::find(peaks.begin(), peaks.end(), start) != peaks.end();
            bool is_start_valley = std::find(valleys.begin(), valleys.end(), start) != valleys.end();
            bool is_end_peak     = std::find(peaks.begin(), peaks.end(), end) != peaks.end();
            bool is_end_valley   = std::find(valleys.begin(), valleys.end(), end) != valleys.end();

            bool is_rising;
            if (is_start_valley && is_end_peak) {
                is_rising = true;
            } else if (is_start_peak && is_end_valley) {
                is_rising = false;
            } else {
                if (is_end_peak) {
                    is_rising = high_list[end] > high_list[start];
                } else if (is_end_valley) {
                    is_rising = low_list[end] < low_list[start];
                } else {
                    is_rising = high_list[end] > high_list[start];
                }
            }

            segments.emplace_back(start, end, is_rising);
        }

        return segments;
    }

    template <Arithmetic T>
    std::vector<WaveSegment> detect_wave_recursive(
        std::span<const T> high_list, std::span<const T> low_list, int start_idx, int end_idx, int level) {
        if (end_idx - start_idx < 3) {
            return {};
        }

        // 提取子区间
        auto high_sub = high_list.subspan(start_idx, end_idx - start_idx + 1);
        auto low_sub  = low_list.subspan(start_idx, end_idx - start_idx + 1);

        // 检测子区间波峰波谷
        auto [peaks_sub, valleys_sub] = detect_peaks_and_valleys(high_sub, low_sub);

        // 构建局部波段
        auto segments = build_wave_segments(high_sub, low_sub, peaks_sub, valleys_sub);

        std::vector<WaveSegment> global_segments;
        for (const auto &[local_start, local_end, is_rising] : segments) {
            int global_start = start_idx + local_start;
            int global_end   = start_idx + local_end;
            if (global_start != global_end) {
                global_segments.push_back({global_start, global_end, level, is_rising});
            }
        }

        // 递归检测次级波(最多到 level 2)
        if (level < 2) {
            for (const auto &[local_start, local_end, is_rising] : segments) {
                int seg_global_start = start_idx + local_start;
                int seg_global_end   = start_idx + local_end;
                if (seg_global_end - seg_global_start >= 3) {
                    auto sub_segments = detect_wave_recursive(
                        high_list, low_list, seg_global_start, seg_global_end, level + 1);
                    global_segments.insert(global_segments.end(), sub_segments.begin(), sub_segments.end());
                }
            }
        }

        return global_segments;
    }

    template <Arithmetic T>
    std::vector<WaveSegment> detect_complete_wave_structure(std::span<const T> high_list, std::span<const T> low_list) {
        int n = static_cast<int>(high_list.size());
        if (n < 3 || n != static_cast<int>(low_list.size())) {
            return {};
        }

        // 第一阶段: 检测主波峰波谷
        auto [peaks, valleys] = detect_peaks_and_valleys(high_list, low_list);

        // 构建主波段(Level 0)
        auto main_segments = build_wave_segments(high_list, low_list, peaks, valleys);

        std::vector<WaveSegment> all_segments;
        for (const auto &[start, end, is_rising] : main_segments) {
            if (start != end) {
                all_segments.push_back({start, end, 0, is_rising});
            }
        }

        // 第二阶段: 递归检测次级波
        for (const auto &[start, end, is_rising] : main_segments) {
            if (end - start >= 3) {
                auto sub_waves = detect_wave_recursive(high_list, low_list, start, end, 1);
                all_segments.insert(all_segments.end(), sub_waves.begin(), sub_waves.end());
            }
        }

        // 排序: 先按 level, 再按 start
        std::sort(all_segments.begin(), all_segments.end());

        return all_segments;
    }

    template <Arithmetic T>
    std::vector<WaveSegment> detect_complete_wave_structure(const std::vector<T> &high, const std::vector<T> &low) {
        return detect_complete_wave_structure(std::span<const T>{high.data(), high.size()},
                                              std::span<const T>{low.data(), low.size()});
    }
}  // namespace ta::waves

#endif  // QUANT1X_TECHNICAL_ANALYSIS_PATTERNS_WAVES_H
