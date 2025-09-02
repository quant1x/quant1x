#include <quant1x/test/test.h>

#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

static void findPeaksAndValleys(const vector<int64_t> &nums) {
    if (nums.size() < 2) {
        cout << "数组元素不足，无法确定波峰波谷" << endl;
        return;
    }

    vector<int64_t> peaks;    // 存储波峰位置
    vector<int64_t> valleys;  // 存储波谷位置

    int64_t count = int64_t(nums.size());

    for (int64_t i = 1; i < count - 1; ++i) {
        // 检查是否为波峰
        if (nums[i] > nums[i-1] && nums[i] > nums[i+1]) {
            peaks.push_back(i);
        }
            // 检查是否为波谷
        else if (nums[i] < nums[i-1] && nums[i] < nums[i+1]) {
            valleys.push_back(i);
        }
    }

    // 处理边界情况（如果数组两端可能被视为波峰或波谷）
    // 例如：[1,2,1]中2是波峰
    if (nums.size() >= 2) {
        if (nums[0] > nums[1]) {
            peaks.push_back(0);
        } else if (nums[0] < nums[1]) {
            valleys.push_back(0);
        }
        int64_t x = int64_t(nums.size());
        if (nums.back() > nums[x-2]) {
            peaks.push_back(x-1);
        } else if (nums.back() < nums[x-2]) {
            valleys.push_back(x-1);
        }
    }

    // 输出结果
    cout << "波峰位置: ";
    for (auto const & pos : peaks) {
        cout << pos << " ";
    }
    cout << endl;

    cout << "波谷位置: ";
    for (auto const & pos : valleys) {
        cout << pos << " ";
    }
    cout << endl;
}

TEST_CASE("find-peaks-v1", "[peaks]") {
    vector<int64_t> nums = {1, 3, 2, 4, 4, 1, 5, 4, 3, 2, 6};
    cout << "数组: ";
    for (int64_t num : nums) {
        cout << num << " ";
    }
    cout << endl;

    findPeaksAndValleys(nums);
}

class Wave {
private:
    vector<double> data;
    vector<int> diff;
    vector<int> peakIndex;
    vector<int> valleyIndex;

public:
    explicit Wave(const vector<double>& inputData) {
        data = inputData;
        int n = int(data.size());
        diff.resize(n, 0);
        peakIndex.clear();
        valleyIndex.clear();
    }

    // Compare function similar to Go's cmp.Compare
    int compare(double a, double b) {
        if (a < b) return -1;
        else if (a > b) return 1;
        else return 0;
    }

    // Normalize: 前向差分归一化，并处理零值（平台）
    void Normalize() {
        int n = int(data.size());
        for (int i = 0; i < n - 1; ++i) {
            diff[i] = compare(data[static_cast<std::vector<double, std::allocator<double>>::size_type>(i) + 1],
                              data[i]);
        }
        diff[n - 1] = 0; // 最后一位无定义，设为0

        // Step 2: 处理 diff[i] == 0 的情况，用左右趋势填充
        for (int i = 0; i < n - 1; ++i) {
            if (diff[i] == 0) {
                if (i == 0) {
                    // 向后找非零趋势
                    for (int j = i + 1; j < n - 1; ++j) {
                        if (diff[j] != 0) {
                            diff[i] = diff[j];
                            break;
                        }
                    }
                } else if (i == n - 2) {
                    // 最后一点，向前取值
                    diff[i] = diff[i - 1];
                } else {
                    // 取后面的趋势作为当前趋势
                    diff[i] = diff[i + 1];
                }
            }
        }
    }

    // Find: 找波峰波谷，基于一阶差分的变化（类似二阶导数）
    void Find() {
        auto n = int(diff.size());
        for (int i = 0; i < n - 1; ++i) {
            int d = diff[i + 1] - diff[i];

            if (d == -2) {
                // 波峰：由上升(+1)转为下降(-1)，说明该点是局部最大值
                peakIndex.push_back(i + 1);
            } else if (d == 2) {
                // 波谷：由下降(-1)转为上升(+1)，说明该点是局部最小值
                valleyIndex.push_back(i + 1);
            }
        }
    }

    // 获取结果
    vector<int> getPeaks() const {
        return peakIndex;
    }

    vector<int> getValleys() const {
        return valleyIndex;
    }
};

