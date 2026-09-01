// 文件状态存储的实现
//
// 跨语言一致性: 文件格式、字节序、CRC 校验与 Go/Python/Rust 完全一致,
// 四种语言写出的状态文件可互相读取.
#include <quant1x/distributed/id/state_store.h>

#include <quant1x/base/mmap.h>
#include <quant1x/base/safe.h>
#include <quant1x/distributed/id/crc32.h>

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <optional>
#include <thread>

#ifdef _WIN32
// windows.h 已由 quant1x/base/mmap.h 引入
#else
#include <cerrno>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace quant1x::distributed::id {

namespace {

// ---------------- 大端读写辅助 ----------------

uint64_t read_u64_be(const uint8_t *p) noexcept {
    uint64_t v = 0;
    v |= static_cast<uint64_t>(p[0]) << 56;
    v |= static_cast<uint64_t>(p[1]) << 48;
    v |= static_cast<uint64_t>(p[2]) << 40;
    v |= static_cast<uint64_t>(p[3]) << 32;
    v |= static_cast<uint64_t>(p[4]) << 24;
    v |= static_cast<uint64_t>(p[5]) << 16;
    v |= static_cast<uint64_t>(p[6]) << 8;
    v |= static_cast<uint64_t>(p[7]);
    return v;
}

void write_u64_be(uint8_t *p, uint64_t v) noexcept {
    p[0] = static_cast<uint8_t>((v >> 56) & 0xFFu);
    p[1] = static_cast<uint8_t>((v >> 48) & 0xFFu);
    p[2] = static_cast<uint8_t>((v >> 40) & 0xFFu);
    p[3] = static_cast<uint8_t>((v >> 32) & 0xFFu);
    p[4] = static_cast<uint8_t>((v >> 24) & 0xFFu);
    p[5] = static_cast<uint8_t>((v >> 16) & 0xFFu);
    p[6] = static_cast<uint8_t>((v >> 8) & 0xFFu);
    p[7] = static_cast<uint8_t>(v & 0xFFu);
}

uint32_t read_u32_be(const uint8_t *p) noexcept {
    uint32_t v = 0;
    v |= static_cast<uint32_t>(p[0]) << 24;
    v |= static_cast<uint32_t>(p[1]) << 16;
    v |= static_cast<uint32_t>(p[2]) << 8;
    v |= static_cast<uint32_t>(p[3]);
    return v;
}

void write_u32_be(uint8_t *p, uint32_t v) noexcept {
    p[0] = static_cast<uint8_t>((v >> 24) & 0xFFu);
    p[1] = static_cast<uint8_t>((v >> 16) & 0xFFu);
    p[2] = static_cast<uint8_t>((v >> 8) & 0xFFu);
    p[3] = static_cast<uint8_t>(v & 0xFFu);
}

// ---------------- 平台相关: 进程号 / 时间戳 / 进程探活 / 锁字原子访问 ----------------

uint32_t current_pid() noexcept {
#ifdef _WIN32
    return static_cast<uint32_t>(::GetCurrentProcessId());
#else
    return static_cast<uint32_t>(::getpid());
#endif
}

/// 当前 Unix 秒 (截断为 u32, 与 Go uint32(time.Now().Unix()) 一致)
uint32_t lock_stamp_now() noexcept {
    const auto now = std::chrono::system_clock::now();
    const auto secs = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
    return static_cast<uint32_t>(secs.count());
}

/// 进程是否存活
bool process_alive(uint32_t pid) noexcept {
    if (pid == 0) {
        return false;
    }
#ifdef _WIN32
    // PROCESS_QUERY_LIMITED_INFORMATION 无需管理员权限; 打开失败且为 ACCESS_DENIED
    // 表示进程存在 (系统进程/其他用户), 其余失败视为不存在
    HANDLE handle = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, static_cast<DWORD>(pid));
    if (handle == nullptr) {
        return ::GetLastError() == ERROR_ACCESS_DENIED;
    }
    const DWORD status = ::WaitForSingleObject(handle, 0);
    ::CloseHandle(handle);
    // WAIT_OBJECT_0 / WAIT_ABANDONED 表示已退出, 其余 (超时/失败) 保守视为存活
    return status != WAIT_OBJECT_0 && status != WAIT_ABANDONED;
#else
    // kill(pid, 0) 不发送信号, 仅探测进程存在性
    const int ret = ::kill(static_cast<pid_t>(pid), 0);
    if (ret == 0) {
        return true;
    }
    // EPERM: 进程存在但无权限, 视为存活; ESRCH: 进程不存在
    return errno == EPERM;
#endif
}

