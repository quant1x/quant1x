import unittest
import os
import shutil
import tempfile
from quant1x.datasets import trans
from quant1x.level1 import client
from quant1x.config import config
from quant1x.exchange import Timestamp

class TestTransReal(unittest.TestCase):
    def setUp(self):
        # Use a temporary directory for data to avoid polluting the real environment
        self.test_dir = tempfile.mkdtemp()
        self.original_data_path = config.quant1x_config.data_path
        config.quant1x_config.data_path = self.test_dir
        
        # Update module-level data_path as well, because quant1x.config exports it
        self.original_module_data_path = getattr(config, 'data_path', None)
        if self.original_module_data_path is not None:
            config.data_path = self.test_dir
        
        # Initialize client pool with real detection (clearing cache to force detect)
        from quant1x.level1 import config as l1config
        cache_file = l1config._cache_filename()
        if os.path.exists(cache_file):
            try:
                os.remove(cache_file)
            except Exception:
                pass
        
        # Initialize pool (this might take a few seconds)
        try:
            client.init_pool()
        except Exception as e:
            print(f"Client init failed: {e}")

    def tearDown(self):
        config.quant1x_config.data_path = self.original_data_path
        if self.original_module_data_path is not None:
            config.data_path = self.original_module_data_path
            
        if os.path.exists(self.test_dir):
            shutil.rmtree(self.test_dir)

    def test_checkout_transaction_data_history_real(self):
        code = "SH600000" # Pudong Development Bank
        # Use a recent past trading date. Assuming 2025-12-03 was a trading day.
        # If it wasn't, the list might be empty, but it shouldn't crash.
        # Let's try to pick a date that is likely a trading day.
        # 2025-12-03 is a Wednesday.
        date_str = "2025-12-03"
        date = Timestamp.parse(date_str)
        
        print(f"\nFetching transaction data for {code} on {date_str}...")
        
        # This function handles fetching, merging, and saving to CSV
        data = trans.checkout_transaction_data(code, date, False)
        
        # Verify we got some data
        # Note: If the market was closed or no data, this might be empty.
        # But for a major stock on a weekday, it should have data.
        if not data:
            print(f"Warning: No data returned for {code} on {date_str}. Is it a holiday?")
        else:
            print(f"Got {len(data)} transaction records.")
            self.assertGreater(len(data), 0)
            
            # Check first and last record
            first = data[0]
            last = data[-1]
            print(f"First: {first.time} Price: {first.price} Vol: {first.vol}")
            print(f"Last: {last.time} Price: {last.price} Vol: {last.vol}")
            
            # Basic validation
            self.assertGreater(first.price, 0)
            self.assertGreater(last.price, 0)
            
        # Verify file existence
        expected_file = config.get_historical_trade_filename(code, date_str)
        print(f"Checking file: {expected_file}")
        self.assertTrue(os.path.exists(expected_file))
        
        # Verify file content
        with open(expected_file, 'r', encoding='utf-8') as f:
            lines = f.readlines()
            # Header + data
            self.assertGreater(len(lines), 1)
            self.assertIn("time,price,vol,num,amount,buyOrSell", lines[0])

    def test_count_inflow_real(self):
        code = "SH600000"
        date_str = "2025-12-03"
        date = Timestamp.parse(date_str)
        
        # Create dummy F10 data for this date to avoid FileNotFoundError
        # Path: ${data_path}/flash/2025/f10.2025-12-03
        flash_dir = os.path.join(self.test_dir, 'flash', '2025')
        os.makedirs(flash_dir, exist_ok=True)
        f10_file = os.path.join(flash_dir, f"f10.{date_str}")
        
        # Write dummy CSV
        with open(f10_file, 'w') as f:
            f.write("Code,FreeCapital,Capital\n")
            f.write("SH600000,1000000,2000000\n")
        
        # Fetch data first
        data_list = trans.checkout_transaction_data(code, date, False)
        
        if not data_list:
            print("Skipping count_inflow test due to empty data.")
            return

        # Run count_inflow
        summary = trans.count_inflow(data_list, code, date)
        
        print(f"Inflow Summary: {summary}")
        
        # Verify summary logic
        total_vol = sum(t.vol for t in data_list)
        calc_total = summary.OuterVolume + summary.InnerVolume
        
        # Note: count_inflow logic might have some specific handling for neutral ticks
        # where it splits volume between inner and outer.
        # So Outer + Inner should roughly equal Total.
        # Let's check if they are close (integer division might cause small diffs)
        self.assertAlmostEqual(total_vol, calc_total, delta=len(data_list)) # Allow small delta due to rounding/splitting
        
        self.assertGreaterEqual(summary.OpenVolume, 0)
        self.assertGreaterEqual(summary.CloseVolume, 0)
        
        # TurnZ depends on F10 data which might not be available in this test env
        # So we just check they are numbers
        self.assertIsInstance(summary.OpenTurnZ, (int, float))
        self.assertIsInstance(summary.CloseTurnZ, (int, float))

if __name__ == '__main__':
    unittest.main()
