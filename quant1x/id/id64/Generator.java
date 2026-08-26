// Copyright (c) 2026 Quant1X. All rights reserved.
// Author: wangfeng <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package quant1x.id.id64;

/**
 * 将 HLC 推进结果与 nodeID 组装为 64 位 ID。
 *
 * <p>位布局（动态位宽，与 Go/Python 版一致）：
 * <pre>
 * | 1bit 符号(恒 0) | Physical(41bit, epoch 相对毫秒) | NodeID(workerBits) | Seq(seqBits) |
 * </pre>
 */
public final class Generator {

    private final HLC hlc;
    private final long nodeID;
    private final int workerBits;
    private final int seqBits;

    /**
     * @param nodeID 节点标识，必须小于 2^workerBits（workerBits = 22 - hlc.seqBits()）
     * @param hlc    HLC 实例
     */
    public Generator(long nodeID, HLC hlc) {
        if (hlc == null) {
            throw new NullPointerException("id64: nil HLC");
        }
        this.hlc = hlc;
        this.seqBits = hlc.seqBits();
        this.workerBits = HLC.PAYLOAD_BITS - this.seqBits;

        if (nodeID < 0 || nodeID >= (1L << workerBits)) {
            throw new IllegalArgumentException(
                    "id64: nodeID " + nodeID + " 超出 " + workerBits + " 位节点位宽");
        }
        this.nodeID = nodeID;
    }

    /** 返回节点位宽 */
    public int workerBits() {
        return workerBits;
    }

    /** 返回下一个 64 位 ID（long 位模式，符号位恒 0，故恒为正） */
    public long next() {
        HLC.Now now = hlc.now();
        long elapsed = now.physical() - HLC.EPOCH_MS;
        if (elapsed < 0) {
            throw new IllegalStateException("id64: 时钟早于 epoch, elapsed=" + elapsed);
        }
        if (elapsed >= (1L << HLC.PHYSICAL_BITS)) {
            throw new IllegalStateException("id64: 时钟超出 41 位容量, elapsed=" + elapsed);
        }
        return (elapsed << HLC.PAYLOAD_BITS)
                | ((nodeID & ((1L << workerBits) - 1)) << seqBits)
                | (now.seq() & ((1L << seqBits) - 1));
    }
}
