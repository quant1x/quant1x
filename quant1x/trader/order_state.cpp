#include <quant1x/trader/order_state.h>
#include <spdlog/spdlog.h>
#include <quant1x/trader/fee.h>
#include <quant1x/exchange/markets.h>
#include <quant1x/engine/strategy.h>
#include <quant1x/trader/order.h>
#include <quant1x/config/base.h>
#include <quant1x/config/cache.h>

namespace trader {
    namespace fs = std::filesystem;

    // 状态文件扩展名
    const std::string orderStateFileExtension = ".done";

    // 获取状态机路径
    std::string state_file_path(const std::string& stateDate) {
        fs::path flagPath = fs::path(config::get_qmt_cache_path()) / "var" / stateDate;
        return flagPath.string();
    }

    // 获取状态文件前缀
    std::string state_file_prefix(const std::string& stateDate,
                                  const std::string& quantStrategyName,
                                  Direction direction) {
        std::string strategyName = strings::to_lower(quantStrategyName);

        std::string prefix = stateDate + "-" + account_id() + "-" +
                             strategyName + "-" + order_flag(direction);
        return prefix;
    }

    // 分拣订单状态字段
    std::pair<std::string, std::string> order_state_fields(const std::string& date,
                                                           const std::string& quantStrategyName,
                                                           Direction direction) {
        std::string stateDate = exchange::timestamp(date).cache_date();
        std::string flagPath = state_file_path(stateDate);
        std::string filenamePrefix = state_file_prefix(stateDate, quantStrategyName, direction);
        return {flagPath, filenamePrefix};
    }

    // 从策略分拣订单状态字段
    std::pair<std::string, std::string> order_state_fields_from_strategy(const std::string& date,
                                                                         const StrategyInfo& model,
                                                                         Direction direction) {
        std::string quantStrategyName = model.QmtStrategyName();
        return order_state_fields(date, quantStrategyName, direction);
    }

    // 获得订单标识文件名
    std::string order_state_filename(const std::string& date,
                                     const StrategyInfo& model,
                                     Direction direction,
                                     const std::string& code) {
        auto [orderFlagPath, filenamePrefix] = order_state_fields_from_strategy(date, model, direction);
        std::string securityCode = exchange::CorrectSecurityCode(code);
        std::string filename = filenamePrefix + "-" + securityCode + orderStateFileExtension;
        fs::path state_filename = fs::path(orderFlagPath) / filename;
        return state_filename.string();
    }

    // 检查订单执行状态
    bool CheckOrderState(const std::string& date,
                         const StrategyInfo& model,
                         const std::string& code,
                         trader::Direction direction) {
        std::string filename = order_state_filename(date, model, direction, code);
        return fs::exists(filename);
    }

    // 推送订单完成状态
    bool PushOrderState(const std::string& date,
                        const StrategyInfo& model,
                        const std::string& code,
                        trader::Direction direction) {
        std::string filename = order_state_filename(date, model, direction, code);
        return io::write_file(filename);
    }

    // 捡出策略订单状态文件列表
    std::vector<std::string> checkoutStrategyOrderFiles(const std::string& date,
                                                        const StrategyInfo& model,
                                                        Direction direction) {
        auto [orderFlagPath, filenamePrefix] = order_state_fields_from_strategy(date, model, direction);
        std::string pattern = filenamePrefix + "-*" + orderStateFileExtension;
        fs::path fullPattern = fs::path(orderFlagPath) / pattern;

        std::vector<std::string> files;
        try {
            for (const auto& entry : fs::directory_iterator(orderFlagPath)) {
                std::string filename = entry.path().filename().string();
                if (filename.find(filenamePrefix) == 0 &&
                    filename.find(orderStateFileExtension) != std::string::npos) {
                    files.push_back(entry.path().string());
                }
            }
        } catch (const fs::filesystem_error& err) {
            spdlog::error(err.what());
            return {};
        }
        return files;
    }

    // 统计策略订单数
    int CountStrategyOrders(const std::string& date,
                            const StrategyInfo& model,
                            trader::Direction direction) {
        std::vector<std::string> files = checkoutStrategyOrderFiles(date, model, direction);
        return static_cast<int>(files.size());
    }

    // 获取指定日期交易的个股列表
    std::vector<std::string> v1FetchListForFirstPurchase(const std::string& date,
                                                       const std::string& quantStrategyName,
                                                       Direction direction) {
        auto [orderFlagPath, filenamePrefix] = order_state_fields(date, quantStrategyName, direction);
        std::vector<std::string> list;

        std::string prefix = filenamePrefix + "-";
        std::string pattern = prefix + "*" + orderStateFileExtension;

        try {
            for (const auto& entry : fs::directory_iterator(orderFlagPath)) {
                std::string filename = entry.path().filename().string();
                if (filename.find(prefix) == 0 &&
                    filename.find(orderStateFileExtension) != std::string::npos) {
                    std::string after = filename.substr(prefix.length());
                    std::string before = after.substr(0, after.length() - orderStateFileExtension.length());
                    list.push_back(before);
                }
            }
        } catch (const fs::filesystem_error& err) {
            spdlog::error(err.what());
            return {};
        }

        return list;
    }

    std::vector<std::string> FetchListForFirstPurchase(
        const std::string& date,
        const std::string& quantStrategyName,
        trader::Direction direction)
    {
        // 1. 保持与Go完全相同的初始化逻辑
        auto [orderFlagPath, filenamePrefix] = order_state_fields(date, quantStrategyName, direction);
        std::vector<std::string> list;

        // 2. 构造与Go完全相同的路径模式
        fs::path prefixPath = fs::path(orderFlagPath) / (filenamePrefix + "-");
        std::string pattern = prefixPath.string() + "*" + orderStateFileExtension; // 实际使用的模式

        try {
            // 3. 精确模拟Go的filepath.Glob行为
            for (const auto& entry : fs::directory_iterator(orderFlagPath)) {
                std::string filename = entry.path().string();

                // 4. 严格按pattern的三个部分进行匹配：
                //    a) 前缀路径 + 策略名前缀 + "-"
                //    b) 中间任意字符（*通配符）
                //    c) 固定扩展名
                if (filename.rfind(prefixPath.string(), 0) == 0 && // 匹配前缀部分
                    filename.size() > prefixPath.string().size() && // 确保有中间部分
                    filename.substr(filename.size() - orderStateFileExtension.size()) == orderStateFileExtension) // 匹配后缀
                {
                    // 5. 精确模拟CutPrefix/CutSuffix
                    std::string after = filename.substr(prefixPath.string().size());
                    std::string before = after.substr(0, after.size() - orderStateFileExtension.size());
                    list.push_back(before);
                }
            }
        } catch (const fs::filesystem_error& err) {
            spdlog::error("File system error: {}", err.what());
            return {};
        }

        return list;
    }

} // namespace trader