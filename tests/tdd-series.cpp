#include <quant1x/test/test.h>

#include <quant1x/pandas/series.h>
#include <quant1x/formula.h>
#include <xtensor/views/xview.hpp>


TEST_CASE("base-1", "[series]") {
    std::vector<int> data1 = {1, 2, 3, 4, 5, 6};
    std::vector<int> data2 = {0, 2, 3, 4, 5, 6};

    ta::Series<int> s1(data1);
    ta::Series<int> s2(data2);

    // shift 示例
    auto shifted = s1.shift(1);
    std::cout << "Shifted:\n";
    for (auto v : shifted.data()) {
        std::cout << v << " ";
    }
    std::cout << "\n";

    // 比较示例
    auto eq = s1 == s2;
    std::cout << "Equal results:\n";
    for (auto v : eq) {
        std::cout << (v ? "true" : "false") << " ";
    }
    std::cout << "\n";

    auto r1 = eq.shift(1);
    std::cout << "Eq shift 1 results:\n";
    for (auto v : r1) {
        std::cout << (v ? "true" : "false") << " ";
    }
    std::cout << "\n";

    auto r2 = r1.align(10);
    std::cout << "Eq algin 10 results:\n";
    for (auto v : r2) {
        std::cout << (v ? "true" : "false") << " ";
    }
    std::cout << "\n";

    // 浮点数测试
    std::vector<double> fdata1 = {1.0, 2.0, NAN, 4.0, 5.0, 6.0};
    std::vector<double> fdata2 = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};

    ta::Series<double> fs1(fdata1);
    ta::Series<double> fs2(fdata2);

    auto feq = fs1 == fs2;
    std::cout << "Float equal:\n";
    for (auto v : feq) {
        std::cout << (v ? "true" : "false") << " ";
    }
    std::cout << "\n";

    auto filled = fs1.fill_nan(999.0);
    std::cout << "Fill NaN with 999:\n";
    for (auto v : filled.data()) std::cout << v << " ";
    std::cout << "\n";
}

TEST_CASE("ref", "[formula]") {
    int period = 1;
    xt::xarray<double> close = {10.1, 10.2, 10.3, 10.4};
    auto values = formula::ref(close, period);
    std::cout << "REF(CLOSE," << period << "): " << values << std::endl;
    // 输出: {nan, 10.1, 10.2, 10.3}
}

TEST_CASE("ma", "[formula]") {
    // 示例数据
    xt::xarray<double> close = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // 计算MA5
    int period = 5;
    auto values = formula::ma(close, period);

    std::cout << "Close prices: " << close << std::endl;
    std::cout << "MA(CLOSE," << period << "):   " << values << std::endl;
}

// 分块大小(根据CPU缓存调整)
constexpr size_t BLOCK_SIZE = 256;

xt::xarray<double> ema_optimized(const xt::xarray<double>& input, int period) {
    if (input.size() == 0 || period <= 0) return input;

    const double alpha = 2.0 / (period + 1);
    xt::xarray<double> output = xt::empty<double>(input.shape());
    output[0] = input[0];

    // 分块处理(减少缓存未命中)
    for (size_t block_start = 1; block_start < input.size(); block_start += BLOCK_SIZE) {
        const size_t block_end = std::min(block_start + BLOCK_SIZE, input.size());

        // 创建局部视图
        auto input_block = xt::view(input, xt::range(block_start, block_end));
        auto output_block = xt::view(output, xt::range(block_start, block_end));

        // 手动展开4次的向量化计算
        const size_t vec_size = input_block.size() - (input_block.size() % 4);
        size_t i = 0;
        for (; i < vec_size; i += 4) {
            const double prev = output[block_start + i - 1];
            output_block[i]   = alpha * input_block[i]   + (1 - alpha) * prev;
            output_block[i+1] = alpha * input_block[i+1] + (1 - alpha) * output_block[i];
            output_block[i+2] = alpha * input_block[i+2] + (1 - alpha) * output_block[i+1];
            output_block[i+3] = alpha * input_block[i+3] + (1 - alpha) * output_block[i+2];
        }

        // 处理剩余元素
        for (; i < input_block.size(); ++i) {
            output_block[i] = alpha * input_block[i] +
                              (1 - alpha) * output[block_start + i - 1];
        }

        // 强制求值并写回内存(利用xtensor的惰性求值优化)
        xt::eval(output_block);
    }

    return output;
}

