// Copyright (c) 2026 Quant1X. All rights reserved.
// Author: wangfeng <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package quant1x.id.id64;

/**
 * BigEndian 字节编解码工具（替代 id128 中 Uint128 的 read/write helper）。
 */
final class ByteIO {

    private ByteIO() {
    }

    static long readUint64(byte[] b, int off) {
        return ((long) (b[off] & 0xFF) << 56)
                | ((long) (b[off + 1] & 0xFF) << 48)
                | ((long) (b[off + 2] & 0xFF) << 40)
                | ((long) (b[off + 3] & 0xFF) << 32)
                | ((long) (b[off + 4] & 0xFF) << 24)
                | ((long) (b[off + 5] & 0xFF) << 16)
                | ((long) (b[off + 6] & 0xFF) << 8)
                | (b[off + 7] & 0xFF);
    }

    static long readUint32(byte[] b, int off) {
        return ((long) (b[off] & 0xFF) << 24)
                | ((long) (b[off + 1] & 0xFF) << 16)
                | ((long) (b[off + 2] & 0xFF) << 8)
                | (b[off + 3] & 0xFF);
    }

    static int readUint16(byte[] b, int off) {
        return ((b[off] & 0xFF) << 8) | (b[off + 1] & 0xFF);
    }

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

    static void writeUint32(byte[] b, int off, int v) {
        b[off] = (byte) (v >>> 24);
        b[off + 1] = (byte) (v >>> 16);
        b[off + 2] = (byte) (v >>> 8);
        b[off + 3] = (byte) v;
    }

    static void writeUint16(byte[] b, int off, int v) {
        b[off] = (byte) (v >>> 8);
        b[off + 1] = (byte) v;
    }
}
