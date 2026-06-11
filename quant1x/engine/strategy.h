#pragma once
#ifndef QUANT1X_ENGINE_STRATEGY_H
#define QUANT1X_ENGINE_STRATEGY_H 1

#include <quant1x/data/adapter.h>
#include <quant1x/engine/rule_engine.h>
#include <quant1x/data/market/instruments.h>
#include <quant1x/contrib/data/tdx/client.h>
#include <quant1x/proto/data.h>
#include <quant1x/proto/snapshot.capnp.h>
#include <quant1x/std/except.h>
#include <quant1x/std/numeric.h>
#include <quant1x/trader/fee.h>
#include <quant1x/data/kline.h>

// 前向声明
struct ResultInfo;

using SecurityCode = std::string;
using StrategyPtr  = std::shared_ptr<class StrategyBase>;

// ModelKind 类型定义
using ModelKind = uint64_t;

// 策略类型常量
const ModelKind ModelZeroSum        = 0;           ///< 零和游戏
const ModelKind ModelNo1            = 1;           ///< 一号策略
const ModelKind ModelForceOverwrite = 0x80000000;  ///< 策略配置允许强制覆盖

// 排序状态枚举
enum class SortedStatus {
    SortNotExecuted,  ///< 排序未执行
    SortFinished,     ///< 排序已完成
    SortDefault,      ///< 默认排序
    SortNotRequired   ///< 没有排序要求
};

// 订单标志常量
const std::string OrderFlagHead = "head";
const std::string OrderFlagTick = "tick";
const std::string OrderFlagTail = "tail";
const std::string OrderFlagSell = "sell";

// 元信息结构体
struct StrategyMetadata {
    std::string name;
    std::string author;
    std::string description;
    std::string version = "1.0.0";
};

struct ResultInfo {
    uint64_t         strategy_id;   // 测试id
    std::string      date;          // 日期
    std::string      code;          // 证券代码
    bool             buy;           // 是否买入
    bool             sell;          // 是否卖出
    bool             win;           // 是否隔日盈利
    double           daily_return;  // 隔日收益率
    trader::TradeFee fee_buy;       // 买入费用
    trader::TradeFee fee_sell;      // 卖出费用
    bool             limit_up;      // 涨停板限制, 不打板
};

// ======================
// 各个功能接口定义
// ======================

/**
 * @brief 利用规则过滤标的, 其中包括风控等等, 看规则如何定义
 */
class Filterable {
public:
    virtual ~Filterable() = default;
    virtual quant1x::error Filter(const config::StrategyParameter &parameter, const Snapshot::Reader &snapshot) const {
        (void)parameter;
        (void)snapshot;
        return quant1x::make_error_code(0, "no problem");
    }
    virtual quant1x::error Filter(const config::StrategyParameter &parameter,
                              const level1::SecurityQuote     &snapshot) const = 0;
};

/**
 * @brief 优先交易的权重排序
 */
class Sortable {
public:
    virtual ~Sortable() = default;
    virtual SortedStatus Sort(std::vector<Snapshot> &snapshots) const {
        (void)snapshots;
        return SortedStatus::SortNotRequired;
    }
};

// 交易方向枚举
enum class TradeDirection {
    HOLD,   ///< 不动
    LONG,   ///< 买入（A股）/做多（期货）
    SHORT,  ///< 卖出（A股）/做空（期货）
    FLAT    ///< 平仓
};

/**
 * @brief 输出对标的评估, 例如权重,分值等等
 */
class Evaluatable {
public:
    virtual ~Evaluatable() = default;
    // 全量计算评估
    virtual void Evaluate(const SecurityCode &code, ResultInfo &result) const = 0;
    // 增量计算评估
    virtual void Evaluate(const SecurityCode &code, ResultInfo &result, const Snapshot::Reader &snapshot) const = 0;
    virtual void Evaluate(const SecurityCode &code, ResultInfo &result, const level1::SecurityQuote &snapshot) const = 0;
    // 更新指标数据（如均线）
    virtual void updateIndicators(const SecurityCode &code) = 0;

    // 根据当前市场状态生成交易信号
    virtual TradeDirection generateSignal(size_t current_index) = 0;

    // 重置策略状态（用于新回测）
    virtual void reset() = 0;
};

// 基础信息接口
class StrategyInfo {
public:
    virtual ~StrategyInfo()                      = default;
    virtual ModelKind        Code() const        = 0;
    virtual StrategyMetadata GetMetadata() const = 0;
    virtual std::string      OrderFlag() const   = 0;
    // 通过策略ID返回用于在QMT系统中表示的string类型的策略名称
    std::string QmtStrategyName() const;
};

// ======================
// 组合策略基类（最终继承使用）
// ======================
class StrategyBase : public Filterable, public Sortable, public Evaluatable, public StrategyInfo {
private:
    exchange::timestamp timestamp_;

protected:
    std::vector<data::KLine> market_data_;

public:
    const std::vector<data::KLine> &market_data() const { return market_data_; }

    const exchange::timestamp &getTimestamp() const { return timestamp_; }

    void setTimestamp(const exchange::timestamp &timestamp) { timestamp_ = timestamp.pre_market_time(); }

public:
    std::string DebugString() const {
        const auto &meta = GetMetadata();
        return "Strategy: " + meta.name + ", Code: " + std::to_string(Code()) + ", Version: " + meta.version +
               ", Author: " + meta.author;
    }
};

// ======================
// 策略管理器（单例）
// ======================
class StrategyManager {
private:
    std::unordered_map<ModelKind, StrategyPtr> strategies_;
    std::unordered_map<ModelKind, bool>        overwriteFlags_;
    mutable std::mutex                         mtx_;

public:
    static StrategyManager &Instance() {
        static StrategyManager instance;
        return instance;
    }

    void Register(StrategyPtr strategy) {
        std::lock_guard<std::mutex> lock(mtx_);
        ModelKind                   code = strategy->Code();

        if ((code & ModelForceOverwrite) == ModelForceOverwrite) {
            code &= ~ModelForceOverwrite;
            overwriteFlags_[code] = true;
        } else {
            if (strategies_.find(code) != strategies_.end()) {
                throw std::runtime_error("The strategy already exists");
            }
        }

        strategies_[code] = strategy;
    }

    StrategyPtr GetStrategy(ModelKind code) {
        std::lock_guard<std::mutex> lock(mtx_);
        auto                        it = strategies_.find(code);
        if (it != strategies_.end()) {
            return it->second;
        }
        throw std::runtime_error("The strategy was not found");
    }

    std::vector<ModelKind> ListStrategyCodes() const {
        std::lock_guard<std::mutex> lock(mtx_);
        std::vector<ModelKind>      codes;
        for (const auto &pair : strategies_) {
            codes.push_back(pair.first);
        }
        std::sort(codes.begin(), codes.end());
        return codes;
    }

    std::string UsageStrategyList() const {
        std::stringstream ss;
        auto              codes = ListStrategyCodes();
        for (auto code : codes) {
            auto it = strategies_.find(code);
            if (it != strategies_.end()) {
                ss << code << ": " << it->second->GetMetadata().name << "\n";
            }
        }
        return ss.str();
    }
};

#endif  // QUANT1X_ENGINE_STRATEGY_H
