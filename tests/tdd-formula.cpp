#include <quant1x/test/test.h>
#include <quant1x/formula.h>
#include <quant1x/std/algo.h>
#ifdef _WIN32
#include <windows.h>
#endif


TEST_CASE("formula-hhv", "[formula]") {
    // 数值类型测试
    xt::xarray<double> values_num = {10.5, 11.2, 12.3, 11.8, 10.9};
    std::cout << "origin:" << values_num << std::endl;
    auto result_num = formula::hhv(values_num, 3); // 正确: 调用数值版本
    std::cout << "Numeric HHV: " << result_num << std::endl;

    // 字符串测试
    xt::xarray<std::string> values_text = {"A", "C", "B", "D", "A"};
    std::cout << "origin:" << values_text << std::endl;
    auto result_str = formula::hhv(values_text, 2); // 正确: 调用特化版本
    std::cout << "String HHV: " << result_str << std::endl;
}

TEST_CASE("formula-llv", "[formula]") {
    // 数值类型测试
    xt::xarray<double> values_num = {10.5, 11.2, 12.3, 11.8, 10.9};
    auto result_num = formula::llv(values_num, 3); // 正确: 调用数值版本
    std::cout << "Numeric HHV: " << result_num << std::endl;

    // 字符串测试
    xt::xarray<std::string> values_text = {"A", "C", "B", "D", "A"};
    auto result_str = formula::llv(values_text, 2); // 正确: 调用特化版本
    std::cout << "String HHV: " << result_str << std::endl;

    // 非法类型(编译时报错)
    // xt::xarray<bool> flags = {true, false};
    // auto err = HHV(flags, 2); // 错误: static_assert触发
}

// 正确的BARSLAST实现
template <typename E>
xt::xarray<int> BARSLAST(E&& cond) {
    const auto size = cond.size();
    xt::xarray<int> result = xt::empty<int>({size});

    // 初始化为-1表示条件从未成立
    result.fill(-1);

    int last_true_pos = -1;  // 记录上一次条件成立的位置

    for (std::size_t i = 0; i < size; ++i) {
        if (cond[i]) {
            // 当前条件成立, 周期数为0
            result[i] = 0;
            // 更新上一次条件成立位置
            last_true_pos = static_cast<int>(i);
        }
        else if (last_true_pos >= 0) {
            // 当前条件不成立, 但有历史成立记录
            result[i] = static_cast<int>(i) - last_true_pos;
        }
        // 否则保持-1
    }

    return result;
}

// 优化版本(保证正确性)
template <typename E>
xt::xarray<int> BARSLAST_optimized(E&& cond) {
    const auto size = cond.size();
    xt::xarray<int> result = xt::empty<int>({size});
    auto* res_ptr = result.data();

    int last_true_pos = -1;

    for (std::size_t i = 0; i < size; ++i) {
        if (cond[i]) {
            res_ptr[i] = 0;
            last_true_pos = static_cast<int>(i);
        }
        else {
            res_ptr[i] = (last_true_pos >= 0) ? (static_cast<int>(i) - last_true_pos) : -1;
        }
    }

    return result;
}

TEST_CASE("formula-barslast-bool", "[formula]") {
    // 数值类型测试
    xt::xarray<double> values_num = {1, 2, 3, 4, 5, 6, 0, 8, 9, 10, 11, 12};
    auto c0 = values_num <6 && values_num>2;
    std::cout << "origin:" << values_num << std::endl;
    auto result_num = BARSLAST_optimized(c0); // 正确: 调用数值版本
    std::cout << "result(num): " << result_num << std::endl;

    auto c1 = values_num >200;
    std::cout << "origin:" << values_num << std::endl;
    result_num = BARSLAST_optimized(c1); // 正确: 调用数值版本
    std::cout << "result(num): " << result_num << std::endl;
}

template <typename E>
xt::xarray<int> BARSLAST_simd(E&& cond) {
    const auto size = cond.size();
    xt::xarray<int> result = xt::empty<int>({size});
    auto* res_ptr = result.data();

    int last_true_pos = -1;

    // 手动展开循环以利用SIMD
    for (int i = 0; i < int(size); ++i) {
        bool current = cond[i];
        res_ptr[i] = current ? 0 : (last_true_pos >= 0 ? i - last_true_pos : -1);
        if (current) {
            last_true_pos = i;
        }
    }
    return result;
}