// 对外接口函数
static pair<vector<int>, vector<int>> PeeksAndValleys(const vector<double> &data) {
    if (data.empty()) {
        return make_pair(vector<int>{}, vector<int>{});
    }

    Wave wave(data);
    wave.Normalize();
    wave.Find();

    return make_pair(wave.getPeaks(), wave.getValleys());
}

TEST_CASE("find-peaks-v2", "[peaks]") {
    vector<double> data = {1, 2, 2, 3, 2, 1, 2, 3, 4, 3, 2, 1};

    auto [peaks, valleys] = PeeksAndValleys(data);

    cout << "Data: ";
    for (auto v : data) cout << v << " ";
    cout << endl;

    cout << "Peaks at indexes: ";
    for (auto p : peaks) cout << p << " ";
    cout << endl;

    cout << "Valleys at indexes: ";
    for (auto v : valleys) cout << v << " ";
    cout << endl;
}

//#include <matplot/matplot.h>
//
//using namespace matplot;
//
//TEST_CASE("find-peaks-v3", "[peaks]") {
//
//    // 原始数据
//    std::vector<double> data = {1, 2, 2, 3, 2, 1, 2, 3, 4, 3, 2, 1};
//    auto [peaks, valleys] = PeeksAndValleys(data);
//
//    // 构建 x 坐标
//    std::vector<double> x(data.size());
//    std::iota(x.begin(), x.end(), 0.0);
//
//    // 提取波峰波谷的 x 和 y 值
//    std::vector<double> peak_x, valley_x, peak_y, valley_y;
//    for (int idx : peaks) {
//        peak_x.push_back(static_cast<double>(idx));
//        peak_y.push_back(data[idx]);
//    }
//    for (int idx : valleys) {
//        valley_x.push_back(static_cast<double>(idx));
//        valley_y.push_back(data[idx]);
//    }
//
//    // 绘制原始数据曲线
//    plot(x, data, "-o")->color("blue").line_width(2).marker_size(3);
//
//    // 绘制波峰
//    scatter(peak_x, peak_y)
//        ->color("red")
//        .marker("X")
//        .marker_size(10) // 调整标记点大小
//        .display_name("Peaks");
//
//    // 绘制波谷
//    scatter(valley_x, valley_y)
//        ->color("green")
//        .marker("O")
//        .marker_size(10) // 调整标记点大小
//        .display_name("Valleys");
//
//    // 设置标题和标签
//    title("Peaks and Valleys Detection");
//    xlabel("Index");
//    ylabel("Value");
//    matplot::legend();
//    grid(true);
//
//    // 设置坐标轴范围
//    std::array<double, 2> x_limits = {0., static_cast<double>(data.size() - 1)};
//    std::array<double, 2> y_limits = {
//        *std::min_element(data.begin(), data.end()) - 1.,
//        *std::max_element(data.begin(), data.end()) + 1.
//    };
//    xlim(x_limits);
//    ylim(y_limits);
//
//    // 强制刷新绘图上下文
//    //matplot::flush();
//
//    // 显示图表
//    matplot::show();
//}
//
//TEST_CASE("find-peaks-v4", "[peaks]") {
//    // 原始数据（示例数据，需替换为实际数据）
//    std::vector<double> data = {3, 2.5, 2, 1.5, 1, 1.5, 1.5, 2, 1.5, 2, 1.5};
//    std::vector<double> x(data.size());
//    std::iota(x.begin(), x.end(), 0);
//
//    // 创建图形对象
//    auto f = figure(true);
//    f->size(800, 600);
//
//    // 1. 先绘制蓝色折线（确保在最底层）
//    auto pl = plot(x, data);
//    pl->color("blue")
//        .line_width(1)
//        .display_name("Data");
//
//    // 2. 绘制所有数据点（蓝色空心圆）
//    auto sc = scatter(x, data);
//    sc->marker("o")
//        .marker_size(5)
//        .marker_face_color("none")
//        .marker_face_color("blue")
//        .line_width(1);
//
//    // 3. 强制绘制波谷点（绿色实心圆）
//    std::vector<size_t> valleys = {4, 6, 8, 10}; // 波谷位置索引
//    std::vector<double> vx, vy;
//    for (auto i : valleys) {
//        if (i < data.size()) {
//            vx.push_back(x[i]);
//            vy.push_back(data[i]);
//        }
//    }
//    auto valley_pts = scatter(vx, vy);
//    valley_pts->marker("o")
//        .marker_size(12)
//        .marker_face_color("green")
//        .marker_face_color("black")
//        .line_width(2)
//        .display_name("Valleys");
//
//    // 图表修饰
//    title("Peaks and Valleys Detection");
//    xlabel("Index");
//    ylabel("Value");
//    matplot::legend()->location(legend::general_alignment::topright);
//    grid(true);
//    xlim({0, 10});
//    ylim({0, 5}); // 调高Y轴上限确保显示完整
//
//    // 重要：强制刷新图形缓冲区
//    //matplot::flush();
//    show();
//}
//
//struct KLine {
//    double open;
//    double close;
//    double high;
//    double low;
//};
//
//void drawManualCandle(const std::vector<double>& x,
//                      const std::vector<double>& opens,
//                      const std::vector<double>& closes,
//                      const std::vector<double>& lows,
//                      const std::vector<double>& highs) {
//    // 绘制上下影线
//    for (size_t i = 0; i < x.size(); ++i) {
//        std::vector<double> vx = {x[i], x[i]};
//        std::vector<double> vy = {lows[i], highs[i]};
//        plot(vx, vy)->color("black").line_width(1);
//    }
//
//    // 绘制实体
//    for (size_t i = 0; i < x.size(); ++i) {
//        double width = 0.4;
//        double left = x[i] - width/2;
//        double right = x[i] + width/2;
//
//        std::vector<double> x_box = {left, right, right, left, left};
//        std::vector<double> y_box = {opens[i], opens[i], closes[i], closes[i], opens[i]};
//
//        std::string color = (closes[i] >= opens[i]) ? "red" : "green";
//        fill(x_box, y_box, color);
//        plot(x_box, y_box)->color("black").line_width(1);
//    }
//}
//
//TEST_CASE("find-peaks-v5", "[peaks]") {
//    std::vector<KLine> klines = {
//        {45.3, 47.8, 48.2, 44.5},
//        {47.8, 46.2, 48.0, 45.0},
//        {46.2, 48.5, 49.1, 45.8},
//        {48.5, 49.1, 49.5, 47.0},
//        {49.1, 50.3, 51.0, 48.5},
//        {50.3, 49.7, 50.8, 49.0},
//        {49.7, 51.2, 52.5, 49.5},
//        {51.2, 52.0, 52.8, 50.8},
//        {52.0, 53.5, 54.0, 51.5},
//        {53.5, 52.8, 54.2, 52.0}
//    };
//
//    std::vector<double> x, high, low, opens, closes;
//    for (size_t i = 0; i < klines.size(); ++i) {
//        x.emplace_back(i+1);
//        high.emplace_back(klines[i].high);
//        low.emplace_back(klines[i].low);
//        opens.emplace_back(klines[i].open);
//        closes.emplace_back(klines[i].close);
//    }
//
//    // 创建蜡烛图
//    drawManualCandle(x, opens, closes, low, high);
//    //p->color("red").line_width(1);
//
//    title("K-Line Chart");
//    xlabel("Day");
//    ylabel("Price");
//    grid(on);
//
//    show();
//}

