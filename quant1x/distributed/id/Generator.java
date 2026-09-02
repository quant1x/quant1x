package quant1x.distributed.id;

/**
 * Generates 64-bit sortable distributed IDs.
 */
public final class Generator {
    private final HLC hlc;
    private final long nodeId;
    private final int workerBits;
    private final int seqBits;

    public Generator(long nodeId, HLC hlc) {
        if (hlc == null) {
            throw new NullPointerException("distributed/id: nil HLC");
        }
        this.hlc = hlc;
        this.seqBits = hlc.seqBits();
        this.workerBits = HLC.PAYLOAD_BITS - this.seqBits;
        if (nodeId < 0 || nodeId >= (1L << workerBits)) {
            throw new IllegalArgumentException(
                    "distributed/id: nodeID " + nodeId + " out of range for " + workerBits + " bits");
        }
        this.nodeId = nodeId;
    }

    public int workerBits() {
        return workerBits;
    }

    public long next() {
        HLC.Now now = hlc.now();
        long elapsed = now.physical() - HLC.EPOCH_MS;
        if (elapsed < 0) {
            throw new IllegalStateException("distributed/id: epoch elapsed out of range: " + elapsed);
        }
        if (elapsed >= (1L << HLC.PHYSICAL_BITS)) {
            throw new IllegalStateException("distributed/id: epoch elapsed out of range: " + elapsed);
        }

        long mask = (1L << workerBits) - 1;
        long seqMask = (1L << seqBits) - 1;
        return (elapsed << HLC.PAYLOAD_BITS)
                | ((nodeId & mask) << seqBits)
                | (now.seq() & seqMask);
    }
}
