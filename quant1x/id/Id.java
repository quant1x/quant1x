// Copyright (c) 2026 Quant1X. All rights reserved.
// Author: wangfeng <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package quant1x.id;

import java.util.Arrays;
import java.util.Base64;

/**
 * ID 的解析视图：从 16 字节原始数据中提取 hlc / nodeID / seq，
 * 并提供 base64url（无填充）字符串与字节数组两种交换格式，
 * 与 Go 版的 {@code ID} / Python 版 {@code id} 对齐。
 */
public final class Id {

    private final byte[] raw;

    private Id(byte[] raw) {
        if (raw.length != 16) {
            throw new IllegalArgumentException("ID expects exactly 16 bytes, got " + raw.length);
        }
        this.raw = raw.clone();
    }

    /**
     * 由 128 位整数构造 ID。
     */
    public static Id fromUint128(Uint128 value) {
        return new Id(value.toBytes());
    }

    /**
     * 由 16 字节原始数据构造 ID。
     *
     * @throws IllegalArgumentException 当 {@code raw.length != 16}
     */
    public static Id fromBytes(byte[] raw) {
        return new Id(raw);
    }

    /**
     * 原始 16 字节（BigEndian）。
     */
    public byte[] bytes() {
        return raw.clone();
    }

    /**
     * base64url 无填充字符串（与 Go 的 RawURLEncoding / Python 的
     * {@code urlsafe_b64encode().rstrip(b"=")} 一致）。
     */
    @Override
    public String toString() {
        return Base64.getUrlEncoder().withoutPadding().encodeToString(raw);
    }

    /** hlc（高 64 位） */
    public long hlc() {
        return Uint128.readUint64(raw, 0);
    }

    /** nodeID（32 位，以 long 承载无符号值） */
    public long nodeId() {
        return Uint128.readUint32(raw, 8);
    }

    /** seq（32 位，以 long 承载无符号值） */
    public long seq() {
        return Uint128.readUint32(raw, 12);
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
        return Arrays.hashCode(raw);
    }
}
