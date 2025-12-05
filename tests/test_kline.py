import unittest
from unittest.mock import MagicMock, patch
import os
import pandas as pd
from quant1x.datasets import kline
from quant1x.level1 import security_bars
from quant1x.exchange import Timestamp
from quant1x.factors import base as factors
from quant1x.level1.xdxr_info import XdxrInfo

class TestKLine(unittest.TestCase):
    def test_kline_adjust(self):
        k = kline.KLine(Open=10.0, Close=11.0, High=12.0, Low=9.0, Volume=100.0, Amount=1000.0)
        adj = factors.CumulativeAdjustment(
            timestamp=Timestamp.parse("2023-01-01"),
            m=0.5,
            a=1.0,
            monetary_adjustment=0.0,
            share_adjustment_ratio=1.0,
            no=1
        )
        
        k.adjust(adj)
        
        self.assertEqual(k.Open, 10.0 * 0.5 + 1.0) # 6.0
        self.assertEqual(k.Close, 11.0 * 0.5 + 1.0) # 6.5
        self.assertEqual(k.High, 12.0 * 0.5 + 1.0) # 7.0
        self.assertEqual(k.Low, 9.0 * 0.5 + 1.0) # 5.5
        
        # Volume: 100 * (1 + 1.0) = 200
        self.assertEqual(k.Volume, 200.0)
        
        # Amount: Volume * AdjustedPrice
        # Original Price = 1000 / 100 = 10.0
        # Adjusted Price = 10.0 * 0.5 + 1.0 = 6.0
        # New Amount = 200 * 6.0 = 1200.0
        self.assertEqual(k.Amount, 1200.0)
        self.assertEqual(k.AdjustmentCount, 1)

    @patch('quant1x.datasets.kline.client.client')
    @patch('quant1x.datasets.kline.protocol.process')
    def test_fetch_kline(self, mock_process, mock_client):
        mock_conn = MagicMock()
        mock_client.return_value.__enter__.return_value = mock_conn
        
        # Mock response
        resp = security_bars.SecurityBarsResponse(False, 4)
        bar = security_bars.SecurityBar()
        bar.Year = 2023
        bar.Month = 1
        bar.Day = 1
        bar.Open = 10.0
        resp.list = [bar]
        
        with patch('quant1x.datasets.kline.SecurityBarsResponse') as MockResp:
            instance = MockResp.return_value
            instance.list = [bar]
            
            res = kline.fetch_kline("SH600000", 0, 1)
            self.assertEqual(len(res), 1)
            self.assertEqual(res[0].Year, 2023)

    def test_save_load_kline(self):
        filename = "test_kline.csv"
        klines = [
            kline.KLine(Date="2023-01-01", Open=10.0, Close=11.0),
            kline.KLine(Date="2023-01-02", Open=11.0, Close=12.0)
        ]
        
        kline.save_kline(filename, klines)
        
        loaded = kline.read_kline_from_csv(filename)
        self.assertEqual(len(loaded), 2)
        self.assertEqual(loaded[0].Date, "2023-01-01")
        self.assertEqual(loaded[0].Open, 10.0)
        
        if os.path.exists(filename):
            os.remove(filename)

    def test_apply_forward_adjustment_for_event(self):
        # Setup klines
        klines = [
            kline.KLine(Date="2023-01-01", Open=10.0, Close=10.0, Volume=100, Amount=1000, AdjustmentCount=0),
            kline.KLine(Date="2023-01-02", Open=10.0, Close=10.0, Volume=100, Amount=1000, AdjustmentCount=0),
            kline.KLine(Date="2023-01-03", Open=10.0, Close=10.0, Volume=100, Amount=1000, AdjustmentCount=0)
        ]
        
        # Setup dividend: Ex-date 2023-01-02. 10 for 10 split (1:1).
        div = XdxrInfo()
        div.Date = "2023-01-02"
        div.Category = 1
        div.SongZhuanGu = 10.0
        
        dividends = [div]
        
        current_start_date = Timestamp.parse("2023-01-01")
        
        kline.apply_forward_adjustment_for_event(klines, current_start_date, dividends)
        
        # 2023-01-01 < 2023-01-02. Should be adjusted.
        self.assertEqual(klines[0].Open, 5.0)
        self.assertEqual(klines[0].Volume, 200.0)
        self.assertEqual(klines[0].AdjustmentCount, 1)
        
        # 2023-01-02 >= 2023-01-02. Should NOT be adjusted.
        self.assertEqual(klines[1].Open, 10.0)
        self.assertEqual(klines[1].AdjustmentCount, 0)
        
        # 2023-01-03 >= 2023-01-02. Should NOT be adjusted.
        self.assertEqual(klines[2].Open, 10.0)
        self.assertEqual(klines[2].AdjustmentCount, 0)

    @patch('quant1x.datasets.kline.save_kline')
    @patch('quant1x.datasets.kline.read_kline_from_csv')
    @patch('quant1x.datasets.kline.config.get_kline_filename')
    @patch('quant1x.datasets.kline.fetch_kline')
    @patch('quant1x.datasets.kline.xdxr.load_xdxr')
    @patch('quant1x.datasets.kline.Timestamp.now')
    def test_data_kline_update(self, mock_now, mock_load_xdxr, mock_fetch_kline, mock_get_filename, mock_read_csv, mock_save_kline):
        # Setup
        code = "SH600000"
        mock_get_filename.return_value = "/tmp/sh600000.csv"
        
        # Mock current time: 2023-01-05
        mock_now.return_value = Timestamp.parse("2023-01-05")
        
        # Mock local cache: Data up to 2023-01-01
        # MAX_KLINE_LOOKBACK_DAYS = 1. So we look at the last 1 record.
        cache_kline_1 = kline.KLine(Date="2023-01-01", Open=10.0, Close=10.0, AdjustmentCount=0)
        mock_read_csv.return_value = [cache_kline_1]
        
        # Mock fetch_kline
        def side_effect_fetch(c, s, cnt, ktype=None):
            if s == 0:
                # Recent data (Newest to Oldest)
                b1 = security_bars.SecurityBar()
                b1.Year, b1.Month, b1.Day = 2023, 1, 4
                b1.Open = 12.0
                b1.Vol = 100
                
                b2 = security_bars.SecurityBar()
                b2.Year, b2.Month, b2.Day = 2023, 1, 3
                b2.Open = 11.0
                b2.Vol = 100
                
                b3 = security_bars.SecurityBar()
                b3.Year, b3.Month, b3.Day = 2023, 1, 2
                b3.Open = 11.0
                b3.Vol = 100

                b4 = security_bars.SecurityBar()
                b4.Year, b4.Month, b4.Day = 2023, 1, 1
                b4.Open = 10.0
                b4.Vol = 100
                
                return [b4, b3, b2, b1]
            else:
                # Older data
                b4 = security_bars.SecurityBar()
                b4.Year, b4.Month, b4.Day = 2023, 1, 1
                b4.Open = 10.0
                b4.Vol = 100
                
                b5 = security_bars.SecurityBar()
                b5.Year, b5.Month, b5.Day = 2022, 12, 31
                b5.Open = 9.0
                b5.Vol = 100
                
                return [b4, b5]

        mock_fetch_kline.side_effect = side_effect_fetch
        
        # Mock dividends
        mock_load_xdxr.return_value = []
        
        # Run update
        dk = kline.DataKLine()
        dk.update(code, Timestamp.now())
        
        # Verify save_kline called with merged data
        # Cache: [2023-01-01]
        # Fetched: [2023-01-02, 2023-01-03, 2023-01-04] (After reverse and filter)
        # Merged: [2023-01-01, 2023-01-02, 2023-01-03, 2023-01-04]
        
        self.assertTrue(mock_save_kline.called)
        args, _ = mock_save_kline.call_args
        filename, saved_klines = args
        
        self.assertEqual(len(saved_klines), 4)
        self.assertEqual(saved_klines[0].Date, "2023-01-01")
        self.assertEqual(saved_klines[-1].Date, "2023-01-04")

if __name__ == '__main__':
    unittest.main()
