#pragma once
#ifndef QUANT1X_FORMULA_H
#define QUANT1X_FORMULA_H 1

#include <quant1x/pandas/series.h>


// ==============================
// 公式指标函数实现
// 变量名大写S开头, 一般是传入数据序列
// 变量名N, 一般标识计算周期
// 变量名M, 一般标识权重
// ==============================

namespace formula {

    constexpr const int invalid_period = -1;  ///< 无效的周期数

    /**
     * @brief 获取序列S的第i个元素
     * @tparam T 模板类型
     * @param S 数据序列
     * @param I 索引, 如果I<0, 则从尾部开始计算索引
     * @return
     */
    template <typename T>
    inline T at(const xt::xarray<T> &S, int64_t I) {
        if (I >= 0) {
            return S[I];
        }
        return S[S.size() + I];
    }

    // ==============================
    // REF
    // ==============================
    inline xt::xarray<double> ref(const xt::xarray<double> &S, size_t N = 1) {
        xt::xarray<double> result = xt::empty<double>({S.size()});
        result.fill(std::numeric_limits<double>::quiet_NaN());

        if (S.size() > N) {
            // 使用全限定名 xt::placeholders::_
            xt::view(result, xt::range(N, xt::placeholders::_)) = xt::view(S, xt::range(0, S.size() - N));
        }
        return result;
    }

    // ==============================
    // MA
    // ==============================

    inline xt::xarray<double> ma(const xt::xarray<double> &S, size_t N) {
        xt::xarray<double> result = xt::empty<double>({S.size()});
        result.fill(std::numeric_limits<double>::quiet_NaN());

        if (S.size() < N) {
            return result;
        }

        // 编译期已知周期，可能触发循环展开
        for (size_t i = N - 1; i < S.size(); ++i) {
            auto window = xt::view(S, xt::range(i - N + 1, i + 1));
            result[i]   = xt::mean(window)();
        }
        return result;
    }

    /**
     * @brief 增量计算移动平均线
     * @param previousHalfValue 前period-1的平均值
     * @param N 周期数
     * @param now 现价
     * @return
     */
    inline double ma_incr(double previousHalfValue, int N, double now) {
        double value = (previousHalfValue * (N - 1) + now) / N;
        return value;
    }

    // ==============================
    // EMA
    // ==============================

    /**
     * @brief 计算ema
     * @param S 数据序列
     * @param N 计算周期
     * @return
     */
    inline xt::xarray<double> ema(const xt::xarray<double> &S, int N = 12) {
        if (S.size() == 0 || N <= 0) {
            return xt::xarray<double>(S.shape());
        }
        const double       alpha = 2.0 / (N + 1);
        xt::xarray<double> ema   = xt::empty<double>({S.size()});

        // 首值处理（可用首价或SMA初始化）
        ema[0] = std::isnan(S[0]) ? double(0) : S[0];

        // 递归计算EMA
        for (size_t i = 1; i < S.size(); ++i) {
            ema[i] = std::isnan(S[i]) ? S[i - 1] : (alpha * S[i] + (1 - alpha) * ema[i - 1]);
        }

        return ema;
    }

    /**
     * @brief 根据周期是计算α值
     * @param N 计算周期
     * @return EMA的计算是全部数据, 所以不用考虑第一个元素的情况
     */
    inline double alpha_of_ema(int N) {
        double alpha = 2.00 / double(1 + N);
        return alpha;
    }

    /**
     * @brief 增量计算EMA, 通过上一条数值last, alpha和最新值增量计算EMA
     * @param now 最新价
     * @param last 上一个ema
     * @param alpha ema的alpha值
     * @return yt = (1−α)*y(t−1) + α*x(t)
     */
    inline double ema_incr(double now, double last, double alpha) {
        double current = (1 - alpha) * last + alpha * now;
        return std::isnan(now) ? last : current;
    }

    // ==============================
    // SMA
    // ==============================