TEST_CASE("BARSLAST_simd", "[formula]") {
    // 数值类型测试
    xt::xarray<double> values_num = {1, 2, 3, 4, 5, 6, 0, 8, 9, 10, 11, 12};
    auto c0 = values_num >10;
    std::cout << "origin:" << values_num << std::endl;
    auto result_num = BARSLAST_simd(c0); // 正确: 调用数值版本
    std::cout << "result(num): " << result_num << std::endl;
}

TEST_CASE("BARSLAST_release", "[formula]") {
    // 数值类型测试
    xt::xarray<double> values_num = {1, 2, 3, 4, 5, 6, 0, 8, 9, 10, 11, 12};
    auto c0 = values_num >10;
    std::cout << "origin:" << values_num << std::endl;
    auto result_num = formula::bars_last(c0); // 正确: 调用数值版本
    std::cout << "result(num): " << result_num << std::endl;
}

// BARSLASTS实现 - 倒数第N次满足条件到当前的周期数
template <typename E>
xt::xarray<double> BARSLASTS(E&& cond, int N) {
    const auto size = cond.size();
    xt::xarray<double> result = xt::empty<double>({size});

    for (int i = 0; i < int(size); ++i) {
        int count = 0;
        result[i] = std::numeric_limits<double>::quiet_NaN(); // 默认NaN

        // 从当前位置往前查找
        for (int j = i; j >= 0; --j) {
            if (cond[j]) {
                count++;
                if (count == N) {
                    // 计算周期数(包含两端)
                    result[i] = i - j + 1;
                    break;
                }
            }
        }
    }

    return result;
}


TEST_CASE("BARSLASTS", "[formula]") {
    // 测试用例1
    xt::xarray<double> values_num = {4, 5, 6, 0, 8, 9, 10, 11, 12, 0};
    std::cout << "origin:" << values_num << std::endl;
    auto cond1 = values_num>3;
    //std::cout << "condition:" << cond1 << std::endl;
    auto res1 = BARSLASTS(cond1, 3); // 倒数第2次满足条件的周期数

    // 测试用例2
    xt::xarray<bool> cond2 = {true, false, true, false, true, false, true};
    auto res2 = BARSLASTS(cond2, 3); // 倒数第3次满足条件的周期数

    std::cout << "条件序列1: " << cond1 << "\n";
    std::cout << "BARSLASTS(2): " << res1 << "\n\n";

    std::cout << "条件序列2: " << cond2 << "\n";
    std::cout << "BARSLASTS(3): " << res2 << std::endl;
}

TEST_CASE("BARSLASTS-v2", "[formula]") {
    // 测试用例1
    xt::xarray<bool> cond1 = {false, true, false, true, false, true, false, false, true};
    auto res1 = BARSLASTS(cond1, 2); // 倒数第2次满足条件的周期数

    // 测试用例2
    xt::xarray<int> series = {4, 5, 6, 0, 8, 9, 10, 11, 12, 0};
    auto res2 = BARSLASTS(series > 3, 3); // 条件可以在外部处理

    std::cout << "条件序列1: " << cond1 << "\n";
    std::cout << "BARSLASTS(2): " << res1 << "\n\n";

    std::cout << "数值序列: " << series << "\n";
    std::cout << "条件: >3\n";
    std::cout << "BARSLASTS(3): " << res2 << std::endl;
}

TEST_CASE("BARSLASTS-release", "[formula]") {
    // 测试用例1
    xt::xarray<bool> cond1 = {false, true, false, true, false, true, false, false, true};
    auto res1 = formula::bars_lasts(cond1, 2); // 倒数第2次满足条件的周期数

    // 测试用例2
    xt::xarray<int> series = {4, 5, 6, 0, 8, 9, 10, 11, 12, 0};
    auto res2 = formula::bars_lasts(series > 3, 3); // 条件可以在外部处理

    std::cout << "条件序列1: " << cond1 << "\n";
    std::cout << "BARSLASTS(2): " << res1 << "\n\n";

    std::cout << "数值序列: " << series << "\n";
    std::cout << "条件: >3\n";
    std::cout << "BARSLASTS(3): " << res2 << std::endl;
}

