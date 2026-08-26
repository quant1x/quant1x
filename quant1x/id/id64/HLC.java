// Copyright (c) 2026 Quant1X. All rights reserved.
// Author: wangfeng <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package quant1x.id.id64;

import java.io.IOException;
import java.security.SecureRandom;
import java.util.Optional;
import java.util.function.LongSupplier;

/**
 * Hybrid Logical Clock（64 位版）。
 *
 * <p>内部维护 (physical, seq) 二元组：
 * <ul>
 *   <li>{@code physical} — 绝对毫秒时间戳（epoch 相对值在组装 ID 时换算）</li>
 *   <li>{@code seq}     — 序列号，达到 seqBits 容量时进位 physical+1（时钟回拨时保持单调）</li>
 * </ul>
 *
 * <p>与 id128 的差异：64 位布局空间有限，无独立的 logical 位；
 * 回拨时的单调性由 seq 递增 + physical+1 承担；{@code seed}
 * 仅用于无状态文件时随机化初始 seq，降低重启碰撞概率。
 */
public final class HLC {

    /** ID 时间戳起点（2026-01-01T00:00:00Z，毫秒）。41 位毫秒量程约 69.7 年（至 2095 年）。 */
    public static final long EPOCH_MS = 1767225600000L;

    /** 时间戳位数（毫秒） */
    static final int PHYSICAL_BITS = 41;

    /** 布局：1 位符号(恒 0) + PHYSICAL_BITS + workerBits + seqBits = 64，故 workerBits + seqBits = 22 */
    static final int PAYLOAD_BITS = 22;

    private static final SecureRandom RANDOM = new SecureRandom();

    /**
     * 进程级随机种子：类加载时生成一次（对齐 Go 的 sync.Once / Python 的模块级缓存）。
     * 仅用于无状态文件时随机化初始 seq，降低重启碰撞概率。
     */
    private static final int RANDOM_SEED = RANDOM.nextInt(0x10000);

    // 以下字段为 package-private，供 Option 与同包测试直接读写
    long physical;
    long seq;
    LongSupplier now;
    int seed;
    int seqBits;
    long syncEvery;
    boolean strict;
    StateStore store;

    private final Object lock = new Object();

    /** 默认构造：节点总数 1024（workerBits=11, seqBits=11） */
    public HLC(Option... options) {
        now = System::currentTimeMillis;
        seed = RANDOM_SEED;
        seqBits = PAYLOAD_BITS - seqBitsFromNodeCount(1024);
        syncEvery = FileStateStore.defaultSyncEvery();

        for (Option option : options) {
            if (option != null) {
                option.apply(this);
            }
        }

        if (store instanceof FileStateStore) {
            ((FileStateStore) store).syncEvery = syncEvery;
            ((FileStateStore) store).strict = strict;
        }

        Optional<PersistentState> restored = loadState();
        if (restored.isPresent()) {
            physical = restored.get().physical;
            seq = restored.get().seq;
        } else {
            physical = now.getAsLong();
            seq = seed & seqMask();
        }
    }

    long seqMask() {
        return (1L << seqBits) - 1;
    }

    /**
     * 返回严格单调递增的 (physical 绝对毫秒, seq)。
     */
    public Now now() {
        synchronized (lock) {
            PersistentState current = PersistentState.of(physical, seq);
            long nowMs = now.getAsLong();
            PersistentState next;
            if (store != null) {
                try {
                    next = store.next(current, nowMs, seqBits);
                } catch (IOException e) {
                    throw new IllegalStateException("id64: 状态存储失败", e);
                }
            } else {
                next = advancePersistentState(current, nowMs, seqBits);
            }
            physical = next.physical;
            seq = next.seq;
            return new Now(physical, seq);
        }
    }

    /** 返回当前序列号位宽 */
    public int seqBits() {
        return seqBits;
    }

    /** 返回当前物理时间（绝对毫秒） */
    public long timestamp() {
        synchronized (lock) {
            return physical;
        }
    }

    /**
     * 把快速路径批量缓冲中尚未落盘的状态记录写入磁盘并同步。
     * 启用状态文件后，进程异常退出最多丢失最近 syncEvery-1 条进度
     * （这些 ID 重启后可能重复）；优雅退出前调用本方法可零丢失。
     * 未启用状态文件时为空操作。可多次调用，幂等。
     *
     * @throws IOException 落盘失败时抛出
     */
    public void close() throws IOException {
        synchronized (lock) {
            if (store instanceof FileStateStore) {
                ((FileStateStore) store).flush();
            }
        }
    }

    private Optional<PersistentState> loadState() {
        if (store == null) {
            return Optional.empty();
        }
        try {
            return store.load();
        } catch (IOException e) {
            throw new IllegalStateException("id64: 加载状态失败", e);
        }
    }

    /**
     * 在共享状态上推进 (physical, seq)：
     * <ul>
     *   <li>物理时间前进：重置 seq 为 0</li>
     *   <li>否则 seq+1；seq 达容量时进位 physical+1 并重置 seq（保持单调，不等待墙钟追平）</li>
     * </ul>
     */
    static PersistentState advancePersistentState(PersistentState state, long nowMs, int seqBits) {
        long physical = state.physical;
        long seq = state.seq;
        if (nowMs > physical) {
            return PersistentState.of(nowMs, 0);
        }
        long mask = (1L << seqBits) - 1;
        if (seq >= mask) {
            return PersistentState.of(physical + 1, 0);
        }
        return PersistentState.of(physical, seq + 1);
    }

    /** 与 Option.withNodeCount 推导公式一致（用于默认值） */
    private static int seqBitsFromNodeCount(long count) {
        long nodeCount = Math.max(1, count);
        int workerBits = Long.SIZE - Long.numberOfLeadingZeros(nodeCount);
        return PAYLOAD_BITS - workerBits;
    }

    /** 一次推进的结果：物理时间与序列号 */
    public static final class Now {
        private final long physical;
        private final long seq;

        Now(long physical, long seq) {
            this.physical = physical;
            this.seq = seq;
        }

        /** 物理时间（绝对毫秒） */
        public long physical() {
            return physical;
        }

        /** 序列号 */
        public long seq() {
            return seq;
        }

        @Override
        public String toString() {
            return "Now{physical=" + physical + ", seq=" + seq + "}";
        }
    }
}