TEST_CASE("ema", "[formula]") {
    int period = 7;
    xt::xarray<double> close = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto values = formula::ema(close,period);
    std::cout << "EMA(CLOSE," << period << "): " << values << std::endl;
    // 输出: {1., 1.25, 1.6875, 2.265625, 2.949219, 3.711914, 4.533936, 5.400452, 6.300339}
}

TEST_CASE("ema_optimized", "[formula]") {
    int period = 7;
    xt::xarray<double> close = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto values = ema_optimized(close,period);
    std::cout << "EMA(CLOSE," << period << "): " << values << std::endl;
    // 输出: {1., 1.25, 1.6875, 2.265625, 2.949219, 3.711914, 4.533936, 5.400452, 6.300339}
}


TEST_CASE("EMA Benchmark Comparison", "[bench]") {
    // 生成测试数据(10万条随机K线)
    constexpr size_t data_size = 100'000;
    xt::xarray<double> close = xt::random::rand<double>({data_size}, 0.0, 100.0);
    constexpr int N = 5;

    // 验证结果一致性
    auto res_basic = formula::ema(close, N);
    auto res_opt = ema_optimized(close, N);
    REQUIRE(xt::allclose(res_basic, res_opt, 1e-6));

    // 基准测试
    BENCHMARK("Basic Recursive EMA") {
                                         return formula::ema(close, N);
                                     };

    BENCHMARK("Optimized EMA (Loop Unroll)") {
                                                 return ema_optimized(close, N);
                                             };
}

TEST_CASE("sma-std", "[formula]") {
    int period = 4;
    int m = 1;
    xt::xarray<double> close = {1, 2, 3, 4,};
    std::cout << "CLOSE: " << close << std::endl;
    auto values = formula::sma_standard(close,period, m);
    std::cout << "SMA(CLOSE," << period << "," << m << "): " << values << std::endl;
    // 输出示例(N=4): [NaN, NaN, NaN, 2.5]
}


TEST_CASE("sma-tdx", "[formula]") {
    int period = 4;
    int m = 1;
    xt::xarray<double> close = {1, 2, 3, 4,};
    std::cout << "CLOSE: " << close << std::endl;
    auto values = formula::sma(close,period, m);
    std::cout << "SMA(CLOSE," << period << "," << m << "): " << values << std::endl;
    // 输出: {1, 1.25, 1.6875, 2.265625}
}

xt::xarray<double> sma_optimized(const xt::xarray<double>& S, int N, int M) {
    // 1. 边界检查
    if (S.size() == 0 || N <= 0 || M <= 0 || M > N) {
        return xt::xarray<double>(S.shape(), std::numeric_limits<double>::quiet_NaN());
    }

    // 2. 预分配内存(避免动态扩容)
    xt::xarray<double> sma = xt::empty<double>(S.shape());
    sma[0] = S[0];

    // 3. 预计算权重系数(减少重复除法)
    const double weight_current = static_cast<double>(M) / N;
    const double weight_prev = static_cast<double>(N - M) / N;

    // 4. 循环展开优化(手动展开2次)
    size_t i = 1;
    for (; i + 1 < S.size(); i += 2) {
        sma[i] = S[i] * weight_current + sma[i-1] * weight_prev;
        sma[i+1] = S[i+1] * weight_current + sma[i] * weight_prev;
    }

    // 5. 处理剩余数据
    for (; i < S.size(); ++i) {
        sma[i] = S[i] * weight_current + sma[i-1] * weight_prev;
    }

    return sma;
}

