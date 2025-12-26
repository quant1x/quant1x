#include <quant1x/proto/data.h>
#include <quant1x/config/base.h>
#include <quant1x/config/cache.h>
#include <quant1x/test/test.h>

TEST_CASE("load-trans", "[chips]") {
}

TEST_CASE("load-chips", "[chips]") {
    std::string securityCode = "sh510050";
    std::string factor_date  = "2025-07-17";
    auto        ofn          = config::get_chip_distribution_filename(securityCode, factor_date);
    std::cout << ofn << std::endl;
    datasets::Chips chips{};
    std::ifstream   is(ofn, std::ios::binary);
    bool            ok = chips.ParseFromIstream(&is);
    std::cout << ok << std::endl;
    auto &date = chips.date();
    std::cout << date << std::endl;
    std::cout << chips.dist().size() << std::endl;
    f64 vol = 0;
    for (auto &pl : chips.dist()) {
        std::cout << pl.DebugString() << std::endl;
        vol += (pl.sell() + pl.buy());
    }
    std::cout << vol << std::endl;
}

#include <quant1x/factors/base.h>
#include <quant1x/factors/f10.h>
#include <quant1x/std/safe.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// TechSignal 技术信号位掩码 (支持组合信号)
typedef uint64_t TechSignal;

const TechSignal ShortTermRebound  = 1 << 0;  // 短线止跌     (0000 0001)
const TechSignal ShortTermBreakout = 1 << 1;  // 短线突破     (0000 0010)
const TechSignal VolumeBreakout    = 1 << 2;  // 放量突破     (0000 0100)
const TechSignal VolumeBreakdown   = 1 << 3;  // 放量破位     (0000 1000)
const TechSignal StrongSupport     = 1 << 4;  // 强支撑       (0001 0000)
// 可继续扩展其他信号...

// 组合信号示例
//const TechSignal ReboundWithSupport = ShortTermRebound | StrongSupport;    // 0001 0001
//const TechSignal BreakoutSignals    = ShortTermBreakout | VolumeBreakout;  // 0000 0110

// Has 判断是否包含某信号
bool TechSignalHas(TechSignal ts, TechSignal signal) {
    return (ts & signal) != 0;
}

// Add 添加信号
void TechSignalAdd(TechSignal *ts, TechSignal signal) {
    *ts |= signal;
}

// Remove 移除信号
void TechSignalRemove(TechSignal *ts, TechSignal signal) {
    *ts &= ~signal;
}

// 转换为可读字符串
std::string TechSignalToString(TechSignal ts) {
    std::stringstream builder;
    if (TechSignalHas(ts, ShortTermRebound)) {
        builder << "短线止跌|";
    }
    if (TechSignalHas(ts, ShortTermBreakout)) {
        builder << "短线突破|";
    }
    if (TechSignalHas(ts, VolumeBreakout)) {
        builder << "放量突破|";
    }
    if (TechSignalHas(ts, VolumeBreakdown)) {
        builder << "放量破位|";
    }
    if (TechSignalHas(ts, StrongSupport)) {
        builder << "强支撑|";
    }
    std::string str = builder.str();
    if (!str.empty()) {
        str = str.substr(0, str.size() - 1);  // 去除末尾的 |
    }
    return str;
}

// DailyData 日线数据结构
struct DailyData {
    datasets::KLine kline;
    double          TurnoverRate;
    double          Avg;
};

// PeakSignal 峰值信号
struct PeakSignal {
    double Closest            = 0;  // 最近峰值价格
    double Extremum           = 0;  // 极值价格, 筹码最密集的价位
    double CurrentToPeakVol   = 0;  // 当前价格到峰值的累计筹码
    double CurrentToPeakRatio = 0;  // 当前价格到峰值的累计筹码占比 (0~1)
    double PeakVolume         = 0;  // 峰值对应筹码量
    double PeakRatio          = 0;  // 峰值价位本身的筹码占比 (0~1)
    double VolumeInRange      = 0;  // 当前区域(上 or 下) 筹码总量
    int    AvgHoldDays        = 0;  // 峰值区平均持仓天数
    double Concentration      = 0;  // 筹码集中度 (标准差)

