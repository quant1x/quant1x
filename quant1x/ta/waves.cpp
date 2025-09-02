#include <quant1x/ta/waves.h>

namespace ta::patterns {

    std::pair<std::vector<point>, std::vector<point>> peaks_and_valleys(const xt::xarray<double> &high,
                                                                        const xt::xarray<double> &low) {
        // 确保输入大小一致
        if (high.shape() != low.shape()) {
            throw std::invalid_argument("High and low arrays must have same size");
        }

        size_t n = high.size();

        // 第一步：计算一阶差分 (向量化操作)
        xt::xarray<int> diff_high = xt::zeros<int>({n});
        xt::xarray<int> diff_low  = xt::zeros<int>({n});

        // 计算差分 (更高效的向量化方式)
        xt::view(diff_high,
                 xt::range(0, n - 1)) = xt::sign(xt::view(high, xt::range(1, n)) - xt::view(high, xt::range(0, n - 1)));
        xt::view(diff_low,
                 xt::range(0, n - 1)) = xt::sign(xt::view(low, xt::range(1, n)) - xt::view(low, xt::range(0, n - 1)));

        // 第二步：处理平台区域 (向量化处理)
        auto process_plateaus = [](xt::xarray<int> &diff) {
            size_t n = diff.size();

            // 前向填充非零值
            int last_non_zero = 0;
            for (size_t i = 0; i < n; ++i) {
                if (diff[i] != 0) {
                    last_non_zero = diff[i];
                } else if (last_non_zero != 0) {
                    diff[i] = last_non_zero;
                }
            }

            // 反向填充剩余零值
            last_non_zero = 0;
            for (size_t i = n; i-- > 0;) {
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
        auto d_high = xt::view(diff_high, xt::range(1, n)) - xt::view(diff_high, xt::range(0, n - 1));
        auto d_low  = xt::view(diff_low, xt::range(1, n)) - xt::view(diff_low, xt::range(0, n - 1));

        // 找出满足条件的索引
        auto peak_indices   = xt::flatten_indices(xt::where(xt::equal(d_high, -2)));
        auto valley_indices = xt::flatten_indices(xt::where(xt::equal(d_low, 2)));

        std::vector<point> peaks;
        peaks.reserve(peak_indices.size());
        std::vector<point> valleys;
        valleys.reserve(valley_indices.size());
        for (auto &idx : peak_indices) {
            // 调整索引(+1)
            ++idx;
            peaks.emplace_back(double(idx), high[idx]);
        }
        for (auto &idx : valley_indices) {
            // 调整索引(+1)
            ++idx;
            valleys.emplace_back(double(idx), low[idx]);
        }
        return {peaks, valleys};
    }
}  // namespace ta::patterns