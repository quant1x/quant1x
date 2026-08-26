// Copyright (c) 2026 Quant1X. All rights reserved.
// Author: wangfeng <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package quant1x.id;

import java.io.IOException;
import java.security.SecureRandom;
import java.util.Optional;
import java.util.function.LongSupplier;

/**
 * 基于本地 HLC（混合逻辑时钟）状态生成可排序的 128 位 ID 的高 64 位与序列号，
 * 与 Go/Python 版保持 1:1 对齐。
 *
 * <p>算法要点：
 * <ul>
 *   <li>高 48 位为物理时间（毫秒），中 16 位为逻辑计数，共同构成高 64 位 hlc；
 *       即使系统时钟回拨，也能保证 hlc 的单调不减。</li>
 *   <li>低 32 位 seq 在同一物理毫秒内递增；seq 溢出时逻辑计数进位，逻辑计数
 *       溢出时物理时间 +1。</li>
 *   <li>启用状态文件后，发号前会将内存状态与磁盘最新状态取 max 再推进，
 *       实现跨重启/多实例强唯一。</li>
 * </ul>
 */
public final class HLC {

    /** 物理时间（毫秒） */
    long physical;
    /** 逻辑计数（16 位） */
    int logical;
    /** 序列号（32 位） */
    long seq;

    /** 时钟源 */
    LongSupplier now;
    /** 逻辑种子（16 位） */
    int seed;
    /** 每次 fsync 之间最多追加的记录条数 */
    long syncEvery;
    /** 状态存储，null 表示不持久化 */
    StateStore store;

    private final Object lock = new Object();
    private static final SecureRandom RANDOM = new SecureRandom();

    /**
     * 构造 HLC 实例；构造时会尝试从状态文件恢复持久化状态，
     * 恢复失败（如存储异常）将抛出 {@link IllegalStateException}。
     *
     * @param options 配置项，可空
     */
    public HLC(Option... options) {
        now = System::currentTimeMillis;
        seed = randomUint16();
        syncEvery = 1;

        for (Option opt : options) {
            if (opt != null) {
                opt.apply(this);
            }
        }

        if (store instanceof FileStateStore) {
            ((FileStateStore) store).syncEvery = syncEvery;
        }

        Optional<PersistentState> restored = loadState();
        if (restored.isPresent()) {
            PersistentState state = restored.get();
            physical = state.physical;
            logical = state.logical;
            seq = state.seq;
        } else {
            physical = now.getAsLong();
            logical = seed;
        }
    }

    /**
     * 推进本地 HLC 状态。
     *
     * @return {@link Now}，其中 {@code hlc = (physical << 16) | logical}，
     *         与 {@code seq} 一起构成 ID 的高 64 位与低 32 位（低 32 位尚有 nodeID 参与）。
     */
    public Now now() {
        synchronized (lock) {
            PersistentState current = PersistentState.of(physical, logical, seq);
            PersistentState next;
            long nowMs = now.getAsLong();
            if (store != null) {
                try {
                    next = store.next(current, nowMs, seed);
                } catch (IOException e) {
                    throw new IllegalStateException("id: 状态存储失败", e);
                }
            } else {
                next = advancePersistentState(current, nowMs, seed);
            }
            physical = next.physical;
            logical = next.logical;
            seq = next.seq;
            return new Now(((long) physical << 16) | (logical & 0xFFFFL), seq);
        }
    }

    /**
     * 当前物理时间（毫秒），对应 Go 版的 {@code Timestamp()}。
     */
    public long timestamp() {
        synchronized (lock) {
            return physical;
        }
    }

    /**
     * 从状态存储恢复持久化状态。
     *
     * @throws IllegalStateException 存储读取失败时抛出
     */
    private Optional<PersistentState> loadState() {
        if (store == null) {
            return Optional.empty();
        }
        try {
            return store.load();
        } catch (IOException e) {
            throw new IllegalStateException("id: 恢复持久化状态失败", e);
        }
    }

    /**
     * 推进持久化状态，与 Go/Python 版逻辑一致。
     *
     * <ul>
     *   <li>物理时间前进时，逻辑计数与序列号重置（logical=seed, seq=0）</li>
     *   <li>否则序列号 +1；seq 溢出时逻辑计数 +1；逻辑计数溢出时物理时间 +1</li>
     * </ul>
     */
    static PersistentState advancePersistentState(PersistentState state, long nowMs, int seed) {
        long physical = state.physical;
        int logical = state.logical;
        long seq = state.seq;

        if (nowMs > physical) {
            return PersistentState.of(nowMs, seed & 0xFFFF, 0);
        }

        seq = (seq + 1) & 0xFFFFFFFFL;
        if (seq == 0) {
            logical = (logical + 1) & 0xFFFF;
            if (logical == 0) {
                physical = physical + 1;
                logical = seed & 0xFFFF;
            }
        }
        return PersistentState.of(physical, logical, seq);
    }

    /** 生成加密随机 16 位种子 */
    private static int randomUint16() {
        byte[] buf = new byte[2];
        RANDOM.nextBytes(buf);
        return ((buf[0] & 0xFF) << 8) | (buf[1] & 0xFF);
    }

    /**
     * HLC 推进结果：hlc（高 64 位）与 seq（低 32 位，未含 nodeID）。
     */
    public static final class Now {
        private final long hlc;
        private final long seq;

        Now(long hlc, long seq) {
            this.hlc = hlc;
            this.seq = seq;
        }

        /** hlc = (physical << 16) | logical */
        public long hlc() {
            return hlc;
        }

        /** 序列号（32 位） */
        public long seq() {
            return seq;
        }

        @Override
        public String toString() {
            return "Now{hlc=0x" + Long.toHexString(hlc) + ", seq=" + seq + "}";
        }
    }
}