/// 锁字原子读
uint64_t lock_word_load(const uint8_t *p) noexcept {
#ifdef _WIN32
    // InterlockedCompareExchange64 返回原值, 以 (0, 0) 交换实现无损读取
    return static_cast<uint64_t>(::InterlockedCompareExchange64(
        reinterpret_cast<volatile LONG64 *>(const_cast<uint8_t *>(p)), 0, 0));
#else
    return __atomic_load_n(reinterpret_cast<const uint64_t *>(p), __ATOMIC_ACQUIRE);
#endif
}

/// 锁字原子 CAS: 成功返回 true
bool lock_word_cas(uint8_t *p, uint64_t expected, uint64_t desired) noexcept {
#ifdef _WIN32
    return ::InterlockedCompareExchange64(reinterpret_cast<volatile LONG64 *>(p),
                                          static_cast<LONG64>(desired),
                                          static_cast<LONG64>(expected)) == static_cast<LONG64>(expected);
#else
    return __atomic_compare_exchange_n(reinterpret_cast<uint64_t *>(p), &expected, desired, false,
                                       __ATOMIC_SEQ_CST, __ATOMIC_ACQUIRE);
#endif
}

/// 编码锁字: 高 32 位 pid, 低 32 位秒级时间戳
uint64_t encode_lock_word(uint32_t pid, uint32_t stamp) noexcept {
    return (static_cast<uint64_t>(pid) << 32) | static_cast<uint64_t>(stamp);
}

void decode_lock_word(uint64_t word, uint32_t &pid, uint32_t &stamp) noexcept {
    pid = static_cast<uint32_t>(word >> 32);
    stamp = static_cast<uint32_t>(word & 0xFFFFFFFFu);
}

/// 锁持有者是否可被抢占: 同进程不抢占; 进程死亡或锁龄超时可抢占
bool lock_holder_stale(uint32_t pid, uint32_t stamp, uint32_t self_pid) noexcept {
    if (pid == self_pid) {
        return false;
    }
    if (!process_alive(pid)) {
        return true;
    }
    // 回绕安全的无符号减法 (与 Rust wrapping_sub 一致)
    const uint32_t elapsed = lock_stamp_now() - stamp;
    return elapsed >= LOCK_TAKEOVER_AFTER_SECONDS;
}

/// 锁等待退避: 先自旋, 再让出, 最后短睡 (对齐 Rust lock_backoff)
void lock_backoff(uint32_t *retries) {
    if (*retries <= 3) {
        // 短时紧自旋, 不做系统调用
    } else if (*retries <= 11) {
        std::this_thread::yield();
    } else {
        uint64_t sleep_us = static_cast<uint64_t>(1) << (*retries - 12);
        if (sleep_us > LOCK_BACKOFF_MAX_SLEEP_US) {
            sleep_us = LOCK_BACKOFF_MAX_SLEEP_US;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(sleep_us));
    }
    if (*retries < UINT32_MAX) {
        *retries += 1;
    }
}

/// 解码并校验 18 字节 legacy 记录 (physical 8B | 保留 2B | seq 4B | crc32 4B)
std::optional<PersistentState> decode_state_record(const uint8_t *record) {
    const uint32_t checksum = read_u32_be(record + 14);
    if (crc32_ieee(record, 14) != checksum) {
        return std::nullopt;
    }
    PersistentState state;
    state.physical = static_cast<int64_t>(read_u64_be(record));
    state.seq = read_u32_be(record + 10);
    return state;
}

}  // namespace

// ---------------- PersistentState 相关自由函数 ----------------

int compare_persistent_state(const PersistentState &a, const PersistentState &b) noexcept {
    if (a.physical != b.physical) {
        return a.physical > b.physical ? 1 : -1;
    }
    if (a.seq > b.seq) {
        return 1;
    }
    if (a.seq < b.seq) {
        return -1;
    }
    return 0;
}

