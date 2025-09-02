#include <quant1x/test/test.h>
#include <quant1x/formula.h>
#ifdef _WIN32
#include <windows.h>
#endif

template <typename E, typename W>
auto rolling(const xt::xexpression<E>& data_expr, const xt::xexpression<W>& window_expr) {
    const auto& data = data_expr.derived_cast();
    const auto& windows = window_expr.derived_cast();

    // 结果容器（与输入数据同形状）
    xt::xarray<double> result = xt::empty<double>(data.shape());

    for (size_t i = 0; i < data.size(); ++i) {
        // 获取当前点的窗口大小
        size_t window_size = windows[i];

        if (i + 1 < window_size) {
            // 窗口不足时填充NaN
            result(i) = std::numeric_limits<double>::quiet_NaN();
        } else {
            // 计算滚动窗口
            auto view = xt::view(data, xt::range(i - window_size + 1, i + 1));
            result(i) = xt::mean(view)(); // 这里可以替换为其他聚合函数
        }
    }

    return result;
}

template <typename AggFunc>
struct rolling_adaptor {
    AggFunc agg;

    template <typename E, typename W>
    auto operator()(const xt::xexpression<E>& data, const xt::xexpression<W>& windows) const {
        const auto& d = data.derived_cast();
        const auto& w = windows.derived_cast();
        xt::xarray<double> result = xt::empty<double>(d.shape());

        for (size_t i = 0; i < d.size(); ++i) {
            size_t window = w[i];
            if (window == 0 || i + 1 < window) {
                result(i) = std::numeric_limits<double>::quiet_NaN();
            } else {
                auto view = xt::view(d, xt::range(i - window + 1, i + 1));
                result(i) = agg(view);
            }
        }
        return result;
    }
};

// 常用聚合函数的快捷方式
auto rolling_mean = rolling_adaptor{[](auto&& v) { return xt::mean(v)(); }};
auto rolling_sum = rolling_adaptor{[](auto&& v) { return xt::sum(v)(); }};
//auto rolling_std = rolling_adaptor{[](auto&& v) { return xt::stddev(v)(); }};

TEST_CASE("rolling-const", "[pandas]") {

    // 示例数据
    xt::xarray<double> prices = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9};

    // 情况1：固定窗口
    xt::xarray<size_t> fixed_window = {3, 3, 3, 3, 3, 3, 3, 3, 3}; // 全部为3
    auto fixed_result = rolling(prices, fixed_window);
    std::cout << "fixed_result: " << fixed_result << std::endl;

    // 情况2：可变窗口
    xt::xarray<size_t> dynamic_window = {1, 2, 3, 4, 3, 2, 3, 4, 5}; // 变化窗口
    auto dynamic_result = rolling_mean(prices, dynamic_window);
    std::cout << "dynamic_result: " << dynamic_result << std::endl;

    // 情况3：边界检查
    xt::xarray<size_t> edge_case_window = {5, 5, 5, 5, 5, 5, 5, 5, 5};
    auto edge_result = rolling_sum(prices, edge_case_window); // 前4个会是NaN
    std::cout << "edge_result: " << edge_result << std::endl;

}

// 统一rolling函数（支持标量和动态窗口）

template <typename DataType, typename WindowType, typename Func>
auto rolling(const xt::xexpression<DataType>& data_expr,
             WindowType&& window,
             Func agg_func)
{
    const auto& data = data_expr.derived_cast();
    using result_type = decltype(agg_func(xt::view(data, xt::range(0,0))));
    xt::xarray<result_type> result = xt::empty<result_type>(data.shape());

    // 初始化全部为NaN
    result.fill(std::numeric_limits<double>::quiet_NaN());

    if constexpr (std::is_integral_v<std::decay_t<WindowType>>) {
        // 标量窗口模式
        size_t window_size = window;
        for (size_t i = window_size - 1; i < data.size(); ++i) {
            auto window_view = xt::view(data, xt::range(i - window_size + 1, i + 1));
            result(i) = agg_func(window_view);
        }
    } else {
        // 动态窗口模式
        const auto& windows = window;
        for (size_t i = 0; i < data.size(); ++i) {
            size_t window_size = windows(i);
            if (window_size == 0 || (i + 1) < window_size) continue;

            size_t start = i + 1 - window_size;
            auto window_view = xt::view(data, xt::range(start, i + 1));
            result(i) = agg_func(window_view);
        }
    }

    return result;
}

// 常用聚合函数快捷方式
namespace rolling_ops {

    inline auto hhv = [](auto&& w) {
        return w.size()>0 ? xt::amax(w)() : std::numeric_limits<double>::quiet_NaN();
    };

    inline auto sum = [](auto&& w) {
        return w.size() > 0 ? xt::sum(w)() : std::numeric_limits<double>::quiet_NaN();
    };

    inline auto mean = [](auto&& w) {
        return w.size() > 0 ? xt::mean(w)() : std::numeric_limits<double>::quiet_NaN();
    };

    inline auto stddev(int ddof=0) {
        return [ddof](auto&& w) {
            return w.size() > ddof ? xt::stddev(w, ddof)()
                                   : std::numeric_limits<double>::quiet_NaN();
        };
    };
}


// 常用聚合函数的lambda包装
const auto sum = [](auto&& w) { return xt::sum(w)(); };
const auto mean = [](auto&& w) { return xt::mean(w)(); };
const auto stddev = [](int ddof = 0) {
    return [ddof](auto&& w) { return xt::stddev(w, ddof)(); };
};

