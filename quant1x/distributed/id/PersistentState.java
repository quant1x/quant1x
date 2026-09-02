package quant1x.distributed.id;

import java.util.Objects;

public final class PersistentState {
    public final long physical;
    public final long seq;

    public PersistentState(long physical, long seq) {
        this.physical = physical;
        this.seq = seq & 0xFFFFFFFFL;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (!(obj instanceof PersistentState)) {
            return false;
        }
        PersistentState other = (PersistentState) obj;
        return physical == other.physical && seq == other.seq;
    }

    @Override
    public int hashCode() {
        return Objects.hash(physical, seq);
    }

    @Override
    public String toString() {
        return "PersistentState{physical=" + physical + ", seq=" + seq + "}";
    }
}