// 比较函数，与Python版本相同
int compare(double a, double b) {
    if (a < b) return -1;
    else if (a > b) return 1;
    else return 0;
}

// 主函数：寻找波峰和波谷
static pair<vector<int>, vector<int>> basic_peaks_and_valleys(const vector<double> &high, const vector<double> &low) {
    /**
     * 输入: high - 高价序列, low - 低价序列
     * 返回: 包含波峰和波谷索引的pair
     * 波峰基于高价序列，波谷基于低价序列
     */

    size_t n = high.size();
    vector<int> diff_high(n, 0);  // 高价差分序列
    vector<int> diff_low(n, 0);   // 低价差分序列

    // 第一步：计算一阶差分
    for (size_t i = 0; i < n - 1; ++i) {
        diff_high[i] = compare(high[i + 1], high[i]);
        diff_low[i] = compare(low[i + 1], low[i]);
    }

    // 第二步：处理平台区域（差分为0的情况）
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

    // 第三步：识别波峰和波谷
    vector<int> peaks;    // 波峰索引
    vector<int> valleys;  // 波谷索引

    for (size_t i = 0; i < n - 1; ++i) {
        int d_high = diff_high[i + 1] - diff_high[i];
        int d_low = diff_low[i + 1] - diff_low[i];

        if (d_high == -2) {  // 高价序列由上升到下降，形成波峰
            peaks.push_back(int(i) + 1);
        }
        if (d_low == 2) {    // 低价序列由下降到上升，形成波谷
            valleys.push_back(int(i) + 1);
        }
    }

    return make_pair(peaks, valleys);
}