PersistentState advance_persistent_state(const PersistentState &state, int64_t now, uint8_t seq_bits) noexcept {
    if (now > state.physical) {
        return PersistentState{now, 0};
    }
    const uint32_t mask = (1u << seq_bits) - 1u;
    if (state.seq >= mask) {
        return PersistentState{state.physical + 1, 0};
    }
    return PersistentState{state.physical, state.seq + 1};
}

uint32_t default_sync_every_value() noexcept {
    // safe::getenv 收敛 MSVC 的 getenv 弃用警告 (C4996) 与平台差异
    const auto value = safe::getenv(ENV_SYNC_EVERY);
    if (value.has_value() && !value->empty()) {
        char *end = nullptr;
        const unsigned long parsed = std::strtoul(value->c_str(), &end, 10);
        if (end != value->c_str() && parsed > 0 && parsed <= UINT32_MAX) {
            return static_cast<uint32_t>(parsed);
        }
    }
    return DEFAULT_SYNC_EVERY;
}

// ---------------- FileStateStore ----------------

FileStateStore::FileStateStore(std::string path, uint32_t sync_every, bool strict)
    : path_(std::move(path)), sync_every_(sync_every), strict_(strict) {}

FileStateStore::~FileStateStore() {
    std::lock_guard<std::mutex> guard(mutex_);
    if (mapped_ != nullptr) {
        mmap_flush(mapped_);
        mmap_close(&mapped_);
        mapped_ = nullptr;
    }
}

FileStateStore::LockGuard::~LockGuard() noexcept {
    release();
}

void FileStateStore::LockGuard::release() noexcept {
    if (word_ != nullptr) {
        // 若期间被抢占 (他人判定本进程已死后接管), CAS 失败即放弃, 不会误清他人的锁
        lock_word_cas(word_, mine_, 0);
        word_ = nullptr;
    }
}

Status FileStateStore::open_mapped() {
    {
        std::lock_guard<std::mutex> guard(mutex_);
        if (mapped_ != nullptr) {
            return Error{};
        }
    }
    // mmap_open 内部会对 parent_path() 建目录, 相对路径的父目录为空会触发异常,
    // 因此统一转换为绝对路径后再交给基建
    const std::string resolved = std::filesystem::absolute(std::filesystem::path(path_)).string();
    const std::filesystem::path target(resolved);
    const std::filesystem::path dir = target.parent_path();
    if (!dir.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        if (ec && !std::filesystem::exists(dir, ec)) {
            return Error::state_file("create dir " + dir.string() + " failed: " + ec.message());
        }
    }
    mmap_t *mapped = nullptr;
    try {
        // 复用 C++ 基建: mmap_open 会按需创建文件并截断/扩展到定长 STATE_FILE_SIZE
        mapped = mmap_open(resolved.c_str(), 0, STATE_FILE_SIZE);
    } catch (const std::exception &e) {
        return Error::state_file("open state file " + path_ + " failed: " + e.what());
    } catch (...) {
        return Error::state_file("open state file " + path_ + " failed: unknown error");
    }
    if (mapped == nullptr || mapped->data == nullptr) {
        if (mapped != nullptr) {
            mmap_close(&mapped);
        }
        return Error::state_file("mmap state file " + path_ + " failed");
    }
    std::lock_guard<std::mutex> guard(mutex_);
    if (mapped_ != nullptr) {
        // 并发下已被其他线程打开, 释放本次映射
        mmap_close(&mapped);
        return Error{};
    }
    mapped_ = mapped;
    return Error{};
}

LoadedState FileStateStore::load_checkpoint() {
    LoadedState result{};
    uint64_t best_generation = 0;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        if (mapped_ == nullptr || mapped_->data == nullptr) {
            return result;
        }
        const uint8_t *data = mapped_->data;
        for (size_t slot = 0; slot < CHECKPOINT_SLOT_COUNT; ++slot) {
            const uint8_t *record = data + slot * CHECKPOINT_SLOT_SIZE;
            const uint64_t generation = read_u64_be(record);
            if (generation == 0) {
                continue;
            }
            const uint32_t checksum = read_u32_be(record + 20);
            if (crc32_ieee(record, 20) != checksum) {
                continue;
            }
            PersistentState state;
            state.physical = static_cast<int64_t>(read_u64_be(record + 8));
            state.seq = read_u32_be(record + 16);
            if (!result.ok || generation > best_generation) {
                best_generation = generation;
                result.state = state;
                result.ok = true;
            }
        }
    }
    if (result.ok) {
        std::lock_guard<std::mutex> guard(mutex_);
        generation_ = best_generation;
        latest_ = result.state;
    }
    return result;
}

