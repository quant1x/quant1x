package quant1x.distributed.id;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;
import java.util.Base64;
import java.util.Objects;

/**
 * 64-bit sortable distributed identifier.
 * Layout:
 * | 1 bit sign(0) | physical(41 bits) | nodeId(workerBits) | seq(seqBits) |
 */
public final class Id {
    private static final int RAW_LENGTH = 8;
    private final byte[] raw;

    private Id(byte[] raw) {
        if (raw == null || raw.length != RAW_LENGTH) {
            throw new IllegalArgumentException("distributed/id: Id expects exactly 8 bytes");
        }
        this.raw = raw.clone();
    }

    public static Id fromLong(long value) {
        byte[] raw = new byte[RAW_LENGTH];
        ByteBuffer.wrap(raw).order(ByteOrder.BIG_ENDIAN).putLong(value);
        return new Id(raw);
    }

    public static Id fromBytes(byte[] raw) {
        return new Id(raw);
    }

    public byte[] bytes() {
        return raw.clone();
    }

    public long toLong() {
        return ByteBuffer.wrap(raw).order(ByteOrder.BIG_ENDIAN).getLong();
    }

    public long physical() {
        return toLong() >>> HLC.PAYLOAD_BITS;
    }

    public long nodeId(int workerBits) {
        int shift = HLC.PAYLOAD_BITS - workerBits;
        return (toLong() >>> shift) & ((1L << workerBits) - 1);
    }

    public long seq(int workerBits) {
        int shift = HLC.PAYLOAD_BITS - workerBits;
        return toLong() & ((1L << shift) - 1);
    }

    @Override
    public String toString() {
        return Base64.getUrlEncoder().withoutPadding().encodeToString(raw);
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (!(obj instanceof Id)) {
            return false;
        }
        Id other = (Id) obj;
        return Arrays.equals(this.raw, other.raw);
    }

    @Override
    public int hashCode() {
        return Objects.hashCode(Arrays.hashCode(raw));
    }
}