// RSI指标实现
template <typename E>
xt::xarray<double> RSI_std(E&& close, size_t period = 14) {
    const auto size = close.size();
    xt::xarray<double> rsi = xt::empty<double>({size});
    rsi.fill(std::numeric_limits<double>::quiet_NaN());

    if (size <= period) {
        return rsi;
    }

    // 计算价格变化
    auto delta = xt::eval(xt::view(close, xt::range(1, xt::placeholders::_)) -
                          xt::view(close, xt::range(0, size - 1)));

    // 计算上涨和下跌幅度
    auto gain = xt::maximum(delta, 0.0);
    auto loss = xt::maximum(-delta, 0.0);

    // 计算初始平均值(简单平均)
    double avg_gain = xt::sum(xt::view(gain, xt::range(0, period)))() / period;
    double avg_loss = xt::sum(xt::view(loss, xt::range(0, period)))() / period;

    // 第一个RSI值
    if (avg_loss == 0) {
        rsi[period] = 100.0;
    } else {
        double rs = avg_gain / avg_loss;
        rsi[period] = 100.0 - (100.0 / (1.0 + rs));
    }

    // 计算后续RSI值(使用Wilder平滑)
    for (size_t i = period + 1; i < size; ++i) {
        avg_gain = (avg_gain * (period - 1) + gain[i - 1]) / period;
        avg_loss = (avg_loss * (period - 1) + loss[i - 1]) / period;

        if (avg_loss == 0) {
            rsi[i] = 100.0;
        } else {
            double rs = avg_gain / avg_loss;
            rsi[i] = 100.0 - (100.0 / (1.0 + rs));
        }
    }

    return rsi;
}

TEST_CASE("RSI-basic", "[formula]") {
// 测试用例
    xt::xarray<double> close = {
        44.34, 44.09, 44.15, 43.61, 44.33, 44.83, 45.10, 45.42, 45.84,
        46.08, 45.89, 46.03, 45.61, 46.28, 46.28, 46.00, 46.03, 46.41,
        46.22, 45.64, 46.21, 46.25, 45.71, 46.45, 45.78, 45.35, 44.03,
        44.18, 44.22, 44.57, 43.42, 42.66, 43.13
    };

    auto rsi = RSI_std(close, 14);

    // 打印前14个周期和后14个周期的RSI值
    std::cout << "RSI(14)结果:\n";
    for (size_t i = 0; i < close.size(); ++i) {
        std::cout << "Day " << i << ": Close=" << close[i]
                  << " RSI=" << rsi[i] << std::endl;
    }
}

// 通达信SMA算法实现
xt::xarray<double> SMA(const xt::xarray<double>& data, size_t n, size_t m) {
    const auto size = data.size();
    xt::xarray<double> sma = xt::zeros<double>({size});

    if (size == 0 || n == 0) return sma;

    double sum = 0.0;
    double weight_sum = 0.0;

    for (size_t i = 0; i < size; ++i) {
        if (i < n) {
            sum += data[i];
            weight_sum += 1.0;
        } else {
            sum = sum * (n - m) / n + data[i] * m / n;
            weight_sum = weight_sum * (n - m) / n + 1.0 * m / n;
        }
        sma[i] = sum / weight_sum;
    }

    return sma;
}

// 通达信RSI指标实现
template <typename E>
xt::xarray<double> RSI_TDX(E&& close, size_t n) {
    const auto size = close.size();
    xt::xarray<double> rsi = xt::zeros<double>({size});

    if (size <= 1 || n == 0) return rsi;

    // 计算1日前的收盘价(LC)
    auto lc = xt::view(close, xt::range(0, size - 1));

    // 计算收盘价变化
    auto delta = xt::eval(xt::view(close, xt::range(1, xt::placeholders::_)) - lc);

    // 计算上涨幅度和绝对变化
    auto gain = xt::maximum(delta, 0.0);
    auto abs_delta = xt::abs(delta);

    // 计算SMA
    auto sma_gain = formula::sma(gain, n, 1);
    auto sma_abs = formula::sma(abs_delta, n, 1);

    // 计算RSI
    for (size_t i = 1; i < size; ++i) {
        if (sma_abs[i - 1] == 0) {
            rsi[i] = 100.0;
        } else {
            rsi[i] = sma_gain[i - 1] / sma_abs[i - 1] * 100.0;
        }
    }

    return rsi;
}