TEST_CASE("sma-tdx-optimized", "[formula]") {
    int period = 4;
    int m = 1;
    xt::xarray<double> close = {1, 2, 3, 4,};
    std::cout << "CLOSE: " << close << std::endl;
    auto values = sma_optimized(close,period, m);
    std::cout << "SMA(CLOSE," << period << "," << m << "): " << values << std::endl;
    // 输出: {1, 1.25, 1.6875, 2.265625}
}

TEST_CASE("SMA Benchmark Comparison", "[bench]") {
    // 生成测试数据(10万条随机K线)
    constexpr size_t data_size = 100'000;
    xt::xarray<double> close = xt::random::rand<double>({data_size}, 0.0, 100.0);
    constexpr int N = 12, M = 1;

    // 验证结果一致性
    auto res_basic = formula::sma(close, N, M);
    auto res_opt = sma_optimized(close, N, M);
    REQUIRE(xt::allclose(res_basic, res_opt, 1e-6));

    // 基准测试
    BENCHMARK("Basic Recursive SMA") {
                                         return formula::sma(close, N, M);
                                     };

    BENCHMARK("Optimized SMA (Loop Unroll)") {
                                                 return sma_optimized(close, N, M);
                                             };
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

TEST_CASE("IFF Function Test", "[formula]") {
    // 标量测试
    SECTION("Scalar Input") {
        bool cond = true;
        double t = 5.0, f = 10.0;
        REQUIRE(IFF(cond, t, f) == Catch::Approx(5.0));
    }

        // 数组测试
    SECTION("Array Input") {
        xt::xarray<bool> cond = {true, false, true};
        xt::xarray<int> t = {1, 2, 3};
        xt::xarray<int> f = {4, 5, 6};
        xt::xarray<int> expected = {1, 5, 3};
        REQUIRE(xt::allclose(IFF(cond, t, f), expected));
    }

        // 混合类型测试(需隐式转换)
    SECTION("Mixed Type") {
        xt::xarray<bool> cond = {true, false};
        auto res = IFF(cond, 3.14, xt::xarray<int>{2}); // 返回 double 类型
        REQUIRE(res[0] == Catch::Approx(3.14));
        REQUIRE(res[1] == Catch::Approx(2.0));
    }
}

// 标量版本: 如果条件为 FALSE 返回 true_expr, 否则返回 false_expr
template <typename T, typename F>
auto IFN(bool condition, T&& true_expr, F&& false_expr) {
    return !condition ? std::forward<T>(true_expr) : std::forward<F>(false_expr);
}

// xtensor 版本: 对条件取反后调用 where
template <typename Cond, typename T, typename F,
    typename = std::enable_if_t<xt::is_xexpression<std::decay_t<Cond>>::value>>
auto IFN(Cond&& condition, T&& true_expr, F&& false_expr) {
    static_assert(
        std::is_same_v<typename std::decay_t<Cond>::value_type, bool>,
        "Condition must be a boolean xtensor expression"
    );
    return xt::where(
        !std::forward<Cond>(condition),  // 关键区别: 对条件取反
        std::forward<T>(true_expr),
        std::forward<F>(false_expr)
    );
}

TEST_CASE("IFN Function", "[formula]") {
    SECTION("Scalar Condition") {
        // 数值测试
        REQUIRE(IFN(true, 1.0, 0.0) == 0.0);
        REQUIRE(IFN(false, 3.14, 6.28) == 3.14);

        // 字符串测试(明确类型)
        REQUIRE(IFN(false, std::string("A"), std::string("B")) == "A");
    }

    SECTION("XTensor Condition") {
        xt::xarray<bool> cond = {true, false};
        auto res = IFN(cond, xt::ones<double>({2}), xt::zeros<double>({2}));
        xt::xarray<double> expected = {0.0, 1.0};
        REQUIRE(xt::allclose(res, expected));
    }
}

// 类型特征: 支持数值和字符串
template <typename T>
using is_supported_type = std::disjunction<
    std::is_arithmetic<T>,
    std::is_same<T, std::string>
>;

// 统一的HHV实现
template <typename T>
xt::xarray<T> HHV(const xt::xarray<T>& data, int period) {
    static_assert(is_supported_type<T>::value,
                  "HHV only supports numeric types and std::string");

    if (data.size() == 0 || period <= 0) {
        if constexpr (std::is_arithmetic_v<T>) {
            return xt::xarray<T>(data.shape(), std::numeric_limits<T>::quiet_NaN());
        } else {
            return xt::xarray<T>(data.shape(), T{});
        }
    }

    xt::xarray<T> result = xt::empty<T>(data.shape());
    const size_t n = data.size();

    for (size_t i = 0; i < n; ++i) {
        if (i < static_cast<size_t>(period - 1)) {
            if constexpr (std::is_arithmetic_v<T>) {
                result[i] = std::numeric_limits<T>::quiet_NaN();
            } else {
                result[i] = T{};
            }
        } else {
            auto window = xt::view(data, xt::range(i - period + 1, i + 1));
            if constexpr (std::is_same_v<T, std::string>) {
                result[i] = *std::max_element(
                    window.begin(), window.end(),
                    [](const auto& a, const auto& b) { return a.compare(b) < 0; }
                );
            } else {
                result[i] = *std::max_element(window.begin(), window.end());
            }
        }
    }

    return result;
}

TEST_CASE("hhv", "[formula]") {
    // 数值类型测试
    xt::xarray<double> prices = {10.5, 11.2, 12.3, 11.8, 10.9};
    auto hhv_num = HHV(prices, 3); // 正确: 调用数值版本
    std::cout << "Numeric HHV: " << hhv_num << std::endl;

    // 字符串测试
    xt::xarray<std::string> texts = {"A", "C", "B", "D", "A"};
    auto hhv_str = HHV(texts, 2); // 正确: 调用特化版本
    std::cout << "String HHV: " << hhv_str << std::endl;

    // 非法类型(编译时报错)
    // xt::xarray<bool> flags = {true, false};
    // auto err = HHV(flags, 2); // 错误: static_assert触发
}

// 统一的LLV实现
template <typename T>
xt::xarray<T> v1LLV(const xt::xarray<T>& data, int period) {
    static_assert(is_supported_type<T>::value,
                  "LLV only supports numeric types and std::string");

    if (data.size() == 0 || period <= 0) {
        if constexpr (std::is_arithmetic_v<T>) {
            return xt::xarray<T>(data.shape(), std::numeric_limits<T>::quiet_NaN());
        } else {
            return xt::xarray<T>(data.shape(), T{});
        }
    }

    xt::xarray<T> result = xt::empty<T>(data.shape());
    const size_t n = data.size();

    for (size_t i = 0; i < n; ++i) {
        if (i < static_cast<size_t>(period - 1)) {
            if constexpr (std::is_arithmetic_v<T>) {
                result[i] = std::numeric_limits<T>::quiet_NaN();
            } else {
                result[i] = T{};
            }
        } else {
            auto window = xt::view(data, xt::range(i - (period - 1), i + 1));
            if constexpr (std::is_same_v<T, std::string>) {
                result[i] = *std::min_element(
                    window.begin(), window.end(),
                    [](const auto& a, const auto& b) { return a.compare(b) < 0; }
                );
            } else {
                result[i] = *std::min_element(window.begin(), window.end());
            }
        }
    }

    return result;
}

//#include <xsimd/xsimd.hpp>
//template <typename T>
//xt::xarray<T> LLV(const xt::xarray<T>& data, int period) {
//    static_assert(xt::is_xexpression<std::decay_t<decltype(data)>>::value,
//                  "Input must be an xtensor expression");
//
//    if (data.size() == 0 || period <= 0) {
//        return xt::xarray<T>(data.shape(), std::numeric_limits<T>::quiet_NaN());
//    }
//
//    xt::xarray<T> result = xt::empty<T>(data.shape());
//    const size_t n = data.size();
//
//    for (size_t i = 0; i < n; ++i) {
//        if (i < static_cast<size_t>(period - 1)) {
//            result[i] = std::numeric_limits<T>::quiet_NaN();
//            continue;
//        }
//
//        auto window = xt::view(data, xt::range(i - period + 1, i + 1));
//
//        // 使用xtensor的SIMD优化reduce
//        if constexpr (std::is_arithmetic_v<T>) {
//            result[i] = xt::xsimd::reduce(window,
//                                          [](auto a, auto b) { return xsimd::min(a, b); },
//                                          std::numeric_limits<T>::max()
//            );
//        } else {
//            result[i] = *std::min_element(
//                window.begin(), window.end(),
//                [](const auto& a, const auto& b) { return a.compare(b) < 0; }
//            );
//        }
//    }
//
//    return result;
//}

template <typename T>
xt::xarray<T> x1LLV(const xt::xarray<T>& data, int period) {
    static_assert(xt::is_xexpression<std::decay_t<decltype(data)>>::value,
                  "Input must be an xtensor expression");

    if (data.size() == 0 || period <= 0) {
        return xt::xarray<T>(data.shape(), std::numeric_limits<T>::quiet_NaN());
    }

    xt::xarray<T> result = xt::empty<T>(data.shape());
    const size_t n = data.size();

    for (size_t i = 0; i < n; ++i) {
        if (i < static_cast<size_t>(period - 1)) {
            result[i] = std::numeric_limits<T>::quiet_NaN();
            continue;
        }

        auto window = xt::view(data, xt::range(i - period + 1, i + 1));

        if constexpr (std::is_arithmetic_v<T>) {
            // 手动SIMD优化实现
            using simd_type = xsimd::batch<T>;
            constexpr size_t simd_size = simd_type::size;
            const auto* ptr = window.data();
            const size_t size = window.size();

            simd_type min_vec(std::numeric_limits<T>::max());
            size_t j = 0;

            // SIMD处理对齐部分
            for (; j + simd_size <= size; j += simd_size) {
                min_vec = xsimd::min(min_vec, simd_type::load_aligned(ptr + j));
            }

            // 标量处理剩余部分
            T min_val = xsimd::reduce_min(min_vec);
            for (; j < size; ++j) {
                min_val = std::min(min_val, ptr[j]);
            }

            result[i] = min_val;
        } else {
            result[i] = *std::min_element(
                window.begin(), window.end(),
                [](const auto& a, const auto& b) { return a.compare(b) < 0; }
            );
        }
    }

    return result;
}

TEST_CASE("llv-v1", "[formula]") {
    // 数值类型测试
    xt::xarray<double> prices = {10.5, 11.2, 12.3, 11.8, 10.9};
    std::cout << "origin:" << prices << std::endl;
    auto hhv_num = v1LLV(prices, 3); // 正确: 调用数值版本
    std::cout << "Numeric LLV: " << hhv_num << std::endl;

    // 字符串测试
    xt::xarray<std::string> texts = {"A", "C", "B", "D", "A"};
    std::cout << "origin:" << texts << std::endl;
    auto hhv_str = v1LLV(texts, 2); // 正确: 调用特化版本
    std::cout << "String LLV: " << hhv_str << std::endl;

    // 非法类型(编译时报错)
    // xt::xarray<bool> flags = {true, false};
    // auto err = HHV(flags, 2); // 错误: static_assert触发
}

template <typename T>
xt::xarray<T> LLV_simd(const xt::xarray<T>& data, size_t period) {
    if (data.size() == 0 || period <= 0) {
        return xt::xarray<T>(data.shape(), std::numeric_limits<T>::quiet_NaN());
    }

    xt::xarray<T> result = xt::empty<T>(data.shape());
    const size_t n = data.size();

    for (size_t i = 0; i < n; ++i) {
        if (i < period - 1) {
            result[i] = std::numeric_limits<T>::quiet_NaN();
            continue;
        }

        // 关键修正: 正确计算滑动窗口范围
        auto window = xt::view(data, xt::range(i - (period - 1), i + 1));

        if constexpr (std::is_arithmetic_v<T>) {
            result[i] = *std::min_element(window.begin(), window.end());
        } else {
            result[i] = *std::min_element(
                window.begin(), window.end(),
                [](const auto& a, const auto& b) { return a.compare(b) < 0; }
            );
        }
    }

    return result;
}

TEST_CASE("llv-simd", "[formula]") {
    // 数值类型测试
    xt::xarray<double> prices = {10.5, 11.2, 12.3, 11.8, 10.9};
    std::cout << "origin:" << prices << std::endl;
    auto hhv_num = LLV_simd(prices, 3); // 正确: 调用数值版本
    std::cout << "Numeric LLV: " << hhv_num << std::endl;

    // 字符串测试
    xt::xarray<std::string> texts = {"A", "C", "B", "D", "A"};
    std::cout << "origin:" << texts << std::endl;
    auto hhv_str = LLV_simd(texts, 2); // 正确: 调用特化版本
    std::cout << "String LLV: " << hhv_str << std::endl;

    // 非法类型(编译时报错)
    // xt::xarray<bool> flags = {true, false};
    // auto err = HHV(flags, 2); // 错误: static_assert触发
}

// 实现LLV函数 - 计算N周期内最低值
template <typename E, typename T = typename std::decay_t<E>::value_type>
auto LLV(E&& close, std::size_t N) -> std::enable_if_t<std::is_arithmetic_v<T>, xt::xarray<T>> {
    auto size = close.size();
    xt::xarray<T> result = xt::empty<T>({size});

    // 填充NaN作为初始值
    result.fill(std::numeric_limits<T>::quiet_NaN());

    for (std::size_t i = N - 1; i < size; ++i) {
        std::size_t start = i - N + 1;
        auto window = xt::view(close, xt::range(start, i + 1));
        result[i] = xt::amin(window)();
    }

    return result;
}

// 字符串类型特化版本
template <typename E>
auto LLV(E&& close, std::size_t N) -> std::enable_if_t<
    std::is_same_v<typename std::decay_t<E>::value_type, std::string>,
    xt::xarray<std::string>> {

    auto size = close.size();
    xt::xarray<std::string> result = xt::empty<std::string>({size});

    // 前N-1个元素设为空字符串
    for (std::size_t i = 0; i < N - 1 && i < size; ++i) {
        result[i] = "";
    }

    for (std::size_t i = N - 1; i < size; ++i) {
        std::size_t start = i - N + 1;
        auto window = xt::view(close, xt::range(start, i + 1));

        // 手动查找最小字符串
        auto begin = window.begin();
        auto end = window.end();
        auto min_it = std::min_element(begin, end);
        result[i] = *min_it;
    }

    return result;
}

// 针对固定维度张量的特化版本
template <std::size_t Dim, typename E, typename T = typename std::decay_t<E>::value_type>
auto LLV(E&& close, std::size_t N) {
    using result_type = xt::xtensor<T, Dim>;

    auto shape = close.shape();
    result_type result = xt::zeros<T>(shape);

    for (std::size_t i = 0; i < shape[0]; ++i) {
        std::size_t start = (i >= N - 1) ? i - N + 1 : 0;
        auto window = xt::view(close, xt::range(start, i + 1));
        result[i] = xt::amin(window)();
    }

    return result;
}

TEST_CASE("llv-xtensor", "[formula]") {
    // 数值类型测试
    xt::xarray<double> prices = {10.5, 11.2, 12.3, 11.8, 10.9};
    std::cout << "origin:" << prices << std::endl;
    auto hhv_num = LLV(prices, 3); // 正确: 调用数值版本
    std::cout << "Numeric LLV: " << hhv_num << std::endl;

    // 字符串测试
    xt::xarray<std::string> texts = {"A", "C", "B", "D", "A"};
    std::cout << "origin:" << texts << std::endl;
    auto hhv_str = LLV(texts, 2); // 正确: 调用特化版本
    std::cout << "String LLV: " << hhv_str << std::endl;

    // 非法类型(编译时报错)
    // xt::xarray<bool> flags = {true, false};
    // auto err = HHV(flags, 2); // 错误: static_assert触发
}