Status FileStateStore::checkpoint_locked(const PersistentState &state, bool flush_now) {
    if (mapped_ == nullptr || mapped_->data == nullptr) {
        return Error::state_file("mapping not open");
    }
    generation_ += 1;
    const uint64_t generation = generation_;
    const size_t base = static_cast<size_t>(generation % CHECKPOINT_SLOT_COUNT) * CHECKPOINT_SLOT_SIZE;
    uint8_t record[CHECKPOINT_SLOT_SIZE];
    std::memset(record, 0, sizeof(record));
    write_u64_be(record, generation);
    write_u64_be(record + 8, static_cast<uint64_t>(state.physical));
    write_u32_be(record + 16, state.seq);
    const uint32_t checksum = crc32_ieee(record, 20);
    write_u32_be(record + 20, checksum);
    std::memcpy(mapped_->data + base, record, CHECKPOINT_SLOT_SIZE);
    // 注意: Go/Rust 参考实现此处未同步 latest, 导致 close() 的 flush() 用旧水位覆盖
    // 新 checkpoint (严格模式重启可能重复 ID). Python 作为 Spec 锚点已修正该缺陷,
    // C++ 侧同步修正: 写入的槽即代表最新水位.
    latest_ = state;
    if (flush_now) {
        mmap_flush(mapped_);
    }
    return Error{};
}

Result<LoadedState> FileStateStore::load_latest_state() {
    std::ifstream in(path_, std::ios::binary);
    if (!in) {
        // 文件不存在或不可读: 视为无状态
        return LoadedState{};
    }
    in.seekg(0, std::ios::end);
    const std::streamoff total = in.tellg();
    if (total <= 0) {
        return LoadedState{};
    }
    const uint64_t size = static_cast<uint64_t>(total);
    const uint64_t end = size - (size % LEGACY_RECORD_SIZE);
    if (end == 0) {
        // 不足一条完整记录: 视为无状态 (下次 checkpoint 重建)
        return LoadedState{};
    }
    uint64_t offset = end - LEGACY_RECORD_SIZE;
    while (true) {
        uint8_t record[LEGACY_RECORD_SIZE] = {0};
        in.clear();
        in.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
        in.read(reinterpret_cast<char *>(record), static_cast<std::streamsize>(LEGACY_RECORD_SIZE));
        if (in.gcount() != static_cast<std::streamsize>(LEGACY_RECORD_SIZE)) {
            return LoadedState{};
        }
        const auto decoded = decode_state_record(record);
        if (decoded.has_value()) {
            if (size > offset + LEGACY_RECORD_SIZE) {
                // 清理尾部残留的半条/损坏记录
                in.close();
                std::error_code ec;
                std::filesystem::resize_file(path_,
                                             static_cast<uint64_t>(offset + LEGACY_RECORD_SIZE), ec);
                if (ec) {
                    return Error::state_file("truncate state file failed: " + ec.message());
                }
            }
            return LoadedState{*decoded, true};
        }
        if (offset == 0) {
            break;
        }
        offset -= LEGACY_RECORD_SIZE;
    }
    return LoadedState{};
}

Result<LoadedState> FileStateStore::load() {
    LoadedState legacy{};
    std::error_code ec;
    const auto status = std::filesystem::status(path_, ec);
    if (ec) {
        if (ec != std::errc::no_such_file_or_directory) {
            return Error::state_file("stat state file " + path_ + " failed: " + ec.message());
        }
    } else if (std::filesystem::exists(status)) {
        const uint64_t size = std::filesystem::file_size(path_, ec);
        if (ec) {
            return Error::state_file("stat state file " + path_ + " failed: " + ec.message());
        }
        if (size != STATE_FILE_SIZE) {
            // 旧版 18 字节追加式日志: 迁移最后一条有效记录
            auto migrated = load_latest_state();
            if (!migrated.ok()) {
                return migrated.error;
            }
            legacy = *migrated;
        }
    }
    const Status opened = open_mapped();
    if (!opened.ok()) {
        return opened;
    }
    const LoadedState mapped_state = load_checkpoint();
    if (mapped_state.ok &&
        (!legacy.ok || compare_persistent_state(mapped_state.state, legacy.state) > 0)) {
        legacy.state = mapped_state.state;
        legacy.ok = true;
    }
    if (legacy.ok) {
        std::lock_guard<std::mutex> guard(mutex_);
        latest_ = legacy.state;
    }
    return legacy;
}

