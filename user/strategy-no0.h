#pragma once
#ifndef QUANT1X_STRATEGIES_NO0_H
#define QUANT1X_STRATEGIES_NO0_H 1

#include <quant1x/engine/strategy.h>

// ======================
// 示例具体策略：0号策略
// ======================
class No0Strategy final : public StrategyBase {
private:
    StrategyMetadata metadata_{
        "0号策略",
        "Quant1X Team",
        "演示策略，用于选股和评估"
    };
    std::vector<bool> buys_;
    std::vector<bool> sells_;

public:
    ModelKind Code() const override { return ModelNo1; }

    StrategyMetadata GetMetadata() const override {
        return metadata_;
    }

    std::string OrderFlag() const override {
        return OrderFlagTail;
    }

    quant1x::error Filter(const config::StrategyParameter& parameter, const Snapshot::Reader& snapshot) const override;

    quant1x::error Filter(const config::StrategyParameter &parameter, const level1::SecurityQuote &snapshot) const override;

    SortedStatus Sort(std::vector<Snapshot> &snapshots) const override {
        (void)snapshots;
        // 示例排序
        return SortedStatus::SortDefault;
    }

    void updateIndicators(const SecurityCode &code) override;

    TradeDirection generateSignal(size_t current_index) override;

    void reset() override;

    // 全量计算评估
    void Evaluate(const SecurityCode &code, ResultInfo &result) const override;
    // 增量计算评估
    void Evaluate(const SecurityCode &code, ResultInfo &result, const Snapshot::Reader &snapshot) const override;

    void Evaluate(const SecurityCode &code, ResultInfo &result, const level1::SecurityQuote &snapshot) const override;
};

#endif //QUANT1X_STRATEGIES_NO0_H
