// Copyright (c) 2026 Quant1X. All rights reserved.
// Author: wangfeng <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package quant1x.id.id128;

/**
 * 不可变的 128 位无符号整数，是 id 的核心数据类型，与 Go/Python 版保持
 * 一致的位布局与语义（真实 128 位、BigEndian 编解码、无符号比较与算术）。
 */
public final class Uint128 implements Comparable<Uint128> {

    /** 0 */
    public static final Uint128 ZERO = new Uint128(0L, 0L);
    /** 1 */
    public static final Uint128 ONE = new Uint128(0L, 1L);
    /** 最大值：2^128 - 1 */
    public static final Uint128 MAX = new Uint128(-1L, -1L);

    private final long hi;
    private final long lo;

    /**
     * 构造 Uint128。
     *
     * @param hi 高 64 位（位模式，无符号语义）
     * @param lo 低 64 位（位模式，无符号语义）
     */
    public Uint128(long hi, long lo) {
        this.hi = hi;
        this.lo = lo;
    }

    /**
     * 等价于 {@code new Uint128(hi, lo)}，便于静态导入。
     */
    public static Uint128 of(long hi, long lo) {
        return new Uint128(hi, lo);
    }

    /**
     * 由 64 位无符号整数构造，高位补零。
     */
    public static Uint128 fromLong(long value) {
        return new Uint128(0L, value);
    }

    /**
     * 由 16 字节 BigEndian 字节序列构造。
     *
     * @throws IllegalArgumentException 当 {@code bytes.length != 16}
     */
    public static Uint128 fromBytes(byte[] bytes) {
        if (bytes.length != 16) {
            throw new IllegalArgumentException("Uint128 expects exactly 16 bytes, got " + bytes.length);
        }
        return new Uint128(readUint64(bytes, 0), readUint64(bytes, 8));
    }

    /**
     * 编码为 16 字节 BigEndian 字节序列。
     */
    public byte[] toBytes() {
        byte[] out = new byte[16];
        writeUint64(out, 0, hi);
        writeUint64(out, 8, lo);
        return out;
    }

    /**
     * 无符号比较。
     *
     * @return 负数/0/正数 表示 this 小于/等于/大于 o
     */
    @Override
    public int compareTo(Uint128 o) {
        int cmp = Long.compareUnsigned(hi, o.hi);
        if (cmp != 0) {
            return cmp;
        }
        return Long.compareUnsigned(lo, o.lo);
    }

    /** 无符号小于 */
    public boolean lt(Uint128 other) {
        return compareTo(other) < 0;
    }

    /** 无符号小于等于 */
    public boolean le(Uint128 other) {
        return compareTo(other) <= 0;
    }

    /** 无符号大于 */
    public boolean gt(Uint128 other) {
        return compareTo(other) > 0;
    }

    /** 无符号大于等于 */
    public boolean ge(Uint128 other) {
        return compareTo(other) >= 0;
    }

    /** 等于 */
    public boolean eq(Uint128 other) {
        return hi == other.hi && lo == other.lo;
    }

    /**
     * 无符号加法，溢出按 2^128 回绕。
     */
    public Uint128 add(Uint128 other) {
        long lo = this.lo + other.lo;
        boolean carry = Long.compareUnsigned(lo, this.lo) < 0;
        long hi = this.hi + other.hi + (carry ? 1L : 0L);
        return new Uint128(hi, lo);
    }

    /**
     * 无符号减法，下溢按 2^128 回绕。
     */
    public Uint128 sub(Uint128 other) {
        long lo = this.lo - other.lo;
        boolean borrow = Long.compareUnsigned(this.lo, other.lo) < 0;
        long hi = this.hi - other.hi - (borrow ? 1L : 0L);
        return new Uint128(hi, lo);
    }

    /** 自增（加 1） */
    public Uint128 inc() {
        return add(ONE);
    }

    /** 自减（减 1） */
    public Uint128 dec() {
        return sub(ONE);
    }

    /**
     * 左移 {@code n} 位（0 <= n <= 127，超出范围视为 &gt;= 128 返回 0）。
     */
    public Uint128 shiftLeft(int n) {
        if (n >= 128) {
            return ZERO;
        }
        if (n >= 64) {
            return new Uint128(lo << (n - 64), 0L);
        }
        if (n == 0) {
            return this;
        }
        return new Uint128((hi << n) | (lo >>> (64 - n)), lo << n);
    }

