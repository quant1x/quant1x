// Copyright (c) 2026 Quant1X. All rights reserved.
// Author: wangfeng <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package quant1x.id.id64;

import java.io.ByteArrayOutputStream;
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
 * <p>状态文件布局：每 18 字节一条记录（与 id128 一致）
 * <pre>
 * |------------|---------|---------|-----------|
 * | Physical   | Logical | Seq     | CRC32     |
 * | 8B         | 2B(恒0) | 4B      | 4B        |
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

    /**
     * 默认落盘间隔：快速路径下状态记录先在内存批量缓冲中累积，
     * 每攒满 N 条才一次性落盘（带跨进程锁 + fsync）。
     * 可用环境变量 {@code QUANT1X_ID64_SYNC_EVERY} 覆盖（显式 Option.withStateSyncEvery 优先级最高）。
     * 默认 1000：大多数请求不碰磁盘；进程异常退出最多丢失最近 1000 条进度
     * （这些 ID 重启后可能重复），优雅退出前调用 {@code HLC.close()} 可零丢失。
     */
    static final long DEFAULT_SYNC_EVERY = 1000;

    /** 返回默认落盘间隔（环境变量 QUANT1X_ID64_SYNC_EVERY，未设置或非法时为 1000）。 */
    static long defaultSyncEvery() {
        String raw = System.getenv("QUANT1X_ID64_SYNC_EVERY");
        if (raw != null) {
            try {
                long value = Long.parseLong(raw.trim());
                if (value > 0) {
                    return value;
                }
            } catch (NumberFormatException ignored) {
                // 回退默认值
            }
        }
        return DEFAULT_SYNC_EVERY;
    }

    private final Path path;
    private final Path lockPath;

    /**
     * 严格模式标志。
     * 默认快速路径（false）：构造时恢复一次水位，运行期纯内存推进，
     * 状态记录先累积在批量缓冲中，攒满 syncEvery 条才一次性落盘（带锁 + force），
     * 热路径零系统调用。适合单写者（含多进程顺序接管 / failover）场景——
     * 新写者构造时读到前任写者最近一次落盘的水位，保证跨进程、跨重启不重复。
     * 开启后每次 {@link #next} 都读盘取 max，保证多写者活跃共享唯一。
     */
    boolean strict;

    /** 目录已创建标志（避免热路径重复 createDirectories） */
    private boolean dirReady;

    /** 每次落盘之间最多累积的记录条数（构造期写入，之后只读） */
    long syncEvery;

    /** 自上次 force 以来已追加的记录条数（严格模式使用） */
    private long unsynced;

    /** 快速路径批量缓冲：尚未落盘的状态记录。仅由 HLC 的锁串行化访问。 */
    private final ByteArrayOutputStream pending;

    FileStateStore(String path) {
        this.path = Paths.get(path);
        this.lockPath = Paths.get(path + ".lock");
        this.syncEvery = defaultSyncEvery();
        this.pending = new ByteArrayOutputStream((int) (syncEvery * RECORD_SIZE));
    }

    @Override
    public Optional<PersistentState> load() throws IOException {
        return loadLatestState();
    }

    @Override
    public PersistentState next(PersistentState local, long nowMs, int seqBits) throws IOException {
        if (!strict) {
            // 快速路径：纯内存推进，记录先入批量缓冲；攒满 syncEvery 条才落盘一次。
            // 进程异常退出最多丢失最近 syncEvery-1 条进度（这些 ID 重启后可能重复），
            // 优雅退出前调用 HLC.close() 可把缓冲完整刷盘、零丢失。
            PersistentState next = HLC.advancePersistentState(local, nowMs, seqBits);
            bufferState(next);
            return next;
        }
        // 加锁：跨进程（以及同 JVM 多实例）串行化发号
        try (FileChannel channel = FileChannel.open(lockPath,
                StandardOpenOption.CREATE, StandardOpenOption.WRITE);
             FileLock ignored = channel.lock()) {
            // 严格模式：以磁盘最新状态为基准（多写者活跃共享唯一性）
            PersistentState base = local;
            Optional<PersistentState> latest = loadLatestState();
            if (latest.isPresent() && comparePersistentState(latest.get(), base) > 0) {
                base = latest.get();
            }
            PersistentState next = HLC.advancePersistentState(base, nowMs, seqBits);
            appendState(next);
            return next;
        }
    }

    /** 把一条状态记录写入批量缓冲，攒满 syncEvery 条时一次性落盘。 */
    private void bufferState(PersistentState state) throws IOException {
        pending.write(encodeState(state));
        if (pending.size() >= syncEvery * RECORD_SIZE) {
            flushPending();
        }
    }

    /** 在跨进程锁保护下，把批量缓冲一次性追加到状态文件并 force。 */
    private void flushPending() throws IOException {
        if (pending.size() == 0) {
            return;
        }
        byte[] records = pending.toByteArray();
        // 目录已就绪时跳过 createDirectories（避免热路径上的 stat 开销）
        if (!dirReady) {
            Path dir = path.getParent();
            if (dir != null) {
                Files.createDirectories(dir);
            }
            dirReady = true;
        }
        try (FileChannel lockChannel = FileChannel.open(lockPath,
                StandardOpenOption.CREATE, StandardOpenOption.WRITE);
             FileLock ignored = lockChannel.lock();
             FileChannel out = FileChannel.open(path,
                StandardOpenOption.CREATE, StandardOpenOption.WRITE, StandardOpenOption.APPEND)) {
            out.write(ByteBuffer.wrap(records));
            out.force(true);
        }
        pending.reset();
    }

    /** 立即把批量缓冲中的记录写入状态文件（带锁 + fsync）。优雅退出前调用可零丢失。 */
    void flush() throws IOException {
        flushPending();
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
            throw new IOException("id64: 状态文件长度非法: " + size);
        }
        long end = size - (size % RECORD_SIZE);
        if (end == 0) {
            throw new IOException("id64: 状态文件长度非法: " + size);
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
                long checksum = ByteIO.readUint32(record, 14);
                if (crc32(record, 0, 14) != checksum) {
                    // 坏损记录，跳过，继续向前找
                    continue;
                }
                return Optional.of(new PersistentState(
                        ByteIO.readUint64(record, 0),
                        ByteIO.readUint32(record, 10)));
            }
        }
        throw new IOException("id64: 状态文件中没有有效记录");
    }

    /** 追加一条状态记录；按 {@link #syncEvery} 间隔执行 fsync（force） */
    void appendState(PersistentState state) throws IOException {
        // 目录已就绪时跳过 createDirectories（避免热路径上的 stat 开销）
        if (!dirReady) {
            Path dir = path.getParent();
            if (dir != null) {
                Files.createDirectories(dir);
            }
            dirReady = true;
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

    /** 编码状态为 18 字节记录（含 CRC32），logical 字段恒 0 */
    static byte[] encodeState(PersistentState state) {
        byte[] record = new byte[RECORD_SIZE];
        ByteIO.writeUint64(record, 0, state.physical);
        ByteIO.writeUint16(record, 8, 0); // logical 恒 0（兼容 id128 的 18B 记录格式）
        ByteIO.writeUint32(record, 10, (int) (state.seq & 0xFFFFFFFFL));
        ByteIO.writeUint32(record, 14, (int) crc32(record, 0, 14));
        return record;
    }

    /** IEEE 802.3 CRC32（对应 Go 的 crc32.ChecksumIEEE） */
    static long crc32(byte[] data, int off, int len) {
        CRC32 crc = new CRC32();
        crc.update(data, off, len);
        return crc.getValue();
    }

    /**
     * 无符号二元组比较（physical/seq）。
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
        if (left.seq < right.seq) {
            return -1;
        }
        if (left.seq > right.seq) {
            return 1;
        }
        return 0;
    }
}