    /**
     * @brief 国际标准 SMA
     * @param S 数据序列
     * @param N 计算周期
     * @return
     */
    inline xt::xarray<double> sma_standard(const xt::xarray<double> &S, size_t N, int /*M*/) {
        if (S.size() == 0 || N <= 0)
            return S;

        xt::xarray<double> result = xt::empty<double>({S.size()});
        // 前N-1个点填充NaN
        for (size_t i = 0; i < N - 1 && i < S.size(); ++i) {
            result[i] = std::nan("");
        }
        for (size_t i = N - 1; i < S.size(); ++i) {
            // 显式调用operator()计算结果
            result[i] = xt::sum(xt::view(S, xt::range(i - N + 1, i + 1)))() / static_cast<double>(N);
        }
        return result;
    }

    /**
     * @brief 通达信算法 SMA
     * @param S 数据序列
     * @param N 计算周期
     * @param M 权重因子
     * @return
     */
    inline xt::xarray<double> sma(const xt::xarray<double> &S, int N, int M) {
        if (S.size() == 0 || N <= 0) {
            return xt::xarray<double>(S.shape());
        }
        xt::xarray<double> sma = xt::empty<double>({S.size()});
        sma[0]                 = std::isnan(S[0]) ? double(0) : S[0];

        for (size_t i = 1; i < S.size(); ++i) {
            sma[i] = std::isnan(S[i]) ? S[i - 1] : (M * S[i] + (N - M) * sma[i - 1]) / N;
        }

        return sma;
    }

    /**
     * @brief 增量计算SMA
     * @param now 现价
     * @param last 上一个SMA值
     * @param N 周期
     * @param M 权重因子
     * @return
     */
    inline double sma_incr(double now, double last, int N, int M) {
        double current = (M * now + (N - M) * last) / N;
        return std::isnan(now) ? last : current;
    }

    // ==============================
    // HHV
    // ==============================

    // 前置声明
    template <typename E, typename = void>
    struct is_string_type : std::false_type {};

    template <typename E>
    struct is_string_type<E, std::enable_if_t<std::is_same_v<typename std::decay_t<E>::value_type, std::string>>>
        : std::true_type {};

    //    // 主模板声明（仅声明，不定义）
    //    template <typename E, typename = void>
    //    xt::xarray<typename std::decay_t<E>::value_type> hhv(E&& high, std::size_t N);

    // 数值类型特化
    template <typename E>
    inline xt::xarray<typename std::decay_t<E>::value_type>
    hhv(E         &&S,
        std::size_t N,
        std::enable_if_t<std::is_arithmetic_v<typename std::decay_t<E>::value_type> && !is_string_type<E>::value> * =
            nullptr) {
        using value_type              = typename std::decay_t<E>::value_type;
        const auto             size   = S.size();
        xt::xarray<value_type> result = xt::empty<value_type>({size});
        result.fill(std::numeric_limits<value_type>::quiet_NaN());

        for (std::size_t i = N - 1; i < size; ++i) {
            const std::size_t start  = i - N + 1;
            auto              window = xt::view(S, xt::range(start, i + 1));
            result[i]                = xt::amax(window)();
        }

        return result;
    }

    // 字符串类型特化
    template <typename E>
    inline xt::xarray<std::string>
    hhv(E &&S, std::size_t N, std::enable_if_t<is_string_type<E>::value> * = nullptr) {
        const auto              size   = S.size();
        xt::xarray<std::string> result = xt::empty<std::string>({size});

        for (std::size_t i = 0; i < N - 1 && i < size; ++i) {
            result[i] = "";
        }

        for (std::size_t i = N - 1; i < size; ++i) {
            const std::size_t start  = i - N + 1;
            auto              window = xt::view(S, xt::range(start, i + 1));
            auto              max_it = std::max_element(window.begin(), window.end());
            result[i]                = *max_it;
        }

        return result;
    }

    // ==============================
    // LLV
    // ==============================

