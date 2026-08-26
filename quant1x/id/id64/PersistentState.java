// Copyright (c) 2026 Quant1X. All rights reserved.
// Author: wangfeng <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package quant1x.id.id64;

import java.util.Objects;

/**
 * 可持久化的 HLC 状态快照，对应状态文件中的一条记录。
 *
 * <p>字段语义与 Go/Python 版一致：
 * <ul>
 *   <li>{@code physical} — 物理时间（毫秒，对应 int64）</li>
 *   <li>{@code seq}     — 序列号（32 位语义，以 long 承载无符号值）</li>
 * </ul>
 */
final class PersistentState {

    final long physical;
    final long seq;

    PersistentState(long physical, long seq) {
        this.physical = physical;
        this.seq = seq & 0xFFFFFFFFL;
    }

    static PersistentState of(long physical, long seq) {
        return new PersistentState(physical, seq);
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
        return physical == other.physical && seq == other.seq;
    }

    @Override
    public int hashCode() {
        return Objects.hash(physical, seq);
    }

    @Override
    public String toString() {
        return "PersistentState{physical=" + physical + ", seq=" + seq + "}";
    }
}
