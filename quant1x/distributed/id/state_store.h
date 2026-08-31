// 文件状态存储 (对应 Go fileStateStore, Python state_store.py, Rust state_store.rs)
//
// 状态与跨进程锁均落在同一块共享 mmap 上:
//   - 偏移 [0, 64)   为双槽 checkpoint (generation 最大且 CRC 校验通过的槽胜出);
//   - 偏移 [64, 72)  为 8 字节锁字 (高 32 位 pid + 低 32 位秒级时间戳, CAS 加解锁,
//                    持有者进程死亡或锁龄超时由等待方抢占, 无独立锁文件);
//   - 偏移 [72, 128) 预留, 文件定长 128 字节不再增长.
//
// 两种工作模式:
//   - 快速路径 (strict=false): 构造时恢复一次水位, 运行期纯内存推进, 攒满 sync_every 条
//     才 checkpoint (msync) 一次, 热路径零系统调用;
//   - 严格模式 (strict=true): 每次 Next 都持跨进程锁, 以共享映射中的最新水位为基准推进,
//     保证多写者活跃共享唯一.
//
// 旧版兼容: 见 LEGACY_RECORD_SIZE, 首次打开时把追加式 18 字节日志迁移到双槽 checkpoint.
#pragma once
#ifndef QUANT1X_DISTRIBUTED_ID_STATE_STORE_H
#define QUANT1X_DISTRIBUTED_ID_STATE_STORE_H 1

#include <cstdint>
#include <mutex>
#include <string>

#include <quant1x/distributed/id/error.h>

// mmap_t 定义在 quant1x/base/mmap.h; 此处仅前向声明, 避免该头文件的符号泄漏到使用方
struct file_mmap;
typedef struct file_mmap mmap_t;

namespace quant1x::distributed::id {

/// 默认落盘间隔 (对应 Go defaultSyncEvery)
constexpr uint32_t DEFAULT_SYNC_EVERY = 1000;
/// 环境变量: 覆盖默认落盘间隔
constexpr const char *ENV_SYNC_EVERY = "QUANT1X_ID64_SYNC_EVERY";

/// 旧版追加式状态记录长度 (physical 8B + 保留 2B + seq 4B + crc32 4B)
constexpr size_t LEGACY_RECORD_SIZE = 18;
/// 单槽 checkpoint 记录长度 (generation 8B + physical 8B + seq 4B + crc32 4B)
constexpr size_t CHECKPOINT_SLOT_SIZE = 32;
/// 双槽
constexpr size_t CHECKPOINT_SLOT_COUNT = 2;
/// 64B 双槽区
constexpr size_t CHECKPOINT_AREA_SIZE = CHECKPOINT_SLOT_SIZE * CHECKPOINT_SLOT_COUNT;
/// 槽区 + 锁字 + 预留, 定长不再增长
constexpr size_t STATE_FILE_SIZE = 128;
/// 锁字偏移 (跨进程互斥锁)
constexpr size_t STATE_LOCK_OFFSET = CHECKPOINT_AREA_SIZE;
/// 锁持有超时 (秒): 超时或持有者进程死亡可被抢占
constexpr uint32_t LOCK_TAKEOVER_AFTER_SECONDS = 30;
/// 锁等待退避上限 (微秒)
constexpr uint64_t LOCK_BACKOFF_MAX_SLEEP_US = 1024;

/// 持久化高水位状态
struct PersistentState {
    int64_t physical = 0;
    uint32_t seq = 0;
};

/// load() 的返回: ok 为 false 表示无有效历史水位
struct LoadedState {
    PersistentState state;
    bool ok = false;
};

/// 比较两个持久化状态 (先比物理时间, 再比序号)
int compare_persistent_state(const PersistentState &a, const PersistentState &b) noexcept;

/// 在给定水位上推进状态 (对应 Go advancePersistentState)
///
/// - now > physical: 重置为 (now, 0), 时钟前进;
/// - seq 达到掩码上限: 借位到 (physical + 1, 0), 保证严格递增;
/// - 其余 (含时钟回拨): seq 递增, 同一毫秒内靠序号维持单调.
PersistentState advance_persistent_state(const PersistentState &state, int64_t now, uint8_t seq_bits) noexcept;

/// 默认落盘间隔 (环境变量 QUANT1X_ID64_SYNC_EVERY 可覆盖, 非法值回退默认)
uint32_t default_sync_every_value() noexcept;

/// 文件状态存储
class FileStateStore {
public:
    FileStateStore(std::string path, uint32_t sync_every, bool strict);
    ~FileStateStore();

    FileStateStore(const FileStateStore &) = delete;
    FileStateStore &operator=(const FileStateStore &) = delete;
    FileStateStore(FileStateStore &&) = delete;
    FileStateStore &operator=(FileStateStore &&) = delete;

    /// 加载最近一次持久化状态; 无历史水位时 ok=false
    Result<LoadedState> load();

    /// 推进状态 (对应 Go Next)
    Result<PersistentState> next(const PersistentState &local, int64_t now, uint8_t seq_bits);

    /// 立即把尚未 checkpoint 的水位写入映射并 msync
    Status flush();

    /// 刷新未落盘水位并释放共享映射 (幂等)
    Status close();

private:
    /// 打开 (必要时创建) 定长状态文件并映射到内存 (调用方不得持锁)
    Status open_mapped();

    /// 扫描双槽 checkpoint, 返回 generation 最大的有效槽 (调用方不得持锁)
    LoadedState load_checkpoint();

    /// 将水位写入下一个槽并 (可选) msync (需持有 mutex_)
    Status checkpoint_locked(const PersistentState &state, bool flush_now);

    /// 从尾部向前读取最近一条有效 legacy 记录 (调用方不得持锁)
    Result<LoadedState> load_latest_state();

    /// 跨进程锁释放守卫: 析构时归还锁字
    class LockGuard {
    public:
        LockGuard() = default;
        LockGuard(uint8_t *word, uint64_t mine) noexcept : word_(word), mine_(mine) {}
        ~LockGuard() noexcept;
        LockGuard(const LockGuard &) = delete;
        LockGuard &operator=(const LockGuard &) = delete;
        LockGuard(LockGuard &&other) noexcept : word_(other.word_), mine_(other.mine_) {
            other.word_ = nullptr;
        }
        LockGuard &operator=(LockGuard &&other) noexcept {
            if (this != &other) {
                release();
                word_ = other.word_;
                mine_ = other.mine_;
                other.word_ = nullptr;
            }
            return *this;
        }
        bool held() const noexcept { return word_ != nullptr; }
        void release() noexcept;

    private:
        uint8_t *word_ = nullptr;
        uint64_t mine_ = 0;
    };

    /// 获取跨进程锁, 失败时返回错误
    Result<LockGuard> lock_mapped();

    std::string path_;
    uint32_t sync_every_;
    bool strict_;
    mutable std::mutex mutex_;
    /// 共享映射 (受 mutex_ 保护)
    mmap_t *mapped_ = nullptr;
    /// 下一个槽的 generation (受 mutex_ 保护)
    uint64_t generation_ = 0;
    /// 内存中的最新水位 (受 mutex_ 保护)
    PersistentState latest_{};
    /// 尚未落盘的条数 (受 mutex_ 保护)
    uint32_t unsynced_ = 0;
};

}  // namespace quant1x::distributed::id

#endif  // QUANT1X_DISTRIBUTED_ID_STATE_STORE_H