    // 数值类型LLV特化
    template <typename E>
    inline xt::xarray<typename std::decay_t<E>::value_type>
    llv(E         &&S,
        std::size_t N,
        std::enable_if_t<std::is_arithmetic_v<typename std::decay_t<E>::value_type> && !is_string_type<E>::value> * =
            nullptr) {
        using value_type              = typename std::decay_t<E>::value_type;
        const auto             size   = S.size();
        xt::xarray<value_type> result = xt::empty<value_type>({size});
        result.fill(std::numeric_limits<value_type>::quiet_NaN());

        for (std::size_t i = N - 1; i < size; ++i) {
            const std::size_t start  = i - N + 1;
            auto              window = xt::view(S, xt::range(start, i + 1));
            result[i]                = xt::amin(window)();
        }

        return result;
    }

    // 字符串类型LLV特化
    template <typename E>
    inline xt::xarray<std::string> llv(E &&S, std::size_t N, std::enable_if_t<is_string_type<E>::value> * = nullptr) {
        const auto              size   = S.size();
        xt::xarray<std::string> result = xt::empty<std::string>({size});

        for (std::size_t i = 0; i < N - 1 && i < size; ++i) {
            result[i] = "";
        }

        for (std::size_t i = N - 1; i < size; ++i) {
            const std::size_t start  = i - N + 1;
            auto              window = xt::view(S, xt::range(start, i + 1));
            auto              min_it = std::min_element(window.begin(), window.end());
            result[i]                = *min_it;
        }

        return result;
    }

    // ==============================
    // BARSLAST 上一次条件成立到当前的周期数
    // ==============================
    template <typename E>
    inline xt::xarray<int> bars_last(E &&cond) {
        const auto      size    = cond.size();
        xt::xarray<int> result  = xt::empty<int>({size});
        auto           *res_ptr = result.data();

        int last_true_pos = -1;

        // 手动展开循环以利用SIMD
        for (int i = 0; i < int(size); ++i) {
            bool current = cond[i];
            res_ptr[i]   = current ? 0 : (last_true_pos >= 0 ? i - last_true_pos : invalid_period);
            if (current) {
                last_true_pos = i;
            }
        }
        return result;
    }

    // ==============================
    // BARSLASTCOUNT 统计连续满足S条件的周期数
    // ==============================

    template <class E>
    inline xt::xarray<int> bars_last_count(const xt::xexpression<E> &cond) {
        // 获取表达式的实际类型
        using value_type = typename std::remove_reference_t<decltype(cond.derived_cast())>::value_type;

        // 如果表达式已经是 bool 类型，则直接使用
        xt::xarray<bool> condition;
        if constexpr (std::is_same_v<value_type, bool>) {
            condition = xt::eval(cond.derived_cast());
        } else {
            // 否则判断是否不等于 0（适用于数值类型）
            condition = xt::eval(cond.derived_cast() != 0);
        }

        // 计算连续满足条件的周期数
        xt::xarray<int> result = xt::zeros<int>(condition.shape());
        int             count  = 0;

        for (size_t i = 0; i < condition.size(); ++i) {
            if (condition(i)) {
                count++;
            } else {
                count = 0;
            }
            result(i) = count;
        }

        return result;
    }

    // ==============================
    // BARSLASTS 倒数第N次X条件成立到当前的周期数
    // ==============================

    template <typename E>
    inline xt::xarray<int> bars_lasts(E &&cond, int N) {
        const auto      size   = cond.size();
        xt::xarray<int> result = xt::empty<int>({size});

        for (int i = 0; i < int(size); ++i) {
            int count = 0;
            // result[i] = std::numeric_limits<double>::quiet_NaN(); // 默认NaN
            result[i] = invalid_period;
            // 从当前位置往前查找
            for (int j = i; j >= 0; --j) {
                if (cond[j]) {
                    count++;
                    if (count == N) {
                        // 计算周期数（包含两端）
                        result[i] = i - j + 1;
                        break;
                    }
                }
            }
        }

        return result;
    }

    // ==============================
    // BARSSINCEN N周期内第一次S条件成立到现在的周期数
    // ==============================