    /**
     * 逻辑右移 {@code n} 位（0 <= n <= 127，超出范围视为 &gt;= 128 返回 0）。
     */
    public Uint128 shiftRight(int n) {
        if (n >= 128) {
            return ZERO;
        }
        if (n >= 64) {
            return new Uint128(0L, hi >>> (n - 64));
        }
        if (n == 0) {
            return this;
        }
        return new Uint128(hi >>> n, (hi << (64 - n)) | (lo >>> n));
    }

    /** 按位或 */
    public Uint128 or(Uint128 other) {
        return new Uint128(hi | other.hi, lo | other.lo);
    }

    /** 按位与 */
    public Uint128 and(Uint128 other) {
        return new Uint128(hi & other.hi, lo & other.lo);
    }

    /** 按位异或 */
    public Uint128 xor(Uint128 other) {
        return new Uint128(hi ^ other.hi, lo ^ other.lo);
    }

    /** 按位取反 */
    public Uint128 not() {
        return new Uint128(~hi, ~lo);
    }

    /** 是否为零 */
    public boolean isZero() {
        return hi == 0L && lo == 0L;
    }

    /** 高 64 位（位模式） */
    public long high64() {
        return hi;
    }

    /** 低 64 位（位模式） */
    public long low64() {
        return lo;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (!(obj instanceof Uint128)) {
            return false;
        }
        Uint128 other = (Uint128) obj;
        return hi == other.hi && lo == other.lo;
    }

    @Override
    public int hashCode() {
        int result = Long.hashCode(hi);
        result = 31 * result + Long.hashCode(lo);
        return result;
    }

    @Override
    public String toString() {
        return String.format("Uint128{hi=0x%016x, lo=0x%016x}", hi, lo);
    }

    // ------------------------------------------------------------------
    // 包内共享的 BigEndian 读写工具
    // ------------------------------------------------------------------

    /** 读取 BigEndian uint64（8 字节） */
    static long readUint64(byte[] b, int off) {
        return ((long) (b[off] & 0xFF) << 56)
                | ((long) (b[off + 1] & 0xFF) << 48)
                | ((long) (b[off + 2] & 0xFF) << 40)
                | ((long) (b[off + 3] & 0xFF) << 32)
                | ((long) (b[off + 4] & 0xFF) << 24)
                | ((long) (b[off + 5] & 0xFF) << 16)
                | ((long) (b[off + 6] & 0xFF) << 8)
                | (long) (b[off + 7] & 0xFF);
    }

    /** 写入 BigEndian uint64（8 字节） */
    static void writeUint64(byte[] b, int off, long v) {
        b[off] = (byte) (v >>> 56);
        b[off + 1] = (byte) (v >>> 48);
        b[off + 2] = (byte) (v >>> 40);
        b[off + 3] = (byte) (v >>> 32);
        b[off + 4] = (byte) (v >>> 24);
        b[off + 5] = (byte) (v >>> 16);
        b[off + 6] = (byte) (v >>> 8);
        b[off + 7] = (byte) v;
    }

    /** 读取 BigEndian uint32（4 字节） */
    static long readUint32(byte[] b, int off) {
        return ((long) (b[off] & 0xFF) << 24)
                | ((long) (b[off + 1] & 0xFF) << 16)
                | ((long) (b[off + 2] & 0xFF) << 8)
                | (long) (b[off + 3] & 0xFF);
    }

    /** 写入 BigEndian uint32（4 字节） */
    static void writeUint32(byte[] b, int off, long v) {
        b[off] = (byte) (v >>> 24);
        b[off + 1] = (byte) (v >>> 16);
        b[off + 2] = (byte) (v >>> 8);
        b[off + 3] = (byte) v;
    }

    /** 读取 BigEndian uint16（2 字节） */
    static int readUint16(byte[] b, int off) {
        return ((b[off] & 0xFF) << 8) | (b[off + 1] & 0xFF);
    }

    /** 写入 BigEndian uint16（2 字节） */
    static void writeUint16(byte[] b, int off, int v) {
        b[off] = (byte) (v >>> 8);
        b[off + 1] = (byte) v;
    }
}
