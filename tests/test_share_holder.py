
import unittest
import pandas as pd
from quant1x.factors import share_holder
from quant1x.config import config
from quant1x import std

class TestShareHolder(unittest.TestCase):
    def test_get_quarter_by_date(self):
        q, f, l = std.get_quarter_by_date("2023-05-15")
        self.assertEqual(q, "2023Q2")
        self.assertEqual(f, "2023-04-01")
        self.assertEqual(l, "2023-06-30")
        
        q, f, l = std.get_quarter_by_date("2023-05-15", diff_quarters=1)
        self.assertEqual(q, "2023Q1")
        self.assertEqual(f, "2023-01-01")
        self.assertEqual(l, "2023-03-31")

    def test_fetch_share_holder(self):
        # Use a real stock and a past date that definitely has data
        code = "SH600000"
        # 2024-04-01 should look for 2024Q1 (ends 2024-03-31)
        # But reports are usually delayed. 
        # Let's use a very safe date: 2023-12-31 report, accessed from 2024-05-01
        date = "2024-05-01" 
        
        # fetch_share_holder(code, date, diff=0) -> looks for quarter of date (2024Q2 if date is May)
        # Wait, get_quarter_by_date("2024-05-01") -> 2024Q2 (Apr-Jun).
        # 2024Q2 report is definitely not out in May 2024 (usually Aug).
        # So we should use diff=1 (2024Q1) or diff=2 (2023Q4).
        
        # Let's try to fetch 2023Q3 data (End Date 2023-09-30)
        # We can pass date="2023-10-01" and diff=0 -> 2023Q4 (Oct-Dec) -> Too early.
        # We want End Date 2023-09-30.
        # get_quarter_by_date("2023-09-30") -> 2023Q3.
        
        df = share_holder.fetch_share_holder(code, "2023-09-30", diff=0)
        print("\nFetch Result:")
        print(df)
        
        if not df.empty:
            self.assertTrue("HolderName" in df.columns)
            self.assertTrue(len(df) > 0)
            first = df.iloc[0]
            print(f"Top Holder: {first['HolderName']} Rank: {first['HolderRank']}")

    def test_cache_share_holder(self):
        code = "SH600000"
        date = "2023-09-30"
        df = share_holder.cache_share_holder(code, date, diff=0)
        self.assertFalse(df.empty)
        
        # Verify file exists
        filename = config.top10_holders_filename(code, date)
        import os
        self.assertTrue(os.path.exists(filename))
        print(f"Cache file: {filename}")

if __name__ == '__main__':
    unittest.main()