    template <typename E>
    inline xt::xarray<int> bars_sincen(E &&cond, int N) {
        const auto      size   = cond.size();
        xt::xarray<int> result = xt::empty<int>({static_cast<std::size_t>(size)});

        for (int i = 0; i < size; ++i) {
            result[i] = invalid_period;  // 默认-1表示无满足条件

            // 计算查找范围的起始位置
            int start_pos = std::max(0, i - N + 1);

            // 在最近N个周期内查找第一次满足条件的位置
            for (int j = start_pos; j <= i; ++j) {
                if (cond[j]) {
                    // 找到第一次满足条件的位置，计算周期数（包含两端）
                    result[i] = i - j + 1;
                    break;
                }
            }
        }

        return result;
    }

    // ==============================
    // IFF
    // ==============================
    // 标量版本
    template <typename T, typename F>
    inline auto iff(bool condition, T &&true_expr, F &&false_expr) {
        return condition ? std::forward<T>(true_expr) : std::forward<F>(false_expr);
    }

    // xtensor 版本
    template <typename Cond,
              typename T,
              typename F,
              typename = std::enable_if_t<xt::is_xexpression<std::decay_t<Cond>>::value>>
    inline auto iff(Cond &&condition, T &&true_expr, F &&false_expr) {
        static_assert(std::is_same_v<typename std::decay_t<Cond>::value_type, bool>,
                      "Condition must be a boolean xtensor expression");
        return xt::where(std::forward<Cond>(condition), std::forward<T>(true_expr), std::forward<F>(false_expr));
    }

    // ==============================
    // IFN
    // ==============================

    // 标量版本：如果条件为 FALSE 返回 true_expr，否则返回 false_expr
    template <typename T, typename F>
    inline auto ifn(bool condition, T &&true_expr, F &&false_expr) {
        return !condition ? std::forward<T>(true_expr) : std::forward<F>(false_expr);
    }

    // xtensor 版本：对条件取反后调用 where
    template <typename Cond,
              typename T,
              typename F,
              typename = std::enable_if_t<xt::is_xexpression<std::decay_t<Cond>>::value>>
    inline auto ifn(Cond &&condition, T &&true_expr, F &&false_expr) {
        static_assert(std::is_same_v<typename std::decay_t<Cond>::value_type, bool>,
                      "Condition must be a boolean xtensor expression");
        return xt::where(!std::forward<Cond>(condition),  // 关键区别：对条件取反
                         std::forward<T>(true_expr),
                         std::forward<F>(false_expr));
    }

    // ==============================
    // ABS
    // ==============================

    /**
     * @brief 计算输入数组的绝对值，返回xarray
     * @param x 输入数组
     * @return 绝对值数组
     */
    template <typename E>
    inline xt::xarray<typename std::decay_t<E>::value_type> abs(E &&x) {
        // 使用xt::abs计算绝对值，并确保返回xarray类型
        return xt::xarray<typename std::decay_t<E>::value_type>(xt::abs(std::forward<E>(x)));
    }

    // ==============================
    // MAX
    // ==============================

    /**
     * @brief 计算两个数组的逐元素最大值，统一返回xarray
     * @param a 第一个输入数组/表达式
     * @param b 第二个输入数组/表达式
     * @return xt::xarray<T> 最大值数组
     */
    // 主模板 - 处理两个xexpression的情况
    template <typename E1, typename E2>
    inline xt::xarray<
        typename std::common_type_t<typename std::decay_t<E1>::value_type, typename std::decay_t<E2>::value_type>>
    max(E1 &&a, E2 &&b) {
        return xt::xarray<
            typename std::common_type_t<typename std::decay_t<E1>::value_type, typename std::decay_t<E2>::value_type>>(
            xt::maximum(std::forward<E1>(a), std::forward<E2>(b)));
    }

    // 特化版本1 - 处理xexpression和标量的情况
    template <typename E, typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    inline xt::xarray<typename std::common_type_t<typename std::decay_t<E>::value_type, T>> max(E &&a, T b) {
        return xt::xarray<typename std::common_type_t<typename std::decay_t<E>::value_type, T>>(
            xt::maximum(std::forward<E>(a), b));
    }