Result<PersistentState> FileStateStore::next(const PersistentState &local, int64_t now, uint8_t seq_bits) {
    if (!strict_) {
        // 快速路径: 纯内存推进; 攒满 sync_every 条才 checkpoint 一次
        const PersistentState next_state = advance_persistent_state(local, now, seq_bits);
        bool should_sync = false;
        {
            std::lock_guard<std::mutex> guard(mutex_);
            latest_ = next_state;
            unsynced_ += 1;
            should_sync = unsynced_ >= sync_every_;
        }
        if (should_sync) {
            std::lock_guard<std::mutex> guard(mutex_);
            const Status written = checkpoint_locked(next_state, true);
            if (!written.ok()) {
                return written;
            }
            unsynced_ = 0;
        }
        return next_state;
    }

    // 严格模式: 以共享映射中的最新状态为基准 (多写者活跃共享唯一性)
    auto locked = lock_mapped();
    if (!locked.ok()) {
        return locked.error;
    }
    LockGuard guard = std::move(*locked);
    Status status{};
    PersistentState next_state{};
    {
        std::lock_guard<std::mutex> inner(mutex_);
        PersistentState base = local;
        const LoadedState latest = load_checkpoint();
        if (latest.ok && compare_persistent_state(latest.state, base) > 0) {
            base = latest.state;
        }
        next_state = advance_persistent_state(base, now, seq_bits);
        const bool should_sync = (unsynced_ + 1 >= sync_every_);
        status = checkpoint_locked(next_state, should_sync);
        if (status.ok()) {
            unsynced_ += 1;
            if (unsynced_ >= sync_every_) {
                unsynced_ = 0;
            }
        }
    }
    guard.release();
    if (!status.ok()) {
        return status;
    }
    return next_state;
}

Status FileStateStore::flush() {
    {
        std::lock_guard<std::mutex> guard(mutex_);
        if (unsynced_ == 0) {
            return Error{};
        }
    }
    auto locked = lock_mapped();
    if (!locked.ok()) {
        return locked.error;
    }
    LockGuard guard = std::move(*locked);
    Status status{};
    {
        std::lock_guard<std::mutex> inner(mutex_);
        status = checkpoint_locked(latest_, true);
        if (status.ok()) {
            unsynced_ = 0;
        }
    }
    guard.release();
    return status;
}

Status FileStateStore::close() {
    Status status = flush();
    std::lock_guard<std::mutex> guard(mutex_);
    if (mapped_ != nullptr) {
        mmap_flush(mapped_);
        mmap_close(&mapped_);
        mapped_ = nullptr;
    }
    return status;
}

auto FileStateStore::lock_mapped() -> Result<LockGuard> {
    const Status opened = open_mapped();
    if (!opened.ok()) {
        return opened;
    }
    uint8_t *word = nullptr;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        if (mapped_ == nullptr || mapped_->data == nullptr) {
            return Error::state_file("mapping not open");
        }
        word = mapped_->data + STATE_LOCK_OFFSET;
    }
    const uint32_t self_pid = current_pid();
    const uint64_t mine = encode_lock_word(self_pid, lock_stamp_now());
    uint32_t retries = 0;
    while (true) {
        const uint64_t current = lock_word_load(word);
        if (current == 0) {
            if (lock_word_cas(word, 0, mine)) {
                return LockGuard(word, mine);
            }
            continue;
        }
        uint32_t pid = 0;
        uint32_t stamp = 0;
        decode_lock_word(current, pid, stamp);
        if (!lock_holder_stale(pid, stamp, self_pid)) {
            lock_backoff(&retries);
            continue;
        }
        if (lock_word_cas(word, current, mine)) {
            return LockGuard(word, mine);
        }
        retries = 0;
    }
}

}  // namespace quant1x::distributed::id
