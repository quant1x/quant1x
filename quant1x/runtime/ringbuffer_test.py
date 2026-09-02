import threading
import time
import unittest

from quant1x.runtime.ringbuffer import (
    QueueClosedError,
    QueueEmptyError,
    QueueFullError,
    RingBuffer,
)


class TestRingBuffer(unittest.TestCase):
    def test_basic_semantics_and_capacity_round_up(self):
        q = RingBuffer(5)
        self.assertEqual(q.cap(), 8)
        self.assertTrue(q.is_empty())
        self.assertFalse(q.is_full())

        for i in range(8):
            q.try_push(i)
        self.assertTrue(q.is_full())
        with self.assertRaises(QueueFullError):
            q.try_push(99)

        for i in range(8):
            self.assertEqual(q.try_pop(), i)
        self.assertTrue(q.is_empty())
        with self.assertRaises(QueueEmptyError):
            q.try_pop()

    def test_blocking_and_close_behavior(self):
        q = RingBuffer(2)
        q.push(1)
        q.push(2)
        with self.assertRaises(QueueFullError):
            q.try_push(3)

        self.assertEqual(q.pop(), 1)
        self.assertEqual(q.pop(), 2)

        q.close()
        with self.assertRaises(QueueClosedError):
            q.push(3)
        with self.assertRaises(QueueClosedError):
            q.pop()
        self.assertTrue(q.is_closed())

    def test_wait_for_close_drains_existing_items(self):
        q = RingBuffer(4)
        q.push(10)
        q.push(20)

        def consumer():
            self.assertEqual(q.pop(), 10)
            self.assertEqual(q.pop(), 20)
            q.close()

        t = threading.Thread(target=consumer)
        t.start()
        q.wait_for_close()
        t.join(timeout=2)
        self.assertFalse(t.is_alive())

    def test_blocking_pop_waits_until_producer(self):
        q = RingBuffer(2)
        result = {}

        def producer():
            time.sleep(0.05)
            q.push(7)

        t = threading.Thread(target=producer)
        t.start()
        self.assertEqual(q.pop(), 7)
        t.join(timeout=2)
        self.assertFalse(t.is_alive())


if __name__ == "__main__":
    unittest.main()