    friend std::ostream &operator<<(std::ostream &os, const PeakSignal &peakSignal) {
        os << "{"
           << "Closest:" << peakSignal.Closest
           << ", Extremum:" << peakSignal.Extremum
           << ", CurrentToPeakVol:" << peakSignal.CurrentToPeakVol
           << ", CurrentToPeakRatio:" << peakSignal.CurrentToPeakRatio
           << ", PeakVolume:" << peakSignal.PeakVolume
           << ", PeakRatio:" << peakSignal.PeakRatio
           << ", VolumeInRange:" << peakSignal.VolumeInRange
           << ", AvgHoldDays:" << peakSignal.AvgHoldDays
           << ", Concentration:" << peakSignal.Concentration
           << "}";
        return os;
    }
};

// Config 计算配置参数
struct Config {
    double PriceStep;     // 价格最小变动单位
    double DecayFactor;   // 衰减系数
    int    ModelType;     // 计算模型 (1:三角形 2:均匀)
    int    SearchWindow;  // 峰值搜索窗口大小
};

const Config defaultConfig = {
    0.01,    // PriceStep
    0.9995,  // DecayFactor
    1,       // ModelType
    5        // SearchWindow
};

const double minValidVolume = 100;   // 最小有效成交量阈值
const int    maxIterations  = 10000;  // 最大迭代次数防止死循环
const int    yearPeriod     = 1;

// ChipDistribution 筹码分布计算器
class ChipDistribution {
private:
    std::map<double, double> chip_;           // 当前筹码分布
    std::vector<DailyData>   data_;           // 日线数据
    Config                   config_;         // 计算配置
    double                   capital_;        // 流通股本
    int                      digits_;         // 小数点位数
    double                   holding_price_;  // 目标价格
    double                   last_close_;     // 最新收盘
    double                   high_;           // 最高价
    double                   low_;            // 最低价

public:
    // NewChipDistribution 创建新的筹码分布计算实例
    ChipDistribution(Config cfg) {
        if (cfg.PriceStep <= 0) {
            cfg.PriceStep = 0.01;
        }
        if (cfg.SearchWindow <= 0) {
            cfg.SearchWindow = 3;
        }

        config_ = cfg;
        digits_ = 2;  // 默认2位小数点
    }

    double LastClose() const { return last_close_; }

    // FiveYearsAgoJanFirst 获取五年前的1月1日零点
    static std::tm FiveYearsAgoJanFirst() {
        std::time_t now       = std::time(nullptr);
        std::tm     tm_now    = safe::localtime(now);
        std::tm     tm_result = tm_now;
        tm_result.tm_year -= yearPeriod;
        tm_result.tm_mon  = 0;  // January
        tm_result.tm_mday = 1;
        tm_result.tm_hour = 0;
        tm_result.tm_min  = 0;
        tm_result.tm_sec  = 0;
        return tm_result;
    }

    // 加载数据
    bool LoadCSV(const std::string &code, const std::string &date) {
        auto klines = factors::checkout_klines(code, date);
        if (klines.empty()) {
            return false;
        }
        auto f10 = factors::get_f10(code, date);
        if (!f10.has_value()) {
            return false;
        }

        capital_ = f10->Capital;
        digits_  = f10->DecimalPoint;

        double high      = -std::numeric_limits<double>::infinity();
        double low       = std::numeric_limits<double>::infinity();
        double lastClose = 0.00;

        // 计算数据的有效起始日期
        std::tm activeDeadlineTm = FiveYearsAgoJanFirst();
        char    activeDeadline[11];
        std::strftime(activeDeadline, sizeof(activeDeadline), "%Y-%m-%d", &activeDeadlineTm);

        // 预分配切片容量
        data_.clear();
        data_.reserve(klines.size());
        for (const auto &record : klines) {
            if (record.date < activeDeadline) {
                continue;
            }
            DailyData d;
            d.kline        = record;
            d.TurnoverRate = 100 * (record.volume / capital_);  // 换手率计算
            d.Avg          = record.amount / record.volume;     // 平均成交价
            d.kline.open   = numeric::decimal(d.kline.open, digits_);
            d.kline.close  = numeric::decimal(d.kline.close, digits_);
            d.kline.high   = numeric::decimal(d.kline.high, digits_);
            d.kline.low    = numeric::decimal(d.kline.low, digits_);
            d.Avg          = numeric::decimal(d.Avg, digits_);  // 平均价四舍五入
            if (d.kline.high > high) {
                high = d.kline.high;
            }
            if (d.kline.low < low) {
                low = d.kline.low;
            }
            data_.push_back(d);
            lastClose = d.kline.close;
        }
        data_.shrink_to_fit();
        last_close_ = lastClose;
        if (!std::isinf(high)) {
            high_ = high;
        }
        if (!std::isinf(low)) {
            low_ = low;
        }
        return true;
    }

