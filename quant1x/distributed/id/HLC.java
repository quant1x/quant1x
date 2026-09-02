package quant1x.distributed.id;

import java.io.IOException;
import java.security.SecureRandom;
import java.util.Optional;
import java.util.function.LongSupplier;

/**
 * HLC 版本的分布式 ID 时钟，用于推进 (physical, seq) 对。
 * 语义与 Go/C++/Rust 版保持一致：
 * - physical 为相对 2026-01-01 的毫秒值
 * - seq 为 0..2^seqBits-1 递增
 * - 发生时钟回拨时强制递增 seq / physical 保持单调
 */
public final class HLC {
    public static final long EPOCH_MS = 1767225600000L;
    public static final int PAYLOAD_BITS = 22;
    public static final int PHYSICAL_BITS = 41;

    private static final SecureRandom RANDOM = new SecureRandom();
    private static final int RANDOM_SEED = RANDOM.nextInt(0x10000);

    private final Object lock = new Object();
    private LongSupplier now;
    private int seed;
    private int seqBits;
    private long physical;
    private long seq;
    private StateStore store;

    public HLC(Option... options) {
        this.now = System::currentTimeMillis;
        this.seed = RANDOM_SEED;
        this.seqBits = PAYLOAD_BITS - bitsForNodeCount(1024);
        for (Option option : options) {
            if (option != null) {
                option.apply(this);
            }
        }
        Optional<PersistentState> restored = loadState();
        if (restored.isPresent()) {
            this.physical = restored.get().physical;
            this.seq = restored.get().seq;
        } else {
            this.physical = now.getAsLong();
            this.seq = seed & seqMask();
        }
    }

    long seqMask() {
        return (1L << seqBits) - 1;
    }

    public long timestamp() {
        synchronized (lock) {
            return physical;
        }
    }

    public int seqBits() {
        return seqBits;
    }

    public Now now() {
        synchronized (lock) {
            PersistentState current = new PersistentState(physical, seq);
            long nowMs = now.getAsLong();
            PersistentState next;
            if (store != null) {
                try {
                    next = store.next(current, nowMs, seqBits);
                } catch (IOException e) {
                    throw new IllegalStateException("distributed/id: state store failure", e);
                }
            } else {
                next = advancePersistentState(current, nowMs, seqBits);
            }
            physical = next.physical;
            seq = next.seq;
            return new Now(physical, seq);
        }
    }

    public void close() throws IOException {
        synchronized (lock) {
            if (store instanceof FileStateStore) {
                ((FileStateStore) store).flush();
            }
        }
    }

    private Optional<PersistentState> loadState() {
        if (store == null) {
            return Optional.empty();
        }
        try {
            return store.load();
        } catch (IOException e) {
            throw new IllegalStateException("distributed/id: load state failed", e);
        }
    }

    static PersistentState advancePersistentState(PersistentState state, long nowMs, int seqBits) {
        long physical = state.physical;
        long seq = state.seq;
        if (nowMs > physical) {
            return new PersistentState(nowMs, 0L);
        }
        long mask = (1L << seqBits) - 1L;
        if (seq >= mask) {
            return new PersistentState(physical + 1L, 0L);
        }
        return new PersistentState(physical, seq + 1L);
    }

    static int bitsForNodeCount(long count) {
        long nodeCount = Math.max(1L, count);
        return Long.SIZE - Long.numberOfLeadingZeros(nodeCount);
    }

    public static final class Now {
        private final long physical;
        private final long seq;

        private Now(long physical, long seq) {
            this.physical = physical;
            this.seq = seq;
        }

        public long physical() {
            return physical;
        }

        public long seq() {
            return seq;
        }
    }

    // package-private setter for Option
    void setNow(LongSupplier now) {
        this.now = now;
    }

    void setSeed(int seed) {
        this.seed = seed & 0xFFFF;
    }

    void setSeqBits(int bits) {
        this.seqBits = bits;
    }

    void setStore(StateStore store) {
        this.store = store;
    }
}
