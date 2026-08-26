// Copyright (c) 2026 Quant1X. All rights reserved.
// Author: wangfeng <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package quant1x.id.id64;

import java.util.function.LongSupplier;

/**
 * HLC 的可选配置项，对应 Go/Python 版的 Option。
 */
@FunctionalInterface
public interface Option {

    void apply(HLC hlc);

    /** 覆盖默认时钟（返回绝对毫秒），测试用 */
    static Option withClock(LongSupplier now) {
        return hlc -> {
            if (now != null) {
                hlc.now = now;
            }
        };
    }

    /** 设置序列号启动种子（默认随机），用于无状态文件时随机化初始 seq */
    static Option withSeqSeed(int seed) {
        return hlc -> hlc.seed = seed & 0xFFFF;
    }

    /** 启用状态文件持久化，跨进程/重启恢复高水位 */
    static Option withStateFile(String path) {
        return hlc -> {
            if (path != null && !path.isEmpty()) {
                hlc.store = new FileStateStore(path);
            }
        };
    }

    /** 设置状态文件落盘间隔（每 N 次生成落盘一次） */
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

    /**
     * 设置预期的节点总数，据此动态推导节点位宽与序列号位宽：
     * <pre>
     * workerBits = bit_length(nodeCount)
     * seqBits    = 64 - 1 - 41 - workerBits
     * </pre>
     * 当 seqBits &lt; 4（节点数 &gt; 2^18）时抛出 IllegalArgumentException。
     */
    static Option withNodeCount(long count) {
        return hlc -> {
            long nodeCount = Math.max(1, count);
            int workerBits = Long.SIZE - Long.numberOfLeadingZeros(nodeCount);
            hlc.seqBits = HLC.PAYLOAD_BITS - workerBits;
            if (hlc.seqBits < 4) {
                throw new IllegalArgumentException("id64: 节点数过多，无法为序列号保留足够的位宽");
            }
        };
    }

    /** 直接设置序列号位宽（底层选项，通常用 withNodeCount 代替） */
    static Option withSeqBits(int bits) {
        return hlc -> {
            if (bits < 4 || bits > HLC.PAYLOAD_BITS - 1) {
                throw new IllegalArgumentException("id64: seqBits 超出有效范围 [4, 21]");
            }
            hlc.seqBits = bits;
        };
    }
}
