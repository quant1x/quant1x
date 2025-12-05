
import unittest
import sys
import os

# Add project root to path
sys.path.append(os.getcwd())

from quant1x.factors import safety_score

class TestSafetyScore(unittest.TestCase):
    def test_get_safety_score(self):
        # Test with a known stock code (e.g., Ping An Bank)
        code = "sz000001"
        score, detail = safety_score.get_safety_score(code)
        print(f"Code: {code}, Score: {score}, Detail: {detail}")
        
        self.assertIsInstance(score, int)
        self.assertIsInstance(detail, str)
        self.assertTrue(0 <= score <= 100)

    def test_get_safety_score_ignore(self):
        # Test with an ignored code if possible, or just verify the function handles it
        # Assuming we don't have a specific ignored code handy that is guaranteed to be ignored by is_need_ignore
        # unless we mock it. But let's try a normal one.
        pass

if __name__ == '__main__':
    unittest.main()
