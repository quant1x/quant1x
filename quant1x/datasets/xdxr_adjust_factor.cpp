#include <quant1x/datasets/xdxr_adjust_factor.h>

// =============================
// 获取事件优先级（用于排序）
// =============================
int GetEventPriority(int category) {
    switch (category) {
        case 1:     // 现金红利
        case 13:    // 利润分配
        case 14:    // 红利发放
            return 1;
        case 6:     // 行权
            return 2;
        case 2:     // 配股
            return 3;
        case 3:     // 送股
        case 4:     // 转增股本
            return 4;
        case 5:     // 缩股
            return 5;
        default:
            return 6; // 其他类型放最后
    }
}

// =============================
// 对事件按优先级排序
// =============================
void SortEvents(std::vector<level1::XdxrInfo>& events) {
    std::sort(events.begin(), events.end(),
              [](const level1::XdxrInfo& a, const level1::XdxrInfo& b) {
                  return GetEventPriority(a.Category) < GetEventPriority(b.Category);
              });
}

// =============================
// 按日期分组事件
// =============================
std::map<std::string, std::vector<level1::XdxrInfo>> GroupEventsByDate(const std::vector<level1::XdxrInfo>& events) {
    std::map<std::string, std::vector<level1::XdxrInfo>> grouped;

    for (const auto& event : events) {
        grouped[event.Date].push_back(event);
    }

    return grouped;
}

// =============================
// 合并每天的事件为一个统一因子
// =============================
std::map<std::string, AdjustFactor> GroupAndCombineEvents(const std::vector<level1::XdxrInfo>& events) {
    auto grouped = GroupEventsByDate(events);
    std::map<std::string, AdjustFactor> dailyFactors;

    for (auto& [date, dayEvents] : grouped) {
        SortEvents(dayEvents);  // 可选：对当天事件排序

        AdjustFactor composite{1.0, 0.0};
        for (const auto& event : dayEvents) {
            auto [m, a] = event.adjustFactor();
            AdjustFactor af(m, a);
            composite = af * composite;
        }
        dailyFactors[date] = composite;
    }

    return dailyFactors;
}

// =============================
// 合并每日因子为总因子
// =============================
AdjustFactor CombineDailyFactors(const std::map<std::string, AdjustFactor>& dailyFactors) {
    AdjustFactor total{1.0, 0.0};

    for (const auto& [date, factor] : dailyFactors) {
        total = factor * total;
    }

    return total;
}

// =============================
// 应用复权因子
// =============================
f64 AdjustFactor::Apply(f64 price) const {
    return m * price + a;
}

void AdjustFactor::ApplyVectorized(const f64* in, f64* out, size_t count) const {
    for (size_t i = 0; i < count; ++i) {
        out[i] = m * in[i] + a;
    }
}

AdjustFactor AdjustFactor::operator*(const AdjustFactor& other) const {
    return {
        .m = this->m * other.m,
        .a = other.m * this->a + other.a
    };
}