// 通达信SMA函数(EMA变种实现)
xt::xarray<double> SMA_TDX(const xt::xarray<double>& data, int n, int m = 1) {
    const auto size = data.size();
    xt::xarray<double> sma = xt::zeros<double>({size});

    if (size == 0 || n <= 0) return sma;

    // 计算平滑系数alpha = m/n
    double alpha = static_cast<double>(m)/n;

    // 第一个值特殊处理
    sma[0] = data[0];

    // EMA计算
    for (int i = 1; i < int(size); ++i) {
        sma[i] = alpha * data[i] + (1 - alpha) * sma[i-1];
    }

    return sma;
}

// 完全正确的通达信RSI实现
xt::xarray<double> RSI_TDX_EXACT(const xt::xarray<double>& close, int period = 14) {
    const auto size = close.size();
    xt::xarray<double> rsi = xt::empty<double>({size});
    rsi.fill(std::numeric_limits<double>::quiet_NaN());

    if (size <=1) return rsi;

    // 计算价格变化
    auto delta = xt::eval(xt::view(close, xt::range(1, xt::placeholders::_)) -
                          xt::view(close, xt::range(0, size - 1)));

    // 计算上涨和下跌幅度
    auto gain = xt::maximum(delta, 0.0);
    auto loss = xt::maximum(-delta, 0.0);

    // 计算SMA(使用EMA变种, M=1)
    auto sma_gain = SMA_TDX(gain, period, 1);
    auto sma_loss = SMA_TDX(loss, period, 1);

    // 计算RSI(从第1天开始计算)
    rsi[0] = 0; // 第0天设为0
    for (int i = 1; i < int(size); ++i) {
        if (sma_loss[i] == 0) {
            rsi[i] = 100.0;
        } else {
            double rs = sma_gain[i] / sma_loss[i];
            rsi[i] = 100.0 - (100.0 / (1.0 + rs));
        }
    }

    return rsi;
}

// 类型特征: 检查是否为 xtensor 布尔表达式
template <typename T>
struct is_xtensor_bool : std::false_type {};

template <>
struct is_xtensor_bool<xt::xarray<bool>> : std::true_type {};

template <typename T>
struct is_xtensor_bool<xt::xexpression<T>> :
    std::is_same<typename std::decay_t<T>::value_type, bool> {};

// 标量版本
template <typename T, typename F>
auto IFF(bool condition, T&& true_expr, F&& false_expr) {
    return condition ? std::forward<T>(true_expr) : std::forward<F>(false_expr);
}

// xtensor 版本
template <typename Cond, typename T, typename F,
    typename = std::enable_if_t<xt::is_xexpression<std::decay_t<Cond>>::value>>
auto IFF(Cond&& condition, T&& true_expr, F&& false_expr) {
    static_assert(
        std::is_same_v<typename std::decay_t<Cond>::value_type, bool>,
        "Condition must be a boolean xtensor expression"
    );
    return xt::where(
        std::forward<Cond>(condition),
        std::forward<T>(true_expr),
        std::forward<F>(false_expr)
    );
}

TEST_CASE("RSI-tdx", "[formula]") {
    // 测试用例
    std::vector<double> close = {
        44.34, 44.09, 44.15, 43.61, 44.33, 44.83, 45.10, 45.42, 45.84,
        46.08, 45.89, 46.03, 45.61, 46.28, 46.28, 46.00, 46.03, 46.41,
        46.22, 45.64, 46.21, 46.25, 45.71, 46.45, 45.78, 45.35, 44.03,
        44.18, 44.22, 44.57, 43.42, 42.66, 43.13
    };

    auto v0 = xt::adapt(close);
    auto ref1 = formula::ref(v0, 1);
    std::cout << "ref1" << ref1 << std::endl;
    auto values = v0 - ref1;
    std::cout << "values" << values << std::endl;
    //auto v1 = IFF(values>0,values, 0);
    auto v1 = formula::max(values, 0);
    std::cout << "v1" << v1 << std::endl;
    auto sma1 = formula::sma(v1, 14, 1);
    std::cout << "sma1" << sma1 << std::endl;
    //xt::xarray<double > v3 = IFF(values>=0,values, -values);
    auto v3 = formula::abs(values);
    std::cout << "v3" << v3 << std::endl;
    //std::fill(v3.begin(), v3.begin()+1, 0);
    std::cout << "v3" << v3 << std::endl;
    auto sma2 = formula::sma(v3, 14, 1);
    std::cout << "sma2" << sma2 << std::endl;

    auto rsi = sma1 / sma2 * 100;

    // 打印前14个周期和后14个周期的RSI值
    std::cout << "RSI(14)结果:\n";
    for (size_t i = 0; i < close.size(); ++i) {
        std::cout << "Day " << i << ": Close=" << close[i]
                  << " RSI=" << rsi[i] << std::endl;
    }
}