    double RealVolume(double proportion) const { return capital_ * proportion; }

    // Calculate 执行筹码分布计算
    bool Calculate(int offset = 0) {
        if (data_.empty()) {
            return false;
        }
        chip_.clear();
        size_t N = 0;
        // 确保 N 不超过 vector 的大小
        N = std::min(offset, static_cast<int>(data_.size()));

        // 创建指向尾部 N 个元素的 span
        std::span<DailyData> lastN = N == 0
                               ? std::span(data_)
                               : std::span(data_).subspan(0, data_.size() - N);  // 如果 size < N，也可以取全部
        for (const auto &day : lastN) {
            switch (config_.ModelType) {
                case 1:
                    if (!calculateTriangular(day)) {
                        return false;
                    }
                    break;
                case 2:
                    if (!calculateUniform(day)) {
                        return false;
                    }
                    break;
                default:
                    return false;
            }
        }
        return true;
    }

    void print() const {
        for (auto const [k, v] : this->chip_) {
            std::cout << k << ", " << v << std::endl;
        }
    }

private:
    // 处理单一价格日的特殊逻辑
    void handleSinglePriceDay(const DailyData &day) {
        double singlePrice = numeric::decimal(day.kline.close, digits_);

        // 构造全量筹码分布
        std::map<double, double> tmpChip;
        tmpChip[singlePrice] = day.kline.volume;

        // 应用衰减合并（需特殊处理衰减率）
        DailyData adjustedDay = day;
        // adjustedDay.TurnoverRate = 100; // 强制全量换手
        applyDecayAndMerge(adjustedDay, tmpChip);
    }

    // 三角形分布计算
    bool calculateTriangular(const DailyData &day) {
        // 情况1：处理最高价等于最低价的情况（一字涨停/跌停）
        if (day.kline.high == day.kline.low) {
            // 直接全部分配到唯一价格点
            handleSinglePriceDay(day);
            return true;
        }

        // 情况2：常规价格区间校验
        if (day.kline.high < day.kline.low) {
            return false;
        }

        // 生成价格网格（包含容差处理）
        std::vector<double> priceGrid = generatePriceGrid(day.kline.low, day.kline.high, config_.PriceStep, digits_);
        std::map<double, double> tmpChip;

        // 计算归一化系数（处理可能的零除问题）
        double priceRange = day.kline.high - day.kline.low;
        double h          = 2.0 / priceRange;  // 保证概率密度积分为1

        for (double price : priceGrid) {
            double x1 = price;
            double x2 = price + config_.PriceStep;
            double area;

            // 分情况处理三角形分布
            if (price < day.Avg) {
                // 左三角形处理（包含Avg=Low的边界情况）
                double denominator = day.Avg - day.kline.low;
                if (denominator <= 1e-8) {  // 处理浮点精度误差
                    // 当Avg=Low时退化为矩形分布
                    area = config_.PriceStep * h;
                } else {
                    double y1 = h / denominator * (x1 - day.kline.low);
                    double y2 = h / denominator * (x2 - day.kline.low);
                    area      = config_.PriceStep * (y1 + y2) / 2;
                }
            } else {
                // 右三角形处理（包含Avg=High的边界情况）
                double denominator = day.kline.high - day.Avg;
                if (denominator <= 1e-8) {
                    // 当Avg=High时退化为矩形分布
                    area = config_.PriceStep * h;
                } else {
                    double y1 = h / denominator * (day.kline.high - x1);
                    double y2 = h / denominator * (day.kline.high - x2);
                    area      = config_.PriceStep * (y1 + y2) / 2;
                }
            }

            tmpChip[price] = area * day.kline.volume;  // 面积映射到实际成交量
        }

        // 应用衰减和合并
        applyDecayAndMerge(day, tmpChip);
        return true;
    }

    // 均匀分布计算
    bool calculateUniform(const DailyData &day) {
        std::vector<double> priceGrid = generatePriceGrid(day.kline.low, day.kline.high, config_.PriceStep, digits_);
        double              eachVol   = day.kline.volume / priceGrid.size();
        std::map<double, double> tmpChip;

        for (double price : priceGrid) {
            tmpChip[price] = eachVol;
        }

        applyDecayAndMerge(day, tmpChip);
        return true;
    }

