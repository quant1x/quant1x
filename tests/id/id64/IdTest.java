// Copyright (c) 2026 Quant1X. All rights reserved.
// Author: wangfeng <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package quant1x.id.id64;

import org.junit.jupiter.api.Test;

import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.junit.jupiter.api.Assertions.fail;

/**
 * Java 版 id64 单元测试，与 Go 版 {@code id_test.go} 一一对应。
 */
class IdTest {

    @Test
    void rollbackMonotonic() {
        AtomicLong fakeNow = new AtomicLong(1000);
        HLC hlc = new HLC(
                Option.withClock(fakeNow::get),
                Option.withSeqSeed(9));

        HLC.Now prev = hlc.now();
        fakeNow.set(500);
        HLC.Now cur = hlc.now();

        boolean monotonic = cur.physical() > prev.physical()
                || (cur.physical() == prev.physical() && cur.seq() > prev.seq());
        if (!monotonic) {
            fail("rollback violated monotonicity: prev=(" + prev.physical() + "," + prev.seq()
                    + ") cur=(" + cur.physical() + "," + cur.seq() + ")");
        }
    }

    @Test
    void usesOptionsAtConstruction() {
        final long fakeNow = 4321;
        HLC hlc = new HLC(
                Option.withClock(() -> fakeNow),
                Option.withSeqSeed(9));

        assertEquals(fakeNow, hlc.timestamp(), "Timestamp()");
        assertEquals(9, hlc.seq, "initial seq");
    }

    @Test
    void nodeCountDerivation() {
        long[][] cases = {
                {1024, 11, 11},
                {5000, 13, 9},
                {3, 2, 20},
                {131072, 18, 4},
        };
        for (long[] c : cases) {
            HLC hlc = new HLC(Option.withNodeCount(c[0]));
            assertEquals((int) c[2], hlc.seqBits(), "seqBits for count=" + c[0]);
            Generator gen = new Generator(0, hlc);
            assertEquals((int) c[1], gen.workerBits(), "workerBits for count=" + c[0]);
        }
    }

    @Test
    void nodeCountTooLarge() {
        assertThrows(IllegalArgumentException.class,
                () -> new HLC(Option.withNodeCount(262144)), "seqBits = 3 < 4");
    }

    @Test
    void fieldDecoding() {
        long elapsed = 0x123456789AL;
        int workerBits = 11;
        int seqBits = 11;
        long nodeID = 0x1F;
        long seq = 0x2A;

        long value = (elapsed << HLC.PAYLOAD_BITS) | (nodeID << seqBits) | seq;
        Id id = Id.fromLong(value);

        assertEquals(elapsed, id.physical(), "physical");
        assertEquals(nodeID, id.nodeId(workerBits), "nodeID");
        assertEquals(seq, id.seq(workerBits), "seq");
        assertEquals(value, id.toLong(), "toLong");
        assertEquals(id, Id.fromBytes(id.bytes()), "bytes round-trip");
    }

    @Test
    void persistentStateAcrossRestart() throws Exception {
        Path stateFile = Files.createTempDirectory("id64").resolve("id64.state");
        long fakeNow = HLC.EPOCH_MS + 1000; // 需在 epoch 之后（Generator 组装时校验）
        Option[] opts = {
                Option.withClock(() -> fakeNow),
                Option.withSeqSeed(9),
                Option.withStateFile(stateFile.toString())
        };

        HLC firstHLC = new HLC(opts);
        long first = new Generator(1, firstHLC).next();
        firstHLC.close(); // 快速路径为批量缓冲：优雅退出前刷盘
        long second = new Generator(1, new HLC(opts)).next();

        assertTrue(first < second, () -> "restart state did not advance: first=" + first + " second=" + second);
    }

    @Test
    void sharedStateFileAcrossInstances() throws Exception {
        Path stateFile = Files.createTempDirectory("id64").resolve("id64.state");
        long fakeNow = HLC.EPOCH_MS + 1000; // 需在 epoch 之后（Generator 组装时校验）
        Option[] opts = {
                Option.withClock(() -> fakeNow),
                Option.withSeqSeed(9),
                Option.withStateFile(stateFile.toString()),
                // 多写者活跃共享：必须显式开启严格模式（每次发号读盘取 max）
                Option.withStateStrict()
        };

        Generator left = new Generator(1, new HLC(opts));
        Generator right = new Generator(1, new HLC(opts));

        long first = left.next();
        long second = right.next();

        assertTrue(first < second, () -> "shared state file did not serialize progress: first=" + first + " second=" + second);
    }

    @Test
    void loadIgnoresCorruptedTail() throws Exception {
        Path stateFile = Files.createTempDirectory("id64").resolve("id64.state");
        FileStateStore store = new FileStateStore(stateFile.toString());

        PersistentState want = PersistentState.of(1234, 99);
        store.appendState(want);

        // 追加 4 字节垃圾，模拟坏损尾部
        Files.write(stateFile, new byte[]{(byte) 0xde, (byte) 0xad, (byte) 0xbe, (byte) 0xef},
                StandardOpenOption.APPEND);

        PersistentState got = store.load().orElseThrow(() -> new AssertionError("Load() empty, want present"));
        assertEquals(want, got);
    }

    @Test
    void nodeIdOutOfRange() {
        HLC hlc = new HLC(Option.withNodeCount(3)); // workerBits=2，nodeID 上限 3
        assertThrows(IllegalArgumentException.class, () -> new Generator(4, hlc));
    }

    @Test
    void concurrent() throws Exception {
        HLC hlc = new HLC();
        Generator gen = new Generator(1, hlc);

        final int n = 200_000;
        final long[] ids = new long[n];

        ExecutorService pool = Executors.newFixedThreadPool(64);
        try {
            CountDownLatch start = new CountDownLatch(1);
            CountDownLatch done = new CountDownLatch(n);
            for (int i = 0; i < n; i++) {
                final int idx = i;
                pool.submit(() -> {
                    try {
                        start.await();
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                        return;
                    }
                    ids[idx] = gen.next();
                    done.countDown();
                });
            }
            start.countDown();
            assertTrue(done.await(120, TimeUnit.SECONDS), "concurrent generation timed out");
        } finally {
            pool.shutdownNow();
        }

        Set<Long> seen = new HashSet<>(n);
        for (long id : ids) {
            assertTrue(seen.add(id), () -> "duplicate id: " + id);
        }

        long[] sorted = ids.clone();
        Arrays.sort(sorted);
        for (int i = 1; i < sorted.length; i++) {
            assertTrue(sorted[i - 1] < sorted[i],
                    "concurrent ordering violation at " + i + "\nprev=" + sorted[i - 1] + "\ncur=" + sorted[i]);
        }
    }
}
