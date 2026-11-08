// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package quant1x.id.id128;

import java.util.function.LongSupplier;

/**
 * HLC 构造期配置项，对应 Go 版的函数式 Option 与 Python 版的
 * {@code with_clock/with_logical_seed/with_state_file/with_state_sync_every}。
 *
 * <pre>
 * HLC hlc = new HLC(
 *         Option.withClock(System::currentTimeMillis),
 *         Option.withLogicalSeed(7),
 *         Option.withStateFile("id.state"),
 *         Option.withStateSyncEvery(1000));
 * </pre>
 */
@FunctionalInterface
public interface Option {

    /**
     * 将配置应用到 {@link HLC} 实例（仅构造期调用）。
     */
    void apply(HLC hlc);

    /**
     * 注入时钟源（返回当前物理时间，毫秒）。
     * 缺省为 {@link System#currentTimeMillis()}。
     *
     * @param now 时钟源，为 null 时忽略
     */
    static Option withClock(LongSupplier now) {
        return hlc -> {
            if (now != null) {
                hlc.now = now;
            }
        };
    }

    /**
     * 注入逻辑种子（16 位），用于回退场景的随机化。
     * 缺省为加密随机数。
     *
     * @param seed 16 位种子
     */
    static Option withLogicalSeed(int seed) {
        return hlc -> hlc.seed = seed & 0xFFFF;
    }

    /**
     * 启用基于文件的状态持久化（跨重启强唯一）。
     *
     * @param path 状态文件路径，为 null 或空时忽略
     */
    static Option withStateFile(String path) {
        return hlc -> {
            if (path != null && !path.isEmpty()) {
                hlc.store = new FileStateStore(path);
            }
        };
    }

    /**
     * 设置每次 fsync 之间最多追加的状态记录条数。
     * 缺省为 1000（可被环境变量 {@code QUANT1X_ID128_SYNC_EVERY} 覆盖）；大于 0。
     *
     * @param every 每次 fsync 之间最多追加的记录条数
     */
    static Option withStateSyncEvery(long every) {
        return hlc -> {
            hlc.syncEvery = Math.max(1, every);
            if (hlc.store instanceof FileStateStore) {
                ((FileStateStore) hlc.store).syncEvery = hlc.syncEvery;
            }
        };
    }

    /**
     * 启用严格模式：每次发号前从磁盘读取最新状态并取 max。
     *
     * <p>默认关闭（快速路径）：构造时从状态文件恢复一次高水位，运行期只追加不读盘，
     * 热路径仅一次写入。适用于单写者，以及多进程顺序接管（failover）场景——
     * 新进程构造时读到前任写者的最新水位，保证跨重启不重复。
     *
     * <p>当多个进程（或同 JVM 多个 HLC 实例）活跃共享同一状态文件、且都期望严格唯一时，
     * 必须开启严格模式：它以每次发号增加一次磁盘读为代价，保证各写者水位同步。
     */
    static Option withStateStrict() {
        return hlc -> {
            hlc.strict = true;
            if (hlc.store instanceof FileStateStore) {
                ((FileStateStore) hlc.store).strict = true;
            }
        };
    }
}
