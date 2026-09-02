package quant1x.tests;

import com.lmax.disruptor.BlockingWaitStrategy;
import com.lmax.disruptor.EventFactory;
import com.lmax.disruptor.RingBuffer;
import com.lmax.disruptor.dsl.Disruptor;
import com.lmax.disruptor.dsl.ProducerType;
import org.junit.jupiter.api.Test;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;

import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * LMAX Disruptor 基准测试：
 * - 采用 minimal-change 方案放在根目录 tests 下
 * - 通过 JUnit 执行，确保 Maven test 会真正跑起来
 * - 适合作为 Java 侧吞吐基准对照
 */
public class LMAXDisruptorBenchmark {
    private static final int RING_SIZE = 1024;
    private static final int PRODUCERS = 4;
    private static final int CONSUMERS = 4;
    private static final int EVENTS_PER_PRODUCER = 200_000;

    @Test
    public void benchmarkDisruptorThroughput() throws Exception {
        final int totalEvents = PRODUCERS * EVENTS_PER_PRODUCER;
        final AtomicLong sum = new AtomicLong();
        final AtomicLong received = new AtomicLong();
        final CountDownLatch latch = new CountDownLatch(1);
        final ExecutorService executor = Executors.newFixedThreadPool(CONSUMERS + PRODUCERS);

        Disruptor<LongEvent> disruptor = new Disruptor<>(
                LongEvent::new,
                RING_SIZE,
                executor,
                ProducerType.MULTI,
                new BlockingWaitStrategy()
        );

        disruptor.handleEventsWith((event, sequence, endOfBatch) -> {
            sum.addAndGet(event.value);
            if (received.incrementAndGet() == totalEvents) {
                latch.countDown();
            }
        });

        RingBuffer<LongEvent> ringBuffer = disruptor.start();
        long startNs = System.nanoTime();

        for (int producerId = 0; producerId < PRODUCERS; producerId++) {
            final int id = producerId;
            executor.submit(() -> {
                for (int i = 0; i < EVENTS_PER_PRODUCER; i++) {
                    long sequence = ringBuffer.next();
                    LongEvent event = ringBuffer.get(sequence);
                    event.value = ((long) id * EVENTS_PER_PRODUCER) + i;
                    ringBuffer.publish(sequence);
                }
            });
        }

        boolean finished = latch.await(30, TimeUnit.SECONDS);
        long elapsedMs = TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startNs);

        disruptor.shutdown();
        executor.shutdown();

        assertTrue(finished, "Disruptor benchmark did not finish within timeout");
        System.out.printf(
                "LMAX Disruptor benchmark: producers=%d consumers=%d events=%d elapsedMs=%d throughput=%d ops/s%n",
                PRODUCERS,
                CONSUMERS,
                totalEvents,
                elapsedMs,
                elapsedMs == 0 ? 0 : (totalEvents * 1000L / elapsedMs)
        );
    }

    public static final class LongEvent {
        private long value;

        public static final EventFactory<LongEvent> FACTORY = LongEvent::new;

        public LongEvent() {
        }
    }
}