TEST_CASE("median", "[formula]") {
    // 测试用例1: 奇数长度
    xt::xarray<int> arr1 = {5, 3, 1, 4, 2};
    std::cout << "降序排序后的中位数 (" << arr1 << ") = "
              << formula::median(arr1) << std::endl;
}

TEST_CASE("stddev", "[formula]") {
    // 测试数据
    xt::xarray<double> prices = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // 计算5日滚动标准差
    auto std_5day = formula::rolling_std(prices, 5);

    std::cout << "5日滚动标准差:\n" << std_5day << std::endl;

    // 使用优化版本
    auto std_5day_opt = formula::rolling_std_optimized(prices, 5);
    std::cout << "优化后的5日滚动标准差:\n" << std_5day_opt << std::endl;
}

// 简单实现(前N-1个为NaN)
xt::xarray<double> rolling_std_simple(const xt::xarray<double>& data, size_t period) {
    // 在函数开头添加: 
    if (period <= 1) {
        xt::xarray<double> result = xt::full_like(data, std::numeric_limits<double>::quiet_NaN());
        return result;
    }
    xt::xarray<double> result = xt::empty<double>(data.shape());

    // 前period-1个位置填NaN
    for(size_t i=0; i<period-1 && i<data.size(); ++i) {
        result(i) = std::numeric_limits<double>::quiet_NaN();
    }

    // 计算有效窗口
    for(size_t i=period-1; i<data.size(); ++i) {
        double sum = 0.0;
        double sum_sq = 0.0;

        // 计算窗口内统计量
        for(size_t j=i-period+1; j<=i; ++j) {
            sum += data(j);
            sum_sq += data(j) * data(j);
        }

        double mean = sum / period;
        double variance = (sum_sq - sum * mean) / period; // 总体方差
        result(i) = std::sqrt(variance);
    }
    return result;
}

// 优化实现(前N-1个为NaN)
xt::xarray<double> rolling_std_optimized(const xt::xarray<double>& data, size_t period) {
    // 1. 初始化全NaN结果
    xt::xarray<double> result = xt::empty<double>(data.shape());
    std::fill_n(result.begin(), std::min(period-1, data.size()),
                std::numeric_limits<double>::quiet_NaN());

    // 2. 主计算逻辑
    if (data.size() >= period && period > 1) {
        // 2.1 使用内存连续访问
        const double* ptr = data.data();
        double* out_ptr = result.data();

        // 2.2 初始窗口计算
        double sum = 0.0, sum_sq = 0.0;
        for (size_t i = 0; i < period; ++i) {
            const double val = ptr[i];
            sum += val;
            sum_sq += val * val;
        }

        // 2.3 计算第一个窗口
        const double inv_period = 1.0 / period;
        out_ptr[period-1] = std::sqrt((sum_sq - sum * sum * inv_period) * inv_period);

        // 2.4 滑动窗口更新
        for (size_t i = period; i < data.size(); ++i) {
            const double outgoing = ptr[i-period];
            const double incoming = ptr[i];

            // 增量更新
            const double delta = incoming - outgoing;
            sum += delta;
            sum_sq += delta * (incoming + outgoing);

            // 计算标准差
            out_ptr[i] = std::sqrt((sum_sq - sum * sum * inv_period) * inv_period);
        }
    }

    return result;
}

TEST_CASE("滚动标准差验证") {
    xt::xarray<double> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    SECTION("period=5") {
        auto result = rolling_std_optimized(data, 5);

        // 前4个应为NaN
        REQUIRE(std::isnan(result[0]));
        REQUIRE(std::isnan(result[3]));

        // 验证计算结果
        REQUIRE(result[4] == Catch::Approx(1.4142135623730951).epsilon(1e-12)); // [1,2,3,4,5]
        REQUIRE(result[9] == Catch::Approx(1.4142135623730951).epsilon(1e-12)); // [6,7,8,9,10]
    }

    SECTION("边界情况") {
        REQUIRE(xt::all(xt::isnan(rolling_std_optimized(data, 1))));
        REQUIRE(xt::all(xt::isnan(rolling_std_optimized(xt::xarray<double>{1,2}, 5))));
    }
}