    // 应用衰减并合并筹码
    void applyDecayAndMerge(const DailyData &day, const std::map<double, double> &newChip) {
        double decayRate = day.TurnoverRate / 100 * config_.DecayFactor;
        decayRate        = std::min(decayRate, 1.0);

        // 衰减现有筹码
        for (auto &[price, vol] : chip_) {
            vol *= (1 - decayRate);
        }

        // 合并新筹码
        for (const auto &[price, vol] : newChip) {
            chip_[price] += vol * decayRate;
        }

        // 清理接近零的筹码
        for (auto it = chip_.begin(); it != chip_.end();) {
            if (it->second < minValidVolume) {
                it = chip_.erase(it);
            } else {
                ++it;
            }
        }
    }

    // 生成价格区间网格
    std::vector<double> generatePriceGrid(double low, double high, double step, int digits) {
        if (high < low) {
            std::swap(low, high);
        }
        double scale   = std::pow(10, digits);
        int    lowInt  = static_cast<int>(std::round(low * scale));
        int    highInt = static_cast<int>(std::round(high * scale));
        int    stepInt = static_cast<int>(std::round(step * scale));

        if (stepInt <= 0) {
            stepInt = 1;
        }

        int length = (highInt - lowInt) / stepInt + 1;
        if (length > maxIterations) {
            stepInt = static_cast<int>(std::ceil(static_cast<double>(highInt - lowInt) / maxIterations));
        }
        std::vector<double> grid;
        grid.reserve(length);
        for (int priceInt = lowInt; priceInt <= highInt; priceInt += stepInt) {
            double price = static_cast<double>(priceInt) / scale;
            price        = numeric::decimal(price, digits);  // 确保四舍五入
            grid.push_back(price);
        }
        return grid;
    }

    // 辅助函数：查找局部峰值
    std::vector<double> findLocalPeaks(const std::vector<double> &prices, const std::map<double, double> &data) const {
        std::vector<double> peaks;
        int                 n = int(prices.size());
        if (n < 3) {
            return peaks;
        }

        int windowSize = config_.SearchWindow;
        for (int i = 0; i < n; ++i) {
            int left  = std::max(0, i - windowSize / 2);
            int right = std::min(n - 1, i + windowSize / 2);

            bool   isPeak     = true;
            double currentVol = data.at(prices[i]);
            for (int j = left; j <= right; ++j) {
                if (j == i) {
                    continue;
                }
                if (currentVol <= data.at(prices[j])) {
                    isPeak = false;
                    break;
                }
            }

            if (isPeak) {
                peaks.push_back(prices[i]);
            }
        }
        return peaks;
    }

    std::vector<double> sortMapKeys(const std::map<double, double> &m) {
        std::vector<double> keys;
        keys.reserve(m.size());
        for (const auto &[k, v] : m) {
            keys.push_back(k);
        }
        std::sort(keys.begin(), keys.end());
        return keys;
    }