    // 特化版本2 - 处理标量和xexpression的情况
    template <typename T, typename E, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    inline xt::xarray<typename std::common_type_t<T, typename std::decay_t<E>::value_type>> max(T a, E &&b) {
        return xt::xarray<typename std::common_type_t<T, typename std::decay_t<E>::value_type>>(
            xt::maximum(a, std::forward<E>(b)));
    }

    /**
     * @brief 返回数组中的最大值（单个值）
     * @param arr 输入数组
     * @return 数组中的最大值
     */
    template <typename E>
    inline auto max(E &&arr) -> typename std::decay_t<E>::value_type {
        return xt::amax(arr)();
    }

    /**
     * @brief 返回数组中的最大值（单个值）的索引
     * @param arr 输入数组
     * @return 数组中的最小值的索引
     */
    template <typename E>
    inline auto arg_max(E &&arr) -> typename std::decay_t<E>::value_type {
        return xt::argmax(arr)();
    }

    // ==============================
    // MIN
    // ==============================

    /**
     * @brief 计算两个数组的逐元素最小值，统一返回xarray
     * @param a 第一个输入数组/表达式
     * @param b 第二个输入数组/表达式
     * @return xt::xarray<T> 最小值数组
     */
    // 主模板 - 处理两个xexpression的情况
    template <typename E1, typename E2>
    inline xt::xarray<
        typename std::common_type_t<typename std::decay_t<E1>::value_type, typename std::decay_t<E2>::value_type>>
    min(E1 &&a, E2 &&b) {
        return xt::xarray<
            typename std::common_type_t<typename std::decay_t<E1>::value_type, typename std::decay_t<E2>::value_type>>(
            xt::minimum(std::forward<E1>(a), std::forward<E2>(b)));
    }

    // 特化版本1 - 处理xexpression和标量的情况
    template <typename E, typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    inline xt::xarray<typename std::common_type_t<typename std::decay_t<E>::value_type, T>> min(E &&a, T b) {
        return xt::xarray<typename std::common_type_t<typename std::decay_t<E>::value_type, T>>(
            xt::minimum(std::forward<E>(a), b));
    }

    // 特化版本2 - 处理标量和xexpression的情况
    template <typename T, typename E, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    inline xt::xarray<typename std::common_type_t<T, typename std::decay_t<E>::value_type>> min(T a, E &&b) {
        return xt::xarray<typename std::common_type_t<T, typename std::decay_t<E>::value_type>>(
            xt::minimum(a, std::forward<E>(b)));
    }

    /**
     * @brief 返回数组中的最小值（单个值）
     * @param arr 输入数组
     * @return 数组中的最小值
     */
    template <typename E>
    inline auto min(E &&arr) -> typename std::decay_t<E>::value_type {
        return xt::amin(arr)();
    }

    /**
     * @brief 返回数组中的最小值（单个值）的索引
     * @param arr 输入数组
     * @return 数组中的最小值
     */
    template <typename E>
    inline auto arg_min(E &&arr) -> typename std::decay_t<E>::value_type {
        return xt::argmin(arr)();
    }

    // ==============================
    // median
    // ==============================

    /**
     * @brief 计算数组或切片的中位数
     * @param arr 输入数组或切片
     * @return 中位数值
     * @throw std::invalid_argument 如果输入为空
     */
    template <typename E>
    inline auto median(E &&arr) -> typename std::decay_t<E>::value_type {
        return xt::median(arr);
    }

    // ==============================
    // STDDEV 标准差
    // ==============================

    template <typename E>
    inline auto stddev(E &&arr) -> typename std::decay_t<E>::value_type {
        return xt::stddev(arr);
    }

    /**
     * @brief 计算N周期滚动标准差
     * @param data 输入数据序列
     * @param period 计算周期N
     * @return xt::xarray<double> 标准差序列
     */
    inline xt::xarray<double> rolling_std(const xt::xarray<double> &data, size_t period) {
        // 初始化全为NaN
        xt::xarray<double> result = xt::full_like(data, std::numeric_limits<double>::quiet_NaN());

        // 只有数据足够时才计算
        if (data.size() >= period) {
            // 计算有效部分（从period-1开始）
            for (size_t i = period - 1; i < data.size(); ++i) {
                auto window = xt::view(data, xt::range(i - period + 1, i + 1));
                result(i)   = xt::stddev(window)();
            }
        }

        return result;
    }

