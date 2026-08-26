// Copyright (c) 2026 Quant1X. All rights reserved.
// Author: wangfeng <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package quant1x.id.id128;

/**
 * HLC 生成器：将 {@link HLC} 推进结果与 nodeID 组装为完整的 128 位 ID。
 *
 * <p>ID 布局：
 * <pre>
 * |-------------------|-----------|-----------|-----------|
 * |  Physical (48bit) | Logical   | NodeID    | Seq       |
 * |  (毫秒)           | (16bit)   | (32bit)   | (32bit)   |
 * |-------------------|-----------|-----------|-----------|
 * |        hlc = High 64bit       |    Low 64bit          |
 * |-------------------------------|-----------------------|
 * </pre>
 */
public final class Generator {

    private final HLC hlc;
    private final long nodeID;

    /**
     * 构造生成器。
     *
     * @param nodeID 节点 ID（32 位，无符号语义）
     * @param hlc    HLC 实例，不可为 null
     */
    public Generator(long nodeID, HLC hlc) {
        if (hlc == null) {
            throw new NullPointerException("id: nil HLC");
        }
        this.hlc = hlc;
        this.nodeID = nodeID & 0xFFFFFFFFL;
    }

    /**
     * 生成下一个 ID。
     *
     * @return 128 位 {@link Uint128}，{@code hi = hlc, lo = (nodeID << 32) | seq}
     */
    public Uint128 next() {
        HLC.Now now = hlc.now();
        return Uint128.of(now.hlc(), (nodeID << 32) | now.seq());
    }
}
