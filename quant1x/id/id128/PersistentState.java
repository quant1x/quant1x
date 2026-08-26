// Copyright (c) 2026 Quant1X. All rights reserved.
// Author: wangfeng <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package quant1x.id.id128;

import java.util.Objects;

/**
 * 可持久化的 HLC 状态快照，对应状态文件中的一条记录。
 *
 * <p>字段语义与 Go/Python 版一致：
 * <ul>
 *   <li>{@code physical} — 物理时间（毫秒，对应 int64）</li>
 *   <li>{@code logical} — 逻辑计数（16 位，对应 uint16）</li>
 *   <li>{@code seq}     — 序列号（32 位，对应 uint32）</li>
 * </ul>
 */
final class PersistentState {

    final long physical;
    final int logical;
    final long seq;

    PersistentState(long physical, int logical, long seq) {
        this.physical = physical;
        this.logical = logical & 0xFFFF;
        this.seq = seq & 0xFFFFFFFFL;
    }

    static PersistentState of(long physical, int logical, long seq) {
        return new PersistentState(physical, logical, seq);
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (!(obj instanceof PersistentState)) {
            return false;
        }
        PersistentState other = (PersistentState) obj;
        return physical == other.physical && logical == other.logical && seq == other.seq;
    }

    @Override
    public int hashCode() {
        return Objects.hash(physical, logical, seq);
    }

    @Override
    public String toString() {
        return "PersistentState{physical=" + physical + ", logical=" + logical + ", seq=" + seq + "}";
    }
}
