
import unittest
from quant1x.exchange.margin_trading import is_margin_trading_target, margin_trading_list, lazy_load_margin_trading

class TestMarginTrading(unittest.TestCase):
    def test_margin_trading(self):
        # Force load
        lazy_load_margin_trading()
        
        targets = margin_trading_list()
        print(f"Margin trading targets count: {len(targets)}")
        
        if len(targets) > 0:
            code = targets[0]
            print(f"Testing code: {code}")
            self.assertTrue(is_margin_trading_target(code))
            
        # Test a known non-target (if any, or just random string)
        self.assertFalse(is_margin_trading_target("sh000000"))

if __name__ == '__main__':
    unittest.main()