TEST_CASE("rolling-release", "[pandas]") {
    // 示例数据
    xt::xarray<double> data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};


    // 情况1：标量窗口（所有窗口大小相同）
    size_t fixed_window = 5;
    auto fixed_mean = rolling(data, fixed_window, mean);
    auto fixed_std = rolling(data, fixed_window, stddev(0));
    auto fixed_hhv = rolling(data, fixed_window, rolling_ops::hhv);

    std::cout << "Fixed window (size=3) mean:\n" << fixed_mean << "\n";
    std::cout << "Fixed window (size=3) std:\n" << fixed_std << "\n";
    std::cout << "Fixed window (size=3) hhv:\n" << fixed_hhv << "\n";

    // 情况2：动态窗口（每个点有独立窗口大小）
    xt::xarray<size_t> dynamic_windows = {5, 5, 5, 5, 5, 100, 5, 5, 5, 5};
    auto dynamic_sum = rolling(data, dynamic_windows, sum);
    auto dynamic_std = rolling(data, dynamic_windows, stddev(0));

    std::cout << "\nDynamic window sum:\n" << dynamic_sum << "\n";
    std::cout << "Dynamic window std (ddof=0):\n" << dynamic_std << "\n";

    // 情况3：边界情况测试
    xt::xarray<size_t> edge_windows = {0, 10, 3, 100, 2}; // 测试超限窗口
    auto edge_result = rolling(data, edge_windows, mean);
    std::cout << "\nEdge case handling:\n" << edge_result << "\n";
}

TEST_CASE("Rolling Window Benchmark", "[benchmark]") {
    const size_t data_size = 10'000;  // 1万数据点
    const size_t window_size = 20;       // 固定窗口大小

    // 生成随机测试数据（每个SECTION独立生成）
    xt::xarray<double> data;

    BENCHMARK_ADVANCED("Fixed Window Rolling Sum")(Catch::Benchmark::Chronometer meter) {
            data = xt::random::randn<double>({data_size});

            meter.measure([&] {
                return rolling(data, window_size, [](auto&& w) {
                    return xt::sum(w)();
                });
            });
        };

    BENCHMARK_ADVANCED("Dynamic Window Rolling Sum")(Catch::Benchmark::Chronometer meter) {
            data = xt::random::randn<double>({data_size});
            auto windows = xt::ones<size_t>({data_size}) * window_size;

            meter.measure([&] {
                return rolling(data, windows, [](auto&& w) {
                    return xt::sum(w)();
                });
            });
        };
}

TEST_CASE("Rolling stddev(0) Benchmark", "[benchmark]") {
    const size_t data_size = 10'000;  // 100万数据点
    const size_t window_size = 20;       // 固定窗口大小

    // 生成随机测试数据
    xt::xarray<double> data = xt::random::randn<double>({data_size});

    // 定义stddev(0)的聚合函数
    auto stddev0 = [](auto&& w) {
        return w.size() > 0 ? xt::stddev(w, 0)()
                            : std::numeric_limits<double>::quiet_NaN();
    };

    BENCHMARK_ADVANCED("Fixed Window stddev(0)")(Catch::Benchmark::Chronometer meter) {
            meter.measure([&] {
                return rolling(data, window_size, stddev0);
            });
        };
}

// SIMD加速的滚动标准差计算
void rolling_stddev_simd(const double* data, size_t size, size_t window, double* result) {
    // 初始化前window-1个位置为NaN
    for (size_t i = 0; i < window-1; ++i) {
        result[i] = std::numeric_limits<double>::quiet_NaN();
    }

    for (size_t i = window-1; i < size; ++i) {
        // 加载窗口数据到SIMD寄存器
        __m256d sum = _mm256_setzero_pd();
        __m256d sum_sq = _mm256_setzero_pd();

        size_t j = 0;
        // 处理完整的4元素块
        for (; j + 4 <= window; j += 4) {
            __m256d x = _mm256_loadu_pd(&data[i-window+1+j]);
            sum = _mm256_add_pd(sum, x);
            sum_sq = _mm256_add_pd(sum_sq, _mm256_mul_pd(x, x));
        }

        // 处理剩余元素（不足4个）
        double tail_sum = 0.0, tail_sum_sq = 0.0;
        for (; j < window; ++j) {
            double val = data[i-window+1+j];
            tail_sum += val;
            tail_sum_sq += val * val;
        }

        // 合并SIMD和标量结果
        __m128d low = _mm256_extractf128_pd(sum, 0);
        __m128d high = _mm256_extractf128_pd(sum, 1);
        low = _mm_add_pd(low, high);
        double total_sum = _mm_cvtsd_f64(_mm_hadd_pd(low, low)) + tail_sum;

        // 计算方差
        double mean = total_sum / window;
        double variance = (_mm_cvtsd_f64(_mm_hadd_pd(
            _mm256_extractf128_pd(sum_sq, 0),
            _mm256_extractf128_pd(sum_sq, 1))) + tail_sum_sq
                           - 2 * mean * total_sum + window * mean * mean) / window;

        result[i] = std::sqrt(variance);
    }
}

TEST_CASE("Rolling stddev(0) Benchmark Comparison", "[benchmark]") {
    const size_t data_size = 10'000;  // 1万数据点
    const size_t window_size = 20;    // 固定窗口大小

    // 生成随机测试数据
    xt::xarray<double> data = xt::random::randn<double>({data_size});
    std::vector<double> raw_data(data.begin(), data.end());
    std::vector<double> result(data_size);

    // 常规实现
    auto stddev0 = [](auto&& w) {
        return w.size() > 0 ? xt::stddev(w, 0)()
                            : std::numeric_limits<double>::quiet_NaN();
    };

    BENCHMARK("Baseline (xtensor)") {
                                        return rolling(data, window_size, stddev0);
                                    };

    // SIMD加速实现
    BENCHMARK("SIMD (AVX2)") {
                                 rolling_stddev_simd(raw_data.data(), data_size, window_size, result.data());
                                 return result;
                             };
}