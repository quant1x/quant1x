import unittest
from quant1x.base.bits import round_up_to_power_of_two

class TestBitsPy(unittest.TestCase):
    def test_zero_and_small_values(self):
        self.assertEqual(round_up_to_power_of_two(0), 1)
        self.assertEqual(round_up_to_power_of_two(1), 1)
        self.assertEqual(round_up_to_power_of_two(2), 2)
        self.assertEqual(round_up_to_power_of_two(3), 4)
        self.assertEqual(round_up_to_power_of_two(5), 8)
        self.assertEqual(round_up_to_power_of_two(1023), 1024)

    def test_max_bits_truncation(self):
        # default max_bits=64 behavior: values > 2^63 should truncate to 2^63
        big = (1 << 63) + 1
        self.assertEqual(round_up_to_power_of_two(big, max_bits=64), 1 << 63)
        # but without truncation (larger max_bits) it increases
        self.assertEqual(round_up_to_power_of_two(big, max_bits=128), 1 << 64)

    def test_non_int_raises(self):
        with self.assertRaises(TypeError):
            round_up_to_power_of_two(3.5)
        with self.assertRaises(TypeError):
            round_up_to_power_of_two("8")

if __name__ == '__main__':
    unittest.main()
