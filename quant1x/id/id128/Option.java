// Copyright (c) 2026 Quant1X. All rights reserved.
// Author: wangfeng <wangfengxy@sina.cn>
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
 *         Option.withStateSyncEvery(1));
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
     * 缺省为 1；大于 0。
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
}
