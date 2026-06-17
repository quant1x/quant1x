#pragma once
#ifndef QUANT1X_REALTIME_SNAPSHOT_H
#define QUANT1X_REALTIME_SNAPSHOT_H 1

#include <quant1x/runtime/core.h>
#include <quant1x/contrib/data/tdx/level1/security_quote.h>
#include <quant1x/proto/data.h>

namespace realtime {

    // 定义 imbalance 结构体返回值(可扩展)
    struct ImbalanceResult {
        double simpleImbalance;   // 简单总量不平衡度
        double weightedImbalance; // 价格加权不平衡度
    };

    /**
     * @brief 从 SecurityQuote 中提取 imbalance 指标
     * @param quote 当前行情快照
     * @return ImbalanceResult 包含 imbalance 指标
     */
    ImbalanceResult calculateImbalance(const level1::SecurityQuote& quote);

    // 同步快照
    void sync_snapshots();

    std::optional<level1::SecurityQuote> get_snapshot(const std::string &code);

    // 加载快照
    void load_snapshots(void);

    // 从缓存中获取快照
    std::optional<Snapshot::Reader> snapshot(const std::string &code);

} // namespace realtime

#endif //QUANT1X_REALTIME_SNAPSHOT_H
