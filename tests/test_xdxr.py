
import unittest
from unittest.mock import MagicMock, patch
import os
import shutil
import tempfile
from quant1x.datasets import xdxr
from quant1x.level1 import xdxr_info
from quant1x.config import config
from quant1x.exchange import Timestamp

class TestXdxr(unittest.TestCase):
    def setUp(self):
        self.test_dir = tempfile.mkdtemp()
        self.original_data_path = config.quant1x_config.data_path
        config.quant1x_config.data_path = self.test_dir
        
    def tearDown(self):
        config.quant1x_config.data_path = self.original_data_path
        shutil.rmtree(self.test_dir)

    @patch('quant1x.level1.client.client')
    @patch('quant1x.level1.protocol.process')
    def test_update_xdxr(self, mock_process, mock_client):
        # Mock connection
        mock_conn = MagicMock()
        mock_client.return_value = mock_conn
        
        # Mock process to do nothing (response population handled by other mocks or side effects)
        mock_process.return_value = None
        
        # Mock XdxrInfoResponse.deserialize
        with patch('quant1x.level1.xdxr_info.XdxrInfoResponse.deserialize') as mock_deserialize:
            pass

    @patch('quant1x.level1.client.client')
    @patch('quant1x.level1.protocol.process')
    @patch('quant1x.level1.xdxr_info.XdxrInfoResponse')
    def test_update_and_load(self, MockResponse, mock_process, mock_client):
        # Setup MockResponse
        instance = MockResponse.return_value
        instance.count = 1
        
        info = xdxr_info.XdxrInfo()
        info.Date = "2023-01-01"
        info.Category = 1
        info.Name = "除权除息"
        info.FenHong = 0.5
        
        instance.list = [info]
        
        # Run update
        code = "sh600000"
        date = Timestamp.now()
        xdxr.DataXdxr().update(code, date)
        
        # Verify file exists
        filename = config.get_xdxr_filename(code)
        self.assertTrue(os.path.exists(filename))
        
        # Run load
        loaded_list = xdxr.load_xdxr(code)
        self.assertEqual(len(loaded_list), 1)
        self.assertEqual(loaded_list[0].Date, "2023-01-01")
        self.assertEqual(loaded_list[0].FenHong, 0.5)

if __name__ == '__main__':
    unittest.main()
