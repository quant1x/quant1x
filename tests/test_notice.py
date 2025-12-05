
import unittest
import sys
import os
from datetime import datetime, timedelta

# Add project root to path
sys.path.append(os.getcwd())

from quant1x.factors import notice
from quant1x.exchange import Timestamp

class TestNotice(unittest.TestCase):
    def test_stock_notices(self):
        # Test with a known stock code (e.g., Ping An Bank)
        code = "sz000001"
        # Use a recent date range
        end_date = Timestamp.now().only_date()
        begin_date = Timestamp.now().offset(hour=-24*30).only_date() # Last 30 days
        
        notices, pages, err = notice.stock_notices(code, begin_date, end_date)
        
        if err:
            print(f"Error fetching notices: {err}")
        else:
            print(f"Fetched {len(notices)} notices, total pages: {pages}")
            for n in notices[:3]:
                print(f"Notice: {n.title} ({n.notice_date}) Keywords: {n.keywords}")
                
        self.assertIsNone(err)
        self.assertIsInstance(notices, list)
        self.assertIsInstance(pages, int)

    def test_get_one_notice(self):
        code = "sz000001"
        current_date = Timestamp.now().only_date()
        
        company_notice = notice.get_one_notice(code, current_date)
        print(f"Company Notice: Increase={company_notice.increase}, Reduce={company_notice.reduce}, Risk={company_notice.risk}, Keywords={company_notice.risk_keywords}")
        
        self.assertIsInstance(company_notice, notice.CompanyNotice)

    def test_notice_date_for_report(self):
        code = "sz000001"
        current_date = Timestamp.now().only_date()
        annual, quarterly = notice.notice_date_for_report(code, current_date)
        print(f"Report Dates: Annual={annual}, Quarterly={quarterly}")
        
        self.assertIsInstance(annual, str)
        self.assertIsInstance(quarterly, str)

if __name__ == '__main__':
    unittest.main()