TEST_CASE("find-peaks-v6", "[peaks]") {
    vector<double> high_prices = {1.1, 1.2, 1.3, 1.2, 1.1};
    vector<double> low_prices = {1.0, 1.1, 1.2, 1.1, 1.0};
    auto [peaks, valleys] = basic_peaks_and_valleys(high_prices, low_prices);
    std::cout << peaks << std::endl;
    std::cout << valleys << std::endl;
}

#include <quant1x/formula.h>

namespace xt {
    using namespace xt;
}

static std::pair<std::vector<int64_t>, std::vector<int64_t>> peaks_and_valleys_xtensor(
    const xt::xarray<double>& high,
    const xt::xarray<double>& low)
{
    // 确保输入大小一致
    if (high.shape() != low.shape()) {
        throw std::invalid_argument("High and low arrays must have same size");
    }

    size_t n = high.size();

    // 第一步：计算一阶差分 (向量化操作)
    xt::xarray<int64_t> diff_high = xt::zeros<int64_t>({n});
    xt::xarray<int64_t> diff_low  = xt::zeros<int64_t>({n});

    // 计算差分 (更高效的向量化方式)
    xt::view(diff_high, xt::range(0, n-1)) = xt::sign(xt::view(high, xt::range(1, n)) - xt::view(high, xt::range(0, n-1)));
    xt::view(diff_low, xt::range(0, n-1)) = xt::sign(xt::view(low, xt::range(1, n)) - xt::view(low, xt::range(0, n-1)));

    // 第二步：处理平台区域 (向量化处理)
    auto process_plateaus = [](xt::xarray<int64_t> &diff) {
        size_t n = diff.size();

        // 前向填充非零值
        int64_t last_non_zero = 0;
        for (size_t i = 0; i < n; ++i) {
            if (diff[i] != 0) {
                last_non_zero = diff[i];
            } else if (last_non_zero != 0) {
                diff[i] = last_non_zero;
            }
        }

        // 反向填充剩余零值
        last_non_zero = 0;
        for (size_t i = n; i-- > 0; ) {
            if (diff[i] != 0) {
                last_non_zero = diff[i];
            } else if (last_non_zero != 0) {
                diff[i] = last_non_zero;
            }
        }
    };

    process_plateaus(diff_high);
    process_plateaus(diff_low);

    // 第三步：识别波峰和波谷 (向量化操作)
    auto d_high = xt::view(diff_high, xt::range(1, n)) - xt::view(diff_high, xt::range(0, n-1));
    auto d_low = xt::view(diff_low, xt::range(1, n)) - xt::view(diff_low, xt::range(0, n-1));

    // 找出满足条件的索引
    auto peak_indices = xt::flatten_indices(xt::where(xt::equal(d_high, -2)));
    auto valley_indices = xt::flatten_indices(xt::where(xt::equal(d_low, 2)));

    // 转换为std::vector并调整索引(+1)
    std::vector<int64_t> peaks(peak_indices.begin(), peak_indices.end());
    std::vector<int64_t> valleys(valley_indices.begin(), valley_indices.end());

    for (auto& idx : peaks) ++idx;
    for (auto& idx : valleys) ++idx;

    return {peaks, valleys};
}

TEST_CASE("find-peaks-v7", "[peaks]") {
    // 创建测试数据
    xt::xarray<double> high = {1.0, 1.1, 1.2, 1.1, 1.0, 1.2, 1.3, 1.2};
    xt::xarray<double> low = {0.9, 1.0, 1.1, 1.0, 0.9, 1.1, 1.2, 1.1};

    // 调用函数
    auto [peaks, valleys] = peaks_and_valleys_xtensor(high, low);

    // 输出结果
    std::cout << "波峰位置: ";
    for (auto p : peaks) std::cout << p << " ";
    std::cout << "\n波谷位置: ";
    for (auto v : valleys) std::cout << v << " ";
}