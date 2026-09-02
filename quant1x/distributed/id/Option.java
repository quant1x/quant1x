package quant1x.distributed.id;

import java.util.function.LongSupplier;

@FunctionalInterface
public interface Option {
    void apply(HLC hlc);

    static Option withClock(LongSupplier now) {
        return hlc -> {
            if (now != null) {
                hlc.setNow(now);
            }
        };
    }

    static Option withSeqSeed(int seed) {
        return hlc -> hlc.setSeed(seed);
    }

    static Option withNodeCount(long count) {
        return hlc -> {
            long nodeCount = Math.max(1L, count);
            int workerBits = Long.SIZE - Long.numberOfLeadingZeros(nodeCount);
            int seqBits = HLC.PAYLOAD_BITS - workerBits;
            if (seqBits < 4) {
                throw new IllegalArgumentException("distributed/id: node count is too large");
            }
            hlc.setSeqBits(seqBits);
        };
    }

    static Option withSeqBits(int bits) {
        return hlc -> {
            if (bits < 4 || bits > HLC.PAYLOAD_BITS - 1) {
                throw new IllegalArgumentException("distributed/id: invalid seq bits");
            }
            hlc.setSeqBits(bits);
        };
    }

    static Option withStateFile(String path) {
        return hlc -> {
            if (path != null && !path.isEmpty()) {
                hlc.setStore(new FileStateStore(path));
            }
        };
    }

    static Option withStateFile(java.nio.file.Path path) {
        return hlc -> {
            if (path != null) {
                hlc.setStore(new FileStateStore(path.toString()));
            }
        };
    }
}
