// Copyright (c) 2026 Quant1X. All rights reserved.
// Author: wangfeng <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package quant1x.id.id128;

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
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.junit.jupiter.api.Assertions.fail;

/**
 * Java 版 id 单元测试，与 Go 版 {@code id_test.go} 一一对应。
 */
class IdTest {

    @Test
    void rollbackMonotonic() {
        AtomicLong fakeNow = new AtomicLong(1000);
        HLC hlc = new HLC(
                Option.withClock(fakeNow::get),
                Option.withLogicalSeed(7));

        HLC.Now prev = hlc.now();
        fakeNow.set(500);
        HLC.Now cur = hlc.now();

        boolean monotonic = cur.hlc() > prev.hlc()
                || (cur.hlc() == prev.hlc() && cur.seq() > prev.seq());
        if (!monotonic) {
            fail("rollback violated monotonicity: prev=(" + Long.toHexString(prev.hlc())
                    + "," + prev.seq() + ") cur=(" + Long.toHexString(cur.hlc()) + "," + cur.seq() + ")");
        }
    }

    @Test
    void usesOptionsAtConstruction() {
        final long fakeNow = 4321;
        HLC hlc = new HLC(
                Option.withClock(() -> fakeNow),
                Option.withLogicalSeed(9));

        assertEquals(fakeNow, hlc.timestamp(), "Timestamp()");
        HLC.Now now = hlc.now();
        assertEquals(9, now.hlc() & 0xFFFF, "logical seed");
    }

    @Test
    void fieldDecoding() {
        long hlcValue = 0x0102030405060708L;
        long nodeID = 0x11223344L;
        long seq = 0xaabbccddL;

        Uint128 raw = Uint128.of(hlcValue, (nodeID << 32) | seq);
        Id id = Id.fromUint128(raw);

        assertEquals(hlcValue, id.hlc(), "hlc");
        assertEquals(nodeID, id.nodeId(), "nodeID");
        assertEquals(seq, id.seq(), "seq");
    }

    @Test
    void persistentStateAcrossRestart() throws Exception {
        Path stateFile = Files.createTempDirectory("id").resolve("hlc.state");
        long fakeNow = 1000;
        Option[] opts = {
                Option.withClock(() -> fakeNow),
                Option.withLogicalSeed(7),
                Option.withStateFile(stateFile.toString())
        };

        Uint128 first = new Generator(1, new HLC(opts)).next();
        Uint128 second = new Generator(1, new HLC(opts)).next();

        assertTrue(first.lt(second), () -> "restart state did not advance: first=" + first + " second=" + second);
    }

    @Test
    void sharedStateFileAcrossInstances() throws Exception {
        Path stateFile = Files.createTempDirectory("id").resolve("hlc.state");
        long fakeNow = 1000;
        Option[] opts = {
                Option.withClock(() -> fakeNow),
                Option.withLogicalSeed(7),
                Option.withStateFile(stateFile.toString())
        };

        Generator left = new Generator(1, new HLC(opts));
        Generator right = new Generator(1, new HLC(opts));

        Uint128 first = left.next();
        Uint128 second = right.next();

        assertTrue(first.lt(second), () -> "shared state file did not serialize progress: first=" + first + " second=" + second);
    }

    @Test
    void loadIgnoresCorruptedTail() throws Exception {
        Path stateFile = Files.createTempDirectory("id").resolve("hlc.state");
        FileStateStore store = new FileStateStore(stateFile.toString());

        PersistentState want = PersistentState.of(1234, 7, 99);
        store.appendState(want);

        // 追加 4 字节垃圾，模拟坏损尾部
        Files.write(stateFile, new byte[]{(byte) 0xde, (byte) 0xad, (byte) 0xbe, (byte) 0xef},
                StandardOpenOption.APPEND);

        PersistentState got = store.load().orElseThrow(() -> new AssertionError("Load() empty, want present"));
        assertEquals(want, got);
    }

    @Test
    void concurrent() throws Exception {
        HLC hlc = new HLC();
        Generator gen = new Generator(1, hlc);

        final int n = 200_000;
        final Uint128[] ids = new Uint128[n];

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

        Set<Uint128> seen = new HashSet<>(n);
        for (Uint128 id : ids) {
            assertTrue(seen.add(id), () -> "duplicate id: " + id);
        }

        Uint128[] sorted = ids.clone();
        Arrays.sort(sorted);
        for (int i = 1; i < sorted.length; i++) {
            assertTrue(sorted[i - 1].lt(sorted[i]),
                    "concurrent ordering violation at " + i + "\nprev=" + sorted[i - 1] + "\ncur=" + sorted[i]);
        }
    }
}