    /**
     * @brief 优化的N周期滚动标准差计算
     * @param data 输入数据序列
     * @param period 计算周期N
     * @return xt::xarray<double> 标准差序列
     */
    inline xt::xarray<double> rolling_std_optimized(const xt::xarray<double> &data, size_t period) {
        // 初始化全为NaN
        xt::xarray<double> result = xt::full_like(data, std::numeric_limits<double>::quiet_NaN());

        if (data.size() < period || period <= 1) {
            return result;
        }

        // 计算滚动平均值和平方和
        double sum    = 0.0;
        double sum_sq = 0.0;

        // 初始化第一个窗口
        for (size_t i = 0; i < period; ++i) {
            sum += data[i];
            sum_sq += data[i] * data[i];
        }

        // 第一个有效点的标准差
        double mean        = sum / period;
        result(period - 1) = std::sqrt((sum_sq - sum * mean) / period);

        // 滑动窗口更新
        for (size_t i = period; i < data.size(); ++i) {
            // 更新sum和sum_sq
            sum += data[i] - data[i - period];
            sum_sq += data[i] * data[i] - data[i - period] * data[i - period];

            // 计算新窗口的标准差
            mean      = sum / period;
            result(i) = std::sqrt((sum_sq - sum * mean) / period);
        }

        return result;
    }

    // ==============================
    // 数组长度对齐, 默认右对齐
    // ==============================

    // 填充策略枚举
    enum class PaddingPolicy {
        Left,            // 左填充（右对齐）
        Right,           // 右填充（左对齐）
        Both,            // 双侧填充
        Default = Left,  // 默认: 左填充右对齐
    };

    /**
     * @brief 对齐两个数组，默认右填充（不抛异常）
     * @param a 第一个数组
     * @param b 第二个数组
     * @param policy 填充策略（默认右填充）
     * @param pad_value 填充值（默认为NaN）
     * @return 对齐后的数组 tuple
     */
    template <typename T>
    inline std::tuple<xt::xarray<T>, xt::xarray<T>> safe_align(const xt::xarray<T> &a,
                                                               const xt::xarray<T> &b,
                                                               PaddingPolicy        policy = PaddingPolicy::Left,
                                                               T pad_value = std::numeric_limits<T>::quiet_NaN()) {
        // 处理空输入：直接返回空数组（不抛异常）
        if (a.size() == 0 || b.size() == 0) {
            return {a, b};
        }

        const size_t max_len = std::max(a.size(), b.size());

        // 计算填充量
        auto get_padding = [max_len](size_t len, PaddingPolicy p) {
            const size_t pad = max_len - len;
            switch (p) {
                case PaddingPolicy::Left:
                    return std::pair{pad, size_t(0)};  // 左填充
                case PaddingPolicy::Right:
                    return std::pair{size_t(0), pad};  // 右填充
                case PaddingPolicy::Both:
                    return std::pair{pad / 2, pad - pad / 2};  // 双侧
                default:
                    return std::pair{size_t(0), pad};  // 默认右填充
            }
        };

        const auto [a_before, a_after] = get_padding(a.size(), policy);
        const auto [b_before, b_after] = get_padding(b.size(), policy);

        return {(a.size() < max_len) ? xt::pad(a, {a_before, a_after}, xt::pad_mode::constant, pad_value) : a,
                (b.size() < max_len) ? xt::pad(b, {b_before, b_after}, xt::pad_mode::constant, pad_value) : b};
    }

    // ==============================
    // CROSS 上穿
    // ==============================

