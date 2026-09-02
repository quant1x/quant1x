package tests.distributed.id;

import org.junit.jupiter.api.Test;
import quant1x.distributed.id.FileStateStore;
import quant1x.distributed.id.Generator;
import quant1x.distributed.id.HLC;
import quant1x.distributed.id.Id;
import quant1x.distributed.id.Option;
import quant1x.distributed.id.PersistentState;

import java.nio.file.Files;
import java.nio.file.Path;

import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class DistributedIdTest {

    @Test
    void idRoundTrip() {
        long elapsed = 0x123456789AL;
        int workerBits = 11;
        int seqBits = 11;
        long nodeId = 0x1F;
        long seq = 0x2A;

        long value = (elapsed << HLC.PAYLOAD_BITS) | (nodeId << seqBits) | seq;
        Id id = Id.fromLong(value);

        assertEquals(elapsed, id.physical());
        assertEquals(nodeId, id.nodeId(workerBits));
        assertEquals(seq, id.seq(workerBits));
        assertEquals(value, id.toLong());
        assertEquals(id, Id.fromBytes(id.bytes()));
    }

    @Test
    void generatorMonotonicAfterClockRollback() {
        AtomicLong now = new AtomicLong(HLC.EPOCH_MS + 1000L);
        HLC hlc = new HLC(Option.withClock(now::get), Option.withSeqSeed(9));

        HLC.Now before = hlc.now();
        now.set(HLC.EPOCH_MS + 500L);
        HLC.Now after = hlc.now();

        assertTrue(after.physical() > before.physical()
                || (after.physical() == before.physical() && after.seq() > before.seq()));
    }

    @Test
    void nodeCountDerivation() {
        HLC hlc = new HLC(Option.withNodeCount(1024));
        assertEquals(11, hlc.seqBits());

        Generator gen = new Generator(1, hlc);
        assertEquals(11, gen.workerBits());
    }

    @Test
    void parallelGenerationIsUnique() throws Exception {
        final int workers = 32;
        final int perWorker = 2000;
        final Set<Long> seen = new HashSet<>();
        final Generator generator = new Generator(1, new HLC());

        ExecutorService pool = Executors.newFixedThreadPool(workers);
        CountDownLatch start = new CountDownLatch(1);
        CountDownLatch done = new CountDownLatch(workers);

        try {
            for (int i = 0; i < workers; i++) {
                pool.submit(() -> {
                    try {
                        start.await();
                        for (int j = 0; j < perWorker; j++) {
                            long value = generator.next();
                            synchronized (seen) {
                                assertTrue(seen.add(value), "duplicate id: " + value);
                            }
                        }
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                        throw new AssertionError(e);
                    } finally {
                        done.countDown();
                    }
                });
            }
            start.countDown();
            assertTrue(done.await(30, TimeUnit.SECONDS));
        } finally {
            pool.shutdownNow();
        }
    }

    @Test
    void persistentStateAcrossRestart() throws Exception {
        Path dir = Files.createTempDirectory("distributed-id");
        Path stateFile = dir.resolve("state.bin");

        long fakeNow = HLC.EPOCH_MS + 1000L;
        HLC first = new HLC(
                Option.withClock(() -> fakeNow),
                Option.withSeqSeed(9),
                Option.withStateFile(stateFile.toString())
        );
        Generator left = new Generator(1, first);
        long firstId = left.next();
        first.close();

        HLC second = new HLC(
                Option.withClock(() -> fakeNow),
                Option.withSeqSeed(9),
                Option.withStateFile(stateFile.toString())
        );
        Generator right = new Generator(1, second);
        long secondId = right.next();

        assertTrue(firstId < secondId, "state did not resume across restart");
        assertTrue(secondId > 0, "second id must be valid");

        PersistentState latest = new FileStateStore(stateFile.toString()).load().orElseThrow();
        assertTrue(latest.physical >= 0, "state should persist");
    }
}
