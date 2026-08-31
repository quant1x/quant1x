// 混合逻辑时钟 (HLC, 对应 Go HLC/Option, Python hlc.py, Rust Hlc/HlcBuilder)
//
// 保证同一进程内生成的 (physical, seq) 严格递增:
//   - 时钟前进 (now > physical): 重置为 (now, 0);
//   - 时钟回拨或停滞 (now <= physical): seq 递增, 序号用尽则借位到 physical + 1.
//
// 可选的 FileStateStore 提供跨进程/跨重启恢复.
#pragma once
#ifndef QUANT1X_DISTRIBUTED_ID_HLC_H
#define QUANT1X_DISTRIBUTED_ID_HLC_H 1

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include <quant1x/distributed/id/error.h>
#include <quant1x/distributed/id/state_store.h>

namespace quant1x::distributed::id {

/// 默认 seq 位数 (22 bit payload - 11 bit worker)
constexpr uint8_t DEFAULT_SEQ_BITS = 11;

/// 当前 Unix 毫秒时间
int64_t unix_millis() noexcept;

/// 混合逻辑时钟
class Hlc {
public:
    Hlc(const Hlc &) = delete;
    Hlc &operator=(const Hlc &) = delete;

    /// 便捷构造: 按节点数推导 seq 位数 (等价于 HlcBuilder().with_node_count(count))
    static Result<std::shared_ptr<Hlc>> create(uint32_t node_count);

    /// 推进并返回 (physical, seq), 保证严格递增
    Result<std::pair<int64_t, uint32_t>> now();

    /// seq 位数
    uint8_t seq_bits() const noexcept { return seq_bits_; }

    /// 当前物理时间 (毫秒, 相对 Unix 纪元)
    int64_t timestamp();

    /// 刷新并释放状态存储 (幂等)
    Status close();

private:
    friend class HlcBuilder;

    Hlc(std::function<int64_t()> clock, uint8_t seq_bits, std::shared_ptr<FileStateStore> store)
        : clock_(std::move(clock)), seq_bits_(seq_bits), store_(std::move(store)) {}

    std::function<int64_t()> clock_;
    uint8_t seq_bits_;
    std::shared_ptr<FileStateStore> store_;
    mutable std::mutex mutex_;
    /// 内存态水位 (受 mutex_ 保护)
    int64_t physical_ = 0;
    uint32_t seq_ = 0;
};

/// HLC 构建器 (对应 Go 的 Option 函数式配置, Rust 的 HlcBuilder)
class HlcBuilder {
public:
    HlcBuilder();

    /// 自定义时钟 (对应 Go WithClock)
    HlcBuilder &with_clock(std::function<int64_t()> clock);

    /// 自定义初始序号种子 (对应 Go WithSeqSeed)
    HlcBuilder &with_seq_seed(uint16_t seed);

    /// 启用文件状态存储 (对应 Go WithStateFile)
    HlcBuilder &with_state_file(std::string path);

    /// 设置落盘间隔 (0 忽略, 对应 Go WithStateSyncEvery)
    HlcBuilder &with_state_sync_every(uint32_t every);

    /// 开启严格模式 (对应 Go WithStateStrict)
    HlcBuilder &with_state_strict();

    /// 按节点数推导 seq 位数 (对应 Go WithNodeCount); 节点数过大返回错误
    Result<HlcBuilder> with_node_count(uint32_t count) const;

    /// 直接指定 seq 位数, 范围 [4, 21] (对应 Go WithSeqBits); 越界返回错误
    Result<HlcBuilder> with_seq_bits(uint8_t seq_bits) const;

    /// 构建 HLC (对应 Go NewHLC)
    Result<std::shared_ptr<Hlc>> build() const;

private:
    std::function<int64_t()> clock_;
    uint16_t seed_;
    uint8_t seq_bits_;
    uint32_t sync_every_;
    bool strict_;
    std::string store_path_;
};

}  // namespace quant1x::distributed::id

#endif  // QUANT1X_DISTRIBUTED_ID_HLC_H