    // 实现cross函数 - 修复版本
    inline xt::xarray<bool> cross_basic(const xt::xarray<double> &S1, const xt::xarray<double> &S2) {
        // 检查输入大小是否一致
        if (S1.shape() != S2.shape()) {
            throw std::runtime_error("Input arrays must have the same shape");
        }

        // 获取数组大小
        auto size = S1.size();

        // 创建结果数组
        xt::xarray<bool> result = xt::xarray<bool>::from_shape(S1.shape());

        // 第一个元素无法判断交叉
        result(0) = false;

        // 使用显式循环避免SIMD类型问题
        for (size_t i = 1; i < size; ++i) {
            result(i) = (S1(i - 1) < S2(i - 1)) && (S1(i) > S2(i));
        }

        return result;
    }

    // 或者使用更安全的向量化实现
    inline xt::xarray<bool> cross_vectorized(const xt::xarray<double> &S1, const xt::xarray<double> &S2) {
        if (S1.shape() != S2.shape()) {
            throw std::runtime_error("Input arrays must have the same shape");
        }

        // 获取前一个和当前元素视图
        auto S1_prev = xt::view(S1, xt::range(0, -1));
        auto S2_prev = xt::view(S2, xt::range(0, -1));
        auto S1_curr = xt::view(S1, xt::range(1, xt::placeholders::_));
        auto S2_curr = xt::view(S2, xt::range(1, xt::placeholders::_));

        // 分别计算两个条件
        auto cond1 = xt::less(S1_prev, S2_prev);
        auto cond2 = xt::greater(S1_curr, S2_curr);

        // 创建结果数组
        xt::xarray<bool> result = xt::xarray<bool>::from_shape(S1.shape());
        result(0)               = false;

        // 使用显式循环合并条件
        for (size_t i = 0; i < cond1.size(); ++i) {
            result(i + 1) = cond1(i) && cond2(i);
        }

        return result;
    }

    inline xt::xarray<bool> cross_optimized(const xt::xarray<double> &S1, const xt::xarray<double> &S2) {
        if (S1.shape() != S2.shape()) {
            throw std::runtime_error("Input arrays must have the same shape");
        }

        xt::xarray<bool> result = xt::xarray<bool>::from_shape(S1.shape());

        // 使用xtensor的element-wise操作
        auto shifted_S1 = xt::view(S1, xt::range(1, xt::placeholders::_));
        auto shifted_S2 = xt::view(S2, xt::range(1, xt::placeholders::_));
        auto prev_S1    = xt::view(S1, xt::range(0, -1));
        auto prev_S2    = xt::view(S2, xt::range(0, -1));

        // 显式转换为bool类型
        auto cross_cond = xt::cast<bool>(xt::less(prev_S1, prev_S2)) &
                          xt::cast<bool>(xt::greater(shifted_S1, shifted_S2));

        result(0)                                           = false;
        xt::view(result, xt::range(1, xt::placeholders::_)) = cross_cond;

        return result;
    }

    /**
     * @brief 计算两条曲线的交叉点（安全版，不抛异常）
     * @param S1 第一条曲线
     * @param S2 第二条曲线
     * @param policy 填充策略（默认右填充）
     * @return 交叉点标记数组
     */
    inline xt::xarray<bool>
    cross(const xt::xarray<double> &S1, const xt::xarray<double> &S2, PaddingPolicy policy = PaddingPolicy::Left) {
        // 安全对齐数据（即使长度不一致或为空也不会抛异常）
        auto [aligned_S1, aligned_S2] = safe_align(S1, S2, policy);
        const size_t n = aligned_S1.size();

        // 初始化结果数组
        xt::xarray<bool> result = xt::xarray<bool>::from_shape({n});
        if (n == 0)
            return result;  // 处理空输入

        result(0) = false;  // 第一个点无交叉

        // 计算有效交叉点
        for (size_t i = 1; i < n; ++i) {
            const bool valid = !std::isnan(aligned_S1(i - 1)) && !std::isnan(aligned_S2(i - 1)) &&
                               !std::isnan(aligned_S1(i)) && !std::isnan(aligned_S2(i));

            result(i) = valid && (aligned_S1(i - 1) < aligned_S2(i - 1)) && (aligned_S1(i) > aligned_S2(i));
        }

        return result;
    }

