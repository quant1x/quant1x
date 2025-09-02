#pragma once
#ifndef QUANT1X_DATASETS_CHIPS_H
#define QUANT1X_DATASETS_CHIPS_H 1

#include "xdxr.h"

namespace datasets {

    struct PriceLine {
        i32 price = 0;  // 价格, 单位厘
        f64 buy   = 0;  // 买入, 成交量, 单位股
        f64 sell  = 0;  // 卖出, 成交量, 单位股
    };

    class DataChips : public cache::DataAdapter {
    public:

        cache::Kind Kind() const override { return BaseChipDistribution; }

        std::string Owner() override { return cache::DefaultDataProvider; }

        std::string Key() const override { return "chips"; }

        std::string Name() const override { return "筹码分布"; }

        std::string Usage() const override { return "筹码分布"; }

        void Print(const std::string &code, const std::vector<exchange::timestamp> &dates) override;

        void Update(const std::string &code, const exchange::timestamp &date) override;
    };
} // namespace datasets

#endif //QUANT1X_DATASETS_CHIPS_H
