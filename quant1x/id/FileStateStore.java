// Copyright (c) 2026 Quant1X. All rights reserved.
// Author: wangfeng <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package quant1x.id;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.Optional;
import java.util.zip.CRC32;

/**
 * 基于文件的 {@link StateStore} 实现，与 Go/Python 版保持 1:1 对齐。
 *
 * <p>状态文件布局：每 18 字节一条记录
 * <pre>
 * |------------|---------|---------|-----------|
 * | Physical   | Logical | Seq     | CRC32     |
 * | 8B         | 2B      | 4B      | 4B        |
 * |------------|---------|---------|-----------|
 * </pre>
 * 全部使用 BigEndian；CRC32 为 IEEE 802.3（对应 Go 的 crc32.ChecksumIEEE），
 * 覆盖前 14 字节。坏损/截断的尾部记录会被忽略。
 *
 * <p>并发串行化：通过 {@code <path>.lock} 文件上的跨进程文件锁实现，
 * Java 的 {@link FileChannel#lock()} 在 Windows（LockFileEx）与 Unix
 * （fcntl/flock）下均可用，且同一 JVM 内多实例也会互斥。
 */
final class FileStateStore implements StateStore {

    /** 单条状态记录大小（字节） */
    static final int RECORD_SIZE = 18;

    private final Path path;
    private final Path lockPath;

    /** 每次 fsync 之间最多追加的记录条数（构造期写入，之后只读） */
    long syncEvery;

    /** 自上次 fsync 以来已追加的记录条数 */
    private long unsynced;

    FileStateStore(String path) {
        this.path = Paths.get(path);
        this.lockPath = Paths.get(path + ".lock");
        this.syncEvery = 1;
    }

    @Override
    public Optional<PersistentState> load() throws IOException {
        return loadLatestState();
    }

    @Override
    public PersistentState next(PersistentState local, long nowMs, int seed) throws IOException {
        // 加锁：跨进程（以及同 JVM 多实例）串行化发号
        try (FileChannel channel = FileChannel.open(lockPath,
                StandardOpenOption.CREATE, StandardOpenOption.WRITE);
             FileLock ignored = channel.lock()) {
            Optional<PersistentState> latest = loadLatestState();
            PersistentState base = local;
            if (latest.isPresent() && comparePersistentState(latest.get(), base) > 0) {
                base = latest.get();
            }
            PersistentState next = HLC.advancePersistentState(base, nowMs, seed);
            appendState(next);
            return next;
        }
    }

    /**
     * 读取状态文件中的最后一条有效记录。
     *
     * @return 状态文件不存在时返回空
     * @throws IOException 文件长度非法或没有有效记录时抛出
     */
    private Optional<PersistentState> loadLatestState() throws IOException {
        if (!Files.exists(path)) {
            return Optional.empty();
        }
        long size = Files.size(path);
        if (size < RECORD_SIZE) {
            throw new IOException("id: 状态文件长度非法: " + size);
        }
        long end = size - (size % RECORD_SIZE);
        if (end == 0) {
            throw new IOException("id: 状态文件长度非法: " + size);
        }
        try (FileChannel channel = FileChannel.open(path, StandardOpenOption.READ)) {
            ByteBuffer buf = ByteBuffer.allocate(RECORD_SIZE);
            for (long offset = end - RECORD_SIZE; offset >= 0; offset -= RECORD_SIZE) {
                buf.clear();
                int n = channel.read(buf, offset);
                if (n < RECORD_SIZE) {
                    // 读不满一条记录（文件被截断），跳过，继续向前找
                    continue;
                }
                byte[] record = buf.array();
                long checksum = Uint128.readUint32(record, 14);
                if (crc32(record, 0, 14) != checksum) {
                    // 坏损记录，跳过，继续向前找
                    continue;
                }
                return Optional.of(new PersistentState(
                        Uint128.readUint64(record, 0),
                        Uint128.readUint16(record, 8),
                        Uint128.readUint32(record, 10)));
            }
        }
        throw new IOException("id: 状态文件中没有有效记录");
    }

    /** 追加一条状态记录；按 {@link #syncEvery} 间隔执行 fsync（force） */
    void appendState(PersistentState state) throws IOException {
        Path dir = path.getParent();
        if (dir != null) {
            Files.createDirectories(dir);
        }
        byte[] record = encodeState(state);
        try (FileChannel channel = FileChannel.open(path,
                StandardOpenOption.CREATE, StandardOpenOption.WRITE, StandardOpenOption.APPEND)) {
            channel.write(ByteBuffer.wrap(record));
            unsynced++;
            long every = syncEvery > 0 ? syncEvery : 1;
            if (unsynced >= every) {
                channel.force(true);
                unsynced = 0;
            }
        }
    }

    // ------------------------------------------------------------------
    // 记录编解码
    // ------------------------------------------------------------------

    /** 编码状态为 18 字节记录（含 CRC32） */
    static byte[] encodeState(PersistentState state) {
        byte[] record = new byte[RECORD_SIZE];
        Uint128.writeUint64(record, 0, state.physical);
        Uint128.writeUint16(record, 8, state.logical & 0xFFFF);
        Uint128.writeUint32(record, 10, state.seq & 0xFFFFFFFFL);
        Uint128.writeUint32(record, 14, crc32(record, 0, 14));
        return record;
    }

    /** IEEE 802.3 CRC32（对应 Go 的 crc32.ChecksumIEEE） */
    static long crc32(byte[] data, int off, int len) {
        CRC32 crc = new CRC32();
        crc.update(data, off, len);
        return crc.getValue();
    }

    /**
     * 无符号三元组比较（physical/logical/seq）。
     *
     * @return 负数/0/正数 表示 left 小于/等于/大于 right
     */
    static int comparePersistentState(PersistentState left, PersistentState right) {
        if (left.physical < right.physical) {
            return -1;
        }
        if (left.physical > right.physical) {
            return 1;
        }
        if (left.logical < right.logical) {
            return -1;
        }
        if (left.logical > right.logical) {
            return 1;
        }
        if (left.seq < right.seq) {
            return -1;
        }
        if (left.seq > right.seq) {
            return 1;
        }
        return 0;
    }
}