    std::tuple<double, double, double, size_t>
    findMaxPeak(double current,
                const std::vector<double> &prices,
                const std::map<double, double> &chip_data,
                bool                            isUpper) {
        double maxVol    = 0.0;
        double peakPrice = 0.0;
        // 找出最大的筹码峰
        for (double p : prices) {
            if (chip_data.at(p) > maxVol) {
                maxVol    = chip_data.at(p);
                peakPrice = p;
            }
        }
        double high_peak = peakPrice;
        double low_peak  = current;
        if (high_peak < low_peak) {
            std::swap(high_peak, low_peak);
        }
        double peakVol = 0.0;
        double all     = 0.0;
        size_t width = 0; // 统计价格区间有多少条记录
        for (auto const &[p, v] : chip_data) {
            if (p >= low_peak && p <= high_peak) {
                peakVol += v;
                ++width;
            }
            if (isUpper && current <= p) {
                all += v;
            } else if (!isUpper && current > p) {
                all += v;
            }
        }
        return {peakPrice, peakVol, all, width};
    }

public:
    // 直接使用当前chip
    std::pair<PeakSignal, PeakSignal> FindMainPeaks(double targetPrice) {
        PeakSignal upper{}, lower{};
        if (chip_.empty()) {
            return {upper, lower};
        }

        double total = calculateTotalVolume(chip_);
        if (total <= 0) {
            return {upper, lower};
        }
        holding_price_             = targetPrice;
        std::vector<double> sorted = sortMapKeys(chip_);
        std::vector<double> peaks  = findLocalPeaks(sorted, chip_);

        // 分离上下峰值
        std::vector<double> lowerPeaks, upperPeaks;
        for (const double &p : peaks) {
            if (p < holding_price_) {
                lowerPeaks.push_back(p);
            } else if (p > holding_price_) {
                upperPeaks.push_back(p);
            }
        }

        // 计算特征点
        lower = calculateChipFeature(lowerPeaks, chip_, total, false);
        upper = calculateChipFeature(upperPeaks, chip_, total, true);
        return {upper, lower};
    }

private:
    /**
     * @brief 计算单个特征点信息
     * @param peak_prices 峰值价格列表
     * @param chip_data 筹码数据
     * @param total 总筹码数
     * @param isUpper 是否高价位(压力)区域
     * @return
     */
    PeakSignal calculateChipFeature(const std::vector<double>      &peak_prices,
                                    const std::map<double, double> &chip_data,
                                    double                          total,
                                    bool                            isUpper) {
        PeakSignal feature{};

        if (peak_prices.empty()) {
            return feature;
        }

        // 极值计算
        auto [maxPeakPrice, vol, all, width] = findMaxPeak(holding_price_, peak_prices, chip_data, isUpper);
        feature.VolumeInRange = all;
        auto chip_records = chip_.size();
        feature.Concentration = double(width) / double(chip_records);

        if (maxPeakPrice > 0) {
            feature.Extremum           = maxPeakPrice;
            feature.PeakVolume         = chip_data.at(maxPeakPrice);
            feature.PeakRatio          = feature.PeakVolume / total;
            feature.CurrentToPeakVol   = vol;
            feature.CurrentToPeakRatio = feature.CurrentToPeakVol / total;
        }

        // 最近峰值计算
        double closestPrice = 0.0;
        if (isUpper) {
            closestPrice = *std::min_element(peak_prices.begin(), peak_prices.end());
        } else {
            closestPrice = *std::max_element(peak_prices.begin(), peak_prices.end());
        }

        if (closestPrice > 0) {
            feature.Closest = closestPrice;
            // 如果极值未找到，使用最近峰值的量能
            if (feature.PeakVolume == 0) {
                feature.PeakVolume = chip_data.at(closestPrice);
                feature.PeakRatio  = feature.PeakVolume / total;
            }
        }

        return feature;
    }

    // 计算总筹码量
    double calculateTotalVolume(const std::map<double, double> &data) {
        double total = 0.0;
        for (const auto &[price, vol] : data) {
            total += vol;
        }
        return total;
    }
};

TEST_CASE("chips-v2", "[chips]") {
    std::string code          = "000999";
    std::string date          = "2025-09-05";
    std::string security_code = exchange::CorrectSecurityCode(code);
    std::cout << "当前日期: " << date << ", 证券代码: " << security_code << std::endl;
    ChipDistribution cd(defaultConfig);
    bool             result = cd.LoadCSV(security_code, date);
    if (!result) {
        return;
    }
    result = cd.Calculate();
    if (!result) {
        return;
    }
    f64 targetPrice     = cd.LastClose();
    auto [upper, lower] = cd.FindMainPeaks(targetPrice);

    std::cout << "当前价格: " << targetPrice << " 附近的主要筹码峰:" << std::endl;
    std::printf("压力(上): 最接近=%.2f, 最大=%.2f, 成交量=%.2f股, 占比=%.2f%%\n",
                upper.Closest,
                upper.Extremum,
                cd.RealVolume(upper.CurrentToPeakRatio),
                100 * upper.CurrentToPeakRatio);
    std::printf("支撑(下): 最接近=%.2f, 最大=%.2f, 成交量=%.2f股, 占比=%.2f%%\n",
                lower.Closest,
                lower.Extremum,
                cd.RealVolume(lower.CurrentToPeakRatio),
                100 * lower.CurrentToPeakRatio);
    std::cout << upper << std::endl;
    std::cout << lower << std::endl;

    std::printf("-------------------------------------------\n");
    cd.Calculate(1);
    auto [upper1, lower1] = cd.FindMainPeaks(targetPrice);
    std::printf("压力(上): 最接近=%.2f, 最大=%.2f, 成交量=%.2f股, 占比=%.2f%%\n",
                upper1.Closest,
                upper1.Extremum,
                cd.RealVolume(upper1.CurrentToPeakRatio),
                100 * upper1.CurrentToPeakRatio);
    std::printf("支撑(下): 最接近=%.2f, 最大=%.2f, 成交量=%.2f股, 占比=%.2f%%\n",
                lower1.Closest,
                lower1.Extremum,
                cd.RealVolume(lower1.CurrentToPeakRatio),
                100 * lower1.CurrentToPeakRatio);
    std::cout << upper1 << std::endl;
    std::cout << lower1 << std::endl;
    //cd.print();
}