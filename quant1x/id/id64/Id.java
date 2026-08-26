// Copyright (c) 2026 Quant1X. All rights reserved.
// Author: wangfeng <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package quant1x.id.id64;

import java.util.Arrays;
import java.util.Base64;
import java.util.Objects;

/**
 * 64 位可排序标识。
 *
 * <p>位布局（动态位宽，与 Go/Python 版一致）：
 * <pre>
 * | 1bit 符号(恒 0) | Physical(41bit, epoch 相对毫秒) | NodeID(workerBits) | Seq(seqBits) |
 * </pre>
 *
 * <p>{@link #nodeId(int)} / {@link #seq(int)} 解析需要传入生成器对应的 workerBits。
 */
public final class Id {

    private static final int RAW_LENGTH = 8;

    private final byte[] raw;

    private Id(byte[] raw) {
        if (raw == null || raw.length != RAW_LENGTH) {
            throw new IllegalArgumentException("id64: Id expects exactly 8 bytes");
        }
        this.raw = raw;
    }

    /** 从 long 位模式构造 */
    public static Id fromLong(long value) {
        byte[] raw = new byte[RAW_LENGTH];
        ByteIO.writeUint64(raw, 0, value);
        return new Id(raw);
    }

    /** 从 BigEndian 8 字节构造 */
    public static Id fromBytes(byte[] raw) {
        return new Id(raw);
    }

    /** BigEndian 8 字节表示 */
    public byte[] bytes() {
        return raw.clone();
    }

    /** long 位模式 */
    public long toLong() {
        return ByteIO.readUint64(raw, 0);
    }

    /** base64url 无填充字符串（8 字节 → 11 字符） */
    @Override
    public String toString() {
        return Base64.getUrlEncoder().withoutPadding().encodeToString(raw);
    }

    /** 返回 epoch 相对毫秒（高 41 位） */
    public long physical() {
        return toLong() >> HLC.PAYLOAD_BITS;
    }

    /** 返回节点标识，workerBits 必须与生成器配置一致 */
    public long nodeId(int workerBits) {
        int shift = HLC.PAYLOAD_BITS - workerBits;
        return (toLong() >>> shift) & ((1L << workerBits) - 1);
    }

    /** 返回序列号，workerBits 必须与生成器配置一致 */
    public long seq(int workerBits) {
        int shift = HLC.PAYLOAD_BITS - workerBits;
        return toLong() & ((1L << shift) - 1);
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (!(obj instanceof Id)) {
            return false;
        }
        return Arrays.equals(raw, ((Id) obj).raw);
    }

    @Override
    public int hashCode() {
        return Objects.hash(Arrays.hashCode(raw));
    }
}
