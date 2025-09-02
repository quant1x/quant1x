#pragma once
#ifndef QUANT1X_DATASETS_XDXR_ADJUST_FACTOR_H
#define QUANT1X_DATASETS_XDXR_ADJUST_FACTOR_H 1

#include <quant1x/level1/xdxr_info.h>
#include <string>
#include <vector>
#include <map>

// =============================
// 统一复权因子模型：仿射变换 P_adj = m * P + a
// =============================
struct AdjustFactor {
    f64 m;  // 比例因子（乘法）
    f64 a;  // 偏移因子（加法）

    [[nodiscard]] f64 Apply(f64 price) const;
    void ApplyVectorized(const f64* in, f64* out, size_t count) const;

    [[nodiscard]] AdjustFactor operator*(const AdjustFactor& other) const;
};

// =============================
// 函数声明
// =============================

int GetEventPriority(int category);
void SortEvents(std::vector<level1::XdxrInfo>& events);

std::map<std::string, std::vector<level1::XdxrInfo>> GroupEventsByDate(const std::vector<level1::XdxrInfo>& events);
std::map<std::string, AdjustFactor> GroupAndCombineEvents(const std::vector<level1::XdxrInfo>& events);
AdjustFactor CombineDailyFactors(const std::map<std::string, AdjustFactor>& dailyFactors);

#endif //QUANT1X_DATASETS_XDXR_ADJUST_FACTOR_H