TEST_CASE("滚动标准差性能基准测试", "[benchmark][rolling_std]") {
#ifdef _WIN32
    // 设置控制台输出和输入代码页为UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    // 1. 准备测试数据
    const size_t data_size = 1'000'000;
    xt::xarray<double> data = xt::random::randn<double>({data_size});

    // 2. 预热缓存(避免冷启动误差)
    volatile auto warmup = rolling_std_optimized(data, 20); (void)warmup;

    // 3. 正式测试
    SECTION("默认周期(20)") {
        BENCHMARK("优化实现") {
                                  auto result = rolling_std_optimized(data, 20);
#ifdef VALIDATE_DURING_BENCHMARK
                                  REQUIRE(result[50] == Approx(/*预期值*/).epsilon(1e-9));
#endif
                                  return result;
                              };

        BENCHMARK("简单实现") {
                                  return rolling_std_simple(data, 20);
                              };
    }

    // 4. 多周期性能分析
    SECTION("不同周期对比") {
        for (size_t period : {5, 20, 60, 250}) {
            BENCHMARK("period=" + std::to_string(period)) {
                                                              return rolling_std_optimized(data, period);
                                                          };
        }
    }

        // 5. 内存访问模式测试
    SECTION("缓存友好性测试") {
        // 准备独立数据块(避免视图带来的潜在问题)
        std::vector<xt::xarray<double>> chunks = {
            xt::random::randn<double>({1'000}),
            xt::random::randn<double>({10'000}),
            xt::random::randn<double>({100'000}),
            xt::random::randn<double>({1'000'000})
        };

        // 预热缓存
        for (auto& chunk : chunks) {
            volatile auto warmup = rolling_std_optimized(chunk, 20);
            (void)warmup;
        }

        // 正式测试
        for (size_t i = 0; i < chunks.size(); ++i) {
            auto size = chunks[i].size();
            BENCHMARK("size=" + std::to_string(size)) {
                                                          // 确保每次测试使用独立内存区域
                                                          auto chunk_copy = chunks[i];
                                                          return rolling_std_optimized(chunk_copy, 20);
                                                      };
        }
    }
}

class WelfordStdDev {
private:
    double mean = 0.0;   // 当前均值
    double M2 = 0.0;     // 平方差的累积和
    int count = 0;       // 已处理的数据点数量

public:
    // 更新统计量(添加一个新数据点)
    void update(double newValue) {
        // 1. 更新数据点计数
        count++;

        // 2. 计算当前值与旧均值的差
        //    delta = xₙ - μₙ₋₁
        double delta = newValue - mean;

        // 3. 更新均值(递推公式)
        //    μₙ = μₙ₋₁ + (xₙ - μₙ₋₁)/n
        mean += delta / count;

        // 4. 计算当前值与新均值的差
        //    delta2 = xₙ - μₙ
        double delta2 = newValue - mean;

        // 5. 更新平方和(关键步骤)
        //    M2ₙ = M2ₙ₋₁ + (xₙ - μₙ₋₁)(xₙ - μₙ)
        M2 += delta * delta2;

        /* 数学解释: 
         * 这里使用 delta * delta2 而不是 delta2² 是为了数值稳定性
         * 展开后: 
         * (xₙ - μₙ₋₁)(xₙ - μₙ)
         * = (xₙ - μₙ₋₁)(xₙ - μₙ₋₁ - (μₙ - μₙ₋₁))
         * = (xₙ - μₙ₋₁)² - (xₙ - μₙ₋₁)(δ/n)
         * 其中 δ = xₙ - μₙ₋₁
         * = δ² - δ²/n = δ²(1 - 1/n) = δ²((n-1)/n)
         * 这种形式避免了直接计算大数的平方
         */
    }

    // 获取当前均值
    double getMean() const {
        return (count > 0) ? mean : numeric::NaN; // 返回NaN如果无数据
    }

    // 获取总体方差
    double getVariancePopulation() const {
        return (count > 0) ? M2 / count : numeric::NaN;  // NaN如果无数据
    }

    // 获取样本方差(无偏估计)
    double getVarianceSample() const {
        return (count > 1) ? M2 / (count - 1) : numeric::NaN;  // NaN如果数据不足
    }

    // 获取总体标准差
    double getStdDevPopulation() const {
        return std::sqrt(getVariancePopulation());
    }

    // 获取样本标准差
    double getStdDevSample() const {
        return std::sqrt(getVarianceSample());
    }

    // 合并两个独立计算的统计量(用于并行计算)
    void combine(const WelfordStdDev& other) {
        if (other.count == 0) return;

        // 合并后的总数据点数
        int new_count = count + other.count;

        // 计算两个均值之间的差
        double delta = other.mean - mean;

        // 计算合并后的均值
        // μ_new = (n₁μ₁ + n₂μ₂)/(n₁+n₂)
        double new_mean = mean + delta * other.count / new_count;

        // 计算合并后的M2(关键步骤)
        // M2_new = M2₁ + M2₂ + δ²n₁n₂/(n₁+n₂)
        double new_M2 = M2 + other.M2 +
                        delta * delta * count * other.count / new_count;

        // 更新统计量
        count = new_count;
        mean = new_mean;
        M2 = new_M2;

        /* 数学解释: 
         * 合并公式来源于: 
         * 总平方和 = Σ(x - μ_new)²
         *          = Σ₁(x - μ₁ + μ₁ - μ_new)² + Σ₂(x - μ₂ + μ₂ - μ_new)²
         *          = [M2₁ + n₁(μ₁ - μ_new)²] + [M2₂ + n₂(μ₂ - μ_new)²]
         * 代入 μ_new = (n₁μ₁ + n₂μ₂)/(n₁+n₂)
         * 经过化简得到上述合并公式
         */
    }
};

TEST_CASE("stddev-Welford", "[formula]") {
// 示例数据集
    std::vector<double> data = {1, 2, 3, 4, 5};

    // 使用Welford算法计算统计量
    WelfordStdDev calculator;
    for (double value : data) {
        calculator.update(value);
    }

    // 输出结果
    std::cout << "计算结果:" << std::endl;
    std::cout << "数据点数量: " << data.size() << std::endl;
    std::cout << "平均值: " << calculator.getMean() << std::endl;
    std::cout << "总体方差: " << calculator.getVariancePopulation() << std::endl;
    std::cout << "样本方差: " << calculator.getVarianceSample() << std::endl;
    std::cout << "总体标准差: " << calculator.getStdDevPopulation() << std::endl;
    std::cout << "样本标准差: " << calculator.getStdDevSample() << std::endl;
}

// 实现CROSS函数 - 修复版本
xt::xarray<bool> CROSS(const xt::xarray<double>& S1, const xt::xarray<double>& S2) {
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
        result(i) = (S1(i-1) < S2(i-1)) && (S1(i) > S2(i));
    }

    return result;
}

// 或者使用更安全的向量化实现
xt::xarray<bool> CROSS_vectorized(const xt::xarray<double>& S1, const xt::xarray<double>& S2) {
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
    result(0) = false;

    // 使用显式循环合并条件
    for (size_t i = 0; i < cond1.size(); ++i) {
        result(i+1) = cond1(i) && cond2(i);
    }

    return result;
}


TEST_CASE("cross-vectorized", "[formula]") {
    // 示例数据
    xt::xarray<double> S1 = {1.0, 2.0, 3.0, 4.0, 5.0};
    xt::xarray<double> S2 = {3.0, 2.5, 2.8, 4.5, 4.0};

    // 计算交叉点
    auto cross_points = CROSS_vectorized(S1, S2);

    // 输出结果
    std::cout << "S1: " << S1 << std::endl;
    std::cout << "S2: " << S2 << std::endl;
    std::cout << "Cross points: " << cross_points << std::endl;
}

xt::xarray<bool> CROSS_optimized(const xt::xarray<double>& S1, const xt::xarray<double>& S2) {
    if (S1.shape() != S2.shape()) {
        throw std::runtime_error("Input arrays must have the same shape");
    }

    xt::xarray<bool> result = xt::xarray<bool>::from_shape(S1.shape());

    // 使用xtensor的element-wise操作
    auto shifted_S1 = xt::view(S1, xt::range(1, xt::placeholders::_));
    auto shifted_S2 = xt::view(S2, xt::range(1, xt::placeholders::_));
    auto prev_S1 = xt::view(S1, xt::range(0, -1));
    auto prev_S2 = xt::view(S2, xt::range(0, -1));

    // 显式转换为bool类型
    auto cross_cond = xt::cast<bool>(xt::less(prev_S1, prev_S2)) &
                      xt::cast<bool>(xt::greater(shifted_S1, shifted_S2));

    result(0) = false;
    xt::view(result, xt::range(1, xt::placeholders::_)) = cross_cond;

    return result;
}

TEST_CASE("cross-optimized", "[formula]") {
    // 示例数据
    xt::xarray<double> S1 = {1.0, 2.0, 3.0, 4.0, 5.0};
    xt::xarray<double> S2 = {3.0, 2.5, 2.8, 4.5, 4.0};

    // 计算交叉点
    auto cross_points = CROSS_optimized(S1, S2);

    // 输出结果
    std::cout << "S1: " << S1 << std::endl;
    std::cout << "S2: " << S2 << std::endl;
    std::cout << "Cross points: " << cross_points << std::endl;
}

TEST_CASE("cross-release", "[formula]") {
    // 示例数据
    xt::xarray<double> S1 = {1.0, 2.0, 3.0, 4.0, 5.0};
    xt::xarray<double> S2 = {2.5, 2.8, 4.5, 4.0};

    // 计算交叉点
    auto cross_points = formula::cross(S1, S2);

    // 输出结果
    std::cout << "S1: " << S1 << std::endl;
    std::cout << "S2: " << S2 << std::endl;
    std::cout << "Cross points: " << cross_points << std::endl;
}

template <class E>
xt::xarray<int> bars_last_count(const xt::xexpression<E>& e) {
    // 获取表达式的实际类型
    using value_type = typename std::remove_reference_t<decltype(e.derived_cast())>::value_type;

    // 如果表达式已经是 bool 类型, 则直接使用
    xt::xarray<bool> condition;
    if constexpr (std::is_same_v<value_type, bool>) {
        condition = xt::eval(e.derived_cast());
    } else {
        // 否则判断是否不等于 0(适用于数值类型)
        condition = xt::eval(e.derived_cast() != 0);
    }

    // 计算连续满足条件的周期数
    xt::xarray<int> result = xt::zeros<int>(condition.shape());
    int count = 0;

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

TEST_CASE("bars_last_count", "[formula]") {
    // 模拟一个布尔条件数组
    xt::xarray<bool> cond = {true, true, false, true, true, true, false};

    auto res = bars_last_count(cond);

    std::cout << "BARSLASTCOUNT:\n" << res << std::endl;
}

/**
 * @brief 使用xtensor实现通达信VALUEWHEN函数
 * @param condition 条件数组
 * @param value 取值数组
 * @return 返回结果数组, 形状与输入相同
 */
xt::xarray<double> valueWhen(const xt::xarray<bool>& condition, const xt::xarray<double>& value) {
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
                // 如果之前没有满足条件的值, 返回NaN
                result.flat(i) = std::numeric_limits<double>::quiet_NaN();
            }
        }
    }

    return result;
}

TEST_CASE("value_when", "[formula]") {
    // 示例数据
    xt::xarray<bool> condition = {true, false, false, true, false, true, false};
    xt::xarray<double> values = {10.5, 11.2, 12.3, 13.4, 14.5, 15.6, 16.7};

    auto result = valueWhen(condition, values);

    std::cout << "条件序列: " << condition << std::endl;
    std::cout << "数值序列: " << values << std::endl;
    std::cout << "结果序列: " << result << std::endl;

    // 二维数组示例
    xt::xarray<bool> condition_2d = {{true, false}, {false, true}, {true, false}};
    xt::xarray<double> values_2d = {{1.1, 2.2}, {3.3, 4.4}, {5.5, 6.6}};

    auto result_2d = valueWhen(condition_2d, values_2d);

    std::cout << "\n二维条件序列:\n" << condition_2d << std::endl;
    std::cout << "二维数值序列:\n" << values_2d << std::endl;
    std::cout << "二维结果序列:\n" << result_2d << std::endl;
}