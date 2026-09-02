package quant1x.distributed.id;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.Optional;
import java.util.zip.CRC32;

public final class FileStateStore implements StateStore {
    static final int RECORD_SIZE = 18;
    static final long DEFAULT_SYNC_EVERY = 1000L;

    private final Path path;
    private final Path lockPath;
    private final ByteArrayOutputStream pending;
    private final Object flushLock = new Object();
    private boolean dirReady;
    private long syncEvery;
    private long unsynced;

    public FileStateStore(String path) {
        this.path = Paths.get(path);
        this.lockPath = Paths.get(path + ".lock");
        this.syncEvery = defaultSyncEvery();
        this.pending = new ByteArrayOutputStream((int) (syncEvery * RECORD_SIZE));
    }

    static long defaultSyncEvery() {
        String raw = System.getenv("QUANT1X_ID64_SYNC_EVERY");
        if (raw != null) {
            try {
                long value = Long.parseLong(raw.trim());
                if (value > 0) {
                    return value;
                }
            } catch (NumberFormatException ignored) {
                // fall through to default
            }
        }
        return DEFAULT_SYNC_EVERY;
    }

    @Override
    public Optional<PersistentState> load() throws IOException {
        if (!Files.exists(path)) {
            return Optional.empty();
        }
        long size = Files.size(path);
        if (size < RECORD_SIZE) {
            return Optional.empty();
        }
        long end = size - (size % RECORD_SIZE);
        if (end == 0) {
            return Optional.empty();
        }
        try (FileChannel channel = FileChannel.open(path, StandardOpenOption.READ)) {
            ByteBuffer buffer = ByteBuffer.allocate(RECORD_SIZE);
            for (long offset = end - RECORD_SIZE; offset >= 0; offset -= RECORD_SIZE) {
                buffer.clear();
                int read = channel.read(buffer, offset);
                if (read < RECORD_SIZE) {
                    continue;
                }
                byte[] record = buffer.array();
                long checksum = ByteBuffer.wrap(record, 14, 4).order(ByteOrder.BIG_ENDIAN).getInt() & 0xffffffffL;
                if (crc32(record, 0, 14) != checksum) {
                    continue;
                }
                return Optional.of(new PersistentState(
                        ByteBuffer.wrap(record, 0, 8).order(ByteOrder.BIG_ENDIAN).getLong(),
                        ByteBuffer.wrap(record, 10, 4).order(ByteOrder.BIG_ENDIAN).getInt() & 0xffffffffL));
            }
        }
        return Optional.empty();
    }

    @Override
    public PersistentState next(PersistentState local, long nowMs, int seqBits) throws IOException {
        PersistentState next = HLC.advancePersistentState(local, nowMs, seqBits);
        bufferState(next);
        return next;
    }

    public void flush() throws IOException {
        flushPending();
    }

    private void bufferState(PersistentState state) throws IOException {
        synchronized (flushLock) {
            pending.write(encodeState(state));
            if (pending.size() >= syncEvery * RECORD_SIZE) {
                flushPending();
            }
        }
    }

    private void flushPending() throws IOException {
        if (pending.size() == 0) {
            return;
        }
        if (!dirReady) {
            Path dir = path.getParent();
            if (dir != null) {
                Files.createDirectories(dir);
            }
            dirReady = true;
        }
        byte[] records = pending.toByteArray();
        try (FileChannel lockChannel = FileChannel.open(lockPath,
                StandardOpenOption.CREATE, StandardOpenOption.WRITE);
             FileLock ignored = lockChannel.lock();
             FileChannel out = FileChannel.open(path,
                     StandardOpenOption.CREATE,
                     StandardOpenOption.WRITE,
                     StandardOpenOption.APPEND)) {
            out.write(ByteBuffer.wrap(records));
            out.force(true);
        }
        pending.reset();
    }

    static long crc32(byte[] data, int off, int len) {
        CRC32 crc = new CRC32();
        crc.update(data, off, len);
        return crc.getValue();
    }

    static byte[] encodeState(PersistentState state) {
        byte[] record = new byte[RECORD_SIZE];
        ByteBuffer.wrap(record).order(ByteOrder.BIG_ENDIAN)
                .putLong(state.physical)
                .putShort((short) 0)
                .putInt((int) (state.seq & 0xFFFFFFFFL));
        long checksum = crc32(record, 0, 14);
        ByteBuffer.wrap(record, 14, 4).order(ByteOrder.BIG_ENDIAN).putInt((int) (checksum & 0xffffffffL));
        return record;
    }
}