    // 统一rolling函数（支持标量和动态窗口）

    template <typename DataType, typename WindowType, typename Func>
    auto rolling(const xt::xexpression<DataType> &data_expr, WindowType &&N, Func agg_func) {
        const auto &data               = data_expr.derived_cast();
        using result_type              = decltype(agg_func(xt::view(data, xt::range(0, 0))));
        xt::xarray<result_type> result = xt::empty<result_type>(data.shape());

        // 初始化全部为NaN
        result.fill(std::numeric_limits<double>::quiet_NaN());

        if constexpr (std::is_integral_v<std::decay_t<WindowType>>) {
            // 标量窗口模式
            size_t window_size = N;
            for (size_t i = window_size - 1; i < data.size(); ++i) {
                auto window_view = xt::view(data, xt::range(i - window_size + 1, i + 1));
                result(i)        = agg_func(window_view);
            }
        } else {
            // 动态窗口模式
            const auto &window = N;
            for (size_t i = 0; i < data.size(); ++i) {
                if (is_nan(window(i)))
                    continue;
                size_t window_size = window(i);
                if (window_size == 0 || (i + 1) < window_size)
                    continue;

                size_t start       = i + 1 - window_size;
                auto   window_view = xt::view(data, xt::range(start, i + 1));
                result(i)          = agg_func(window_view);
            }
        }

        return result;
    }

    // 常用聚合函数快捷方式
    namespace rolling_ops {

        inline auto hhv = [](auto &&w) {
            return w.size() > 0 ? xt::amax(w)() : std::numeric_limits<double>::quiet_NaN();
        };

        inline auto llv = [](auto &&w) {
            return w.size() > 0 ? xt::amin(w)() : std::numeric_limits<double>::quiet_NaN();
        };

        inline auto sum = [](auto &&w) {
            return w.size() > 0 ? xt::sum(w)() : std::numeric_limits<double>::quiet_NaN();
        };

        inline auto mean = [](auto &&w) {
            return w.size() > 0 ? xt::mean(w)() : std::numeric_limits<double>::quiet_NaN();
        };

        inline auto stddev(int ddof = 0) {
            return [ddof](auto &&w) {
                return w.size() > ddof ? xt::stddev(w, ddof)() : std::numeric_limits<double>::quiet_NaN();
            };
        };
    }  // namespace rolling_ops

    /**
     * @brief 使用xtensor实现通达信VALUEWHEN函数
     * @param condition 条件数组
     * @param value 取值数组
     * @return 返回结果数组，形状与输入相同
     */
    xt::xarray<double> value_when(const xt::xarray<bool>& condition, const xt::xarray<double>& value);

    /**
     * @brief 使用xt::xexpression作为条件的VALUEWHEN实现
     * @tparam E 条件表达式类型
     * @param cond 条件表达式(可以是任何xtensor表达式)
     * @param value 取值数组
     * @return 返回结果数组，形状与输入相同
     */
    template <typename E>
    xt::xarray<double> value_when(const xt::xexpression<E>& cond, const xt::xarray<double>& value) {
        // 获取条件表达式的具体实现
        const auto& condition = cond.derived_cast();

        // 检查输入形状是否相同
        if (condition.shape() != value.shape()) {
            throw std::invalid_argument("Condition and value arrays must have the same shape");
        }

        // 创建结果数组
        xt::xarray<double> result = xt::empty_like(value);

        // 用于存储上一次满足条件的值
        std::optional<double> lastValidValue;

        // 遍历数组元素
        for (size_t i = 0; i < condition.size(); ++i) {
            if (condition.flat(i)) {
                lastValidValue = value.flat(i);
                result.flat(i) = value.flat(i);
            } else {
                if (lastValidValue.has_value()) {
                    result.flat(i) = lastValidValue.value();
                } else {
                    // 如果之前没有满足条件的值，返回NaN
                    result.flat(i) = std::numeric_limits<double>::quiet_NaN();
                }
            }
        }

        return result;
    }

}  // namespace formula

#endif  // QUANT1X_FORMULA_H
